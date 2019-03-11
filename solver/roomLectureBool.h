//
// Created by Alexandre Lemos on 11/03/2019.
//

#ifndef PROJECT_ROOMLECTUREBOOL_H
#define PROJECT_ROOMLECTUREBOOL_H


#include <gurobi_c++.h>
#include "GurobiAux.h"
#include "../solver/roomLecture.h"


class roomLectureBool : public roomLecture {
    bool **vector;
public:
    roomLectureBool(Instance *instance) : roomLecture(instance) {}

    void slackStudent() {}

    void definedRoomLecture() {
        vector = new bool *[instance->getNumClasses()];
        for (int i = 0; i < instance->getNumClasses(); i++) {
            vector[i] = new bool[instance->getClasses()[i]->getPossibleRooms().size()];

        }


    }

    bool isStatic() { return true; }

    bool **getBool() { return vector; }

    void warmStart(int **sol) {
        for (int l = 0; l < instance->getNumClasses(); ++l) {
            int rs = 0, rg = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClasses()[l]->containsRoom(instance->getRoom(rs + 1))) {
                    vector[l][rg] = sol[rs][l];
                    rg++;
                }
                rs++;
            }
        }
    }


};


#endif //PROJECT_ROOMLECTURE_H
