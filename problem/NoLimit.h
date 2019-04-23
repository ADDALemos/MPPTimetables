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
                                                               {"Overlap",       Overlap},
                                                               {"Precedence",    Precedence},
                                                               {"SameAttendees", SameAttendees},
                                                               {"SameStart",     SameStart},
                                                               {"SameTime",      SameTime},
                                                               {"SameDays",      SameDays},
                                                               {"DifferentDays", DifferentDays},
                                                               {"SameStart",     SameStart},
                                                               {"SameWeeks",     SameWeeks},
                                                               {"SameRoom",      SameRoom},
                                                               {"DifferentRoom", DifferentRoom}};

        type = theMap[s];

    }

    virtual int getLimit() const override {
        throw "Not Implemented";
    }

    virtual void setLimit(int limit) override {
        throw "Not Implemented";
    }

    virtual int getLimit1() const override {
        throw "Not Implemented";
    }

    virtual void setLimit1(int limit1) override {
        throw "Not Implemented";
    }

};


#endif //PROJECT_NOLIMIT_H
