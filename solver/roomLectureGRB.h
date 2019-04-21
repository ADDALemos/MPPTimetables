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
                    model->addConstr(vector[instance->getClassesWeek(currentW)[c]->getOrderID()][r] == room[j][c]);
                    r++;
                }
            }
        }
    }



    roomLectureGRB(Instance *instance, int w) : roomLecture(instance, w) {}

    void definedRoomLecture() {
        try {
            vector = new GRBVar *[instance->getClassesWeek(currentW).size()];
            for (Class *c : instance->getClassesWeek(currentW)) {
                vector[c->getOrderID()] = new GRBVar[c->getPossibleRooms().size()];
                for (int r = 0; r < c->getPossibleRooms().size(); ++r) {
                    std::string name = "X_" + itos(c->getOrderID()) + "_" + itos(r);
                    vector[c->getOrderID()][r] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);

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
            for (Class *c: instance->getClassesWeek(currentW)) {
                if (c->getPossibleRooms().size() > 0) {
                    GRBLinExpr temp = 0;
                    for (int i = 0; i < c->getPossibleRooms().size(); i++) {
                        temp += vector[c->getOrderID()][i];
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
                                      vector[instance->getClassesWeek(currentW)[l]->getOrderID()][j]));
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

            for (Class *j : instance->getClassesWeek(currentW)) {

                for (Class *j1 : instance->getClassesWeek(currentW)) {
                    int i1 = 0, i = 0;
                    for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                         it != instance->getRooms().end(); it++) {
                        if (j->containsRoom(instance->getRoom(i1 + 1)) &&
                            j1->containsRoom(instance->getRoom(i1 + 1))) {
                            GRBVar tempV = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                         "temp" + itos(i) + "_" + itos(j->getOrderID()) + "_" +
                                                         itos(j1->getOrderID()));
                            GRBVar tempC = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                         "tempC" + itos(i) + "_" + itos(j->getOrderID()) + "_" +
                                                         itos(j1->getOrderID()));

                            model->addGenConstrIndicator(tempC, 1, (order[j->getOrderID()][j1->getOrderID()] +
                                                                    order[j1->getOrderID()][j->getOrderID()]) == 2);
                            model->addGenConstrIndicator(tempC, 0, (order[j->getOrderID()][j1->getOrderID()] +
                                                                    order[j1->getOrderID()][j->getOrderID()]) <= 1);
                            model->addGenConstrIndicator(tempV, 1, vector[j->getOrderID()][i]
                                                                   + vector[j1->getOrderID()][i] == 2);
                            model->addGenConstrIndicator(tempV, 0, vector[j->getOrderID()][i]
                                                                   + vector[j1->getOrderID()][i] <= 1);
                            model->addConstr(tempV + tempC + sameday[j1->getOrderID()][j->getOrderID()] <= 2);
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

    virtual GRBLinExpr travel(Class *c1, Class *c2) {
        GRBLinExpr temp = 0;
        for (int i = 0; i < c1->getPossibleRooms().size(); ++i) {
            for (int y = i + 1; y < c2->getPossibleRooms().size(); ++y) {
                GRBVar temp_r1_r2 = model->addVar(0, 1, 0, GRB_BINARY);
                model->addGenConstrIndicator(temp_r1_r2, 1,
                                             vector[c1->getOrderID()][i] - vector[c2->getOrderID()][y] == 0);
                model->addGenConstrIndicator(temp_r1_r2, 0,
                                             vector[c1->getOrderID()][i] + vector[c2->getOrderID()][y] <= 1);
                temp += temp_r1_r2 * c2->getPossibleRoom(y).
                        getTravel(c1->getPossibleRoom(i).getId());
            }
        }
        return temp;
        //model->setObjective(temp,GRB_MINIMIZE);
    }

    virtual void travel(std::vector<Class *> c, GRBVar *time, GRBVar **sameday, int pen) throw() {
        try {
            GRBLinExpr travelL = 0;
            for (Class *l1: c) {
                if (l1->isActive(currentW)) {
                    for (Class *l2: c) {
                        if (l2->isActive(currentW)) {
                            GRBVar orV[3];
                            //(Ci.end + Ci.room.travel[Cj .room] ≤ Cj .start)
                            std::string name = "T_" + itos(l1->getOrderID()) + "_" + itos(l2->getOrderID());
                            GRBVar temp_l1_l2 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l1_l2, 1,
                                                         time[l1->getOrderID()] + l1->getLenght() + travel(l1, l2) <=
                                                         time[l2->getOrderID()]);
                            model->addGenConstrIndicator(temp_l1_l2, 0,
                                                         (time[l1->getOrderID()] + l1->getLenght() + travel(l1, l2)) >=
                                                         time[l2->getOrderID()] - 1);
                            //(Cj .end + Cj .room.travel[Ci.room] ≤ Ci.start)
                            name = "T_" + itos(l2->getOrderID()) + "_" + itos(l1->getOrderID());
                            GRBVar temp_l2_l1 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l2_l1, 1,
                                                         time[l2->getOrderID()] + l2->getLenght() + travel(l2, l1) <=
                                                         time[l1->getOrderID()]);
                            model->addGenConstrIndicator(temp_l2_l1, 0,
                                                         (time[l2->getOrderID()] + l2->getLenght() + travel(l2, l1)) >=
                                                         time[l1->getOrderID()] - 1);
                            orV[0] = temp_l1_l2;
                            orV[1] = temp_l2_l1;
                            orV[2] = sameday[l1->getOrderID()][l2->getOrderID()];

                            name = "TOR_" + itos(l2->getOrderID()) + "_" + itos(l1->getOrderID());
                            GRBVar tor = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrOr(tor, orV, 3, name);
                            if (pen == -1)
                                model->addConstr(tor >= 1);
                            else {
                                GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
                                model->addGenConstrIndicator(temp, 0, tor >= 1);
                                model->addGenConstrIndicator(temp, 1, tor == 0);
                                travelL += pen * temp;
                            }
                        }

                    }
                }

            }
            if (pen > -1) {
                model->setObjective(travelL, GRB_MINIMIZE);
            }
        } catch (GRBException e) {
            printError(e, "travel");
        }

    }





};


#endif //PROJECT_ROOMLECTURE_H
