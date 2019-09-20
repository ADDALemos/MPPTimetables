//
// Created by Alexandre Lemos on 25/08/2019.
//

#ifndef PROJECT_CONSTRAINTSHORT_H
#define PROJECT_CONSTRAINTSHORT_H


//USE this file or two readfunction: one for split other to genarete

#include <ostream>
#include <set>
#include "ClusterClassesbyRoom.h"

class ConstraintShort {
    std::string type;
    int weight=-1;
    std::vector<Class*> classes;
    int parameter1=-1;
    int parameter2=-1;

public:
    ConstraintShort(std::string type, int weight, const std::vector<Class*> &classes, int parameter1,
                    int parameter2) : type(type), weight(weight), classes(classes), parameter1(parameter1),
                                      parameter2(parameter2) { }

    ConstraintShort(std::string type, int weight, const std::vector<Class*> &classes) : type(type),
                                                                                                    weight(weight),
                                                                                                    classes(classes) { }

    std::string getType() const {
        return type;
    }

    void setType(std::string type) {
        ConstraintShort::type = type;
    }

    int getWeight() const {
        return weight;
    }

    void setWeight(int weight) {
        ConstraintShort::weight = weight;
    }

    const std::vector<Class*> &getClasses() const {
        return classes;
    }

    void setClasses(const std::vector<Class*> &classes) {
        ConstraintShort::classes = classes;
    }

    int getParameter1() const {
        return parameter1;
    }

    void setParameter1(int parameter1) {
        ConstraintShort::parameter1 = parameter1;
    }

    int getParameter2() const {
        return parameter2;
    }

    void setParameter2(int parameter2) {
        ConstraintShort::parameter2 = parameter2;
    }

    friend std::ostream &operator<<(std::ostream &os, const ConstraintShort &aShort) {
        os << "type: " << aShort.type << " weight: " << aShort.weight << " classes: " << aShort.classes.size()
           << " parameter1: " << aShort.parameter1 << " parameter2: " << aShort.parameter2;
        return os;
    }

};


#endif //PROJECT_CONSTRAINTSHORT_H
