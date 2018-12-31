//
// Created by Alexandre Lemos on 17/12/2018.
//

#ifndef PROJECT_PERTURBATION_H
#define PROJECT_PERTURBATION_H

#include "Instance.h"

class Perturbation {
    std::ofstream seedFile;

    std::vector<std::pair<int, int>, std::allocator<std::pair<int, int>>>
    randomEnrolment(unsigned int classNumber, int change, bool increase, double factor);
    std::set<int> randomClassSelection(int classNumber, double factor, unsigned int t);

    unsigned int seedHandler();


public:
    void randomEnrolmentChanges(Instance *i, int changeLimit, bool increase, double factor);

    void randomCloseRoom(Instance *i, double factor);

    void randomSlotClose(Instance *i, double factor);


};


#endif //PROJECT_PERTUBATION_H
