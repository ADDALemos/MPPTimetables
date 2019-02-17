//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_ONEVARGUROBIEXECUTER_H
#define PROJECT_ONEVARGUROBIEXECUTER_H


#include "/Library/gurobi810/mac64/include/gurobi_c++.h"

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../solver/GurobiExecuter.h"


class OneVarGurobiExecuter : public GurobiExecuter {

protected:
    GRBVar **timetable;
    GRBVar ***order;



public:

    void definedAuxVar() {
        try {
            order = new GRBVar **[instance->getRooms().size()];
            for (int r = 0; r < instance->getRooms().size(); r++) {
                order[r] = new GRBVar *[instance->getClasses().size()];
                for (int j = 0; j < instance->getClasses().size(); j++) {
                    order[r][j] = new GRBVar[instance->getClasses().size()];
                    for (int j1 = 0; j1 < instance->getClasses().size(); j1++) {
                        order[r][j][j1] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                        "order" + itos(r) + "_" + itos(j) + "_" + itos(j1));
                    }
                }
            }

            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int j = 0; j < instance->getClasses().size(); j++) {
                    for (int j1 = 1; j1 < instance->getClasses().size(); j1++) {
                        model->addGenConstrIndicator(order[i][j][j1], 1,
                                                     (timetable[i][j] +
                                                      instance->getClasses()[j]->getLenght()) <=
                                                     timetable[i][j1]);
                        model->addGenConstrIndicator(order[i][j1][j], 1,
                                                     (timetable[i][j1] +
                                                      instance->getClasses()[j1]->getLenght()) <=
                                                     timetable[i][j]);

                    }
                }
            }
        } catch (GRBException e) {
            printError(e, "definedAuxVar");
        }

    }

    void printConfiguration() {
        std::cout << "One variable type int" << std::endl;
    }

    void definedRoomLecture() {
        try {
            timetable = new GRBVar *[instance->getRooms().size()];
            for (int i = 0; i < instance->getRooms().size(); i++) {
                timetable[i] = new GRBVar[instance->getClasses().size()];
                for (int j = 0; j < instance->getNumClasses(); ++j) {
                    std::string name = "X_" + itos(i) + "_" + itos(j);
                    timetable[i][j] = model->addVar(-1, instance->getNdays() * instance->getSlotsperday(), -1,
                                                    GRB_INTEGER, name);
                }

            }
        } catch (GRBException e) {
            printError(e, "definedTimetable");

        }


    }


    void definedLectureTime() {
        //Deleted constraint
        for (int r = 0; r < instance->getRooms().size(); ++r) {
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                model->addConstr(timetable[r][i] + instance->getClasses()[i]->getLenght() <=
                                 (((timetable[r][i] / instance->getSlotsperday()) + 1) * instance->getSlotsperday()) -
                                 1);
            }
        }

    }

    /**
     * The lecture can only be scheduled in one slot
     */

    void oneLectureperSlot() {
        //Not actually needed
    }

    /**
     * Force lectures to be in slot n
     */


    void oneLectureSlot() {
        ;
    }


    /**
     * A lecture can only be in one room at time
     */

    void oneLectureRoom() {
        try {
            for (int j = 0; j <
                            instance->getClasses().size(); j++) {
                GRBLinExpr temp = 0;
                for (int i = 0; i < instance->getRooms().size(); i++) {
                    if (instance->getClasses()[j]->getPossibleRooms().size() > 0) {
                        GRBVar ro = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(ro, 1, timetable[i][j] >= 0);
                        temp += ro;
                    }

                }
                model->addConstr(temp == 1);
            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoom");
        }

    }


    /**
     * Ensure room r is used to lecture l
     */
    void roomPreference(int r, int l) {
        model->addConstr(timetable[r][l] >= 0);
    }

    /***
     * The room can only have one lecture per slot
     */

    void oneLectureRoomConflict() {
        try {

            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int j = 0; j < instance->getClasses().size(); j++) {
                    for (int j1 = 1; j1 < instance->getClasses().size(); j1++) {
                        model->addConstr(order[i][j][j1] + order[i][j1][j] <= 1);
                    }
                }


            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoomConflict");
        }

    }

    /**
    * Ensure Room closed cannot be used
    */
    void roomClose() {
        for (int i = 0; i < instance->getNumRoom(); i++) {
            for (int j = 0; j < instance->getNumClasses(); ++j) {
                if (instance->isRoomBlocked(i + 1)) {
                    model->addConstr(timetable[i][j] == -1);
                }
            }
        }
    }

    /**
    * Ensure Room closed in a day cannot be used
    */
    virtual void roomClosebyDay() {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int i = 0; i < instance->getNumRoom(); i++) {
                for (int j = 0; j < instance->getNumClasses(); ++j) {
                    if (instance->isRoomBlockedbyDay(i + 1, d)) {
                        GRBVar roomC = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                     "roomC_" + itos(d) + "_" + itos(i) + "_" + itos(j));
                        model->addGenConstrIndicator(roomC, 1, timetable[i][j] / instance->getSlotsperday() == d);
                        model->addConstr(roomC == 0);

                    }
                }
            }

        }


    }


    /**
    * Ensure times lot in a day is closed cannot be used
    */
    void slotClose() {
        for (int i = 0; i < instance->getClasses().size(); i++) {
            for (int j = 0; j < instance->getRooms().size(); ++j) {
                for (int t = 0; t < instance->getNdays() * instance->getSlotsperday(); ++t) {
                    if (instance->isTimeUnavailable(i * t))
                        model->addConstr(timetable[j][i] == t);

                }

            }


        }
    }

    /**
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    void assignmentInvalid() {
        int value = 0;

        for (int j = 0; j < instance->getNdays(); ++j) {
            for (int i = 0; i < instance->getSlotsperday(); ++i) {
                for (int k = 0; k < instance->getNumClasses(); ++k) {
                    if (solutionTime[j][i][k] == 1) {
                        value = j + i;
                        goto label;
                    }

                }

            }

        }
        label:
        for (int i = 0; i < instance->getNumClasses(); ++i) {
            if (instance->isIncorrectAssignment(i)) {
                GRBLinExpr temp = 0;
                for (int l = 0; l < instance->getNumRoom(); ++l) {
                    if (solutionRoom[l][i] == 1) {
                        GRBVar t = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                        model->addGenConstrIndicator(t, 1, timetable[l][i] == value);
                        model->addGenConstrIndicator(t, 0, timetable[l][i] <= value - 1);
                        model->addGenConstrIndicator(t, 0, timetable[l][i] >= value + 1);
                        temp += t;
                        model->addConstr(temp <= 0);
                        break;
                    }

                }
            }

        }
    }

    /**Teacher's conflict*/
    void teacher() {
        GRBVar andV[2];
        for (std::map<std::string, Course *>::const_iterator it = instance->getCourses().begin();
             it != instance->getCourses().end(); it++) {
            for (std::map<int, std::vector<Subpart *>>::iterator sub = it->second->getConfiguratons().begin();
                 sub != it->second->getConfiguratons().end(); ++sub) {
                for (int i = 0; i < sub->second.size(); ++i) {
                    GRBLinExpr conflict = 0;
                    for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                        for (int c1 = 1; c1 < sub->second[i]->getClasses().size(); c1++) {
                            for (int r = 0; r < instance->getRooms().size(); ++r) {


                                GRBVar tempT = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "tempT" + it->first + "_" + itos(i) + "_" +
                                                             itos(c) +
                                                             "_" + itos(c1) + "_" + itos(r));

                                model->addGenConstrIndicator(tempT, 1, order[r][c][c1] + order[r][c1][c] == 0);
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
                    for (int r = 0; r < instance->getRooms().size(); ++r) {
                        GRBVar orv[2];
                        GRBVar tempX = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);

                        orv[0] = order[r][c][j1];
                        orv[01] = order[r][c][j1];
                        model->addGenConstrOr(tempX, orv, 2);
                        model->addConstr(tempX == 1);
                    }
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
                        for (int r = 0; r < instance->getRooms().size(); ++r) {

                            model->addConstr(order[r][c][j1] + order[r][j1][c] <= 1);


                        }
                    }

                }


            }
        } catch (GRBException e) {
            printError(e, "studentConflictSolution");
        }
    }


    void saveEncoding() {
        model->write("/Volumes/MAC/ClionProjects/timetabler/model.lp");

    }

