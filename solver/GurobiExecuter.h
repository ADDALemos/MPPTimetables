//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_GUROBIEXECUTER_H
#define PROJECT_GUROBIEXECUTER_H


#include <gurobi_c++.h>
#include <math.h>       /* floor */
#include "GurobiAux.h"
#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "StudentSectioning.h"
#include "../utils/TimeUtil.h"


class GurobiExecuter : public ILPExecuter {

public:
    StudentSectioning *studentSectioning; //TODO: init in other encodings
    //The variables must be defined beforehand!
    virtual void loadPreviousWeekSolution(int ***time, int **room) override =0;

    virtual void definedAuxVar() override = 0;


    virtual void definedRoomLecture() override =0;


    virtual void definedLectureTime() override =0;

    /**
     * The lecture can only be scheduled in one slot
     */

    virtual void oneLectureperSlot() override =0;

    /**
     * Force lectures to be in slot n
     */


    virtual void oneLectureSlot() override =0;

    /**
     * A lecture can only be in one room at time
     */

    virtual void oneLectureRoom() override =0;


    /**
     * Ensure room r is used to lecture l
     */
    virtual void roomPreference(int r, int l) override =0;

    /***
     * The room can only have one lecture per slot
     */

    virtual void oneLectureRoomConflict() override =0;

    /**
    * Ensure Room closed cannot be used
    */
    virtual void roomClose() override =0;

    /**
    * Ensure Room closed in a day cannot be used
    */
    virtual void roomClosebyDay() override =0;


    /**
    * Ensure times lot in a day is closed cannot be used
    */
    virtual void slotClose() override =0;

    /**
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    virtual void assignmentInvalid() override =0;

    /**Teacher's conflict*/
    virtual void teacher() override =0;


    /** Student conflicts hard constraint based on the input model
     *
     */
    virtual void studentConflict() override =0;

    /** Student conflicts hard constraint based on the original solution
     *
     */
    virtual void studentConflictSolution() override =0;


    void saveEncoding(std::string name) {
        saveEncoding(name, *model);

    }

    void saveEncoding(std::string name, GRBModel m) {
        m.write("/Volumes/MAC/ClionProjects/timetabler/" + name + ".lp");
    }

protected:
//Number of seated students for optimization or constraint
    virtual GRBQuadExpr numberSeatedStudents()  =0;

    virtual GRBQuadExpr usage() =0;

    virtual GRBLinExpr gapStudentsTimetable() =0;

    virtual void cuts() override =0;


public:

    /**
     * Creates a hard constriant to the number of students.
     * All students must be seated.
     * For slack cosntraint use slackStudent()
     * @param students
     */
    void constraintSeatedStudents(double students) override {
        model->addConstr(numberSeatedStudents() == students);
    }

    void optimizeRoomUsage() override {
        model->setObjective(usage(), GRB_MINIMIZE);
    }

    void optimizeSeatedStudents() override {
        model->setObjective(numberSeatedStudents(), GRB_MAXIMIZE);
    }


    void optimizeGapStudentsTimetable() override {
        model->setObjective(gapStudentsTimetable(), GRB_MINIMIZE);
    }

    virtual void printConfiguration() =0;


    bool run2019(bool warm) override {
        //model->set(GRB_IntParam_Presolve, 0);
        //model->set(GRB_IntParam_MIPFocus,3);
        warm = false;
        model->set(GRB_IntParam_Threads, 3);
        //model->set(GRB_DoubleParam_TimeLimit, 600.0);

        if (warm)
            warmStart();
        try {
            model->optimize();
            saveEncoding("AGORA");
            int status = model->get(GRB_IntAttr_Status);

            if (status == GRB_INF_OR_UNBD ||
                status == GRB_INFEASIBLE ||
                status == GRB_UNBOUNDED) {
                std::cout << "The model cannot be solved " <<
                          "because it is infeasible or unbounded" << std::endl;
                return false;
            }
            if (status != GRB_OPTIMAL) {
                std::cout << "Optimization was stopped with status " << status << std::endl;
            }

        } catch (GRBException e) {
            printError(e, "run");
            std::exit(-1);
        }
        std::cout << "Solution " << model->get(GRB_IntAttr_SolCount) << std::endl;

        double value = model->get(GRB_DoubleAttr_ObjVal);
        std::cout << value << std::endl;
        double time = model->get(GRB_DoubleAttr_Runtime);
        std::cout << time << std::endl;
        return true;

    }

