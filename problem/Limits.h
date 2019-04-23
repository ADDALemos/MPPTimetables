//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_LIMITS_H
#define PROJECT_LIMITS_H

#include <string>
#include <map>
#include "Constraint.h"

class Limits : public Constraint {
    int limit;
    int limit1;
public:
    Limits(int limit, int limit1) : limit(limit), limit1(limit1) {}


    void setType(std::string s) override {
        static std::map<std::string, ConstraintType> theMap = {{"MaxBreaks", MaxBreaks},
                                                               {"MaxBlock",  MaxBlock}};

        type = theMap[s];

    }

    int getLimit() const override {
        return limit;
    }

    void setLimit(int limit) override {
        Limits::limit = limit;
    }

    int getLimit1() const override {
        return limit1;
    }

    void setLimit1(int limit1) override {
        Limits::limit1 = limit1;
    }
};


#endif //PROJECT_LIMITS_H
