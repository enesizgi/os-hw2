#include <iostream>
#include <vector>
#include <pthread.h>
#include "hw2_output.h"
#include <sys/time.h>

int gi, gj; // grid size
int pp_count; // Number of proper privates
std::vector <std::vector<int> > grid;
pthread_mutex_t grid_cigbutt_count_mutex = PTHREAD_MUTEX_INITIALIZER;

// 1 is locked.
// 0 is unlocked.
//std::vector <std::vector<int> > grid_mutex_status; // NOT USED RIGHT NOW
std::vector <std::vector<pthread_mutex_t> > grid_mutex;
pthread_mutex_t grid_status_mutex = PTHREAD_MUTEX_INITIALIZER;
//pthread_cond_t grid_status_cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t grid_status_exit_mutex = PTHREAD_MUTEX_INITIALIZER;

// First integer is unique gid.
// Second integer is coord_i.
// Third integer is coord_j.
std::vector < std::vector<int> > pps_current_area;
pthread_mutex_t pps_current_area_mutex = PTHREAD_MUTEX_INITIALIZER;

//Areas to gather from.
//First parameter is unique gid.
//Second parameter is a vector of pair of top-left coordinates.
std::vector<std::pair<std::vector<int>, std::vector<std::pair<int, int> > > > pps;

pthread_mutex_t pp_wait_mutexes_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t pp_wait_conds_lock = PTHREAD_MUTEX_INITIALIZER;
std::vector< std::pair <int, pthread_mutex_t> > pp_wait_mutexes;
std::vector< std::pair <int, pthread_cond_t> > pp_wait_conds;

pthread_mutex_t order_type_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t order_type_cond = PTHREAD_COND_INITIALIZER;
int order_type = 0;

std::vector< std::pair< int, int> > pp_last_obeyed_order;
pthread_mutex_t pp_last_obeyed_order_mutex = PTHREAD_MUTEX_INITIALIZER;

void input_part1 (int& gi, int& gj, int &pp_count, std::vector <std::vector<int> >& grid,
                  std::vector < std::pair < std::vector<int>, std::vector < std::pair < int, int > > > >& pps) {
    std::cin >> gi;
    std::cin >> gj;
    for (int i = 0; i < gi; i++) {
        std::vector<int> row;
        std::vector<int> row_mutex_status;
        std::vector<pthread_mutex_t> row_mutex;
        for (int j = 0; j < gj; j++) {
            int cell;
            std::cin >> cell;
            row.push_back(cell);
            row_mutex_status.push_back(0);
            pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
            row_mutex.push_back(mutex);
        }
        grid.push_back(row);
//        grid_mutex_status.push_back(row_mutex_status);
        grid_mutex.push_back(row_mutex);
    }

    std::cin >> pp_count;
    for (int i = 0; i < pp_count; i++) {
        std::vector<int> pp;
        std::pair < std::vector<int>, std::vector < std::pair < int, int > > > area;

        for (int j = 0; j < 5; j++) {
            int temp;
            std::cin >> temp;
            pp.push_back(temp);
        }
        area.first = pp;

        int gid = pp[0];
        pp_last_obeyed_order.push_back(std::make_pair(gid, 0));

        int area_count = pp[4];
        std::vector < std::pair < int, int > > coordinates_vec;
        for (int j = 0; j < area_count; j++) {
            std::pair<int, int> coordinates;
            std::cin >> coordinates.first >> coordinates.second;
            coordinates_vec.push_back(coordinates);
        }
        area.second = coordinates_vec;
        pps.push_back(area);
    }
}

void input_part2 (int& order_count, std::vector <std::pair<int, std::string> >& orders) {
    std::cin >> order_count;
    for (int i = 0; i < order_count; i++) {
        std::pair <int, std::string> order;
        std::cin >> order.first >> order.second;
        orders.push_back(order);
    }
}