    double run(bool mpp) override {
        std::cout << "GAP Min " << gapsSolution() << std::endl;
        //validator();
        //std::exit(42);
        printConfiguration();



        //printsolutionTime();
        //printRoomSolution();
        model->set(GRB_IntParam_Presolve, 0);

        model->set(GRB_IntParam_Threads, 3);
        //model->set(GRB_DoubleParam_TimeLimit, 600.0);

        if (mpp)
            warmStart();
        try {
            //GRBModel p = model->presolve();
            //saveEncoding("modelAfter" + instance->getName(), p);
            model->optimize();
            saveEncoding("model1" + instance->getName());
            int status = model->get(GRB_IntAttr_Status);

            if (status == GRB_INF_OR_UNBD ||
                status == GRB_INFEASIBLE ||
                status == GRB_UNBOUNDED) {
                std::cout << "The model cannot be solved " <<
                          "because it is infeasible or unbounded" << std::endl;
                return 1;
            }
            if (status != GRB_OPTIMAL) {
                std::cout << "Optimization was stopped with status " << status << std::endl;
            }

        } catch (GRBException e) {
            printError(e, "run");
            return -1;
        }
        double value = model->get(GRB_DoubleAttr_ObjVal);
        std::cout << value << std::endl;
        double time = model->get(GRB_DoubleAttr_Runtime);
        std::cout << time << std::endl;
        std::cout << "Unweighted Distance" << std::endl;
        printdistanceToSolutionRooms(false);
        printdistanceToSolutionLectures(false);
        std::cout << "Weighted Distance" << std::endl;
        printdistanceToSolutionRooms(true);
        printdistanceToSolutionLectures(true);


        switchSolution();
        std::cout << "GAP: " << gapsSolution() << std::endl;

        std::cout << "New Found Solution" << std::endl;
        //printRoomSolution();
        //printsolutionTime();

        return value;


    }

    /**
     * Create a hard constraint on the number of studnets seated.
     * Can be controled by the value of slack of each instance
     */

    virtual void slackStudent() override =0;


    /***
     * Prints the current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved
     */
    virtual void printdistanceToSolutionRooms(bool w) =0;

    virtual void printdistanceToSolutionLectures(bool w)=0;

    /***
     * The current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved
     * @param oldRoom
     * @param weighted
     * @return IloExpr
     */

    virtual GRBQuadExpr distanceToSolutionRooms(int **oldRoom, bool weighted) =0;

    /***
    * The current distance of the solution with the old solution
    * The distante is base on the weighted Hamming distance of the lectureTime variable (time slot atributions)
    * The weighted is baed on the number of students moved
    * @param oldTime
    * @param weighted
    * @return IloExpr
    */


    virtual GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted) =0;

    void distanceToSolution(bool w) override {
        distanceToSolution(solutionRoom, solutionTime, w);
    }

    /**
     * Minimization statement distance solutions
     * TODO: Generalize the distance metric
     * @param oldRoom
     * @param oldTime
     * @param weighted
     */

    void distanceToSolution(int **oldRoom, int ***oldTime, bool weighted) override {
        //+distanceToSolutionLectures(oldTime, weighted)
        model->setObjective(distanceToSolutionRooms(oldRoom, weighted), GRB_MINIMIZE);

    }

    void restart() override {
        restartModel();
    }

    /**
     * Minimization statement distance between solutions: Number of students seated
     */
    void minimizeDifferenceSeatedStudents() override {
        GRBVar opt = model->addVar(0.0, std::numeric_limits<int>::max(), 0.0, GRB_INTEGER, "opt");
        GRBVar abs = model->addVar(0.0, std::numeric_limits<int>::max(), 0.0, GRB_INTEGER, "abse");
        model->addConstr(opt == numberSeatedStudents() - instance->getTotalNumberSteatedStudent());
        model->addGenConstrAbs(abs, opt, "absconstr");
        GRBLinExpr temp = abs;
        model->setObjective(temp, GRB_MINIMIZE);
    }


private:
    /**
     * Warm starting procedure with the solution found before
     * Used the class atributes: solutionTime and roomLecture
     */

    virtual void warmStart()  =0;


