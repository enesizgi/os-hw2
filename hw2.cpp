#include <iostream>
#include <vector>
#include <pthread.h>
#include "hw2_output.h"

//void input_function(int &gi, int &gj, std::vector <std::vector<int>> &grid) {
//
//}

int main() {
    int gi, gj; // grid size
    int pp_count; // Number of proper privates
    int order_count; // Number of orders
    int smoker_count; // Number of smokers
    std::vector <std::vector<int> > grid;
    std::vector <std::pair<int, std::string> > orders;


    //Areas to gather from.
    //First parameter is unique gid.
    //Second parameter is a vector of pair of top-left coordinates.
    std::vector < std::pair < std::vector<int>, std::vector < std::pair < int, int > > > > pps;

    // Smokers vector consists of pairs.
    // First pair parameter is a pair of 3: id, time_to_smoke, cell_count.
    // Second pair parameter is also a pair of 3: coord_i, coord_j, cigbutt_count.
    std::vector< std::pair < std::vector <int>, std::vector<int> > >  smokers;

    std::cin >> gi;
    std::cin >> gj;
    for (int i = 0; i < gi; i++) {
        std::vector<int> row;
        for (int j = 0; j < gj; j++) {
            int cell;
            std::cin >> cell;
            row.push_back(cell);
        }
        grid.push_back(row);
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

    std::cin >> order_count;
    for (int i = 0; i < order_count; i++) {
        std::pair <int, std::string> order;
        std::cin >> order.first >> order.second;
        orders.push_back(order);
    }

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

    return 0;
}