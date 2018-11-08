//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_WITHLIMIT_H
#define PROJECT_WITHLIMIT_H


#include "Constraint.h"

const enum ConstraintType {
    WorkDay;MinGap; MaxDays;MaxDayLoad;
};

class WithLimit: public Constraint {
    int limit;
    ConstraintType type;


};


#endif //PROJECT_WITHLIMIT_H