private:


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     *
     */

    virtual void switchSolutionTime() =0;

    /** Switch solution
    * Updates the solution structures with new data
    *
    */
    void switchSolution() {
        removeSolution();
        switchSolutionRoom();
        switchSolutionTime();

    }

    /**
     * Switch solution room
     * Updates the solution room structure with new data
     * @Requires delete the previous found solution
     *
     */
    virtual void switchSolutionRoom() =0;

    virtual GRBLinExpr travel(std::vector<Class *> c, int pen) override { return 0; }

    virtual GRBLinExpr differentRoom(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        return 0;
    }

    virtual GRBLinExpr sameStart(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        return 0;
    }

    virtual GRBLinExpr maxDays(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) {
        return 0;
    }

    virtual GRBLinExpr minGap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) {
        return 0;
    }

    virtual GRBLinExpr workDay(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) {
        return 0;
    }

    virtual GRBLinExpr
    maxBreaks(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit, int limit1) {
        return 0;
    }

    virtual GRBLinExpr
    maxBlock(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit, int limit1) {
        return 0;
    }

    virtual GRBLinExpr maxDayLoad(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int limit) {
        return 0;
    }

    virtual GRBLinExpr sameTime(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        return 0;
    }

    virtual GRBLinExpr sameWeek(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        return 0;
    }

    virtual GRBLinExpr sectioning() {
        return 0;
    }


    virtual void dist(bool hard) override {
        GRBLinExpr opt = (hard ? 0 : roomPen());
        std::cout << "Room Pen : Done " << getTimeSpent() << std::endl;
        GRBLinExpr temp = sectioning();
        opt += (hard ? 0 : temp);
        std::cout << "Section: Done " << getTimeSpent() << std::endl;
        temp = timeOptions();
        opt += (hard ? 0 : temp);
        std::cout << "Time Pen : Done " << getTimeSpent() << std::endl;
        for (int i = 0; i < instance->getDist().size(); ++i) {
            if (instance->getDist()[i]->getPenalty() == -1 || !hard) {
                /*if (instance->getDist()[i]->getType()->getType() == SameAttendees) {
                    opt += travel(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == NotOverlap) {
                    opt += overlap(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(), true);
                } /*else if (instance->getDist()[i]->getType()->getType() == Overlap) {
                    opt += overlap(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(), false);
                } else if (instance->getDist()[i]->getType()->getType() == SameTime) {
                    opt += sameTime(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(), true);
                } else if (instance->getDist()[i]->getType()->getType() == DifferentTime) {
                    opt += sameTime(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(), false);
                } else if (instance->getDist()[i]->getType()->getType() == SameWeeks) {
                    opt += sameWeek(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(), true);
                } else if (instance->getDist()[i]->getType()->getType() == DifferentWeeks) {
                    opt += sameWeek(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(), false);
                } else if (instance->getDist()[i]->getType()->getType() == SameDays) {
                    opt += differentDay(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                        false);
                } else if (instance->getDist()[i]->getType()->getType() == DifferentDays) {
                    opt += differentDay(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                        true);
                } else if (instance->getDist()[i]->getType()->getType() == Precedence) {
                    opt += precedence(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameRoom) {
                    opt += differentRoom(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                         false);
                } else if (instance->getDist()[i]->getType()->getType() == DifferentRoom) {
                    opt += differentRoom(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                         true);
                } else if (instance->getDist()[i]->getType()->getType() == SameStart) {
                    opt += sameStart(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == MaxDays) {
                    opt += maxDays(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                   instance->getDist()[i]->getType()->getLimit());
                } else if (instance->getDist()[i]->getType()->getType() == MinGap) {
                    opt += minGap(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                  instance->getDist()[i]->getType()->getLimit());
                } else if (instance->getDist()[i]->getType()->getType() == WorkDay) {
                    opt += workDay(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                   instance->getDist()[i]->getType()->getLimit());
                } else if (instance->getDist()[i]->getType()->getType() == MaxDayLoad) {
                    opt += maxDayLoad(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                      instance->getDist()[i]->getType()->getLimit());
                } else if (instance->getDist()[i]->getType()->getType() == MaxBreaks) {
                    opt += maxBreaks(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                     instance->getDist()[i]->getType()->getLimit(),
                                     instance->getDist()[i]->getType()->getLimit1());
                } else if (instance->getDist()[i]->getType()->getType() == MaxBlock) {
                    opt += maxBlock(instance->getDist()[i]->getClasses(), instance->getDist()[i]->getPenalty(),
                                    instance->getDist()[i]->getType()->getLimit(),
                                    instance->getDist()[i]->getType()->getLimit1());
                }*/
            }

        }
        std::cout << "Distribution Constraint : Done " << getTimeSpent() << std::endl;
        model->setObjective(opt, GRB_MINIMIZE);

    }


};


#endif //PROJECT_ILPEXECUTER_H