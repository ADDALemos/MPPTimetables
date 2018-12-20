//
// Created by Alexandre Lemos on 17/12/2018.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <map>
#include <chrono>
#include <set>
#include <random>
#include <sstream>
#include "Perturbation.h"

std::vector<std::pair<int, int>> Perturbation::randomIncreaseCapacity(int classNumber, int increase, double factor) {
    unsigned int t = std::chrono::steady_clock::now().time_since_epoch().count();
    seedFile.open("seed.txt", std::ios::app);
    seedFile << t << std::endl;
    std::set<int> classes = randomClassSelection(classNumber, factor, t);
    std::default_random_engine generator(t);
    std::uniform_int_distribution<int> distribution(0, increase - 1);
    std::vector<std::pair<int, int>> number;
    for (std::set<int>::iterator it = classes.begin(); it != classes.end(); ++it) {
        int n = 0;
        for (; number.size() < floor(factor * increase);) {
            n = distribution(generator);
            number.push_back(std::pair<int, int>(*it, n));

        }
    }
    return number;


}

std::set<int> Perturbation::randomClassSelection(int classNumber, double factor, unsigned int t) {

    std::default_random_engine generator(t);
    std::uniform_int_distribution<int> distribution(0, classNumber - 1);
    std::set<int> number;
    int n = 0;
    for (; number.size() < floor(factor * classNumber);) {
        n = distribution(generator);
        number.insert(n);
    }
    return number;


}
