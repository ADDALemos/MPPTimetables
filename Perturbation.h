//
// Created by Alexandre Lemos on 17/12/2018.
//

#ifndef PROJECT_PERTURBATION_H
#define PROJECT_PERTURBATION_H

#include "Instance.h"

class Perturbation {
    std::ofstream seedFile;

    std::vector<std::pair<int, int>, std::allocator<std::pair<int, int>>>
    randomEnrolment(unsigned int classNumber, int change, bool factor, double d);
    std::set<int> randomClassSelection(int classNumber, double factor, unsigned int t);

public:
    void randomeEnrolmentChanges(Instance *i, int changeLimit, bool factor, double d);


};


#endif //PROJECT_PERTUBATION_H