protected:
    GRBLinExpr gapStudentsTimetable() {
        GRBLinExpr min = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            GRBLinExpr all = 0;
            GRBVar num = model->addVar(0.0, std::numeric_limits<int>::max(), 0.0, GRB_INTEGER);
            for (int r = 0; r < instance->getRooms().size(); ++r) {
                for (int l = 0; l < instance->getClasses().size(); ++l) {
                    GRBVar tmin = model->addVar(0.0, 2.0, 0.0, GRB_INTEGER);
                    for (int l1 = 1; l1 < instance->getClasses().size(); ++l1) {
                        if (instance->getStudent(i).isEnrolled(l) && instance->getStudent(i).isEnrolled(l1)) {
                            GRBVar before = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                            model->addGenConstrIndicator(before, 1,
                                                         (timetable[r][l] + instance->getClasses()[l]->getLenght()) ==
                                                         (timetable[r][l1]));
                            model->addGenConstrIndicator(before, 1,
                                                         (timetable[r][l1] + instance->getClasses()[l1]->getLenght()) ==
                                                         (timetable[r][l]));
                            all += before;
                        }
                    }
                    model->addConstr(tmin == (2 - all));
                    min += tmin;

                }

            }



        }

        return min;
        //std::cout << min << std::endl;
    }

    GRBQuadExpr usage() {
        GRBQuadExpr temp = 0;

        for (int l = 0; l < instance->getClasses().size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                int d = 0;
                for (char &c :instance->getClasses()[l]->getDays()) {
                    if (c != '0') {
                        for (int i = 0; i < instance->getClasses()[l]->getLenght(); i++) {
                            GRBVar var = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                            model->addGenConstrIndicator(var, 1, timetable[j][l] >= 1);
                            temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                    var;
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

    GRBQuadExpr numberSeatedStudents() {
        GRBQuadExpr temp = 0;
        for (int l = 0; l < instance->getClasses().size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                int d = 0;
                for (char &c :instance->getClasses()[l]->getDays()) {
                    if (c != '0') {
                        for (int i = 0; i < instance->getClasses()[l]->getLenght(); i++) {
                            GRBVar var = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                            model->addGenConstrIndicator(var, 1, timetable[j][l] >= 1);
                            if (it->second.getCapacity() >= instance->getClasses()[l]->getLimit()) {
                                temp += instance->getClasses()[l]->getLimit() *
                                        var;
                            } else {
                                temp += it->second.getCapacity() *
                                        var;
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


public:


    /**
     * Create a hard constraint on the number of studnets seated.
     * Can be controled by the value of slack of each instance
     */

    void slackStudent() {
        for (int l = 0; l <
                        instance->getClasses().size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                GRBVar temp = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                model->addGenConstrIndicator(temp, 1, timetable[j][l] >= 1);
                model->addConstr(it->second.getCapacity() >=
                                 ((instance->getClasses()[l]->getLimit() -
                                   (instance->getClasses()[l]->getLimit() * instance->getAlfa())) *
                                  temp));


                j++;
            }
        }
    }


    /***
     * Prints the current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved

     */
    void printdistanceToSolutionRooms() {
        int temp = 0;
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                if (solutionRoom[i][j] == 1)
                    temp += instance->getClasses()[j]->getLimit() * (-1
                                                                     != timetable[i][j].get(GRB_DoubleAttr_X));
            }
        }
        std::cout << temp << std::endl;
    }

    /***
     * The current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved
     * @param oldRoom
     * @param weighted
     * @return IloExpr
     */

    GRBQuadExpr distanceToSolutionRooms(int **oldRoom, bool weighted) {
        GRBQuadExpr temp = 0;
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                GRBVar tempv = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                if (oldRoom[i][j] == 1) {
                    model->addGenConstrIndicator(tempv, 0, timetable[i][j] >= 0);
                    model->addGenConstrIndicator(tempv, 1, timetable[i][j] <= -1);
                } else {
                    model->addGenConstrIndicator(tempv, 1, timetable[i][j] >= 0);
                    model->addGenConstrIndicator(tempv, 0, timetable[i][j] <= -1);

                }
                temp += instance->getClasses()[j]->getLimit() * tempv;
            }

        }
        return temp;
    }

    /***
    * The current distance of the solution with the old solution
    * The distante is base on the weighted Hamming distance of the lectureTime variable (time slot atributions)
    * The weighted is baed on the number of students moved
    * @param oldTime
    * @param weighted
    * @return IloExpr
    */


    GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted) {
        GRBQuadExpr temp = 0;

        for (int j = 0; j < instance->getClasses().size(); ++j) {
            for (int r = 0; r < instance->getRooms().size(); ++r) {
                GRBVar tempT = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                GRBVar var = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                model->addGenConstrIndicator(var, 1, timetable[r][j] >= 0);
                model->addGenConstrIndicator(var, 0, timetable[r][j] <= -1);

                model->addGenConstrIndicator(tempT, 1, var - instance->getClasses()[j]->getSolDay() *
                                                             instance->getSlotsperday() <=
                                                       (instance->getClasses()[j]->getSolStart() - 1));
                model->addGenConstrIndicator(tempT, 1, var - instance->getClasses()[j]->getSolDay() *
                                                             instance->getSlotsperday() >=
                                                       (instance->getClasses()[j]->getSolStart() + 1));


                temp += instance->getClasses()[j]->getLimit() * tempT;

            }

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
                for (int r = 0; r < instance->getRooms().size(); ++r) {
                    for (int l = 0; l < instance->getClasses().size(); ++l) {
                        if (solutionRoom[r][l] == 1 && solutionTime[k][i][l] == 1)
                            timetable[r][l].set(GRB_DoubleAttr_Start, k * i);
                        else
                            timetable[r][l].set(GRB_DoubleAttr_Start, -1);

                    }

                }
            }
        }


    }


private:


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     *
     */

    void switchSolutionTime() {
        for (int r = 0; r < instance->getRooms().size(); r++) {
            for (int i = 0; i < instance->getClasses().size(); i++) {
                if (timetable[r][i].get(GRB_DoubleAttr_X) != -1) {
                    int day = timetable[r][i].get(GRB_DoubleAttr_X) / instance->getSlotsperday();
                    int slot = timetable[r][i].get(GRB_DoubleAttr_X) - day * instance->getSlotsperday();

                    solutionTime[day][slot][i] = 1;
                    for (int j = slot; j < instance->getClasses()[i]->getLenght(); ++j) {
                        instance->getClasses()[i]->setSolutionTime(j,
                                                                   strdup(std::to_string(day).c_str()));
                    }
                }

            }
        }

    }


    /**
     * Switch solution room
     * Updates the solution room structure with new data
     * @Requires delete the previous found solution
     *
     */
    void switchSolutionRoom() {
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = (timetable[i][j].get(GRB_DoubleAttr_X) == -1 ? 0 : 1);
                if (timetable[i][j].get(GRB_DoubleAttr_X) != -1) {
                    instance->getClasses()[j]->setSolRoom(i);
                }
            }

        }
    }


};


#endif //PROJECT_ILPEXECUTER_H