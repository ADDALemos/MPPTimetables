//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_INTEGERTIMEGUROBIEXECUTER_H
#define PROJECT_INTEGERTIMEGUROBIEXECUTER_H


#include "/Library/gurobi810/mac64/include/gurobi_c++.h"

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../solver/TwoVarGurobiExecuter.h"

class IntegerTimeGurobiExecuter : public TwoVarGurobiExecuter {
    GRBVar **lectureTime;


public:


    void definedLectureTime() {
        lectureTime = new GRBVar *[instance->getNdays() * instance->getSlotsperday()];
        for (int i = 0; i < instance->getNdays() * instance->getSlotsperday(); i++) {
            lectureTime[i] = new GRBVar[instance->getClasses().size()];
            for (int k = 0; k < instance->getClasses().size(); ++k) {
                std::string name = "A_" + itos(i) + "_" + itos(k);
                lectureTime[i][k] = model->addVar(0.0, instance->getNdays() * instance->getSlotsperday(), 0.0,
                                                  GRB_INTEGER, name);

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
     * The room can only have one lecture per slot
     */

    void oneLectureRoomConflict() {
        try {
            GRBVar sameRoom[3];
            GRBVar sameRoom1[3];

            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int d = 0; d < instance->getNdays() * instance->getSlotsperday(); d++) {
                    for (int j = 0; j < instance->getClasses().size(); j++) {
                        for (int j1 = 1; j1 < instance->getClasses().size(); j1++) {
                            GRBVar tempV = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                         "temp" + itos(i) + "_" + itos(d) + "_" + itos(j) + "_" +
                                                         itos(j1));
                            GRBVar tempV1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                          "tempV" + itos(i) + "_" + itos(d) + "_" + itos(j) + "_" +
                                                          itos(j1));
                            GRBVar tempX = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                         "tempX" + itos(i) + "_" + itos(d) + "_" + itos(j) + "_" +
                                                         itos(j1));
                            GRBVar tempX1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                          "tempY" + itos(i) + "_" + itos(d) + "_" + itos(j) + "_" +
                                                          itos(j1));
                            model->addGenConstrIndicator(tempX, 1,
                                                         (lectureTime[d][j] + instance->getClasses()[j]->getLenght()) <=
                                                         lectureTime[d][j1]);
                            model->addGenConstrIndicator(tempX1, 1, (lectureTime[d][j1] +
                                                                     instance->getClasses()[j1]->getLenght()) <=
                                                                    lectureTime[d][j]);
                            sameRoom1[0] = roomLecture[i][j];
                            sameRoom1[1] = roomLecture[i][j1];
                            sameRoom1[2] = tempX1;
                            sameRoom[0] = roomLecture[i][j];
                            sameRoom[1] = roomLecture[i][j1];
                            sameRoom[2] = tempX;
                            model->addGenConstrAnd(tempV, sameRoom, 3);
                            model->addGenConstrAnd(tempV1, sameRoom1, 3);
                            model->addConstr(tempV + tempV1 <= 1);
                        }
                    }

                }
            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoomConflict");
        }

    }


    /**
    * Ensure Room closed in a day cannot be used
    */
    void roomClosebyDay() {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int i = 0; i < instance->getNumRoom(); i++) {
                for (int j = 0; j < instance->getNumClasses(); ++j) {
                    if (instance->isRoomBlockedbyDay(i, d)) {
                        for (int t = 0; t < instance->getSlotsperday(); ++t) {
                            model->addConstr(
                                    roomLecture[i][j] * lectureTime[d * (instance->getSlotsperday() + t)][j] == 0);
                        }
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
            for (int t = 0; t < instance->getNdays() * instance->getSlotsperday(); ++t) {
                if (instance->isTimeUnavailable(i * t))
                    model->addConstr(lectureTime[t][i] == 0);

            }


        }
    }

    /**
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    void assignmentInvalid() {
        for (int i = 0; i < instance->getNumClasses(); ++i) {
            if (instance->isIncorrectAssignment(i)) {
                for (int j = 0; j < instance->getNdays(); ++j) {
                    for (int k = 0; k < instance->getSlotsperday(); ++k) {
                        if (solutionTime[j][k][i] == 1) {
                            GRBLinExpr temp = 0;
                            temp = lectureTime[k * j][i];
                            for (int l = 0; l < instance->getNumRoom(); ++l) {
                                if (solutionRoom[l][i] == 1) {
                                    temp += roomLecture[l][i];
                                    model->addConstr(temp <= 1);
                                    break;
                                }

                            }
                        }

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
                    for (int t = 0; t < instance->getNdays() * instance->getSlotsperday(); ++t) {
                        GRBLinExpr conflict = 0;
                        for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                            for (int c1 = 1; c1 < sub->second[i]->getClasses().size(); c1++) {

                                GRBVar tempT = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                             "tempT" + itos(i) + "_" + itos(t) + "_" + itos(c) +
                                                             "_" + itos(c1));
                                GRBVar tempT1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                              "tempT1" + itos(i) + "_" + itos(t) + "_" + itos(c) +
                                                              "_" + itos(c1));
                                GRBVar tempTA = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                              "tempTA" + itos(i) + "_" + itos(t) + "_" + itos(c) +
                                                              "_" + itos(c1));
                                model->addGenConstrIndicator(tempT, 1, lectureTime[t][c] <= lectureTime[t][c1]);
                                model->addGenConstrIndicator(tempT1, 1, lectureTime[t][c1] <= (lectureTime[t][c] +
                                                                                               sub->second[i]->getClasses()[c]->getLenght() -
                                                                                               1));
                                andV[0] = tempT;
                                andV[1] = tempT1;
                                model->addGenConstrAnd(tempT, andV, 2);
                                conflict += tempT;
                            }
                        }
                        model->addConstr(conflict <= sub->second[i]->getOverlap());
                    }


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
            for (int d = 0; d < instance->getNdays() * instance->getSlotsperday(); ++d) {
                for (int c = 0; c < it->second.getCourse().size(); ++c) {
                    for (int j1 = 1; j1 < instance->getClasses().size(); j1++) {
                        GRBVar tempX = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                     "tempX" + itos(d) + "_" + itos(c) + "_" + itos(j1));
                        GRBVar tempX1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                      "tempY" + itos(d) + "_" + itos(c) + "_" + itos(j1));
                        model->addGenConstrIndicator(tempX, 1,
                                                     (lectureTime[d][c] + instance->getClasses()[c]->getLenght()) <=
                                                     lectureTime[d][j1]);
                        model->addGenConstrIndicator(tempX1, 1,
                                                     (lectureTime[d][j1] + instance->getClasses()[j1]->getLenght()) <=
                                                     lectureTime[d][c]);
                        model->addConstr(tempX + tempX1 <= 1);
                    }
                }
            }


        }
    }

    /** Student conflicts hard constraint based on the original solution
     *
     */
    void studentConflictSolution() {
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); it++) {
            for (int d = 0; d < instance->getSlotsperday() * instance->getNdays(); ++d) {
                for (int c = 0; c < it->second.getClasses().size(); ++c) {
                    for (int j1 = 1; j1 < instance->getClasses().size(); j1++) {
                        GRBVar tempX = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                     "tempX" + itos(d) + "_" + itos(c) + "_" + itos(j1));
                        GRBVar tempX1 = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                      "tempY" + itos(d) + "_" + itos(c) + "_" + itos(j1));
                        model->addGenConstrIndicator(tempX, 1,
                                                     (lectureTime[d][c] + instance->getClasses()[c]->getLenght()) <=
                                                     lectureTime[d][j1]);
                        model->addGenConstrIndicator(tempX1, 1,
                                                     (lectureTime[d][j1] + instance->getClasses()[j1]->getLenght()) <=
                                                     lectureTime[d][c]);
                        model->addConstr(tempX + tempX1 <= 1);
                    }

                }

            }
        }
    }


