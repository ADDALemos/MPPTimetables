//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_MIXEDMODELGUROBIEXECUTER_H
#define PROJECT_MIXEDMODELGUROBIEXECUTER_H

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../utils/TimeUtil.h"
#include "../solver/TwoVarGurobiExecuter.h"
#include "../solver/roomLectureGRB.h"
#include "../solver/roomLectureBool.h"

class MixedModelGurobiExecuter : public TwoVarGurobiExecuter {
    GRBVar *lectureTime;//start time
    GRBVar *endTime;//end time
    GRBVar ***sameday; //same day between two classes in one day
    GRBVar **samedays; //one day overlap two classes
    GRBVar **day; //day
    GRBVar **we; //week
    GRBVar **isWeek; //which time does the week is from
    GRBVar **isDay; //which time does the day is from
    GRBVar **isStart; //which time does the start is from


public:

    virtual void save() override {
        switchSolutionTime();
        switchSolutionRoom();
        studentSectioning->save();


    }

    void loadPreviousWeekSolution(int ***time, int **room) override {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int t = 0; t < instance->getSlotsperday(); ++t) {
                for (int i = 0; i < instance->getClassesWeek(currentW).size(); ++i) {
                    if (time[d][t][instance->getClassesWeek(currentW)[i]->getOrderID()]) {
                        model->addConstr(lectureTime[i] == (d * instance->getSlotsperday()) + t);
                    }
                }
            }

        }
        roomLecture->loadPreviousWeekSolution(room);
    }


    MixedModelGurobiExecuter(Instance *i) {
        setInstance(i);
        roomLecture = new roomLectureGRB(instance, currentW);
    }

    void setCurrrentW(int currrentW) override {
        ILPExecuter::currentW = currrentW;
        roomLecture->setCurrrentW(currrentW);
    }

    MixedModelGurobiExecuter(bool isStatic, bool isWeekly, Instance *i) {
        setInstance(i);
        if (isStatic)
            roomLecture = new roomLectureBool(instance, currentW);
        else
            roomLecture = new roomLectureGRB(instance, currentW);
        studentSectioning = new StudentSectioning(instance);
    }

    void definedAuxVar() override {

    }

    void printConfiguration() override {
        std::cout << "Three variable: type int for time" << std::endl;
        std::cout << "            : type bool for days" << std::endl;
        std::cout << "            : type bool for rooms" << std::endl;

    }

    void dayConst() override {
        try {
            isDay = new GRBVar *[instance->getClassesWeek(currentW).size()];
            for (int k = 0; k < instance->getClassesWeek(currentW).size(); ++k) {
                isDay[k] = new GRBVar[instance->getClassesWeek(currentW)[k]->getLectures().size()];
                for (int i = 0; i < instance->getClassesWeek(currentW)[k]->getLectures().size(); ++i) {
                    GRBVar andDay[instance->getNdays()];
                    for (int j = 0; j < instance->getNdays(); j++) {
                        andDay[j] = model->addVar(0, 1, 0, GRB_BINARY, "DayAND_" + itos(instance->getClassesWeek(
                                currentW)[k]->getOrderID()) + "_" + itos(i) + "_" +
                                                                       itos(j));
                        int d = (instance->getClassesWeek(currentW)[k]->getLectures()[i]->getDays().at(j) == '0' ? 0
                                                                                                                 : 1);
                        model->addGenConstrIndicator(andDay[j], 1,
                                                     day[instance->getClassesWeek(currentW)[k]->getOrderID()][j] ==
                                                     d);
                        model->addGenConstrIndicator(andDay[j], 0,
                                                     day[instance->getClassesWeek(currentW)[k]->getOrderID()][j] +
                                                     d == 1);

                    }
                    isDay[k][i] = model->addVar(0, 1, 0, GRB_BINARY, "DayOR_" + itos(instance->getClassesWeek(
                            currentW)[k]->getOrderID()) + "_" + itos(i));
                    model->addGenConstrAnd(isDay[k][i], andDay, instance->getNdays());
                }
                GRBVar t = model->addVar(0, 1, 0, GRB_BINARY, "orday_" + itos(instance->getClassesWeek(
                        currentW)[k]->getOrderID()));
                model->addGenConstrOr(t, isDay[k], instance->getClassesWeek(currentW)[k]->getLectures().size());
                model->addConstr(t == 1);
            }
        } catch (GRBException e) {
            printError(e, "day");
        }
    }


    void definedLectureTime() override {
        lectureTime = new GRBVar[instance->getClassesWeek(currentW).size()];
        endTime = new GRBVar[instance->getClassesWeek(currentW).size()];
        samedays = new GRBVar *[instance->getClassesWeek(currentW).size()];
        sameday = new GRBVar **[instance->getClassesWeek(currentW).size()];
        day = new GRBVar *[instance->getClassesWeek(currentW).size()];
        for (Class *k :instance->getClassesWeek(currentW)) {
            samedays[k->getOrderID()] = new GRBVar[instance->getClassesWeek(currentW).size()];
            sameday[k->getOrderID()] = new GRBVar *[instance->getClassesWeek(currentW).size()];
            day[k->getOrderID()] = new GRBVar[instance->getNdays()];
            for (int i = 0; i < instance->getNdays(); ++i) {
                day[k->getOrderID()][i] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                        "day" + itos(k->getOrderID()) + "_" + itos(i));
            }
            for (Class *k1 :instance->getClassesWeek(currentW)) {
                sameday[k->getOrderID()][k1->getOrderID()] = new GRBVar[instance->getNdays()];
                samedays[k->getOrderID()][k1->getOrderID()] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                                            "samedays" + itos(k->getOrderID()) + "_" +
                                                                            itos(k1->getOrderID()));

            }

            endTime[k->getOrderID()] = model->addVar(0.0, instance->getSlotsperday(), 0.0,
                                                     GRB_INTEGER,
                                                     "E_" + itos(k->getOrderID()));

            lectureTime[k->getOrderID()] = model->addVar(0.0, instance->getSlotsperday(), 0.0,
                                                         GRB_INTEGER,
                                                         "A_" + itos(k->getOrderID()));

        }
        for (Class *k :instance->getClassesWeek(currentW)) {
            for (Class *k1 :instance->getClassesWeek(currentW)) {
                GRBLinExpr som = 0;
                for (int i = 0; i < instance->getNdays(); ++i) {
                    sameday[k->getOrderID()][k1->getOrderID()][i] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                                                  "sameday" + itos(k->getOrderID()) +
                                                                                  "_"
                                                                                  + itos(k1->getOrderID()) + "_" +
                                                                                  itos(i));
                    model->addGenConstrIndicator(sameday[k->getOrderID()][k1->getOrderID()][i], 1,
                                                 day[k->getOrderID()][i] +
                                                 day[k1->getOrderID()][i] ==
                                                 2);
                    model->addGenConstrIndicator(sameday[k->getOrderID()][k1->getOrderID()][i], 0,
                                                 day[k->getOrderID()][i] +
                                                 day[k1->getOrderID()][i] <=
                                                 1);
                    som += sameday[k->getOrderID()][k1->getOrderID()][i];
                }
                model->addGenConstrIndicator(samedays[k->getOrderID()][k1->getOrderID()], 1, som >= 1);
                model->addGenConstrIndicator(samedays[k->getOrderID()][k1->getOrderID()], 0, som <= 0);
            }
        }


    }

    /**
     * The lecture can only be scheduled in one slot
     */

    void oneLectureperSlot() override {
        //Deleted constraint

    }

    /**
     * Force lectures to be in slot n
     */


    void oneLectureSlot() override {
        //Not actually needed

    }


    /***
     * The room can only have one lecture per slot without and quadratic expressions
     */

    void oneLectureRoomConflict() override {
        roomLecture->oneLectureRoomConflict(lectureTime, endTime, samedays, we);

    }


    /** TODO::
    * Ensure Room closed in a day cannot be used
    */
    void roomClosebyDay() override {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
                for (int i = 0; i < instance->getClassesWeek(currentW)[j]->getPossibleRooms().size(); i++) {
                    if (instance->isRoomBlockedbyDay(i + 1, d) && !roomLecture->isStatic()) {
                        GRBVar roomC = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                     "roomC_" + itos(d) + "_" + itos(i) + "_" +
                                                     itos(instance->getClassesWeek(currentW)[j]->getOrderID()));
                        model->addGenConstrIndicator(roomC, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[j]->getOrderID()]
                                                     / instance->getSlotsperday() == d);
                        //model->addGenConstrIndicator(roomC, 0, lectureTime[j] / instance->getSlotsperday() <= (d-1));
                        //model->addGenConstrIndicator(roomC, 0, lectureTime[j] / instance->getSlotsperday() >= (d+1));
                        if (!roomLecture->isStatic())
                            model->addConstr(
                                    roomLecture->getGRB()[j][i] * roomC == 0);

                    }
                }
            }


        }


    }


    /**
    * Ensure times lot in a day is closed cannot be used
    */
    void slotClose() override {
        try {
            for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
                for (int t = 0; t < instance->getNdays() * instance->getSlotsperday(); ++t) {
                    if (instance->isTimeUnavailable(t)) {
                        GRBVar temp = model->addVar(0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(temp, 0,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] >=
                                                     t + 1);
                        model->addGenConstrIndicator(temp, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] <=
                                                     t);
                        GRBVar temp1 = model->addVar(0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(temp1, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] >=
                                                     t);
                        model->addGenConstrIndicator(temp1, 0,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] <=
                                                     t - 1);
                        model->addConstr((temp1 + temp) <= 1);
                    }

                }


            }
        } catch (GRBException e) {
            printError(e, "slotclose");
        }
    }

    /**TODO::
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    void assignmentInvalid() override {
        int value = 0;
        for (int j = 0; j < instance->getNdays(); ++j) {
            for (int i = 0; i < instance->getSlotsperday(); ++i) {
                for (int k = 0; k < instance->getClassesWeek(currentW).size(); ++k) {
                    if (solutionTime[j][i][k] == 1) {
                        value = j + i;
                        goto label;
                    }

                }

            }

        }
        label:
        for (int i = 0; i < instance->getClassesWeek(currentW).size(); ++i) {
            if (instance->isIncorrectAssignment(i)) {
                GRBLinExpr temp = 0;
                for (int l = 0; l < instance->getNumRoom(); ++l) {
                    if (solutionRoom[l][i] == 1) {
                        GRBVar t = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(t, 1,
                                                     lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()] ==
                                                     value);
                        // model->addGenConstrIndicator(t, 0, lectureTime[i] <= value - 1);
                        temp = t;// + roomLecture[l][i];
                        model->addConstr(temp <= 1);
                        break;
                    }

                }
            }

        }
    }

    GRBLinExpr sectioning() override {
        GRBLinExpr cost = 0;
        studentSectioning->init();
        std::cout << "init : Done " << getTimeSpent() << std::endl;
        studentSectioning->requiredClasses();
        std::cout << "classes : Done " << getTimeSpent() << std::endl;
        studentSectioning->parentChild();
        std::cout << "family : Done " << getTimeSpent() << std::endl;
        cost = studentSectioning->conflicts(lectureTime, endTime);
        std::cout << "conflicts : Done " << getTimeSpent() << std::endl;
        studentSectioning->limit();
        std::cout << "limit : Done " << getTimeSpent() << std::endl;
        return cost;

    }


    /**Teacher's conflict*/
    void teacher() override {
        throw "Not Supported";

    }


    /** Student conflicts hard constraint based on the input model
     *
     */
    void studentConflict() override {
        throw "Not Supported";
    }

    /** Student conflicts hard constraint based on the original solution
     *
     */
    void studentConflictSolution() override {
        throw "Not Supported";

    }


