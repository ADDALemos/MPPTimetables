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
    void loadPreviousWeekSolution(int **room) override {
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

    void definedRoomLecture() override {
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

    void roomClose() override {
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


    void oneLectureRoom() override {
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

    void roomPreference(int r, int l) override {
        model->addConstr(vector[l][r] == 1);
    }

    void slackStudent() override {
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

    bool isStatic() override { return false; }

    GRBVar **getGRB() override { return vector; }

    void warmStart(int **sol) override {
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


    void oneLectureRoomConflict(GRBVar *time, GRBVar *end, GRBVar **sameday, GRBVar **week) override {
        try {

            for (Class *j : instance->getClassesWeek(currentW)) {

                for (Class *j1 : instance->getClassesWeek(currentW)) {
                    GRBLinExpr weekL = 0;
                    for (int k = 0; k < instance->getNweek(); ++k) {
                        GRBVar sameweek = model->addVar(0, 1, 0, GRB_BINARY);
                        model->addGenConstrIndicator(sameweek, 1,
                                                     week[k][j->getOrderID()] + week[k][j1->getOrderID()] == 2);
                        model->addGenConstrIndicator(sameweek, 0,
                                                     week[k][j->getOrderID()] + week[k][j1->getOrderID()] <= 1);
                        weekL += sameweek;
                    }
                    GRBVar sameweek = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(sameweek, 1, weekL >= 1);
                    model->addGenConstrIndicator(sameweek, 0, weekL == 0);
                    GRBVar sameTime = model->addVar(0, 1, 0, GRB_BINARY);
                    //(Ci.end  ≤ Cj .start)
                    std::string name = "T_" + itos(j->getOrderID()) + "_" + itos(j1->getOrderID());
                    GRBVar temp_l1_l2 = model->addVar(0, 1, 0, GRB_BINARY, name);
                    model->addGenConstrIndicator(temp_l1_l2, 1,
                                                 end[j->getOrderID()] <=
                                                 time[j1->getOrderID()]);
                    model->addGenConstrIndicator(temp_l1_l2, 0,
                                                 end[j->getOrderID()] >=
                                                 time[j1->getOrderID()] - 1);
                    //(Cj .end  ≤ Ci.start)
                    name = "T_" + itos(j1->getOrderID()) + "_" + itos(j->getOrderID());
                    GRBVar temp_l2_l1 = model->addVar(0, 1, 0, GRB_BINARY, name);
                    model->addGenConstrIndicator(temp_l2_l1, 1,
                                                 end[j1->getOrderID()] <=
                                                 time[j->getOrderID()]);
                    model->addGenConstrIndicator(temp_l2_l1, 0,
                                                 end[j1->getOrderID()] >=
                                                 time[j->getOrderID()] - 1);

                    model->addGenConstrIndicator(sameTime, 1,
                                                 temp_l1_l2 + temp_l2_l1 >= 1);
                    model->addGenConstrIndicator(sameTime, 0,
                                                 temp_l1_l2 + temp_l2_l1 == 0);


                    int i1 = 0, i = 0;
                    for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                         it != instance->getRooms().end(); it++) {
                        if (j->containsRoom(instance->getRoom(i1 + 1)) &&
                            j1->containsRoom(instance->getRoom(i1 + 1))) {


                            GRBVar tempV = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                         "SameRoom" + itos(i) + "_" + itos(j->getOrderID()) + "_" +
                                                         itos(j1->getOrderID()));
                            model->addGenConstrIndicator(tempV, 1, vector[j->getOrderID()][i]
                                                                   + vector[j1->getOrderID()][i] == 2);
                            model->addGenConstrIndicator(tempV, 0, vector[j->getOrderID()][i]
                                                                   + vector[j1->getOrderID()][i] <= 1);
                            model->addConstr(
                                    tempV + sameTime + sameday[j1->getOrderID()][j->getOrderID()] + sameweek <= 3);
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

    void cuts() override {
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

    void force(int **room, GRBVar *le, int ***lect) override {
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
            for (int y = 0; y < c2->getPossibleRooms().size(); ++y) {
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
    }

    virtual GRBLinExpr travel(std::vector<Class *> c, GRBVar *time, GRBVar *end, GRBVar **sameday, int pen) override {
        try {
            GRBLinExpr travelL = 0;
            for (int cla = 0; cla < c.size(); cla++) {
                Class *l1 = c[cla];
                if (l1->isActive(currentW)) {
                    for (int cla1 = cla + 1; cla1 < c.size(); cla1++) {
                        Class *l2 = c[cla1];
                        if (l2->isActive(currentW) && l2->getOrderID() != l1->getOrderID()) {
                            GRBVar orV[3];
                            //(Ci.end + Ci.room.travel[Cj .room] ≤ Cj .start)
                            std::string name = "T_" + itos(l1->getOrderID()) + "_" + itos(l2->getOrderID());
                            GRBVar temp_l1_l2 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l1_l2, 1,
                                                         end[l1->getOrderID()] + travel(l1, l2) <=
                                                         time[l2->getOrderID()]);
                            model->addGenConstrIndicator(temp_l1_l2, 0,
                                                         (end[l1->getOrderID()] + travel(l1, l2)) >=
                                                         time[l2->getOrderID()] - 1);
                            //(Cj .end + Cj .room.travel[Ci.room] ≤ Ci.start)
                            name = "T_" + itos(l2->getOrderID()) + "_" + itos(l1->getOrderID());
                            GRBVar temp_l2_l1 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l2_l1, 1,
                                                         end[l2->getOrderID()] + travel(l2, l1) <=
                                                         time[l1->getOrderID()]);
                            model->addGenConstrIndicator(temp_l2_l1, 0,
                                                         (end[l2->getOrderID()] + travel(l2, l1)) >=
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
            return travelL;
        } catch (GRBException e) {
            printError(e, "travel");
        }
        return 0;

    }

    virtual GRBLinExpr
    differentRoom(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) override {
        GRBLinExpr result = 0;
        for (int c1 = 0; c1 < vector.size(); c1++) {
            for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                    int i = 0, i1 = 0, i2 = 0;
                    for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                         it != instance->getRooms().end(); it++) {
                        if (vector[c1]->containsRoom(instance->getRoom(i + 1))) {
                            if (vector[c2]->containsRoom(instance->getRoom(i + 1))) {
                                if (penalty == -1) {
                                    if (b)
                                        model->addConstr(
                                                this->vector[vector[c1]->getOrderID()][i1] ==
                                                this->vector[vector[c2]->getOrderID()][i2]);
                                    else
                                        model->addConstr(
                                                this->vector[vector[c1]->getOrderID()][i1] +
                                                this->vector[vector[c2]->getOrderID()][i2] <= 1);
                                } else {
                                    GRBVar sameRoom = model->addVar(0, 1, 0, GRB_BINARY);
                                    model->addGenConstrIndicator(sameRoom, (b ? 1 : 0),
                                                                 this->vector[vector[c1]->getOrderID()][i1] ==
                                                                 this->vector[vector[c2]->getOrderID()][i2]);
                                    model->addGenConstrIndicator(sameRoom, (b ? 0 : 1),
                                                                 this->vector[vector[c1]->getOrderID()][i1] +
                                                                 this->vector[vector[c2]->getOrderID()][i2] <= 1);
                                    result += sameRoom;
                                }

                                i2++;
                            }
                            i1++;
                        }
                        i++;

                    }
                }
            }
        }
        return result;
    }

    virtual GRBLinExpr roomPen() override {
        GRBLinExpr pen = 0;
        for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int r = 0, rl = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClassesWeek(currentW)[j]->containsRoom(instance->getRoom(r + 1))) {
                    pen += vector[instance->getClassesWeek(currentW)[j]->getOrderID()][rl] *
                           instance->getClassesWeek(currentW)[j]->getPen(instance->getRoom(r + 1));
                    rl++;
                }
                r++;

            }

        }
        return pen;

    }

    /**
     * roomUnavailable
     * @param week
     * @param day
     * @param lecture
     */
    virtual void roomUnavailable(GRBVar **week, GRBVar **day, GRBVar *lecture) override {//duration, weeks,days of una??
        for (int r = 0; r < instance->getRooms().size(); ++r) {
            for (int c = 0; c < instance->getClasses().size(); ++c) {
                for (int s = 0; s < instance->getRoom(r + 1).getSlots().size(); ++s) {
                    for (const char &w : instance->getRoom(r + 1).getSlots()[s].getWeeks()) {
                        for (const char &d : instance->getRoom(r + 1).getSlots()[s].getDays()) {
                            GRBVar t = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(t, 1, lecture[instance->getClasses()[c]->getOrderID()] >=
                                                               instance->getRoom(r + 1).getSlots()[s].getStart());
                            model->addGenConstrIndicator(t, 0, lecture[instance->getClasses()[c]->getOrderID()] <=
                                                               instance->getRoom(r + 1).getSlots()[s].getStart() - 1);
                            GRBVar t1 = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(t1, 1, lecture[instance->getClasses()[c]->getOrderID()] <=
                                                                instance->getRoom(r + 1).getSlots()[s].getStart() +
                                                                instance->getRoom(r + 1).getSlots()[s].getLenght() - 1);
                            model->addGenConstrIndicator(t1, 0, lecture[instance->getClasses()[c]->getOrderID()] >=
                                                                instance->getRoom(r + 1).getSlots()[s].getStart() +
                                                                instance->getRoom(r + 1).getSlots()[s].getLenght());
                            GRBVar t2 = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(t2, 1, week[w][instance->getClasses()[c]->getOrderID()] == 1);
                            model->addGenConstrIndicator(t2, 1, day[instance->getClasses()[c]->getOrderID()][d] == 1);
                            model->addGenConstrIndicator(t2, 1, t + t1 == 2);
                            model->addGenConstrIndicator(t2, 0, week[w][instance->getClasses()[c]->getOrderID()] <= 1);
                            model->addGenConstrIndicator(t2, 0, day[instance->getClasses()[c]->getOrderID()][d] <= 1);
                            model->addGenConstrIndicator(t2, 0, t + t1 <= 1);
                            model->addConstr(vector[instance->getClasses()[c]->getOrderID()][r] + t1 <= 1);

                        }

                    }

                }

            }
        }
    }





};


#endif //PROJECT_ROOMLECTURE_H
