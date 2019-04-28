//
// Created by Alexandre Lemos on 28/04/2019.
//

#ifndef PROJECT_STUDENTVAR_H
#define PROJECT_STUDENTVAR_H

#include <gurobi_c++.h>
#include "GurobiAux.h"
#include "../problem/Instance.h"

class StudentSectioning {
    GRBVar **student;
    Instance *instance;
public:
    StudentSectioning(Instance *instance) : instance(instance) {}

    void init() {
        student = new GRBVar *[instance->getStudent().size()];
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            student[i] = new GRBVar[instance->getClasses().size()];//Reduce size
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                model->addVar(0, 1, 0, GRB_BINARY);
            }
        }
    }

    void requiredClasses() {
        //for subpart

    }

    void parentChild() {
        //enforce relation
    }

    void conflicts() {

    }

};


#endif //PROJECT_STUDENT_H