private:
//Number of seated students for optimization or constraint
    GRBQuadExpr numberSeatedStudents() override {
        throw "Not Supported";


        return 0;
    }

    GRBQuadExpr usage() override {
        GRBQuadExpr temp = 0;
        throw "Not Supported";


        return temp;

    }

    void gapStudentsTimetableVar() {
        throw "Not Supported";

    }


    GRBLinExpr gapStudentsTimetable() override {
        throw "Not Supported";
        return 0;
        //std::cout << min << std::endl;
    }


public:
    void printdistanceToSolutionLectures(bool w) override {
        int temp = 0;

        for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int day = lectureTime[instance->getClassesWeek(currentW)[j]->getOrderID()].get(GRB_DoubleAttr_X) /
                      instance->getSlotsperday();
            int slot = lectureTime[instance->getClassesWeek(currentW)[j]->getOrderID()].get(GRB_DoubleAttr_X) -
                       day * instance->getSlotsperday();
            temp += (w ? instance->getClasses()[j]->getLimit() : 1) * (solutionTime[day][slot][j] != 1);
        }
        std::cout << temp << std::endl;


    }

    /***
    * The current distance of the solution with the old solution
    * The distante is base on the weighted Hamming distance of the lectureTime variable (time slot atributions)
    * The weighted is baed on the number of students moved
    * @param oldTime
    * @param weighted
    * @return IloExpr
    */

    //int slot=lectureTime[i].get(GRB_DoubleAttr_X)-day*instance->getSlotsperday();
    GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted) override {
        GRBQuadExpr temp = 0;

        for (int j = 0; j < instance->getNumClasses(); ++j) {
            GRBVar tempT = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
            GRBVar tempT1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
            model->addGenConstrIndicator(tempT, 0, lectureTime[j] ==
                                                   (instance->getClasses()[j]->getSolStart() +
                                                    instance->getClasses()[j]->getSolDay() *
                                                    instance->getSlotsperday()));
            /* model->addGenConstrIndicator(tempT, 1, lectureTime[j] >=
                                                    (1+instance->getClasses()[j]->getSolStart() +
                                                     instance->getClasses()[j]->getSolDay() *
                                                     instance->getSlotsperday()));*/

            model->addGenConstrIndicator(tempT1, 0, lectureTime[j] ==
                                                    (instance->getClasses()[j]->getSolStart() +
                                                     instance->getClasses()[j]->getSolDay() *
                                                     instance->getSlotsperday()));
            /*model->addGenConstrIndicator(tempT1, 1, lectureTime[j] <=
                                                   (1-instance->getClasses()[j]->getSolStart() +
                                                    instance->getClasses()[j]->getSolDay() *
                                                    instance->getSlotsperday()));*/
            temp += (weighted ? instance->getClasses()[j]->getLimit() : 1) * (tempT + tempT1);
        }


        return temp;
    }


