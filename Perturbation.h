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
    //Change the number of students enrolled
    void randomEnrolmentChanges(Instance *i, int changeLimit, bool increase, double factor);

    //Close down a room
    void randomCloseRoom(Instance *i, double factor);

    //Close down a room for Day
    void randomCloseRoomDay(Instance *i, double factor);

    //Close time slot
    void randomSlotClose(Instance *i, double factor);

    //Unavailable Assigment
    void randomClassSelection(Instance *i, double factor);

    //Change the number of shifts
    void randomShiftChange(Instance *i, double factor);


};


#endif //PROJECT_PERTUBATION_H