void input_part3 (int& smoker_count, std::vector< std::pair < std::vector <int>, std::vector<int> > >&  smokers) {
    std::cin >> smoker_count;
    for (int i = 0; i < smoker_count ;i++) {
        std::pair < std::vector <int>, std::vector<int> > smoker;
        int id, time_to_smoke, cell_count;
        std::cin >> id >> time_to_smoke >> cell_count;
        std::vector<int> smoker_properties;
        smoker_properties.push_back(id);
        smoker_properties.push_back(time_to_smoke);
        smoker_properties.push_back(cell_count);
        smoker.first = smoker_properties;

        std::vector<int> smoker_locations;
        for (int j = 0; j < cell_count ;j++) {
            int coord_i, coord_j, cigbutt_count;
            std::cin >> coord_i >> coord_j >> cigbutt_count;
            smoker_locations.push_back(coord_i);
            smoker_locations.push_back(coord_j);
            smoker_locations.push_back(cigbutt_count);
        }
        smoker.second = smoker_locations;
        smokers.push_back(smoker);
    }
}

int wait(int timeInMs, pthread_cond_t* condition, pthread_mutex_t* mutex)
{
    struct timeval tv;
    struct timespec ts;

    gettimeofday(&tv, NULL);
    ts.tv_sec = time(NULL) + timeInMs / 1000;
    ts.tv_nsec = tv.tv_usec * 1000 + 1000 * 1000 * (timeInMs % 1000);
    ts.tv_sec += ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec %= (1000 * 1000 * 1000);

    int n = pthread_cond_timedwait(condition, mutex, &ts);
    return n;
//    if (n == 0)
//        // TODO: singaled
//            else if (n == ETIMEDOUT)
//    // TODO: Time out.
}

void* order_routine (void* arg) {
    std::pair<int, std::string>* order = (std::pair<int, std::string>*)arg;
    pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
    pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
    wait(order->first, &cond, &mutex);
    if (order->second == "break") {
        pthread_mutex_lock(&order_type_mutex);
        hw2_notify(ORDER_BREAK, 0, 0, 0);
        order_type = 1;
        for (int i = 0; i < pp_count ;i++) {
            pthread_cond_signal(&pp_wait_conds[i].second);
        }
        pthread_mutex_unlock(&order_type_mutex);
    }
    else if (order->second == "continue") {
        pthread_mutex_lock(&order_type_mutex);
        order_type = 2;
        hw2_notify(ORDER_CONTINUE, 0, 0, 0);
        pthread_cond_broadcast(&order_type_cond);
        pthread_mutex_unlock(&order_type_mutex);
    }
    else {
        // TODO: implement stop case
        pthread_mutex_lock(&order_type_mutex);
        order_type = 3;
        hw2_notify(ORDER_STOP, 0, 0, 0);
        for (int i = 0; i < pp_count ;i++) {
            pthread_cond_signal(&pp_wait_conds[i].second);
        }
        pthread_cond_broadcast(&order_type_cond);
        pthread_mutex_unlock(&order_type_mutex);
    }
    return NULL;
}

void lock_grid (std::pair<int,int>& top_left, std::pair<int,int>& bottom_left, std::pair<int,int>& top_right) {
    for (int i = top_left.first; i <= bottom_left.first; i++) {
        for (int j = top_left.second; j <= top_right.second; j++) {
            pthread_mutex_lock(&grid_mutex[i][j]);
        }
    }
}

void unlock_grid (std::pair<int,int>& top_left, std::pair<int,int>& bottom_left, std::pair<int,int>& top_right) {
    for (int i = top_left.first; i <= bottom_left.first; i++) {
        for (int j = top_left.second; j <= top_right.second; j++) {
            pthread_mutex_unlock(&grid_mutex[i][j]);
        }
    }
}

