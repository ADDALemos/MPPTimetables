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
    void loadPreviousWeekSolution(int **room) {
        for (int c = 0; c < instance->getClassesWeek(currentW).size(); ++c) {
            for (int i = 0; i < instance->getClassesWeek(currentW)[c]->getPossibleRooms().size(); ++i) {
                model->addConstr(vector[c][i] == room[c][i]);
            }

        }
    }


    roomLectureGRB(Instance *instance, int w) : roomLecture(instance, w) {}

    void definedRoomLecture() {
        try {
            vector = new GRBVar *[instance->getClassesWeek(currentW).size()];
            for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
                vector[i] = new GRBVar[instance->getClassesWeek(currentW)[i]->getPossibleRooms().size()];
                for (int r = 0; r < instance->getClassesWeek(currentW)[i]->getPossibleRooms().size(); ++r) {
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

            for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
                int rs = 0, rg = 0;
                for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                     it != instance->getRooms().end(); it++) {
                    if (instance->isRoomBlocked(rs + 1) &&
                        instance->getClassesWeek(currentW)[j]->containsRoom(instance->getRoom(rs + 1))) {
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
                            instance->getClassesWeek(currentW).size(); j++) {
                GRBLinExpr temp = 0;
                if (instance->getClassesWeek(currentW)[j]->getPossibleRooms().size() > 0) {
                    for (int i = 0; i < instance->getClassesWeek(currentW)[j]->getPossibleRooms().size(); i++) {
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
                        instance->getClassesWeek(currentW).size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClassesWeek(currentW)[l]->containsRoom(instance->getRoom(j + 1))) {
                    model->addConstr(it->second.getCapacity() >=
                                     ((instance->getClassesWeek(currentW)[l]->getLimit() -
                                       (instance->getClassesWeek(currentW)[l]->getLimit() * instance->getAlfa())) *
                                      vector[l][j]));
                    j++;
                }


            }
        }
    }

    bool isStatic() { return false; }

    GRBVar **getGRB() { return vector; }

    void warmStart(int **sol) {
        for (int l = 0; l < instance->getClassesWeek(currentW).size(); ++l) {
            int rs = 0, rg = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClassesWeek(currentW)[l]->containsRoom(instance->getRoom(rs + 1))) {
                    vector[l][rg].set(GRB_DoubleAttr_Start, sol[rs][l]);
                    rg++;
                }
                rs++;
            }
        }
    }


    void oneLectureRoomConflict(GRBVar **order) {
        try {

            for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
                int i1 = 0, i = 0;
                for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                     it != instance->getRooms().end(); it++) {
                    if (instance->getClassesWeek(currentW)[j]->containsRoom(instance->getRoom(i1 + 1))) {
                        for (int j1 = 1; j1 < instance->getNumClasses(); j1++) {
                            if (instance->getClassesWeek(currentW)[j]->containsRoom(instance->getRoom(i1 + 1)) &&
                                instance->getClassesWeek(currentW)[j1]->containsRoom(instance->getRoom(i1 + 1))) {
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

    void cuts() {
        for (int i = 0; i < instance->getNumClasses(); ++i) {
            int r = 0, r1 = 0;
            for (std::map<Room, int>::const_iterator it = instance->getClassesWeek(
                    currentW)[i]->getPossibleRooms().begin();
                 it != instance->getClassesWeek(currentW)[i]->getPossibleRooms().end(); it++) {
                for (std::map<Room, int>::const_iterator it1 = it;
                     it1 != instance->getClassesWeek(currentW)[i]->getPossibleRooms().end(); it1++) {
                    if (it->first.getCapacity() == it1->first.getCapacity())
                        model->addConstr(vector[i][r] >= vector[i][r1]);
                    r1++;
                }
                r++;


            }
        }
    }

    void force(int **room, GRBVar *le, int ***lect) {
        GRBLinExpr exp = 0;
        int size = 0;
        for (int c = 0; c < instance->getClassesWeek(currentW).size(); ++c) {
            for (int i = 0; i < instance->getClassesWeek(currentW)[c]->getPossibleRooms().size(); ++i) {
                if (room[c][i]) {
                    exp += vector[c][i];
                    size++;
                }
            }

        }
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int t = 0; t < instance->getSlotsperday(); ++t) {
                for (int i = 0; i < instance->getClassesWeek(currentW).size(); ++i) {
                    if (lect[d][t][instance->getClassesWeek(currentW)[i]->getOrderID()]) {
                        GRBVar temp1 = model->addVar(0, 2, 0, GRB_INTEGER);
                        GRBVar temp2 = model->addVar(0, 1, 0, GRB_BINARY);
                        model->addGenConstrIndicator(temp1, 1, le[i] <= (d * instance->getSlotsperday()) + t - 1);
                        model->addGenConstrIndicator(temp1, 2, le[i] >= (d * instance->getSlotsperday()) + t + 1);
                        model->addGenConstrIndicator(temp1, 0, le[i] == (d * instance->getSlotsperday()) + t + 1);
                        model->addGenConstrIndicator(temp2, 1, temp1 == 0);
                        model->addGenConstrIndicator(temp2, 0, temp1 >= 1);
                        exp += temp2;
                        size++;
                    }
                }
            }

        }
        size--;
        model->addConstr(exp <= size);
    }


};


#endif //PROJECT_ROOMLECTURE_H
