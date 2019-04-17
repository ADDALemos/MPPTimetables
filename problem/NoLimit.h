//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_NOLIMIT_H
#define PROJECT_NOLIMIT_H

#include <string>
#include <map>
#include <iostream>
#include "Constraint.h"


class NoLimit : public Constraint {
public:
    void setType(std::string s) override {
        static std::map<std::string, ConstraintType> theMap = {{"NotOverlap",    NotOverlap},
                                                               {"Prencedence",   Prencedence},
                                                               {"SameAttendees", SameAttendees},
                                                               {"SameStart",     SameStart},
                                                               {"SameTime",      SameTime},
                                                               {"SameDays",      SameDays},
                                                               {"SameStart",     SameStart},
                                                               {"SameWeeks",     SameWeeks},
                                                               {"SameRoom",      SameRoom}};

        type = theMap[s];

    }

};


#endif //PROJECT_NOLIMIT_H
