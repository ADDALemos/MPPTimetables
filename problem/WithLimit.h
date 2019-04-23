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
public:
    WithLimit(int limit) : limit(limit) {}

    void setType(std::string s) override {
        static std::map<std::string, ConstraintType> theMap = {{"WorkDay",    WorkDay},
                                                               {"MinGap",     MinGap},
                                                               {"MaxDays",    MaxDays},
                                                               {"MaxDayLoad", MaxDayLoad}};

        type = theMap[s];

    }

    virtual int getLimit() const override {
        return limit;
    }

    virtual void setLimit(int limit) override {
        WithLimit::limit = limit;
    }

    virtual int getLimit1() const override {
        throw "Not Implemented";
    }

    virtual void setLimit1(int limit1) override {
        throw "Not Implemented";
    }


};


#endif //PROJECT_WITHLIMIT_H