void* routine (void* arg) {
    int gid = *(int*)arg;
    pthread_mutex_t* pp_wait_mutex;
    pthread_cond_t* pp_wait_cond;
    std::pair<std::vector<int>, std::vector<std::pair<int, int> > >* pp;
    int cell_i, cell_j, si, sj, wait_time;

    // Find the proper private
    for (int i = 0; i < pp_count; i++) {
        if (gid == pp_wait_mutexes[i].first) {
            pthread_mutex_lock(&pp_wait_mutexes_lock);
            pp_wait_mutex = &pp_wait_mutexes[i].second;
            pthread_mutex_unlock(&pp_wait_mutexes_lock);
        }
        if (gid == pp_wait_conds[i].first) {
            pthread_mutex_lock(&pp_wait_conds_lock);
            pp_wait_cond = &pp_wait_conds[i].second;
            pthread_mutex_unlock(&pp_wait_conds_lock);
        }
    }

    // get values of proper private from pps vector
    for (int i = 0; i < pp_count; i++) {
        if (gid == pps[i].first[0]) {
            pp = &pps[i];
        }
    }

    si = pp->first[1];
    sj = pp->first[2];
    wait_time = pp->first[3];

    int area_count = pp->second.size();
    for (int x = 0; x < area_count; x++) {
        cell_i = pp->second[x].first;
        cell_j = pp->second[x].second;

        std::pair<int,int> top_left = std::make_pair(cell_i, cell_j);
        std::pair<int,int> bottom_right = std::make_pair(cell_i + si - 1, cell_j + sj - 1);
        std::pair<int,int> top_right = std::make_pair(cell_i, cell_j + sj - 1);
        std::pair<int,int> bottom_left = std::make_pair(cell_i + si - 1, cell_j);

        // Part 3 complex case'de burasi muhtemelen patlar. (case 1)
        pthread_mutex_lock(&grid_status_mutex);
        lock_grid(top_left, bottom_left, top_right);
        pthread_mutex_unlock(&grid_status_mutex);
        hw2_notify(PROPER_PRIVATE_ARRIVED, gid, top_left.first, top_left.second);

        for (int i = top_left.first; i <= bottom_left.first; i++) {
            for (int j = top_left.second; j <= top_right.second; j++) {
                pthread_mutex_lock(&grid_cigbutt_count_mutex);
                while (true) {
                    if (grid[i][j] == 0) {
                        break;
                    }
                    pthread_mutex_lock(&pp_last_obeyed_order_mutex);
                    int waitRetVal = wait(wait_time, pp_wait_cond, pp_wait_mutex);
                    int last_obeyed_order = 0;
                    for (int i = 0; i < pp_count; i++) {
                        if (gid == pp_last_obeyed_order[i].first) {
                            last_obeyed_order = pp_last_obeyed_order[i].second;
                        }
                    }
                    pthread_mutex_lock(&order_type_mutex);
//                    std::cout << "last_obeyed_order: " << last_obeyed_order << std::endl;
//                    std::cout << "order_type: " << order_type << std::endl;
                    if (last_obeyed_order != order_type) {
                        if (order_type == 1) {
                            pthread_mutex_unlock(&pp_last_obeyed_order_mutex);
                            // TODO: UNLOCK GATHER AREA
                            hw2_notify(PROPER_PRIVATE_TOOK_BREAK, gid, 0, 0);
                            pthread_cond_wait(&order_type_cond, &order_type_mutex);
                            pthread_mutex_lock(&pp_last_obeyed_order_mutex);
                            for (int i = 0; i < pp_count; i++) {
                                if (gid == pp_last_obeyed_order[i].first) {
                                    pp_last_obeyed_order[i].second = 2;
                                }
                            }
//                            std::cout << "soldier break" << std::endl;
                        }
                        else if (order_type == 3) {
                            std::cout << order_type << std::endl;
                            // THREAD SHOULD BE STOPPED
                            pthread_mutex_lock(&grid_status_exit_mutex);
                            unlock_grid(top_left, bottom_left, top_right);
                            pthread_mutex_unlock(&grid_status_exit_mutex);

                            pthread_mutex_unlock(&order_type_mutex);
                            pthread_mutex_unlock(&pp_last_obeyed_order_mutex);
                            pthread_mutex_unlock(&grid_cigbutt_count_mutex);
                            hw2_notify(PROPER_PRIVATE_STOPPED, gid, 0, 0);
                            return NULL;
                        }
                    }
                    else {
                        grid[i][j]--;
                        hw2_notify(PROPER_PRIVATE_GATHERED, gid, i, j);
                    }
                    pthread_mutex_unlock(&order_type_mutex);
                    pthread_mutex_unlock(&pp_last_obeyed_order_mutex);

                }
                pthread_mutex_unlock(&grid_cigbutt_count_mutex);
            }
        }
        hw2_notify(PROPER_PRIVATE_CLEARED, gid, 0, 0);

        // unlocking the gathering area
        pthread_mutex_lock(&grid_status_exit_mutex);
        unlock_grid(top_left, bottom_left, top_right);
        pthread_mutex_unlock(&grid_status_exit_mutex);

    }

    hw2_notify(PROPER_PRIVATE_EXITED, gid, 0, 0);
    return NULL;
}

