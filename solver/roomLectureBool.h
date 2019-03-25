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
    void cuts() {
        // left blank
    }
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

    void oneLectureRoomConflict(GRBVar **order) {
        try {

            for (int j = 0; j < instance->getNumClasses(); j++) {
                int i1 = 0, i = 0;
                for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                     it != instance->getRooms().end(); it++) {
                    if (instance->getClasses()[j]->containsRoom(instance->getRoom(i1 + 1))) {
                        for (int j1 = 1; j1 < instance->getNumClasses(); j1++) {
                            if (instance->getClasses()[j]->containsRoom(instance->getRoom(i1 + 1)) &&
                                instance->getClasses()[j1]->containsRoom(instance->getRoom(i1 + 1))) {
                                GRBVar tempV = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "temp" + itos(i) + "_" + itos(j) + "_" +
                                                             itos(j1));
                                GRBVar tempC = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "tempC" + itos(i) + "_" + itos(j) + "_" +
                                                             itos(j1));
                                model->addGenConstrIndicator(tempC, 1, (order[j][j1] + order[j1][j]) == 2);
                                model->addGenConstrIndicator(tempC, 0, (order[j][j1] + order[j1][j]) <= 1);
                                if (vector[j][i] + vector[j1][i] == 2)
                                    model->addConstr(1 + tempC <= 1);


                            }
                        }
                        i++;
                    }
                    i1++;

                }
            }


        } catch (GRBException e) {
            printError(e, "oneLectureRoomConflict");
        }
    }


};


#endif //PROJECT_ROOMLECTURE_H