private:
//Number of seated students for optimization or constraint
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
                            if (it->second.getCapacity() >= instance->getClasses()[l]->getLimit()) {
                                temp += instance->getClasses()[l]->getLimit() *
                                        lectureTime[d * (instance->getClasses()[l]->getStart() + i)][l]
                                        * roomLecture[j][l];
                            } else {
                                temp += it->second.getCapacity() *
                                        lectureTime[d * (instance->getClasses()[l]->getStart() + i)][l]
                                        * roomLecture[j][l];
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
        for (int l = 0; l < instance->getClasses().size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                int d = 0;
                for (char &c :instance->getClasses()[l]->getDays()) {
                    if (c != '0') {
                        for (int i = 0; i < instance->getClasses()[l]->getLenght(); i++) {

                            temp += abs(it->second.getCapacity() - instance->getClasses()[l]->getLimit()) *
                                    lectureTime[d * (instance->getClasses()[l]->getStart() + i)][l]
                                    * roomLecture[j][l];
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

    GRBLinExpr gapStudentsTimetable() {
        GRBLinExpr min = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int k = 1; k < instance->getNdays() * instance->getSlotsperday(); ++k) {
                GRBLinExpr all = 0;
                GRBVar t = model->addVar(0.0, std::numeric_limits<int>::max(), 0.0, GRB_INTEGER);
                GRBVar tmin = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                for (int l = 0; l < instance->getClasses().size(); ++l) {
                    //std::cout << l << " " << instance->getStudent(i).isEnrolled(l) << std::endl;
                    if (instance->getStudent(i).isEnrolled(l))
                        all += lectureTime[k][l] + lectureTime[k - 1][l];
                }
                model->addConstr(t == all);
                model->addGenConstrIndicator(tmin, 1, t == 1);

                min += tmin;


            }

        }
        return min;
        //std::cout << min << std::endl;
    }


public:


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
        for (int i = 0; i < instance->getNdays(); i++) {
            for (int t = 0; t < instance->getSlotsperday(); t++) {
                for (int j = 0; j < instance->getClasses().size(); ++j) {
                    GRBVar tempv = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    model->addGenConstrIndicator(tempv, 0, oldTime[i][t][j] == lectureTime[i * t][j]);
                    model->addGenConstrIndicator(tempv, 1, lectureTime[i * t][j] - oldTime[i][t][j] - 1 <= 0);
                    temp += instance->getClasses()[j]->getLimit() * tempv;
                }

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
                for (int j = 0; j < instance->getClasses().size(); j++) {
                    lectureTime[k * i][j].set(GRB_DoubleAttr_Start, solutionTime[k][i][j]);
                }
            }
        }
        for (int r = 0; r < instance->getRooms().size(); ++r) {
            for (int l = 0; l < instance->getClasses().size(); ++l) {
                roomLecture[r][l].set(GRB_DoubleAttr_Start, solutionRoom[r][l]);
            }

        }

    }


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     *
     */

    void switchSolutionTime() {
        for (int i = 0; i < instance->getClasses().size(); i++) {
            for (int k = 0; k < instance->getNdays(); ++k) {
                for (int j = 0; j < instance->getSlotsperday(); ++j) {
                    solutionTime[k][j][i] = lectureTime[k * j][i].get(GRB_DoubleAttr_X);

                    if (solutionTime[k][j][i] != 0) {
                        instance->getClasses()[i]->setSolutionTime(j,
                                                                   strdup(std::to_string(k).c_str()));
                    }

                }

            }

        }

    }


};


#endif //PROJECT_ILPEXECUTER_H