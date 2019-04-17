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
            for (int j = 0, r = 0; j < instance->getNumRoom(); ++j) {
                if (instance->getClassesWeek(currentW)[c]->containsRoom(instance->getRoom(j + 1))) {
                    model->addConstr(vector[c][r] == room[j][c]);
                    r++;
                }
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
                if (instance->getClassesWeek(currentW)[j]->getPossibleRooms().size() > 0) {
                    GRBLinExpr temp = 0;
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
            int j = 0, r = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClassesWeek(currentW)[l]->containsRoom(instance->getRoom(r + 1))) {
                    model->addConstr(it->second.getCapacity() >=
                                     ((instance->getClassesWeek(currentW)[l]->getLimit() -
                                       (instance->getClassesWeek(currentW)[l]->getLimit() * instance->getAlfa())) *
                                      vector[l][j]));
                    j++;
                }
                r++;


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


    void oneLectureRoomConflict(GRBVar **order, GRBVar **sameday) {
        try {

            for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {

                for (int j1 = 1; j1 < instance->getClassesWeek(currentW).size(); j1++) {
                    int i1 = 0, i = 0;
                    for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                         it != instance->getRooms().end(); it++) {
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

                            model->addConstr(tempV + tempC + sameday[j][j1] <= 2);
                            //same time same room same day
                            //1 1 1 conflict
                            //1 0 1 no problem
                            //1 1 0 no problem
                            //1 0 0 no problem
                            //0 0 0 no problem
                            //0 0 1 no problem
                            //0 1 0 no problem
                            //0 1 1 no problem

                            i++;
                        }
                        i1++;
                    }

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
            for (int i = 0, r = 0; i < instance->getRooms().size(); ++i) {
                if (instance->getClassesWeek(currentW)[c]->containsRoom(instance->getRoom(i + 1))) {
                    if (room[c][i]) {
                        exp += vector[c][r];
                        size++;
                    }
                    r++;
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

    virtual GRBLinExpr travel(int c1, int c2) {
        GRBLinExpr temp = 0;
        for (int i = 0; i < instance->getClasses()[c1]->getPossibleRooms().size(); ++i) {
            for (int y = i + 1; y < instance->getClasses()[c2]->getPossibleRooms().size(); ++y) {
                GRBVar temp_r1_r2 = model->addVar(0, 1, 0, GRB_BINARY);
                model->addGenConstrIndicator(temp_r1_r2, 1, vector[c1][i] - vector[c2][y] == 0);
                model->addGenConstrIndicator(temp_r1_r2, 0, vector[c1][i] + vector[c2][y] <= 1);
                temp += temp_r1_r2 * instance->getClasses()[c2]->getPossibleRoom(y).
                        getTravel(instance->getClasses()[c1]->getPossibleRoom(i).getId());
            }
        }
        return temp;
        //model->setObjective(temp,GRB_MINIMIZE);
    }

    virtual void travel(std::vector<int> c, GRBVar *time, GRBVar **order) throw() {
        try {
            for (int l1: c) {
                for (int l2: c) {
                    GRBVar orV[2];
                    //(Ci.end + Ci.room.travel[Cj .room] ≤ Cj .start)
                    std::string name = "T_" + itos(l1) + "_" + itos(l2);
                    GRBVar temp_l1_l2 = model->addVar(0, 1, 0, GRB_BINARY, name);
                    model->addGenConstrIndicator(temp_l1_l2, 1,
                                                 time[l1] + instance->getClasses()[l1]->getLenght() + travel(l1, l2) <=
                                                 time[l2]);
                    //model->addGenConstrIndicator(temp_l1_l2,0,(time[l1]+instance->getClasses()[l1]->getLenght()+travel(l1, l2))>=time[l2]-1);
                    //(Cj .end + Cj .room.travel[Ci.room] ≤ Ci.start)
                    name = "T_" + itos(l2) + "_" + itos(l1);
                    GRBVar temp_l2_l1 = model->addVar(0, 1, 0, GRB_BINARY, name);
                    model->addGenConstrIndicator(temp_l2_l1, 1,
                                                 time[l2] + instance->getClasses()[l2]->getLenght() + travel(l2, l1) <=
                                                 time[l1]);
                    //model->addGenConstrIndicator(temp_l2_l1,0,(time[l2]+instance->getClasses()[l2]->getLenght()+travel(l2, l1))>=time[l1]-1);
                    orV[0] = temp_l1_l2;
                    orV[1] = temp_l2_l1;
                    name = "TOR_" + itos(l2) + "_" + itos(l1);
                    GRBVar tor = model->addVar(0, 1, 0, GRB_BINARY, name);
                    model->addGenConstrOr(tor, orV, 2, name);
                    //model->addConstr(tor>=1);

                }

            }
        } catch (GRBException e) {
            printError(e, "travel");
        }

    }


    virtual GRBLinExpr travel(std::vector<int> c, int pen) {
        GRBLinExpr temp = 0;
        for (int l1: c) {
            for (int l2: c) {
                temp += travel(l1, l2) * pen;
            }
            //(Ci.end + Ci.room.travel[Cj .room] ≤ Cj .start)
            //GRBVar temp_l1_l1 = model->addVar(0,1,0,GRB_BINARY);
            //model->addGenConstrIndicator(temp_l1_l1,1,0>0);
        }
        model->setObjective(temp, GRB_MINIMIZE);


    }


};


#endif //PROJECT_ROOMLECTURE_H
