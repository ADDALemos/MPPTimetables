//
// Created by Alexandre Lemos on 17/12/2018.
//

#ifndef PROJECT_PERTURBATION_H
#define PROJECT_PERTURBATION_H

#include "Instance.h"

class Perturbation {
    std::ofstream seedFile;

    std::vector<std::pair<int, int>> randomIncreaseCapacity(int classNumber, int increase, double factor);
    std::set<int> randomClassSelection(int classNumber, double factor, unsigned int t);

public:
    void randomIncreaseCapacity(Instance *i, int increaseMAX, double factor);


};


#endif //PROJECT_PERTUBATION_H
