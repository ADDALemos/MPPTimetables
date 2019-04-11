//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_MIXEDMODELGUROBIEXECUTER_H
#define PROJECT_MIXEDMODELGUROBIEXECUTER_H

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../solver/TwoVarGurobiExecuter.h"
#include "../solver/roomLectureGRB.h"
#include "../solver/roomLectureBool.h"

class MixedModelGurobiExecuter : public TwoVarGurobiExecuter {
    GRBVar *lectureTime;
    GRBVar **order;
    GRBVar **gap;


public:

    virtual void save() {
        /*for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int day = lectureTime[j].get(GRB_DoubleAttr_X) / instance->getSlotsperday();
            int slot = lectureTime[j].get(GRB_DoubleAttr_X) - day * instance->getSlotsperday();
            solutionTime[day][slot][j] = 1;
        }
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
                if (instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1))) {
                    if (!roomLecture->isStatic())
                        solutionRoom[i][j] = roomLecture->getGRB()[j][i].get(GRB_DoubleAttr_X);
                }
            }
        }*/
        switchSolutionTime();
        switchSolutionRoom();


    }

    void loadPreviousWeekSolution(int ***time, int **room) {
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

    void setCurrrentW(int currrentW) {
        ILPExecuter::currentW = currrentW;
        roomLecture->setCurrrentW(currrentW);
    }

    MixedModelGurobiExecuter(bool isStatic, Instance *i) {
        setInstance(i);
        if (isStatic)
            roomLecture = new roomLectureBool(instance, currentW);
        else
            roomLecture = new roomLectureGRB(instance, currentW);
    }

    void definedAuxVar() {

    }

    void printConfiguration() {
        std::cout << "Two variable: type int for schedule" << std::endl;
        std::cout << "            : type bool for rooms" << std::endl;

    }


    void definedLectureTime() {
        lectureTime = new GRBVar[instance->getClassesWeek(currentW).size()];

        order = new GRBVar *[instance->getClassesWeek(currentW).size()];
        for (int k = 0; k < instance->getClassesWeek(currentW).size(); ++k) {
            order[k] = new GRBVar[instance->getClassesWeek(currentW).size()];
            for (int j1 = 0; j1 < instance->getClassesWeek(currentW).size(); j1++) {
                if (k == 0) {
                    std::string name = "A_" + itos(k);

                    lectureTime[j1] = model->addVar(0.0, instance->getSlotsperday(), 0.0,
                                                    GRB_INTEGER,
                                                    name);//instance->getClassesWeek(currentW)[k]->getLectures().size()
                    model->addConstr(lectureTime[j1] + instance->getClasses()[j1]->getLenght() <=
                                     (((lectureTime[j1] / instance->getSlotsperday()) + 1) *
                                      instance->getSlotsperday()) -
                                     1);
                }
                order[k][j1] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                             "order" + itos(k) + "_" + itos(j1));
                model->addGenConstrIndicator(order[k][j1], 1,
                                             (lectureTime[k]
                                              + instance->getClasses()[k]->getLenght()) <=
                                             lectureTime[j1]);
                model->addGenConstrIndicator(order[k][j1], 0,
                                             (lectureTime[k] +
                                              instance->getClasses()[k]->getLenght() - 1) >=
                                             lectureTime[j1]);
            }


        }


    }

    /**
     * The lecture can only be scheduled in one slot
     */

    void oneLectureperSlot() {
        //Deleted constraint

    }

    /**
     * Force lectures to be in slot n
     */


    void oneLectureSlot() {
        //Not actually needed

    }


    /***
     * The room can only have one lecture per slot with ands

    void oneLectureRoomConflict2() {
        try {
            GRBVar sameRoom[3];
            GRBVar sameRoom1[3];

            for (int j = 0; j < instance->getNumClasses(); j++) {
                for (int i = 0; i < instance->getClasses()[j]->getPossibleRooms().size(); i++) {
                    for (int j1 = 1; j1 < instance->getNumClasses(); j1++) {
                            if (instance->getClasses()[j1]->containsRoom(
                                    instance->getRoom(i + 1))) {

                                GRBVar tempV = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "temp" + itos(i) + "_" + itos(j) + "_" +
                                                             itos(j1));
                                GRBVar tempV1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                              "tempV" + itos(i) + "_" + itos(j) + "_" +
                                                              itos(j1));
                                sameRoom1[0] = roomLecture[j][i];
                                sameRoom1[1] = roomLecture[j1][i];
                                sameRoom1[2] = order[j][j1];
                                sameRoom[0] = roomLecture[j][i];
                                sameRoom[1] = roomLecture[j1][i];
                                sameRoom[2] = order[j1][j];
                                model->addGenConstrAnd(tempV, sameRoom, 3);
                                model->addGenConstrAnd(tempV1, sameRoom1, 3);
                                model->addConstr(tempV + tempV1 <= 1);
                            }
                        }
                    }
                }


        } catch (GRBException e) {
            printError(e, "oneLectureRoomConflic: ands");
        }

    }*/

