//
// Created by Alexandre Lemos on 11/03/2019.
//

#ifndef PROJECT_ROOMLECTUREGRB_H
#define PROJECT_ROOMLECTUREGRB_H


#include <gurobi_c++.h>
#include "../solver/roomLecture.h"

class roomLectureGRB : public roomLecture {
    GRBVar **vector;
public:
    roomLectureGRB(Instance *instance) : roomLecture(instance) {}

    void definedRoomLecture() {
        try {
            vector = new GRBVar *[instance->getNumClasses()];
            for (int i = 0; i < instance->getNumClasses(); i++) {
                vector[i] = new GRBVar[instance->getClasses()[i]->getPossibleRooms().size()];
                for (int r = 0; r < instance->getClasses()[i]->getPossibleRooms().size(); ++r) {
                    std::string name = "X_" + itos(i) + "_" + itos(r);
                    vector[i][r] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);

                }

            }
        } catch (GRBException e) {
            printError(e, "definedRoomLecture");

        }


    }

    void roomClose() {
        try {

            for (int j = 0; j < instance->getNumClasses(); ++j) {
                int rs = 0, rg = 0;
                for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                     it != instance->getRooms().end(); it++) {
                    if (instance->isRoomBlocked(rs + 1) &&
                        instance->getClasses()[j]->containsRoom(instance->getRoom(rs + 1))) {
                        model->addConstr(vector[j][rg] == 0);
                        rg++;
                    }
                    rs++;
                }

            }
        } catch (GRBException e) {
            printError(e, "roomClose");
        }
    }


    void oneLectureRoom() {
        try {
            for (int j = 0; j <
                            instance->getNumClasses(); j++) {
                GRBLinExpr temp = 0;
                if (instance->getClasses()[j]->getPossibleRooms().size() > 0) {
                    for (int i = 0; i < instance->getClasses()[j]->getPossibleRooms().size(); i++) {
                        temp += vector[j][i];
                    }
                    model->addConstr(temp == 1);
                }
            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoom");
        }

    }

    void roomPreference(int r, int l) {
        model->addConstr(vector[l][r] == 1);
    }

    void slackStudent() {
        for (int l = 0; l <
                        instance->getNumClasses(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                    model->addConstr(it->second.getCapacity() >=
                                     ((instance->getClasses()[l]->getLimit() -
                                       (instance->getClasses()[l]->getLimit() * instance->getAlfa())) *
                                      vector[l][j]));
                    j++;
                }


            }
        }
    }

    bool isStatic() { return false; }

    GRBVar **getGRB() { return vector; }

    void warmStart(int **sol) {
        for (int l = 0; l < instance->getNumClasses(); ++l) {
            int rs = 0, rg = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClasses()[l]->containsRoom(instance->getRoom(rs + 1))) {
                    vector[l][rg].set(GRB_DoubleAttr_Start, sol[rs][l]);
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
                                model->addGenConstrIndicator(tempV, 1, vector[j][i] + vector[j1][i] == 2);
                                model->addGenConstrIndicator(tempV, 0, vector[j][i] + vector[j1][i] <= 1);
                                model->addConstr(tempV + tempC <= 1);


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
