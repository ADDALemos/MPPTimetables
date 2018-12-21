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
#include "Instance.h"

std::vector<std::pair<int, int>> Perturbation::randomIncreaseCapacity(int classNumber, int increase, double factor) {
    unsigned int t = std::chrono::steady_clock::now().time_since_epoch().count();
    seedFile.open("seed.txt", std::ios::app);
    seedFile << t << std::endl;
    std::set<int> classes = randomClassSelection(classNumber, factor, t);
    std::default_random_engine generator(t);
    std::uniform_int_distribution<int> distribution(0, increase - 1);
    std::vector<std::pair<int, int>> number;
    for (std::set<int>::iterator it = classes.begin(); it != classes.end(); ++it) {
        number.push_back(std::pair<int, int>(*it, distribution(generator)));
    }
    return number;


}

std::set<int> Perturbation::randomClassSelection(int classNumber, double factor, unsigned int t) {

    std::default_random_engine generator(t);
    std::uniform_int_distribution<int> distribution(1, classNumber);
    std::set<int> number;
    int n = 0;
    for (; number.size() < floor(factor * classNumber);) {
        n = distribution(generator);
        number.insert(n);
    }
    return number;


}


void Perturbation::randomIncreaseCapacity(Instance *i, int increaseMAX, double factor) {
    std::vector<std::pair<int, int>> set = randomIncreaseCapacity(i->getNumClasses(), increaseMAX, factor);
    for (std::pair<int, int> pair: set) {
        std::cout << pair.first << " " << pair.second << std::endl;
        i->updateStudentEnrollment(pair.first, pair.second);
    }
}