    /***
     * The room can only have one lecture per slot with quadratic and ands
     *

    void oneLectureRoomConflict1() {
        try {
            GRBVar sameRoom[2];

            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int j = 0; j < instance->getNumClasses(); j++) {
                    if (instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1))) {
                        for (int j1 = 1; j1 < instance->getNumClasses(); j1++) {
                            if (instance->getClasses()[j1]->containsRoom(instance->getRoom(i + 1))) {

                                GRBVar tempV = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "temp" + itos(i) + "_" + itos(j) + "_" +
                                                             itos(j1));

                                sameRoom[0] = roomLecture[j][i];
                                sameRoom[1] = roomLecture[j1][i];
                                model->addGenConstrAnd(tempV, sameRoom, 2);
                                model->addQConstr(tempV * (order[j][j1] + order[j1][j]) <= tempV);
                            }
                        }
                    }
                }


            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoomConflict: quatratic + and");
        }

    }*/



    /***
     * The room can only have one lecture per slot without and quadratic expressions
     */

    void oneLectureRoomConflict() {
        roomLecture->oneLectureRoomConflict(order);

    }


    /** TODO::
    * Ensure Room closed in a day cannot be used
    */
    void roomClosebyDay() {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
                for (int i = 0; i < instance->getClassesWeek(currentW)[j]->getPossibleRooms().size(); i++) {
                        if (instance->isRoomBlockedbyDay(i + 1, d) && !roomLecture->isStatic()) {
                            GRBVar roomC = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                         "roomC_" + itos(d) + "_" + itos(i) + "_" + itos(j));
                            model->addGenConstrIndicator(roomC, 1, lectureTime[j] / instance->getSlotsperday() == d);
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
    void slotClose() {
        try {
            for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
                for (int t = 0; t < instance->getNdays() * instance->getSlotsperday(); ++t) {
                    if (instance->isTimeUnavailable(t)) {
                        GRBVar temp = model->addVar(0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(temp, 0, lectureTime[i] >= t + 1);
                        model->addGenConstrIndicator(temp, 1, lectureTime[i] <= t);
                        GRBVar temp1 = model->addVar(0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(temp1, 1, lectureTime[i] >= t);
                        model->addGenConstrIndicator(temp1, 0, lectureTime[i] <= t - 1);
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
    void assignmentInvalid() {
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
                        model->addGenConstrIndicator(t, 1, lectureTime[i] == value);
                        // model->addGenConstrIndicator(t, 0, lectureTime[i] <= value - 1);
                        temp = t;// + roomLecture[l][i];
                        model->addConstr(temp <= 1);
                        break;
                    }

                }
            }

        }
    }


    /**Teacher's conflict*/
    void teacher() {
        for (std::map<std::string, Course *>::const_iterator it = instance->getCourses().begin();
             it != instance->getCourses().end(); it++) {
            for (std::map<int, std::vector<Subpart *>>::iterator sub = it->second->getConfiguratons().begin();
                 sub != it->second->getConfiguratons().end(); ++sub) {
                for (int i = 0; i < sub->second.size(); ++i) {
                    GRBLinExpr conflict = 0;
                    for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                        for (int c1 = 0; c1 < sub->second[i]->getClasses().size(); c1++) {
                            if (c1 != c && sub->second[i]->getClasses()[c]->isActive(currentW)
                                && sub->second[i]->getClasses()[c1]->isActive(currentW)) {

                                GRBVar tempT = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "tempT" + it->first + "_" + itos(i) + "_" +
                                                             itos(c) +
                                                             "_" + itos(c1));

                                model->addGenConstrIndicator(tempT, 1, order[c][c1] + order[c1][c] >= 2);
                                model->addGenConstrIndicator(tempT, 0, order[c][c1] + order[c1][c] <= 1);


                                conflict += tempT;
                            }
                        }
                    }
                    model->addConstr(conflict <= sub->second[i]->getOverlap());
                }


            }
        }
    }


    /** Student conflicts hard constraint based on the input model
     *
     */
    void studentConflict() {
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); it++) {
            for (int c = 0; c < it->second.getClasses().size(); ++c) {
                for (int j1 = 1; j1 < it->second.getClasses().size(); ++j1) {
                    GRBVar orv[2];
                    GRBVar tempX2 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    orv[0] = order[c][j1];
                    orv[01] = order[j1][c];
                    model->addGenConstrOr(tempX2, orv, 2);
                    model->addConstr(tempX2 == 1);

                }

            }


        }
    }

    /** Student conflicts hard constraint based on the original solution
     *
     */
    void studentConflictSolution() {
        try {
            for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
                 it != instance->getStudent().end(); it++) {
                for (int c = 0; c < it->second.getClasses().size(); ++c) {
                    for (int j1 = 1; j1 < it->second.getClasses().size(); ++j1) {
                        model->addConstr(order[c][j1] + order[j1][c] <= 1);
                    }
                }
            }
        } catch (GRBException e) {
            printError(e, "studentConflictSolution");
        }
    }


