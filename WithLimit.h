//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_WITHLIMIT_H
#define PROJECT_WITHLIMIT_H


#include <string>
#include <map>
#include "Constraint.h"



class WithLimit: public Constraint {
    int limit;
    ConstraintType type;
public:
    WithLimit(int limit) : limit(limit) {}

    void setType(std::string s) override {
        static std::map<std::string, ConstraintType> theMap = {{"WorkDay",    WorkDay},
                                                               {"MinGap",     MinGap},
                                                               {"MaxDays",    MaxDays},
                                                               {"MaxDayLoad", MaxDayLoad}};

        type = theMap[s];

    }


};


#endif //PROJECT_WITHLIMIT_H
