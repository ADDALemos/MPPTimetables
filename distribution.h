//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_DISTRIBUTION_H
#define PROJECT_DISTRIBUTION_H


#include <vector>
#include "Course.h"
#include "Constraint.h"


class distribution {
    std::vector<int> classes;
    Constraint type;
public:
    distribution(const std::vector<int, std::allocator<int>> &classes, const Constraint &type) : classes(classes),
                                                                                                 type(type) {}

};


#endif //PROJECT_DISTRIBUTION_H
