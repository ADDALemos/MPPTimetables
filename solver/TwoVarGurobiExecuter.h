//
// Created by Alexandre Lemos on 09/01/2019.
//

#ifndef PROJECT_TWOVARGUROBIEXECUTER_H
#define PROJECT_TWOVARGUROBIEXECUTER_H


#include <exception>
#include <stdlib.h>
#include <fstream>
#include "../problem/Instance.h"
#include "../solver/GurobiExecuter.h"
#include "../solver/roomLecture.h"


class TwoVarGurobiExecuter : public GurobiExecuter {

protected:
    roomLecture *roomLecture;


public:
    virtual void definedAuxVar()= 0;

    void cuts() {
        roomLecture->cuts();
    }


    void printConfiguration() {
        std::cout << "Two variable: type bool for schedule" << std::endl;
        std::cout << "            : type bool for rooms" << std::endl;

    }

    void definedRoomLecture() {
        roomLecture->definedRoomLecture();
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
        roomLecture->oneLectureRoom();
    }


    /**
     * Ensure room r is used to lecture l
     */
    void roomPreference(int r, int l) {
        roomLecture->roomPreference(r, l);
    }

    /***
     * The room can only have one lecture per slot
     */

    virtual void oneLectureRoomConflict()=0;

    /**
    * Ensure Room closed cannot be used
    */
    void roomClose() {
        roomLecture->roomClose();
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
        roomLecture->slackStudent();
    }


    /***
     * Prints the current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved

     */
    std::ofstream File;
    void printdistanceToSolutionRooms(bool w) {
        int temp = 0;
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
                if (instance->getClasses()[j]->containsRoom(instance->getRoom(i + 1))) {
                    if (!roomLecture->isStatic())
                        temp += (w ? instance->getClasses()[j]->getLimit() : 1) * (solutionRoom[i][j]
                                                                                   != roomLecture->getGRB()[j][i].get(
                                GRB_DoubleAttr_X));
                }
            }
        }
        std::cout << temp << std::endl;
    }

    void printdistanceToSolutionLectures(bool w) {

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
        GRBLinExpr temp = 0;
        for (int j = 0; j < instance->getNumClasses(); ++j) {
            for (int i = 0; i < instance->getClasses()[j]->getPossibleRooms().size(); i++) {
                if (!roomLecture->isStatic()) {
                    GRBVar tempv = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    model->addGenConstrIndicator(tempv, 0, oldRoom[i][j] == roomLecture->getGRB()[j][i]);
                    if (oldRoom[i][j])
                        model->addGenConstrIndicator(tempv, 1, roomLecture->getGRB()[j][i] - oldRoom[i][j] == -1);
                    if (!oldRoom[i][j])
                        model->addGenConstrIndicator(tempv, 1, roomLecture->getGRB()[j][i] - oldRoom[i][j] == 1);
                    temp += (weighted ? instance->getClasses()[j]->getLimit() : 1) * tempv;
                }

            }

        }
        // model->addConstr(temp >= 0);
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


public:


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
        for (int j = 0; j < instance->getClassesWeek(currentW).size(); ++j) {
            if (!roomLecture->isStatic()) {
                for (int i = 0, r = 0; i < instance->getRooms().size(); ++i) {
                    if (instance->getClassesWeek(currentW)[j]->containsRoom(instance->getRoom(i + 1))) {
                        //solutionRoom[i][j] = roomLecture->getGRB()
                        //[instance->getClassesWeek(currentW)[j]->getOrderID()][r].get(GRB_DoubleAttr_X);
                        if (roomLecture->getGRB()
                            [instance->getClassesWeek(currentW)[j]->getOrderID()][r].get(GRB_DoubleAttr_X) != 0) {
                            instance->getClasses()[j]->setSolRoom(i + 1);
                        }

                        r++;
                    }
                    }
                }
            }


    }



};


#endif //PROJECT_ILPEXECUTER_H