int main() {
    hw2_init_notifier();
    int order_count; // Number of orders
    int smoker_count; // Number of smokers
    std::vector <std::pair<int, std::string> > orders; // Part 2
    std::vector < pthread_t > order_threads; // Part 2


    // Smokers vector consists of pairs.
    // First pair parameter is a pair of 3: id, time_to_smoke, cell_count.
    // Second pair parameter is also a pair of 3: coord_i, coord_j, cigbutt_count.
    std::vector< std::pair < std::vector <int>, std::vector<int> > >  smokers;

    // First parameters of pair is unique gid.
    std::vector<std::pair<int, pthread_mutex_t> > mutex_vec;
    std::vector<std::pair<int, pthread_t> > pp_threads;

    input_part1(gi, gj, pp_count, grid, pps);
    input_part2(order_count, orders);
//    input_part3(smoker_count, smokers);

    // Initialize order threads
    for (int i = 0; i < order_count; i++) {
        pthread_t order_thread;
        order_threads.push_back(order_thread);
//        std::pair<int, std::string>* order = &orders[i];
//        pthread_create(&order_thread, NULL, order_routine, order);
    }

    // Initialize pp_wait_conds and pp_wait_mutexes
    for (int i = 0; i < pp_count; i++) {
        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, NULL);
        pp_wait_mutexes.push_back(std::make_pair(pps[i].first[0], mutex));

        pthread_cond_t cond;
        pthread_cond_init(&cond, NULL);
        pp_wait_conds.push_back(std::make_pair(pps[i].first[0], cond));
    }

    // Creating mutex for each proper private
    for (int i = 0; i < pp_count ; i++) {
        pthread_mutex_t mutex;
        pthread_mutex_init(&mutex, NULL);
        mutex_vec.push_back(std::make_pair(pps[i].first[0], mutex));
    }

    // Initialize threads here
    for (int i = 0; i < pp_count; i++) {
        pthread_t thread;
        pp_threads.push_back(std::make_pair(pps[i].first[0], thread));
    }

    // Create order threads here
    for (int i = 0; i < order_count; i++) {
        std::pair<int, std::string>* order = &orders[i];
        pthread_create(&order_threads[i], NULL, order_routine, order);
    }

    // Create threads here
    for (int i = 0; i < pp_count;i++) {
        int* gid = new int();
        *gid = pp_threads[i].first;
        pthread_create(&pp_threads[i].second, NULL, routine, gid);
        hw2_notify(PROPER_PRIVATE_CREATED,*gid,0,0);
    }

    // Join threads here
    for (int i = 0; i < pp_count; i++) {
        pthread_join(pp_threads[i].second, NULL);
    }

    for (int i = 0; i < order_count; i++) {
        pthread_join(order_threads[i], NULL);
    }

    // Destroying mutex
    for (int i = 0; i < pp_count; i++) {
        pthread_mutex_destroy(&mutex_vec[i].second);
    }

    return 0;
}