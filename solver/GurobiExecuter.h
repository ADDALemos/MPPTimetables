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

    GRBEnv env = GRBEnv();

    GRBModel *model = new GRBModel(env);
    GRBVar ***lectureTime;
    GRBVar **roomLecture;

    std::string itos(int i) {
        std::stringstream s;
        s << i;
        return s.str();
    }




public:

    void definedRoomLecture() {
        try {
            roomLecture = new GRBVar *[instance->getRooms().size()];
            for (int i = 0; i < instance->getRooms().size(); i++) {
                roomLecture[i] = new GRBVar[instance->getClasses().size()];
                for (int j = 0; j < instance->getNumClasses(); ++j) {
                    std::string name = "X_" + itos(i) + "_" + itos(j);
                    roomLecture[i][j] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
                }

            }
        } catch (GRBException e) {
            printError(e, "definedRoomLecture");

        }


    }


    void definedLectureTime() {
        lectureTime = new GRBVar **[instance->getNdays()];
        for (int i = 0; i < instance->getNdays(); i++) {
            lectureTime[i] = new GRBVar *[instance->getSlotsperday()];
            for (int k = 0; k < instance->getSlotsperday(); ++k) {
                lectureTime[i][k] = new GRBVar[instance->getClasses().size()];
                for (int j = 0; j < instance->getClasses().size(); ++j) {
                    std::string name = "A_" + itos(i) + "_" + itos(k) + "_" + itos(j);
                    lectureTime[i][k][j] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY, name);
                }

            }

        }


    }

    /**
     * The lecture can only be scheduled in one slot
     */

    void oneLectureperSlot() {
        try {
            for (int j = 0; j <
                            instance->getClasses().size(); j++) {
                GRBLinExpr expr = 0;
                for (int d = 0; d < instance->getNdays(); d++) {
                    for (int t = 0; t < instance->getSlotsperday(); t++) {
                        expr += lectureTime[d][t][j];
                        for (int t1 = t + 1; t1 < instance->getSlotsperday(); t1++) {
                            for (int t2 = t1 + 1; t2 < instance->getSlotsperday(); t2++) {
                                GRBVar temp = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                                model->addGenConstrIndicator(temp, 1,
                                                             (lectureTime[d][t][j] - lectureTime[d][t2][j]) == 0);
                                model->addConstr(lectureTime[d][t1][j] == temp);
                            }
                        }
                    }

                }
                model->addConstr(expr == instance->getClasses()[j]->getLenght());

            }
        } catch (GRBException e) {
            printError(e, "oneLectureperSlot");
        }

    }

    /**
     * Force lectures to be in slot n
     */


    void oneLectureSlot() {
        try {
            for (int j = 0; j <
                            instance->getClasses().size(); j++) {
                for (int i = 0; i < instance->getClasses()[j]->getLenght(); i++) {
                    int k = 0;
                    for (char &c :instance->getClasses()[j]->getDays()) {
                        if (c == '1')
                            model->addConstr(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 1);
                        else if (c == '0')
                            model->addConstr(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 0);
                        k++;
                    }
                }

            }
        } catch (GRBException e) {
            printError(e, "oneLectureSlot");
        }

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
                    if (instance->getClasses()[j]->getPossibleRooms().size() > 0)
                        temp += roomLecture[i][j];
                }
                model->addConstr(temp == 1);
            }
        } catch (GRBException e) {
            printError(e, "oneLectureRoom");
        }

    }

    void printError(const GRBException &e, std::string local) const {
        std::cout << "Error found: " << local << std::endl;
        std::cout << "Error code = " << e.getErrorCode() << std::endl;
        std::cout << e.getMessage() << std::endl;
    }

    /**
     * Ensure room r is used to lecture l
     */
    void roomPreference(int r, int l) {
        model->addConstr(roomLecture[r][l] == 1);
    }

    /***
     * The room can only have one lecture per slot
     */

    void oneLectureRoomConflict() {
        try {
            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int d = 0; d < instance->getNdays(); d++) {
                    for (int k = 0; k < instance->getSlotsperday(); k++) {
                        GRBQuadExpr temp = 0;
                        for (int j = 0; j < instance->getClasses().size(); j++) {
                            temp += (lectureTime[d][k][j] * roomLecture[i][j]);
                            //lectureTime[Qui][12]["ACED"]*roomLecture["FA1"]["ACED"]+lectureTime[Qui][12]["LP"]*roomLecture["FA1"]["LP"]
                        }
                        model->addQConstr(temp <= 1);
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
                if (instance->isRoomBlocked(i)) {
                    model->addConstr(roomLecture[i][j] == 0);
                }
            }
        }
    }

    /**
    * Ensure Room closed in a day cannot be used
    */
    void roomClosebyDay() {
        for (int d = 0; d < instance->getNdays(); ++d) {
            for (int t = 0; t < instance->getSlotsperday(); ++t) {
                for (int i = 0; i < instance->getNumRoom(); i++) {
                    for (int j = 0; j < instance->getNumClasses(); ++j) {
                        if (instance->isRoomBlockedbyDay(i, d)) {
                            model->addConstr(roomLecture[i][j] * lectureTime[d][t][j] == 0);
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
            for (int d = 0; d < instance->getNdays(); d++) {
                for (int t = 0; t < instance->getSlotsperday(); ++t) {
                    if (instance->isTimeUnavailable(i * t))
                        model->addConstr(lectureTime[d][t][i] == 0);

                }
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
                            temp = lectureTime[j][k][i];
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
        for (std::map<std::string, Course *>::const_iterator it = instance->getCourses().begin();
             it != instance->getCourses().end(); it++) {
            for (std::map<int, std::vector<Subpart *>>::iterator sub = it->second->getConfiguratons().begin();
                 sub != it->second->getConfiguratons().end(); ++sub) {
                for (int i = 0; i < sub->second.size(); ++i) {
                    for (int d = 0; d < instance->getNdays(); ++d) {
                        for (int t = 0; t < instance->getSlotsperday(); ++t) {
                            GRBLinExpr conflict = 0;
                            for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                                conflict += lectureTime[d][t][c];
                            }
                            model->addConstr(conflict <= sub->second[i]->getOverlap());
                        }

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
            for (int d = 0; d < instance->getNdays(); ++d) {
                for (int t = 0; t < instance->getSlotsperday(); ++t) {
                    GRBLinExpr conflict = 0;
                    for (int c = 0; c < it->second.getCourse().size(); ++c) {
                        conflict += lectureTime[d][t][c];
                    }
                    model->addConstr(conflict <= 1);
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
            for (int d = 0; d < instance->getNdays(); ++d) {
                for (int t = 0; t < instance->getSlotsperday(); ++t) {
                    GRBLinExpr conflict = 0;
                    for (int c = 0; c < it->second.getClasses().size(); ++c) {
                        conflict += lectureTime[d][t][c];
                    }
                    model->addConstr(conflict <= 1);
                }

            }
        }
    }




    void saveEncoding() {
        model->write("/Volumes/MAC/ClionProjects/timetabler/model.lp");

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
                                        lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
                                        * roomLecture[j][l];
                            } else {
                                temp += it->second.getCapacity() *
                                        lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
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
                                    lectureTime[d][instance->getClasses()[l]->getStart() + i][l]
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


    GRBLinExpr gapStudentsTimetable() {
        GRBLinExpr min = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int j = 0; j < instance->getNdays(); ++j) {
                for (int k = 1; k < instance->getSlotsperday(); ++k) {
                    GRBLinExpr all = 0;
                    GRBVar t = model->addVar(0.0, std::numeric_limits<int>::max(), 0.0, GRB_INTEGER);
                    GRBVar tmin = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    for (int l = 0; l < instance->getClasses().size(); ++l) {
                        //std::cout << l << " " << instance->getStudent(i).isEnrolled(l) << std::endl;
                        if (instance->getStudent(i).isEnrolled(l))
                            all += lectureTime[j][k][l] + lectureTime[j][k - 1][l];
                    }
                    model->addConstr(t == all);
                    model->addGenConstrIndicator(tmin, 1, t == 1);

                    min += tmin;

                }
            }

        }
        return min;
        //std::cout << min << std::endl;
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
        model->set(GRB_DoubleParam_TimeLimit, 6000.0);
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

    void slackStudent() {
        for (int l = 0; l <
                        instance->getClasses().size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                model->addConstr(it->second.getCapacity() >=
                                 ((instance->getClasses()[l]->getLimit() -
                            (instance->getClasses()[l]->getLimit() * instance->getAlfa())) *
                           roomLecture[j][l]));


                j++;
            }
        }
    }


    /***
     * Prints the current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved
     * @param cplex
     */
    void printdistanceToSolutionRooms(IloCplex cplex) {
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


    GRBQuadExpr distanceToSolutionLectures(int ***oldTime, bool weighted) {
        GRBQuadExpr temp = 0;
        for (int i = 0; i < instance->getNdays(); i++) {
            for (int t = 0; t < instance->getSlotsperday(); t++) {
                for (int j = 0; j < instance->getClasses().size(); ++j) {
                    GRBVar tempv = model->addVar(0.0, 1.0, 0.0, GRB_BINARY);
                    model->addGenConstrIndicator(tempv, 0, oldTime[i][t][j] == lectureTime[i][t][j]);
                    model->addGenConstrIndicator(tempv, 1, lectureTime[i][t][j] - oldTime[i][t][j] - 1 <= 0);
                    temp += instance->getClasses()[j]->getLimit() * tempv;
                }

            }
        }
        return temp;
    }

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

    void warmStart() {
        for (int k = 0; k < instance->getNdays(); k++) {
            for (int i = 0; i < instance->getSlotsperday(); i++) {
                for (int j = 0; j < instance->getClasses().size(); j++) {
                    lectureTime[k][i][j].set(GRB_DoubleAttr_Start, solutionTime[k][i][j]);
                }
            }
        }
        for (int r = 0; r < instance->getRooms().size(); ++r) {
            for (int l = 0; l < instance->getClasses().size(); ++l) {
                roomLecture[r][l].set(GRB_DoubleAttr_Start, solutionRoom[r][l]);
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
        for (int i = 0; i < instance->getClasses().size(); i++) {
            for (int k = 0; k < instance->getNdays(); ++k) {
                for (int j = 0; j < instance->getSlotsperday(); ++j) {
                    solutionTime[k][j][i] = lectureTime[k][j][i].get(GRB_DoubleAttr_X);

                    if (solutionTime[k][j][i] != 0) {
                        instance->getClasses()[i]->setSolutionTime(j,
                                                                   strdup(std::to_string(k).c_str()));
                    }

                }

            }

        }

    }

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
    void switchSolutionRoom() {
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = roomLecture[i][j].get(GRB_DoubleAttr_X);
                if (roomLecture[i][j].get(GRB_DoubleAttr_X) != 0) {
                    instance->getClasses()[j]->setSolRoom(i);
                }
            }

        }
    }


};


#endif //PROJECT_ILPEXECUTER_H