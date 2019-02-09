//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_GUROBIEXECUTER_H
#define PROJECT_GUROBIEXECUTER_H


#include "/Library/gurobi810/mac64/include/gurobi_c++.h"

#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"


class GurobiExecuter : public ILPExecuter {
protected:

    GRBEnv env = GRBEnv();

    GRBModel *model = new GRBModel(env);
    std::string itos(int i) {
        std::stringstream s;
        s << i;
        return s.str();
    }




public:

    virtual void definedRoomLecture()=0;


    virtual void definedLectureTime()=0;

    /**
     * The lecture can only be scheduled in one slot
     */

    virtual void oneLectureperSlot() =0;

    /**
     * Force lectures to be in slot n
     */


    virtual void oneLectureSlot() =0;

    /**
     * A lecture can only be in one room at time
     */

    virtual void oneLectureRoom() =0;

    void printError(const GRBException &e, std::string local) const {
        std::cout << "Error found: " << local << std::endl;
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    /**
     * Ensure room r is used to lecture l
     */
    virtual void roomPreference(int r, int l) =0;

    /***
     * The room can only have one lecture per slot
     */

    virtual void oneLectureRoomConflict() =0;
    /**
    * Ensure Room closed cannot be used
    */
    virtual void roomClose() =0;

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

    virtual GRBLinExpr gapStudentsTimetable() =0;



public:

    /**
     * Creates a hard constriant to the number of students.
     * All students must be seated.
     * For slack cosntraint use slackStudent()
     * @param students
     */
    void constraintSeatedStudents(double students) {
        model->addConstr(numberSeatedStudents() == students);
    }

    void optimizeRoomUsage() {
        model->setObjective(usage(), GRB_MINIMIZE);
    }

    void optimizeSeatedStudents() {
        model->setObjective(numberSeatedStudents(), GRB_MAXIMIZE);
    }


    void optimizeGapStudentsTimetable() {
        model->setObjective(gapStudentsTimetable(), GRB_MAXIMIZE);
    }

    double run(bool mpp) {


        createSol();
        std::cout << "Original Solution" << std::endl;
        loadOutput();
        printsolutionTime();
        printRoomSolution();
        model->set(GRB_DoubleParam_TimeLimit, 60.0);
        if (mpp)
            warmStart();
        saveEncoding();
        try {
            model->optimize();
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
                return 1;
            } else {
                double value = model->get(GRB_DoubleAttr_ObjVal);
                std::cout << value << std::endl;
                switchSolution();
                std::cout << "New Found Solution" << std::endl;
                printRoomSolution();
                printsolutionTime();

                return value;

            }
        } catch (GRBException e) {
            printError(e, "run");
        }
        return -1;


    }

    /**
     * Create a hard constraint on the number of studnets seated.
     * Can be controled by the value of slack of each instance
     */

    virtual void slackStudent() =0;


    /***
     * Prints the current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved
     */
    virtual void printdistanceToSolutionRooms() =0;

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

    void distanceToSolution() {
        distanceToSolution(solutionRoom, solutionTime, false);
    }

    /**
     * Minimization statement distance solutions
     * TODO: Generalize the distance metric
     * @param oldRoom
     * @param oldTime
     * @param weighted
     */

    void distanceToSolution(int **oldRoom, int ***oldTime, bool weighted) {

        model->setObjective(distanceToSolutionRooms(oldRoom, weighted), GRB_MINIMIZE);

    }

    /**
     * Minimization statement distance between solutions: Number of students seated
     */
    void minimizeDifferenceSeatedStudents() {
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

    virtual void warmStart()=0;


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


};


#endif //PROJECT_ILPEXECUTER_H