private:
//Number of seated students for optimization or constraint
    GRBQuadExpr numberSeatedStudents() {
        GRBQuadExpr temp = 0;
        for (int l = 0; l < instance->getNumClasses(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                int d = 0;
                for (char &c :instance->getClasses()[l]->getDays()) {
                    if (c != '0') {
                        for (int i = 0; i < instance->getClasses()[l]->getLenght(); i++) {
                            if (it->second.getCapacity() >= instance->getClasses()[l]->getLimit() &&
                                instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                                if (roomLecture->isStatic())
                                    temp += instance->getClasses()[l]->getLimit() *
                                            lectureTime[l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += instance->getClasses()[l]->getLimit() *
                                            lectureTime[l]
                                            * roomLecture->getGRB()[l][j];
                            } else if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                                if (roomLecture->isStatic())
                                    temp += it->second.getCapacity() *
                                            lectureTime[l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += it->second.getCapacity() *
                                            lectureTime[l]
                                            * roomLecture->getGRB()[l][j];
                            }
                        }
                    }
                    d++;
                }
                j++;
            }
        }
        //std::cout<<temp<<std::endl;

        return temp;
    }

    GRBQuadExpr usage() {
        GRBQuadExpr temp = 0;
        for (int l = 0; l < instance->getNumClasses(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                int d = 0;
                for (char &c :instance->getClasses()[l]->getDays()) {
                    if (c != '0') {
                        for (int i = 0; i < instance->getClasses()[l]->getLenght(); i++) {
                            if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                                if (roomLecture->isStatic())
                                    temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                            lectureTime[l]
                                            * roomLecture->getBool()[l][j];
                                else
                                    temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                            lectureTime[l]
                                            * roomLecture->getGRB()[l][j];
                            }
                        }
                    }

                    d++;
                }
                j++;
            }
        }
        //std::cout<<temp<<std::endl;

        return temp;

    }

    void gapStudentsTimetableVar() {
        gap = new GRBVar *[instance->getNumClasses()];
        for (int j = 0; j < instance->getNumClasses(); j++) {
            gap[j] = new GRBVar[instance->getNumClasses()];
            for (int j1 = 0; j1 < instance->getNumClasses(); j1++) {
                if (j1 != j)
                    gap[j][j1] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                           "gap" + itos(j) + "_" + itos(j1));
            }
        }
        for (int l = 0; l < instance->getNumClasses(); ++l) {
            instance->getClasses()[l]->setOrderID(l);
            for (int l1 = 1; l1 < instance->getNumClasses(); ++l1) {
                if (l != l1) {
                    model->addGenConstrIndicator(gap[l][l1], 1,
                                                 (lectureTime[l] + instance->getClasses()[l]->getLenght()) ==
                                                 (lectureTime[l1]));
                    model->addGenConstrIndicator(gap[l][l1], 0, order[l][l1] + gap[l][l1] <= 1);
                    model->addGenConstrIndicator(gap[l1][l], 1,
                                                 (lectureTime[l1] + instance->getClasses()[l1]->getLenght()) ==
                                                 (lectureTime[l]));
                    model->addGenConstrIndicator(gap[l1][l], 0, order[l1][l] + gap[l1][l] <= 1);
                }

            }
        }

    }





    GRBLinExpr gapStudentsTimetable() {
        gapStudentsTimetableVar();
        std::cout << "Aux Var: Done" << std::endl;
        GRBLinExpr min = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
                GRBLinExpr all = 0;
                for (int l1 = 1; l1 < instance->getStudent(i + 1).getClasses().size(); ++l1) {
                    if (l != l1) {
                        all += gap[instance->getStudent(i + 1).getClass(l)->getOrderID()][instance->getStudent(
                                i + 1).getClass(l1)->getOrderID()]
                               + gap[instance->getStudent(i + 1).getClass(l1)->getOrderID()][instance->getStudent(
                                i + 1).getClass(l)->getOrderID()];
                    }

                }
                min += (2 - all);

            }


        }

        return min;
        //std::cout << min << std::endl;
    }




public:
    void printdistanceToSolutionLectures(bool w) {
        int temp = 0;

        for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int day = lectureTime[j].get(GRB_DoubleAttr_X) / instance->getSlotsperday();
            int slot = lectureTime[j].get(GRB_DoubleAttr_X) - day * instance->getSlotsperday();
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
    GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted) {
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

    void warmStart() {
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

    void switchSolutionTime() {
        for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
            int day = lectureTime[i].get(GRB_DoubleAttr_X) / instance->getSlotsperday();
            int slot = lectureTime[i].get(GRB_DoubleAttr_X) - day * instance->getSlotsperday();

            solutionTime[day][slot][i] = 1;
            instance->getClassesWeek(currentW)[i]->setSolutionTime(slot,
                                                                   strdup(std::to_string(day).c_str()));



        }

    }

    void force() {
        roomLecture->force(solutionRoom, lectureTime, solutionTime);
    }


};


#endif //PROJECT_ILPEXECUTER_H