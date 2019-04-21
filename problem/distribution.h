//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_DISTRIBUTION_H
#define PROJECT_DISTRIBUTION_H


#include <vector>
#include "Course.h"
#include "Constraint.h"


class distribution {
    std::vector<Class *> classes;
    Constraint type;
public:
    const std::vector<Class *> &getClasses() const {
        return classes;
    }

    void setClasses(const std::vector<Class *> &classes) {
        distribution::classes = classes;
    }

    const Constraint &getType() const {
        return type;
    }

    void setType(const Constraint &type) {
        distribution::type = type;
    }

    virtual int getPenalty()=0;

    distribution(const std::vector<Class *> &classes, const Constraint &type) : classes(classes),
                                                                                type(type) {}

};


#endif //PROJECT_DISTRIBUTION_H
