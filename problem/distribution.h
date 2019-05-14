//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_DISTRIBUTION_H
#define TIMETABLER_DISTRIBUTION_H


#include <vector>
#include "Course.h"
#include "Constraint.h"


class distribution {
    std::vector<int> classes;
    Constraint *type;
public:
    const std::vector<int> &getClasses() const {
        return classes;
    }

    void setClasses(const std::vector<int> &classes) {
        distribution::classes = classes;
    }

    const Constraint *getType() const {
        return type;
    }

    void setType(Constraint *type) {
        distribution::type = type;
    }

    virtual int getPenalty()=0;

    distribution(const std::vector<int> &classes, Constraint *type) : classes(classes),
                                                                          type(type) {}

};


#endif //PROJECT_DISTRIBUTION_H
