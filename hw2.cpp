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
    std::vector <std::vector<int> > grid;
    std::vector <std::vector<int> > pps; // Proper privates

    //Areas to gather from.
    //First parameter is unique gid.
    //Second parameter is a vector of pair of top-left coordinates.
    std::vector < std::pair < int, std::vector < std::pair < int, int > > > > areas;

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
        for (int j = 0; j < 5; j++) {
            int temp;
            std::cin >> temp;
            pp.push_back(temp);
        }

        int area_count = pp[4];
        std::pair < int, std::vector < std::pair < int, int > > > area;
        area.first = pp[0];
        std::vector < std::pair < int, int > > coordinates_vec;
        for (int j = 0; j < area_count; j++) {
            std::pair<int, int> coordinates;
            std::cin >> coordinates.first >> coordinates.second;
            coordinates_vec.push_back(coordinates);
        }
        area.second = coordinates_vec;
        areas.push_back(area);
    }

    std::cout << gi;

    return 0;
}