//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_TWOVARGUROBIEXECUTER_H
#define PROJECT_TWOVARGUROBIEXECUTER_H


#include "/Library/gurobi810/mac64/include/gurobi_c++.h"

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../solver/GurobiExecuter.h"


class TwoVarGurobiExecuter : public GurobiExecuter {

protected:
    GRBVar **roomLecture;


public:
    virtual void definedAuxVar()= 0;


    void printConfiguration() {
        std::cout << "Two variable: type bool for schedule" << std::endl;
        std::cout << "            : type bool for rooms" << std::endl;

    }

    void definedRoomLecture() {
        try {
            roomLecture = new GRBVar *[instance->getNumClasses()];
            for (int i = 0; i < instance->getNumClasses(); i++) {
                roomLecture[i] = new GRBVar[instance->getRooms().size()];
                for (int j = 0; j < instance->getRooms().size(); ++j) {
                    std::string name = "X_" + itos(i) + "_" + itos(j);
                    roomLecture[i][j] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
                }

            }
        } catch (GRBException e) {
            printError(e, "definedRoomLecture");

        }


    }


    virtual void definedLectureTime()=0;

    /**
     * The lecture can only be scheduled in one slot
     */

    virtual void oneLectureperSlot()=0;

    /**
     * Force lectures to be in slot n
     */


    virtual void oneLectureSlot()=0;

    /**
     * A lecture can only be in one room at time
     */

    void oneLectureRoom() {
        try {
            for (int j = 0; j <
                            instance->getClasses().size(); j++) {
                GRBLinExpr temp = 0;
                if (instance->getClasses()[j]->getPossibleRooms().size() > 0) {
                    for (int i = 0; i < instance->getRooms().size(); i++) {
                        if (instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1)))
                            temp += roomLecture[j][i];
                    }
                    model->addConstr(temp == 1);
                }
            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoom");
        }

    }


    /**
     * Ensure room r is used to lecture l
     */
    void roomPreference(int r, int l) {
        model->addConstr(roomLecture[l][r] == 1);
    }

    /***
     * The room can only have one lecture per slot
     */

    virtual void oneLectureRoomConflict()=0;

    /**
    * Ensure Room closed cannot be used
    */
    void roomClose() {
        for (int i = 0; i < instance->getNumRoom(); i++) {
            for (int j = 0; j < instance->getNumClasses(); ++j) {
                if (instance->isRoomBlocked(i + 1) &&
                    instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1))) {
                    model->addConstr(roomLecture[j][i] == 0);
                }
            }
        }
    }

    /**
    * Ensure Room closed in a day cannot be used
    */
    virtual void roomClosebyDay() =0;


    /**
    * Ensure times lot in a day is closed cannot be used
    */
    virtual void slotClose() =0;

    /**
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    virtual void assignmentInvalid() =0;

    /**Teacher's conflict*/
    virtual void teacher() =0;

    /** Student conflicts hard constraint based on the input model
     *
     */
    virtual void studentConflict() =0;

    /** Student conflicts hard constraint based on the original solution
     *
     */
    virtual void studentConflictSolution() =0;


    void saveEncoding() {
        model->write("/Volumes/MAC/ClionProjects/timetabler/model.lp");

    }

protected:
//Number of seated students for optimization or constraint
    virtual GRBQuadExpr numberSeatedStudents() =0;

    virtual GRBQuadExpr usage() =0;

    virtual GRBLinExpr gapStudentsTimetable()=0;


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
                if (
                        instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                    model->addConstr(it->second.getCapacity() >=
                                     ((instance->getClasses()[l]->getLimit() -
                                       (instance->getClasses()[l]->getLimit() * instance->getAlfa())) *
                                      roomLecture[l][j]));
                }


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
                temp += instance->getClasses()[j]->getLimit() * (solutionRoom[i][j]
                                                                 != roomLecture[i][j].get(GRB_DoubleAttr_X));
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
                model->addGenConstrIndicator(tempv, 0, oldRoom[i][j] == roomLecture[i][j]);
                model->addGenConstrIndicator(tempv, 1, roomLecture[i][j] - oldRoom[i][j] - 1 <= 0);
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


    virtual GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted)=0;


private:
    /**
     * Warm starting procedure with the solution found before
     * Used the class atributes: solutionTime and roomLecture
     */

    virtual void warmStart() =0;


private:


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     *
     */

    virtual void switchSolutionTime() =0;


    /**
     * Switch solution room
     * Updates the solution room structure with new data
     * @Requires delete the previous found solution
     *
     */
    void switchSolutionRoom() {
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = roomLecture[j][i].get(GRB_DoubleAttr_X);
                if (roomLecture[j][i].get(GRB_DoubleAttr_X) != 0) {
                    instance->getClasses()[j]->setSolRoom(i);
                }
            }

        }
    }


};


#endif //PROJECT_ILPEXECUTER_H