private:
    /**
     * Warm starting procedure with the solution found before
     * Used the class atributes: solutionTime and roomLecture
     */

    void warmStart() override {
        for (int k = 0; k < instance->getNdays(); k++) {
            for (int i = 0; i < instance->getSlotsperday(); i++) {
                for (int j = 0; j < instance->getNumClasses(); j++) {
                    if (solutionTime[k][i][j] && !instance->isTimeUnavailable(k * i))
                        lectureTime[j].set(GRB_DoubleAttr_Start, k * i);
                }
            }
        }
        roomLecture->warmStart(solutionRoom);
    }


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     *
     */

    void switchSolutionTime() override {
        for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
            char d[instance->getNdays()];
            int slot = lectureTime[instance->getClassesWeek(currentW)[i]->getOrderID()].get(GRB_DoubleAttr_X);
            std::cerr << instance->getClassesWeek(currentW)[i]->getId() << " ";
            for (int j = 0; j < instance->getNdays(); ++j) {
                d[j] = day[instance->getClassesWeek(currentW)[i]->getOrderID()][j].get(GRB_DoubleAttr_X);
                int dI = day[instance->getClassesWeek(currentW)[i]->getOrderID()][j].get(GRB_DoubleAttr_X);
                //solutionTime[dI][slot][i] = 1;
                std::cerr << dI;

            }
            std::cerr << " " << slot << " L " << instance->getClassesWeek(currentW)[i]->getLenght() << std::endl;

            instance->getClassesWeek(currentW)[i]->setSolutionTime(slot,
                                                                   d);


        }

    }

    void force() override {
        roomLecture->force(solutionRoom, lectureTime, solutionTime);
    }

    /*
     * for (int l = 0; l < instance->getClassesWeek(currentW).size(); ++l) {
            GRBLinExpr orL=0;
            for (int temp: instance->getClassesWeek(currentW)[l]->getSlots()) {
                GRBVar t = model->addVar(0,1,0,GRB_BINARY,"TempG_"+itos(l)+itos(temp));
                model->addGenConstrIndicator(t,0,lectureTime[l] >= temp);
                model->addGenConstrIndicator(t,1,lectureTime[l] <= temp-1);//Not acceptable
                GRBVar t1 = model->addVar(0,1,0,GRB_BINARY,"TempG1_"+itos(l)+itos(temp));
                model->addGenConstrIndicator(t1,1,lectureTime[l] >= temp+1);//Not acceptable
                model->addGenConstrIndicator(t1,0,lectureTime[l] <= temp);
                GRBVar t2= model->addVar(0,1,0,GRB_BINARY,"TempOR_"+itos(l)+itos(temp));
                model->addGenConstrIndicator(t2,1,lectureTime[l] ==temp);
                model->addGenConstrIndicator(t2,0,t1+t==1);//Not acceptable OR
                orL+=t2;
            }
            model->addConstr(orL==1);
        }
     */
    /**
     * Time UNA
     */
    virtual void block() override {
        try {
            isStart = new GRBVar *[instance->getClassesWeek(currentW).size()];
            for (int l = 0; l < instance->getClassesWeek(currentW).size(); ++l) {
                isStart[l] = new GRBVar[instance->getClassesWeek(currentW)[l]->getLectures().size()];
                Class *c = instance->getClassesWeek(currentW)[l];
                GRBLinExpr x = 0;
                for (int te = 0; te < c->getLectures().size(); te++) {
                    int temp = c->getLectures()[te]->getStart() - instance->minTimeSlot();
                    GRBVar t = model->addVar(0, 1, 0, GRB_BINARY,
                                             "TempUNA_ID_" + itos(c->getOrderID()) + "_" + itos(te));
                    model->addGenConstrIndicator(t, 1, lectureTime[c->getOrderID()] >= temp);
                    model->addGenConstrIndicator(t, 0, lectureTime[c->getOrderID()] <= temp - 1);//Not acceptable
                    GRBVar t1 = model->addVar(0, 1, 0, GRB_BINARY,
                                              "TempUNA1_ID_" + itos(c->getOrderID()) + "_" + itos(te));
                    model->addGenConstrIndicator(t1, 0, lectureTime[c->getOrderID()] >= temp + 1);//Not acceptable
                    model->addGenConstrIndicator(t1, 1, lectureTime[c->getOrderID()] <= temp);
                    isStart[l][te] = model->addVar(0, 1, 0, GRB_BINARY,
                                                   "TempUNA2_ID_" + itos(c->getOrderID()) + "_" + itos(te));
                    model->addGenConstrIndicator(isStart[l][te], 1, t1 + t == 2);
                    model->addGenConstrIndicator(isStart[l][te], 0, t1 + t <= 1);//Not acceptable
                    x += isStart[l][te];

                }
                model->addConstr(x >= 1);
            }
        } catch (GRBException e) {
            printError(e, "block");
        }

    }

    /**
     * Given classes cannot overlap in time, and if they are placedon overlapping days of week and weeks, they must
     * be placed far enough so that the attendees can travel between the two classes. This means that
     * (Ci.end + Ci.room.travel[Cj .room] ≤ Cj .start) ∨ (Cj .end + Cj .room.travel[Ci.room] ≤ Ci.start) ∨
     * ((Ci.days and Cj .days) = 0) ∨ ((Ci.weeks and Cj .weeks) = 0) for any two classes Ci and Cj from the constraint;
     * Ci.room.travel[Cj .room] is the travel time between the assigned rooms of Ci and Cj .
     * @param c
     * @param pen
     * @return
     */

    virtual GRBLinExpr travel(std::vector<Class *> c, int pen) override {
        try {
            return roomLecture->travel(c, lectureTime, endTime, samedays, pen);
        } catch (GRBException e) {
            printError(e, "travel");
        }
        return 0;
    }

    /**
      * Precedence Given classes must be one after the other in the order provided in
      *  the constraint definition. For classes that have multiple meetings in a week or
      *  that are on different weeks, the constraint only cares about the first meeting
      *  of the class. That is,
      *      – the first class starts on an earlier week or
      *      – they start on the same week and the first class starts on an earlier day of
      *      the week or
      *      – they start on the same week and day of the week and the first class is
      *      earlier in the day.
      * @param vector
      * @param penalty
     **/


    virtual GRBLinExpr precedence(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) override {
        GRBLinExpr result = 0;
        try {
            for (int c1 = 1; c1 < vector.size(); c1++) {
                if (vector[c1 - 1]->isActive(currentW) && vector[c1]->isActive(currentW)) {
                    //week
                    GRBLinExpr prevWc1 = 0;
                    GRBLinExpr prevWc2 = 0;
                    GRBLinExpr resultW = 0;
                    for (int w = 1; w < instance->getNweek(); ++w) {
                        prevWc1 += we[w - 1][vector[c1]->getOrderID()];
                        prevWc2 += we[w - 1][vector[c1]->getOrderID() - 1];
                        GRBVar sameW = model->addVar(0, 1, 0, GRB_BINARY, "sameWeek_" + itos(vector[c1]->getOrderID()) +
                                                                          "_" + itos(vector[c1]->getOrderID() - 1) +
                                                                          "_" + itos(w));
                        model->addGenConstrIndicator(sameW, 0, prevWc1 == 0);
                        model->addGenConstrIndicator(sameW, 1, prevWc1 == 1);
                        model->addGenConstrIndicator(sameW, 1, prevWc2 == 1);
                        resultW += sameW;

                    }
                    GRBVar sameW = model->addVar(0, 1, 0, GRB_BINARY, "sameWeekS_" + itos(vector[c1]->getOrderID()) +
                                                                      "_" + itos(vector[c1]->getOrderID() - 1));
                    model->addGenConstrIndicator(sameW, 1, resultW >= 1);
                    model->addGenConstrIndicator(sameW, 0, resultW == 0);

                    //day
                    GRBLinExpr resultD = 0;
                    GRBLinExpr prevDc1 = 0;
                    GRBLinExpr prevDc2 = 0;
                    for (int d = 1; d < instance->getNdays(); ++d) {
                        prevDc1 += day[vector[c1]->getOrderID()][d - 1];
                        prevDc2 += day[vector[c1]->getOrderID() - 1][d - 1];
                        GRBVar sameD = model->addVar(0, 1, 0, GRB_BINARY, "sameDay_" + itos(vector[c1]->getOrderID()) +
                                                                          "_" + itos(vector[c1]->getOrderID() - 1) +
                                                                          "_" + itos(d));
                        model->addGenConstrIndicator(sameD, 0,
                                                     prevDc1 == 0);
                        model->addGenConstrIndicator(sameD, 1,
                                                     prevDc1 == 1);
                        model->addGenConstrIndicator(sameD, 1,
                                                     prevDc2 == 0);
                        resultD += sameD;

                    }
                    GRBVar sameD = model->addVar(0, 1, 0, GRB_BINARY, "sameDayS_" + itos(vector[c1]->getOrderID()) +
                                                                      "_" + itos(vector[c1]->getOrderID() - 1));
                    model->addGenConstrIndicator(sameD, 1, resultD >= 1);
                    model->addGenConstrIndicator(sameD, 0, resultD == 0);
                    //time
                    GRBVar sameT = model->addVar(0, 1, 0, GRB_BINARY, "sameTime_" + itos(vector[c1]->getOrderID()) +
                                                                      "_" + itos(vector[c1]->getOrderID() - 1));
                    model->addGenConstrIndicator(sameT, 1, endTime[vector[c1]->getOrderID() - 1] <=
                                                           lectureTime[vector[c1]->getOrderID()]);
                    model->addGenConstrIndicator(sameT, 0, endTime[vector[c1]->getOrderID() - 1] >=
                                                           lectureTime[vector[c1]->getOrderID()] + 1);

                    if (penalty == -1)
                        model->addConstr(sameT + sameD + sameW >= 1);
                    else {
                        GRBVar pen = model->addVar(0, 1, 0, GRB_BINARY,
                                                   "precedence_" + itos(vector[c1 - 1]->getOrderID()) +
                                                   "_" + itos(vector[c1]->getOrderID()));
                     model->addGenConstrIndicator(pen, 1,
                                                  sameT + sameD + sameW >= 1);
                     model->addGenConstrIndicator(pen, 0,
                                                  sameT + sameD + sameW == 0);
                        result += pen * penalty;

                    }
                }


            }
        } catch (GRBException e) {
            printError(e, "precedence");
        }
        return result;
    }

    /**
     * Given classes must start at the same time slot, regardless of their
     * days of week or weeks. This means that Ci.start = Cj .start for any two
     * classes Ci and Cj from the constraint; Ci.start is the assigned start time slot
     * of a class Ci.
     * @param vector
     * @param penalty
     * @return
     */

    GRBLinExpr sameStart(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) override {
        GRBLinExpr result = 0;
        try {
            for (int c1 = 0; c1 < vector.size(); c1++) {
                for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                    if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                        if (penalty == -1) {
                            model->addConstr(
                                    lectureTime[vector[c1]->getOrderID()] == lectureTime[vector[c1]->getOrderID()]);
                        } else {
                            GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY,
                                                        "smaStart_" + itos(vector[c2]->getOrderID()) +
                                                        "_" + itos(vector[c1]->getOrderID()));
                            model->addGenConstrIndicator(temp, 1, lectureTime[vector[c1]->getOrderID()] >=
                                                                  lectureTime[vector[c1]->getOrderID()] + 1);
                            model->addGenConstrIndicator(temp, 0, lectureTime[vector[c1]->getOrderID()] <=
                                                                  lectureTime[vector[c1]->getOrderID()]);
                            GRBVar temp1 = model->addVar(0, 1, 0, GRB_BINARY,
                                                         "smaStart_" + itos(vector[c1]->getOrderID()) +
                                                         "_" + itos(vector[c2]->getOrderID()));
                            model->addGenConstrIndicator(temp1, 1, lectureTime[vector[c1]->getOrderID()] <=
                                                                   lectureTime[vector[c1]->getOrderID()] - 1);
                            model->addGenConstrIndicator(temp1, 0, lectureTime[vector[c1]->getOrderID()] >=
                                                                   lectureTime[vector[c1]->getOrderID()]);
                            result += (temp1 + temp) * penalty;
                        }

                    }
                }
            }
        } catch (GRBException e) {
            printError(e, "SmaStart");
        }
        return result;

    }

    /**
     * Given classes must be taught on different days of the week,
     * regardless of their start time slots and weeks. This means that
     * (Ci.days and Cj .days) = 0
     * for any two classes Ci and Cj from the constraint; doing binary ,,and” between
     * the bit strings representing the assigned days.
     * @param vector
     * @param penalty
     * @param b
     * @return
     */

    virtual GRBLinExpr
    differentDay(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) override {
        GRBLinExpr result = 0;
        try {
            for (int c1 = 0; c1 < vector.size(); c1++) {
                for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                    if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                        GRBLinExpr c1L = 0, c2L = 0;
                        if (!b) {
                            for (int i = 0; i < instance->getNdays(); ++i) {
                                GRBVar dayT = model->addVar(0, 1, 0, GRB_BINARY,
                                                            "dayT_" + itos(vector[c1]->getOrderID()) +
                                                            "_" + itos(vector[c2]->getOrderID()) + "_" + itos(i));
                                GRBVar dayComp[2];
                                dayComp[0] = day[vector[c1]->getOrderID()][i];
                                dayComp[1] = day[vector[c2]->getOrderID()][i];
                                model->addGenConstrOr(dayT, dayComp, 2);
                                GRBVar tc1 = model->addVar(0, 1, 0, GRB_BINARY,
                                                           "tc1S_" + itos(vector[c1]->getOrderID()) +
                                                           "_" + itos(vector[c2]->getOrderID()) + "_" +
                                                           itos(i));
                                model->addGenConstrIndicator(tc1, 0, dayT - day[vector[c1]->getOrderID()][i] == 0);
                                model->addGenConstrIndicator(tc1, 1, dayT + day[vector[c1]->getOrderID()][i] == 1);
                                GRBVar tc2 = model->addVar(0, 1, 0, GRB_BINARY,
                                                           "tc1S_" + itos(vector[c1]->getOrderID()) +
                                                           "_" + itos(vector[c2]->getOrderID()) + "_" +
                                                           itos(i));
                                model->addGenConstrIndicator(tc2, 0, dayT - day[vector[c2]->getOrderID()][i] == 0);
                                model->addGenConstrIndicator(tc2, 1, dayT + day[vector[c2]->getOrderID()][i] == 1);
                                c1L += tc1;
                                c2L += tc2;
                            }
                            GRBVar tc1 = model->addVar(0, 1, 0, GRB_BINARY, "tc1S_" + itos(vector[c1]->getOrderID()) +
                                                                            "_" + itos(vector[c2]->getOrderID()));
                            model->addGenConstrIndicator(tc1, 1, c1L == 0);
                            model->addGenConstrIndicator(tc1, 1, c1L >= 1);
                            GRBVar tc2 = model->addVar(0, 1, 0, GRB_BINARY, "tc2S_" + itos(vector[c1]->getOrderID()) +
                                                                            "_" + itos(vector[c2]->getOrderID()));
                            model->addGenConstrIndicator(tc2, 1, c2L == 0);
                            model->addGenConstrIndicator(tc2, 1, c2L >= 1);

                            if (penalty == -1) {
                                model->addConstr(tc1 + tc2 >= 1);
                            } else {
                                GRBVar t = model->addVar(0, 1, 0, GRB_BINARY, "dd_" + itos(vector[c1]->getOrderID()) +
                                                                              "_" + itos(vector[c2]->getOrderID()));
                                model->addGenConstrIndicator(t, 1, tc1 + tc2 == 0);
                                model->addGenConstrIndicator(t, 0, tc1 + tc2 >= 1);
                                result += t * penalty;

                            }
                        } else {

                            for (int d = 0; d < instance->getNdays(); ++d) {
                                if (penalty == -1) {

                                    model->addConstr(
                                            day[vector[c1]->getOrderID()][d] + day[vector[c2]->getOrderID()][d] <= 1);

                                } else {
                                    GRBVar tempDay = model->addVar(0, 1, 0, GRB_BINARY, "tempDay_"
                                                                                        +
                                                                                        itos(vector[c1]->getOrderID()) +
                                                                                        "_" +
                                                                                        itos(vector[c2]->getOrderID())
                                                                                        + "_" + itos(d));
                                    model->addGenConstrIndicator(tempDay, (b ? 1 : 0),
                                                                 day[vector[c1]->getOrderID()][d] +
                                                                 day[vector[c2]->getOrderID()][d] <= 1);
                                    model->addGenConstrIndicator(tempDay, (b ? 0 : 1),
                                                                 day[vector[c1]->getOrderID()][d] +
                                                                 day[vector[c2]->getOrderID()][d] == 2);
                                    result += tempDay;

                                }


                            }
                        }
                    }
                }
            }
            return result;
        } catch (GRBException e) {
            printError(e, "DifferentDays");
        }
        return result;
    }

    /**
     * Given classes overlap in time. Two classes overlap in time when they
     * overlap in time of day, days of the week, as well as weeks. This means that
     * (Cj .start < Ci.end) ∧ (Ci.start < Cj .end) ∧ ((Ci.days and Cj .days) 6= 0) ∧
     * ((Ci.weeks and Cj .weeks) 6= 0)
     * for any two classes Ci and Cj from the constraint, doing binary ,,and” between
     * days and weeks of Ci and Cj .
     * @param vector
     * @param penalty
     * @param b
     * @return
     */

    virtual GRBLinExpr
    overlap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) override {
        GRBLinExpr result = 0;
        try {
            for (int c1 = 0; c1 < vector.size(); c1++) {
                for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                    if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                        GRBVar sameweek[instance->getNweek()];
                        for (int i = 0; i < instance->getNweek(); ++i) {
                            sameweek[i] = model->addVar(0, 1, 0, GRB_BINARY, "sameWeekOver_" + itos(i) + "_" +
                                                                             itos(vector[c1]->getOrderID()) + "_" +
                                                                             itos(vector[c2]->getOrderID()));
                            model->addGenConstrIndicator(sameweek[i], 1,
                                                         we[i][vector[c1]->getOrderID()] +
                                                         we[i][vector[c2]->getOrderID()] == 2);
                            model->addGenConstrIndicator(sameweek[i], 0,
                                                         we[i][vector[c1]->getOrderID()] +
                                                         we[i][vector[c2]->getOrderID()] <= 1);
                        }
                        GRBVar weekTime = model->addVar(0, 1, 0, GRB_BINARY,
                                                        "sameWeekSum_" + itos(vector[c1]->getOrderID())
                                                        + "_" +
                                                        itos(vector[c2]->getOrderID()));
                        model->addGenConstrAnd(weekTime, sameweek, instance->getNweek());

                        if (b) {
                            /*Same time
                            GRBVar sameTime = model->addVar(0, 1, 0, GRB_BINARY,
                                                            "sameTime_" + itos(vector[c1]->getOrderID()) + "_" +
                                                            itos(vector[c2]->getOrderID()));
                            //(Ci.end  ≤ Cj .start)
                            std::string name =
                                    "T_" + itos(vector[c1]->getOrderID()) + "_" + itos(vector[c2]->getOrderID());
                            GRBVar temp_l1_l2 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l1_l2, 1,
                                                         endTime[vector[c1]->getOrderID()] <=
                                                         lectureTime[vector[c2]->getOrderID()]);
                            model->addGenConstrIndicator(temp_l1_l2, 0,
                                                         endTime[vector[c2]->getOrderID()] >=
                                                         lectureTime[vector[c1]->getOrderID()] + 1);
                            //(Cj .end  ≤ Ci.start)
                            name = "T_" + itos(vector[c2]->getOrderID()) + "_" + itos(vector[c1]->getOrderID());
                            GRBVar temp_l2_l1 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l2_l1, 1,
                                                         endTime[vector[c2]->getOrderID()] <=
                                                         lectureTime[vector[c1]->getOrderID()]);
                            model->addGenConstrIndicator(temp_l2_l1, 0,
                                                         endTime[vector[c2]->getOrderID()] >=
                                                         lectureTime[vector[c1]->getOrderID()] + 1);

                            model->addGenConstrIndicator(sameTime, 1,
                                                         temp_l1_l2 + temp_l2_l1 >= 1);
                            model->addGenConstrIndicator(sameTime, 0,
                                                         temp_l1_l2 + temp_l2_l1 == 0);*/

                            if (penalty == -1)
                                model->addConstr(
                                        samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] + /*sameTime +*/
                                        weekTime <= 2);
                            else {
                                GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY,
                                                            "overpen_" + itos(vector[c2]->getOrderID()) + "_" +
                                                            itos(vector[c1]->getOrderID()));
                                model->addGenConstrIndicator(temp, 0, weekTime +
                                                                      samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] //+
                                                                      /*sameTime*/ <= 1);
                                model->addGenConstrIndicator(temp, 1, weekTime +
                                                                      samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] //+
                                                                      /*sameTime*/ == 2);
                                result += penalty * temp;
                            }

                        } else {
                            /*//Same time
                            GRBVar sameTime = model->addVar(0, 1, 0, GRB_BINARY,
                                                            "sameTime_" + itos(vector[c1]->getOrderID()) + "_" +
                                                            itos(vector[c2]->getOrderID()));
                            //(Ci.end  ≤ Cj .start)
                            std::string name =
                                    "T_" + itos(vector[c1]->getOrderID()) + "_" + itos(vector[c2]->getOrderID());
                            GRBVar temp_l1_l2 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l1_l2, 1,
                                                         endTime[vector[c1]->getOrderID()] <=
                                                         lectureTime[vector[c1]->getOrderID()]);
                            model->addGenConstrIndicator(temp_l1_l2, 0,
                                                         endTime[vector[c1]->getOrderID()] >=
                                                         lectureTime[vector[c1]->getOrderID()] + 1);
                            //(Cj .end  ≤ Ci.start)
                            name = "T_" + itos(vector[c2]->getOrderID()) + "_" + itos(vector[c1]->getOrderID());
                            GRBVar temp_l2_l1 = model->addVar(0, 1, 0, GRB_BINARY, name);
                            model->addGenConstrIndicator(temp_l2_l1, 1,
                                                         endTime[vector[c2]->getOrderID()] <=
                                                         lectureTime[vector[c1]->getOrderID()]);
                            model->addGenConstrIndicator(temp_l2_l1, 0,
                                                         endTime[vector[c2]->getOrderID()] >=
                                                         lectureTime[vector[c1]->getOrderID()] + 1);

                            model->addGenConstrIndicator(sameTime, 1,
                                                         temp_l1_l2 + temp_l2_l1 >= 1);
                            model->addGenConstrIndicator(sameTime, 0,
                                                         temp_l1_l2 + temp_l2_l1 == 0);*/
                            if (penalty == -1) {
                                //Same time
                                //model->addConstr(sameTime == 1);
                                //Same day
                                model->addConstr(samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 1);
                                //Same week!
                                model->addConstr(weekTime == 1);
                            } else {
                                GRBVar temp2 = model->addVar(0, 1, 0, GRB_BINARY,
                                                             "overpen_" + itos(vector[c2]->getOrderID()) + "_" +
                                                             itos(vector[c1]->getOrderID()));
                                model->addGenConstrIndicator(temp2, 1, weekTime +
                                                                       samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] //+
                                                                       /*sameTime <= 2*/<= 1);
                                model->addGenConstrIndicator(temp2, 0, weekTime +
                                                                       samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] //+
                                                                       /*sameTime == 3*/== 2);
                                result += penalty * temp2;
                            }
                        }
                    }

                }
            }
            return result;
        } catch (GRBException e) {
            printError(e, "overlap");
        }
        return result;


    }

    /**
     * Given classes should be placed in the same room. This means that
     *(Ci.room = Cj .room) for any two classes Ci and Cj from the constraint;
     *Ci.room is the assigned room of Ci
     * @param vector
     * @param penalty
     * @param b
     * @return
     */

    GRBLinExpr
    differentRoom(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) override {
        return roomLecture->differentRoom(vector, penalty, b);
    }


    /**
     * Given classes cannot spread over more than D days of the week,regardless whether they are in the same week of
     * semester or not. This means that the total number of days of the week that have at least one class of this
     * distribution constraint C1, . . . , Cn is not greater than D,
     * countNonzeroBits(C1.days or C2.days or · · · Cn.days) ≤ D where countNonzeroBits(x) returns the number of
     * non-zero bits in the bit string x. When the constraint is soft, the penalty is multiplied by the number of
     * days that exceed the given constant D.
     * @param vector
     * @param penalty
     * @param limit
     * @return GRBLinExpr
     */
    virtual GRBLinExpr
    maxDays(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) override {
        GRBLinExpr days = 0;
        for (int d = 0; d < instance->getNdays(); ++d) {
            GRBLinExpr sumday = 0;
            for (int c1 = 0; c1 < vector.size(); ++c1) {
                if (vector[c1]->isActive(currentW)) {
                    sumday += day[vector[c1]->getOrderID()][d];
                }
            }
            GRBVar resDay = model->addVar(0, 1, 0, GRB_BINARY);
            model->addGenConstrIndicator(resDay, 1, sumday >= 1);
            model->addGenConstrIndicator(resDay, 0, sumday == 0);
            days += resDay;
        }
        if (penalty == -1) {
            model->addConstr(days <= limit);
        } else {
            GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
            model->addGenConstrIndicator(temp, 1, days >= limit + 1);
            model->addGenConstrIndicator(temp, 0, days <= limit);
            GRBVar temp1 = model->addVar(0, instance->getSlotsperday(), 0, GRB_BINARY);
            model->addGenConstrIndicator(temp, 1, temp1 == days - limit);
            model->addGenConstrIndicator(temp, 0, temp1 == 0);
            days = temp1 * penalty;
        }
        return days;
    }

    virtual GRBLinExpr roomPen() override {
        return roomLecture->roomPen();
    }


    /** MinGap(G)
     * Any two classes that are taught on the same day (they are placed on overlapping days and weeks) must be
     * at least G slots apart. This means that there must be at least G slots between the end of the earlier class and
     * the start of the later class. That is ((Ci.days and Cj.days) = 0) ∨ ((Ci.weeks and Cj.weeks) = 0) ∨
     * (Ci.end + G ≤ Cj.start) ∨ (Cj.end + G ≤ Ci.start) for any two classes Ci and Cj from the constraint.
     * @param vector
     * @param penalty
     * @param limit
     * @return
     */
    virtual GRBLinExpr
    minGap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) override {
        GRBLinExpr res = 0;
        for (int c1 = 0; c1 < vector.size(); c1++) {
            GRBLinExpr t = 0;
            for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                    GRBVar notSameDay = model->addVar(0, 1, 0, GRB_BINARY);
                    /*model->addGenConstrIndicator(notSameDay, 0,
                                                 sameday[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 1);
                    model->addGenConstrIndicator(notSameDay, 1,
                                                 sameday[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 0);*/
                    GRBVar gap1 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(gap1, 1,
                                                 endTime[vector[c1]->getOrderID()] +
                                                 limit <=
                                                 lectureTime[vector[c2]->getOrderID()]);
                    model->addGenConstrIndicator(gap1, 0,
                                                 endTime[vector[c1]->getOrderID()] +
                                                 limit - 1 >=
                                                 lectureTime[vector[c2]->getOrderID()]);
                    GRBVar gap2 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(gap2, 1,
                                                 endTime[vector[c2]->getOrderID()] +
                                                 limit <=
                                                 lectureTime[vector[c1]->getOrderID()]);
                    model->addGenConstrIndicator(gap2, 0,
                                                 endTime[vector[c2]->getOrderID()] +
                                                 limit - 1 >=
                                                 lectureTime[vector[c1]->getOrderID()]);
                    t += gap1 + gap2 + notSameDay;
                    if (penalty == -1) {
                        model->addConstr(t >= 1);
                    } else {
                        GRBVar pen = model->addVar(0, 1, 0, GRB_BINARY);
                        model->addGenConstrIndicator(pen, 1, t >= 1);
                        model->addGenConstrIndicator(pen, 0, t == 0);
                        res += pen * penalty;
                    }

                }
            }
        }
        return res;
    }

    /** WorkDay(S)
     * There should not be more than S time slots between the start of the first class and the end of the last class on
     * any given day. This means that classes that are placed on the overlapping days and weeks that have more than S
     * time slots between the start of the earlier class and the end of the later class are violating the constraint.
     * That is ((Ci.days and Cj.days) = 0) ∨ ((Ci.weeks and Cj.weeks) = 0) ∨
     * (max(Ci.end,Cj.end)−min(Ci.start,Cj.start) ≤ S) for any two classes Ci and Cj from the constraint.
     */
    virtual GRBLinExpr
    workDay(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) override {
        GRBLinExpr res = 0;
        for (int c1 = 0; c1 < vector.size(); c1++) {
            GRBLinExpr t = 0;
            for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                if (vector[c1]->isActive(currentW) && vector[c2]->isActive(currentW)) {
                    GRBVar notSameDay = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(notSameDay, 0,
                                                 samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 1);
                    model->addGenConstrIndicator(notSameDay, 1,
                                                 samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] == 0);
                    GRBVar gap1 = model->addVar(0, instance->getSlotsperday(), 0, GRB_INTEGER);
                    GRBVar minV[2];
                    minV[0] = lectureTime[vector[c1]->getOrderID()];
                    minV[1] = lectureTime[vector[c2]->getOrderID()];
                    model->addGenConstrMin(gap1, minV, 2);
                    GRBVar gap2 = model->addVar(0, instance->getSlotsperday(), 0, GRB_INTEGER);
                    GRBVar maxV[2];
                    maxV[0] = lectureTime[vector[c1]->getOrderID()];
                    maxV[1] = lectureTime[vector[c2]->getOrderID()];
                    model->addGenConstrMax(gap2, maxV, 2);
                    GRBVar gapSum = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(gapSum, 1, (gap2 - gap1) <= limit);
                    model->addGenConstrIndicator(gapSum, 0, (gap2 - gap1) >= (limit + 1));
                    t += gapSum + notSameDay;
                    if (penalty == -1) {
                        model->addConstr(t >= 1);
                    } else {
                        GRBVar pen = model->addVar(0, 1, 0, GRB_BINARY);
                        model->addGenConstrIndicator(pen, 1, t >= 1);
                        model->addGenConstrIndicator(pen, 0, t == 0);
                        res += pen * penalty;
                    }

                }
            }
        }
        return res;
    }

    /**
     * Given classes must be spread over the days of the week (and weeks) in a way that there is no more than a given
     * number of S time slots on every day. This means that for each week w ∈ {0,1,...,nrWeeks − 1} of the semester
     * and each day of the week d ∈ {0,1,...,nrDays − 1}, the total number of slots assigned to classes C that overlap
     * with the selected day d and week w is not more than S, DayLoad(d,w) ≤ S E_i {Ci.length | (Ci.days and 2d) ̸= 0
     * ∧ (Ci.weeks and 2w) ̸= 0)} where 2d is a bit string with the only non-zero bit on position d and 2w is a bit
     * string with the only non-zero bit on position w. When the constraint is soft (it is not required and there is a
     * penalty), its penalty is multiplied by the number of slots that exceed the given constant S over all days of the
     * semester and divided by the number of weeks of the semester (using integer division). Importantly the integer
     * division is computed at the very end.
     */

    virtual GRBLinExpr
    maxDayLoad(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) override {
        GRBLinExpr res = 0;
        for (int i = 0; i < instance->getNdays(); ++i) {
            GRBLinExpr t = 0;
            for (int c1 = 0; c1 < vector.size(); c1++) {
                if (vector[c1]->isActive(currentW)) {
                    GRBVar slot = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(day[vector[c1]->getOrderID()][i], 1,
                                                 slot == lectureTime[vector[c1]->getOrderID()]);
                    model->addGenConstrIndicator(day[vector[c1]->getOrderID()][i], 0, slot == 0);
                    t += slot; // lenght
                }
            }
            if (penalty == -1) {
                model->addConstr(t <= limit);
            } else {
                GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
                model->addGenConstrIndicator(temp, 1, t >= limit + 1);
                model->addGenConstrIndicator(temp, 0, t <= limit);
                GRBVar temp1 = model->addVar(0, instance->getSlotsperday(), 0, GRB_BINARY);
                model->addGenConstrIndicator(temp, 1, temp1 == t - limit);
                model->addGenConstrIndicator(temp, 0, temp1 == 0);//max(DayLoad(d,w)−S,0) the 0 part
                res = (temp1 * penalty) / instance->getNweek();//how to do in iterative mode?

            }
        }
        return res;


    }

    /**
     * This constraint limits the number of breaks during a day between a given set of classes (not more than R breaks
     * during a day). For each day of week and week, there is a break between classes if there is more than S empty
     * time slots in between. Two consecutive classes are considered to be in the same block if the gap between them is
     * not more than S time slots. This means that for each week w ∈ {0,1,...,nrWeeks − 1} of the semester and each day
     * of the week d ∈ {0,1,...,nrDays−1}, the number of blocks is not greater than R + 1,|
     * MergeBlocks( { (C.start, C.end)| (C.days and 2d) ̸= 0 ∧ (C.weeks and 2w) ̸= 0})|≤R+1 where 2d is a bit string
     * with the only non-zero bit on position d and 2w is a bit string with the only non-zero bit on position w.
     * The MergeBlocks function recursively merges to the block B any two blocks Ba and Bb that are identified by their
     * start and end slots that overlap or are not more than S slots apart, until there are no more blocks that could
     * be merged. (Ba.end + S > Bb.start) ∧ (Bb.end + S > Ba.start) ⇒ (B.start = min(Ba.start, Bb.start)) ∧ (B.end =
     * max(Ba.end, Bb.end)) When the constraint is soft, the penalty is multiplied by the total number of additional
     * breaks computed over each day of the week and week of the semester and divided by the number of weeks of the
     * semester at the end (using integer division, just like for the MaxDayLoad constraint).
     * @param vector
     * @param penalty
     * @param R breaks during a day
     * @param S empty time slots in between
     * @return
     */
    virtual GRBLinExpr
    maxBreaks(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit,
              int limit1) override {
        GRBLinExpr res = 0;
        for (int i = 0; i < instance->getNdays(); ++i) {
            GRBVar minmax[vector.size()];
            for (int c1 = 0; c1 < vector.size(); c1++) {
                if (vector[c1]->isActive(currentW)) {
                    minmax[c1] = lectureTime[vector[c1]->getOrderID()];
//                    model->addGenConstrIndicator(day[vector[c1]->getOrderID()][i], 0, slot == 0);
//                    model->addGenConstrMin(min,,2);
//                    t += slot; // lenght
                } else {

                }
            }
            /*if (penalty == -1) {
                //model->addConstr(t <= limit);
            } else {
                /*GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
                model->addGenConstrIndicator(temp, 1, t >= limit + 1);
                model->addGenConstrIndicator(temp, 0, t <= limit);
                GRBVar temp1 = model->addVar(0, instance->getSlotsperday(), 0, GRB_BINARY);
                model->addGenConstrIndicator(temp, 1, temp1 == t - limit);
                model->addGenConstrIndicator(temp, 0, temp1 == 0);//max(DayLoad(d,w)−S,0) the 0 part
                //res = (temp1 * penalty)/instance->getNweek();//how to do in iterative mode?

            }*/
        }
        return res;

    }

    /** MaxBlock(M,S)
     * This constraint limits the length of a block of consecutive classes during a day (not more than M slots in a
     * block). For each day of week and week, two consecutive classes are considered to be in the same block if the gap
     * between them is not more than S time slots. For each block, the number of time slots from the start of the first
     * class in a block till the end of the last class in a block must not be more than M time slots. This means that
     * for each week w ∈ {0,1,...,nrWeeks − 1} of the semester and each day of the week d ∈ {0, 1, . . . , nrDays − 1},
     * the maximal length of a block does not exceed M slots max( { B.end − B.start | B ∈
     * MergeBlocks({(C.start, C.end) | (C.days and 2d) ̸= 0 ∧ (C.weeks and 2w) ̸= 0})})≤M
     * When the constraint is soft, the penalty is multiplied by the total number of blocks that are over the M time
     * slots, computed over each day of the week and week of the semester and divided by the number of weeks of the
     * semester at the end (using integer division, just like for the MaxDayLoad constraint).
     * @param vector
     * @param penalty
     * @param limit
     * @param limit1
     * @return
     */
    virtual GRBLinExpr
    maxBlock(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit, int limit1) override {
        return 0;
    }

    virtual void roomUnavailable() override {
        roomLecture->roomUnavailable(we, day, lectureTime);

    }


    /**
     * Weeks
     */
    virtual void week() override {
        we = new GRBVar *[instance->getNweek()];
        for (int i = 0; i < instance->getNweek(); ++i) {
            we[i] = new GRBVar[instance->getClasses().size()];
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                we[i][j] = model->addVar(0, 1, 0, GRB_BINARY, "week_" + itos(i) + "_" + itos(j));
            }
        }
        isWeek = new GRBVar *[instance->getClassesWeek(currentW).size()];
        for (int cla = 0; cla < instance->getClassesWeek(currentW).size(); ++cla) {
            isWeek[cla] = new GRBVar[instance->getClassesWeek(currentW)[cla]->getLectures().size()];
            for (int time = 0; time < instance->getClasses()[cla]->getLectures().size(); ++time) {
                GRBVar weekAND[instance->getNweek()];
                for (int w = 0; w < instance->getNweek(); ++w) {
                    weekAND[w] = model->addVar(0, 1, 0, GRB_BINARY, "WeekAND_" + itos(instance->getClassesWeek(
                            currentW)[cla]->getId()) + "_" + itos(time) + "_" +
                                                                    itos(w));

                    model->addGenConstrIndicator(weekAND[w], 1,
                                                 we[w][instance->getClasses()[cla]->getOrderID()] ==
                                                 ('0' ==
                                                  instance->getClasses()[cla]->getLectures()[time]->getWeeks()[w] ? 0
                                                                                                                  : 1));

                }
                isWeek[cla][time] = model->addVar(0, 1, 0, GRB_BINARY, "WeekOR_" + itos(instance->getClassesWeek(
                        currentW)[cla]->getOrderID()) + "_" + itos(time));
                model->addGenConstrAnd(isWeek[cla][time], weekAND, instance->getNweek());
            }
            GRBVar t = model->addVar(0, 1, 0, GRB_BINARY, "weekF_" + itos(instance->getClassesWeek(
                    currentW)[cla]->getOrderID()));
            model->addGenConstrOr(t, isWeek[cla], instance->getClassesWeek(currentW)[cla]->getLectures().size());
            model->addConstr(t == 1);


        }

    }


    virtual GRBLinExpr timeOptions() override {
        GRBLinExpr resl = 0;
        for (int cla = 0; cla < instance->getClassesWeek(currentW).size(); ++cla) {
            GRBLinExpr temp = 0;
            for (int time = 0; time < instance->getClasses()[cla]->getLectures().size(); ++time) {
                GRBVar t = model->addVar(0, 1, 0, GRB_BINARY, "timeOpts_" +
                                                              itos(instance->getClassesWeek(
                                                                      currentW)[cla]->getOrderID()) + "_" + itos(time));
                model->addGenConstrIndicator(t, 1, isWeek[cla][time] + isStart[cla][time] + isDay[cla][time] == 3);
                model->addGenConstrIndicator(t, 1, endTime[cla] == lectureTime[cla] +
                                                                   instance->getClasses()[cla]->getLectures()[time]->getLenght());
                model->addGenConstrIndicator(t, 0, isWeek[cla][time] + isStart[cla][time] + isDay[cla][time] <= 2);
                temp += t;
                resl += t * instance->getClasses()[cla]->getLectures()[time]->getPenalty();
            }
            model->addConstr(temp == 1);
        }
        return resl;
    }


    /**
     * Given classes must be taught in the same weeks, regardless of their time slots or days of the week. In case of
     * classes of different weeks, a class with fewer weeks must meet on a subset of the weeks used by the class with
     * more weeks. This means that (Ci.weeks or Cj.weeks) = Ci.weeks) ∨ (Ci.weeks or Cj.weeks) = Cj.weeks) for any two
     * classes Ci and Cj from the constraint; doing binary ”or” between the bit strings representing the assigned weeks.
     * @param vector
     * @param penalty
     * @param b
     * @return
     */
    virtual GRBLinExpr
    sameWeek(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) override {
        GRBLinExpr res = 0;
        for (int c1 = 0; c1 < vector.size(); ++c1) {
            for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                GRBLinExpr wC1 = 0;
                GRBLinExpr wC2 = 0;
                for (int i = 0; i < instance->getNweek(); ++i) {
                    if (b) {
                        //Binary OR
                        GRBVar weekT = model->addVar(0, 1, 0, GRB_BINARY);
                        GRBVar weekTI[2];
                        weekTI[0] = we[i][vector[c1]->getOrderID()];
                        weekTI[0] = we[i][vector[c2]->getOrderID()];
                        model->addGenConstrOr(weekT, weekTI, 2);
                        //OR
                        GRBVar weekTC1 = model->addVar(0, 1, 0, GRB_BINARY);
                        model->addGenConstrIndicator(weekTC1, 0, weekT == we[i][vector[c1]->getOrderID()]);
                        model->addGenConstrIndicator(weekTC1, 1, weekT + we[i][vector[c1]->getOrderID()] == 1);
                        wC1 += weekTC1;

                        GRBVar weekTC2 = model->addVar(0, 1, 0, GRB_BINARY);
                        model->addGenConstrIndicator(weekTC2, 0, weekT == we[i][vector[c2]->getOrderID()]);
                        model->addGenConstrIndicator(weekTC2, 1, weekT + we[i][vector[c2]->getOrderID()] == 1);
                        wC2 += weekTC2;

                    } else {
                        if (penalty == -1) {
                            model->addGenConstrIndicator(we[i][vector[c1]->getOrderID()], 1,
                                                         we[i][vector[c2]->getOrderID()] == 0);
                            model->addGenConstrIndicator(we[i][vector[c2]->getOrderID()], 1,
                                                         we[i][vector[c1]->getOrderID()] == 0);
                        } else {
                            GRBVar temp = model->addVar(0, 1, 0, GRB_BINARY);
                            model->addGenConstrIndicator(temp, 1, we[i][vector[c1]->getOrderID()] +
                                                                  we[i][vector[c2]->getOrderID()] == 2);
                            model->addGenConstrIndicator(temp, 0, we[i][vector[c1]->getOrderID()] +
                                                                  we[i][vector[c2]->getOrderID()] <= 1);
                            res += penalty * temp;

                        }
                    }

                }
                if (b) {
                    GRBVar weekTe = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(weekTe, 1, wC1 == 0);
                    model->addGenConstrIndicator(weekTe, 0, wC1 >= 1);
                    GRBVar weekTe1 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(weekTe1, 1, wC2 == 0);
                    model->addGenConstrIndicator(weekTe1, 0, wC2 >= 1);
                    if (penalty == -1) {
                        model->addConstr(weekTe1 + weekTe1 >= 1);
                    } else {
                        GRBVar pen = model->addVar(0, 1, 0, GRB_BINARY);
                        model->addGenConstrIndicator(pen, 1, weekTe1 + weekTe1 == 0);
                        model->addGenConstrIndicator(weekTe, 0, weekTe1 + weekTe1 >= 1);
                        res += pen * penalty;
                    }

                }
            }
        }


        return 0;
    }


    /** SameTime
     * Given classes must be taught at the same time of day, regardless of their days of week or weeks. For the classes
     * of the same length, this is the same constraint as SameStart (classes must start at the same time slot).
     * For the classes of different lengths, the shorter class can start after the longer class but must end before or
     * at the same time as the longer class. This means that
     * (Ci.start ≤ Cj.start ∧ Cj.end ≤ Ci.end) ∨ (Cj.start ≤ Ci.start ∧ Ci.end ≤ Cj.end)
     * for any two classes Ci and Cj from the constraint; Ci.end = Ci.start + Ci.length is the assigned end time slot
     * of a class Ci.
     * @param vector
     * @param penalty
     * @param b
     * @return
     */
    virtual GRBLinExpr
    sameTime(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) override {
        GRBLinExpr result = 0;
        for (int c1 = 0; c1 < vector.size(); ++c1) {
            for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                if (b) {
                    GRBVar s_c1_c2 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(s_c1_c2, 1, lectureTime[vector[c1]->getOrderID()] <=
                                                             lectureTime[vector[c2]->getOrderID()]);
                    model->addGenConstrIndicator(s_c1_c2, 0, lectureTime[vector[c1]->getOrderID()] + 1 >=
                                                             lectureTime[vector[c2]->getOrderID()]);
                    GRBVar e_c1_c2 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(e_c1_c2, 1, endTime[vector[c2]->getOrderID()] <=
                                                             endTime[vector[c1]->getOrderID()]);
                    model->addGenConstrIndicator(e_c1_c2, 0, endTime[vector[c2]->getOrderID()] + 1 >=
                                                             endTime[vector[c1]->getOrderID()]);
                    GRBVar and1fill[2];
                    and1fill[0] = s_c1_c2;
                    and1fill[1] = e_c1_c2;
                    GRBVar and1 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrAnd(and1, and1fill, 2);
                    GRBVar s_c2_c1 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(s_c2_c1, 1, lectureTime[vector[c2]->getOrderID()] <=
                                                             lectureTime[vector[c1]->getOrderID()]);
                    model->addGenConstrIndicator(s_c2_c1, 0, lectureTime[vector[c2]->getOrderID()] + 1 >=
                                                             lectureTime[vector[c1]->getOrderID()]);
                    GRBVar e_c2_c1 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(e_c2_c1, 1, endTime[vector[c1]->getOrderID()] <=
                                                             endTime[vector[c2]->getOrderID()]);
                    model->addGenConstrIndicator(e_c2_c1, 0, endTime[vector[c1]->getOrderID()] + 1 >=
                                                             endTime[vector[c2]->getOrderID()]);
                    and1fill[0] = s_c2_c1;
                    and1fill[1] = e_c2_c1;
                    GRBVar and2 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrAnd(and2, and1fill, 2);
                    GRBVar orfill[2];
                    orfill[0] = and1;
                    orfill[1] = and2;
                    GRBVar orF = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrOr(orF, and1fill, 2);
                    if (penalty == -1) {
                        model->addConstr(orF == 1);
                    } else {
                        result += penalty * orF;
                    }
                } else {
                    GRBVar c1_c2 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(c1_c2, 1, lectureTime[vector[c1]->getOrderID()] <=
                                                           endTime[vector[c2]->getOrderID()]);
                    model->addGenConstrIndicator(c1_c2, 0, lectureTime[vector[c1]->getOrderID()] + 1 >=
                                                           endTime[vector[c2]->getOrderID()]);
                    GRBVar c2_c1 = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrIndicator(c2_c1, 1, lectureTime[vector[c2]->getOrderID()] <=
                                                           endTime[vector[c1]->getOrderID()]);
                    model->addGenConstrIndicator(c2_c1, 0, lectureTime[vector[c2]->getOrderID()] + 1 >=
                                                           endTime[vector[c1]->getOrderID()]);
                    GRBVar or1fill[2];
                    or1fill[0] = c1_c2;
                    or1fill[1] = c2_c1;
                    GRBVar orF = model->addVar(0, 1, 0, GRB_BINARY);
                    model->addGenConstrOr(orF, or1fill, 2);
                    if (penalty == -1) {
                        model->addConstr(orF == 1);
                    } else {
                        result += penalty * orF;
                    }

                }
            }
        }
        return result;
    }
};

#endif //PROJECT_ILPEXECUTER_H