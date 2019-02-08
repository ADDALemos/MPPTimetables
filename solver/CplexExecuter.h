//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_CPLEXEXECUTER_H
#define PROJECT_CPLEXEXECUTER_H

#include <ilconcert/iloenv.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "ILPExecuter.h"


class CplexExecuter : public ILPExecuter {
    IloEnv env; //CPLEX execution
    IloModel model = IloModel(env);




    typedef IloArray<IloBoolVarArray> NumVarMatrix;// Matrix
    typedef IloArray<NumVarMatrix> NumVar3Matrix;// 3d Matrix

    //Lectures
    NumVarMatrix roomLecture = NumVarMatrix(env);
    //Time
    NumVar3Matrix lectureTime = NumVar3Matrix(env);
    //int ***lectureTime;

public:

    void definedRoomLecture() {
        try {
            int size = instance->getNumClasses();
            for (auto i = 0; i < instance->getRooms().size(); i++) {
                roomLecture.add(IloBoolVarArray(env, size));
            }

        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }


    }

    /*int ***staticTime() {
        int size = instance->getClasses().size();
        lectureTime = new int **[instance->getNdays()];
        for (auto i = 0; i < instance->getNdays(); i++) {
            lectureTime[i] = new int *[instance->getSlotsperday()];
            for (int j = 0; j < instance->getSlotsperday(); ++j) {
                lectureTime[i][j] = new int[size];
                for (int k = 0; k < size; ++k) {
                    lectureTime[i][j][k] = 0;
                }
            }
        }
        return lectureTime;

    }*/

    void definedLectureTime() {
        //staticTime();
        int size = instance->getClasses().size();
        lectureTime = NumVar3Matrix(env, instance->getNdays());
        for (auto i = 0; i < instance->getNdays(); i++) {
            lectureTime[i] = NumVarMatrix(env, instance->getSlotsperday());
            for (int j = 0; j < instance->getSlotsperday(); ++j) {
                lectureTime[i][j] = IloBoolVarArray(env, size);
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
                IloNumExpr temp = IloNumExpr(env);
                for (int d = 0; d < instance->getNdays(); d++) {
                    for (int t = 0; t < instance->getSlotsperday(); t++) {
                        temp += lectureTime[d][t][j];
                        for (int t1 = t + 1; t1 < instance->getSlotsperday(); t1++) {
                            for (int t2 = t1 + 1; t2 < instance->getSlotsperday(); t2++) {
                                //model.add(lectureTime[d][t1][j] == ((lectureTime[d][t][j] - lectureTime[d][t2][j]) == 0));
                                model.add(IloIfThen(env, lectureTime[d][t][j] == 1 && lectureTime[d][t2][j] == 1,
                                                    lectureTime[d][t1][j] == 1));
                            }
                        }
                    }

                }
                // model.add(temp == instance->getClasses()[j]->getLenght());

            }
        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
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
                            // lectureTime[k][instance->getClasses()[j]->getStart() + i][j] = 1;
                            model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 1);
                        else if (c == '0')
                            //lectureTime[k][instance->getClasses()[j]->getStart() + i][j] = 0;
                            model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 0);
                        k++;
                    }
                }

            }
        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }

    }

    /**
     * A lecture can only be in one room at time
     */

    void oneLectureRoom() {
        try {
            for (int j = 0; j <
                            instance->getClasses().size(); j++) {
                IloNumExpr temp = IloNumExpr(env);
                for (int i = 0; i < instance->getRooms().size(); i++) {
                    if (instance->getClasses()[j]->getPossibleRooms().size() > 0)
                        temp += roomLecture[i][j];
                }
                model.add(temp == 1);
            }
        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }

    }

    /**
     * Ensure room r is used to lecture l
     */
    void roomPreference(int r, int l) {
        model.add(roomLecture[r][l] == 1);
        //std::cout<<instance->getRoom(r-1).getName()<<" "<<instance->getClasses()[l]->getId()<<std::endl;
    }

    /***
     * The room can only have one lecture per slot
     */

    void oneLectureRoomConflict() {
        try {
            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int d = 0; d < instance->getNdays(); d++) {
                    for (int k = 0; k < instance->getSlotsperday(); k++) {
                        IloNumExpr temp(env);
                        for (int j = 0; j < instance->getClasses().size(); j++) {
                            temp += (lectureTime[d][k][j] * roomLecture[i][j]);
                            //lectureTime[Qui][12]["ACED"]*roomLecture["FA1"]["ACED"]+lectureTime[Qui][12]["LP"]*roomLecture["FA1"]["LP"]
                        }
                        model.add(temp <= 1);
                    }
                }
            }
        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }

    }

    /**
    * Ensure Room closed cannot be used
    */
    void roomClose() {
        for (int i = 0; i < instance->getNumRoom(); i++) {
            for (int j = 0; j < instance->getNumClasses(); ++j) {
                if (instance->isRoomBlocked(i)) {
                    model.add(roomLecture[i][j] < 1);
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
                            model.add(roomLecture[i][j] * lectureTime[d][t][j] < 1);
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
                        model.add(lectureTime[d][t][i] < 0);

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
                            IloExpr temp(env);
                            temp = lectureTime[j][k][i];
                            for (int l = 0; l < instance->getNumRoom(); ++l) {
                                if (solutionRoom[l][i] == 1) {
                                    temp += roomLecture[l][i];
                                    model.add(temp <= 1);
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
                            IloExpr conflict = IloExpr(env);
                            for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                                conflict += lectureTime[d][t][c];
                            }
                            model.add(conflict <= sub->second[i]->getOverlap());
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
                    IloExpr conflict = IloExpr(env);
                    for (int c = 0; c < it->second.getCourse().size(); ++c) {
                        conflict += lectureTime[d][t][c];
                    }
                    model.add(conflict <= 1);
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
                    IloExpr conflict = IloExpr(env);
                    for (int c = 0; c < it->second.getClasses().size(); ++c) {
                        conflict += lectureTime[d][t][c];
                    }
                    model.add(conflict <= 1);
                }

            }
        }
    }



    void saveEncoding() {
        IloCplex cplex(model);
        //std::cout<<"Room "<<instance->getRooms().size()<<" Lecture"<<instance->getClasses().size()<< " Slot"<<instance->getSlotsperday()
        //          <<" days"<<instance->getNdays()<<std::endl;
        cplex.exportModel("/Volumes/MAC/ClionProjects/timetabler/model.lp");

    }

private:
//Number of seated students for optimization or constraint
    IloExpr numberSeatedStudents() {
        IloExpr temp(env);
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

    IloExpr usage() {
        IloExpr temp(env);
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
        try {
            model.add(numberSeatedStudents() >= students);
            model.add(numberSeatedStudents() <= students);
        } catch (IloAlgorithm::CannotExtractException &e) {
            std::cout << e.getExtractables() << std::endl;
            e.end();
        }

    }

    void optimizeRoomUsage() {
        model.add(IloMinimize(env, usage()));
    }

    void optimizeSeatedStudents() {
        model.add(IloMaximize(env, numberSeatedStudents()));
    }


    IloNum gapStudentsTimetable() {
        IloExpr min(env);
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int j = 0; j < instance->getNdays(); ++j) {
                for (int k = 1; k < instance->getSlotsperday(); ++k) {
                    IloExpr temp(env);
                    for (int l = 0; l < instance->getClasses().size(); ++l) {
                        //std::cout << l << " " << instance->getStudent(i).isEnrolled(l) << std::endl;
                        if (instance->getStudent(i).isEnrolled(l))
                            temp += lectureTime[j][k][l] + lectureTime[j][k - 1][l];
                    }
                    min += (temp == 1);

                }
            }

        }
        //std::cout << min << std::endl;
    }

    void optimizeGapStudentsTimetable() {
        model.add(IloMaximize(env, gapStudentsTimetable()));
    }

    double run(bool mpp) {
        std::cout << env.getVersion() << std::endl;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            roomPreference(instance->getClasses()[i]->getSolRoom() - 1, i);
        }

        createSol();
        std::cout << "Original Solution" << std::endl;
        loadOutput();
        printsolutionTime();
        printRoomSolution();
        IloCplex cplex(model);
        cplex.setParam(IloCplex::TiLim, 100.000);
        if (mpp);
        // warmStart(cplex);
        saveEncoding();
        try {
            if (cplex.solve()) {
                std::cout << "solved" << std::endl;
                //printdistanceToSolutionRooms(cplex);
                double value = cplex.getObjValue();
                std::cout << value << std::endl;
                switchSolution(cplex);
                std::cout << "New Found Solution" << std::endl;
                printRoomSolution();
                printsolutionTime();

                return value;

            } else {
                std::cout << "false" << std::endl;
            }
        } catch (IloCplex::Exception e) {
            std::cerr << e.getMessage() << std::endl;
        }

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
                model.add(it->second.getCapacity() >=
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
                                                                 != cplex.getValue(roomLecture[i][j]));
            }
        }
        //std::cout << temp << std::endl;
    }

    /***
     * The current distance of the solution with the old solution
     * The distante is base on the weighted Hamming distance of the roomLecture variable (room atributions)
     * The weighted is baed on the number of students moved
     * @param oldRoom
     * @param weighted
     * @return IloExpr
     */

    IloExpr distanceToSolutionRooms(int **oldRoom, bool weighted) {
        IloExpr temp(env);
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                temp += instance->getClasses()[j]->getLimit() * (oldRoom[i][j] != roomLecture[i][j]);
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


    IloExpr distanceToSolutionLectures(int ***oldTime, bool weighted) {
        IloExpr temp(env);
        for (int i = 0; i < instance->getNdays(); i++) {
            for (int t = 0; t < instance->getSlotsperday(); t++) {
                for (int j = 0; j < instance->getClasses().size(); ++j) {
                    temp += instance->getClasses()[j]->getLimit() * (oldTime[i][t][j] != lectureTime[i][t][j]);
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

        model.add(IloMinimize(env, distanceToSolutionRooms(oldRoom, weighted)));

    }

    /**
     * Minimization statement distance between solutions: Number of students seated
     */
    void minimizeDifferenceSeatedStudents() {
        model.add(IloMinimize(env, IloAbs(numberSeatedStudents() - instance->getTotalNumberSteatedStudent())));
    }


private:
    /**
     * Warm starting procedure with the solution found before
     * Used the class atributes: solutionTime and roomLecture
     * @param cplex
     */
    void warmStart(IloCplex cplex) {
        IloNumVarArray startVar(env, instance->getNdays() * instance->getSlotsperday() * instance->getClasses().size()
                                     + instance->getRooms().size() * instance->getClasses().size());
        IloNumArray startVal(env, instance->getNdays() * instance->getSlotsperday() * instance->getClasses().size()
                                  + instance->getRooms().size() * instance->getClasses().size());
        int idx = 0;
        for (int k = 0; k < instance->getNdays(); k++) {
            for (int i = 0; i < instance->getSlotsperday(); i++) {
                for (int j = 0; j < instance->getClasses().size(); j++) {
                    startVar[idx] = lectureTime[k][i][j];
                    startVal[idx] = solutionTime[k][i][j];
                    ++idx;
                }
            }
        }
        for (int r = 0; r < instance->getRooms().size(); ++r) {
            for (int l = 0; l < instance->getClasses().size(); ++l) {
                startVar[idx] = roomLecture[r][l];
                startVal[idx] = solutionRoom[r][l];
                ++idx;

            }

        }

        cplex.addMIPStart(startVar, startVal);
    }




private:


    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     * @param cplex
     */

    void switchSolutionTime(IloCplex cplex) {
        for (int i = 0; i < instance->getClasses().size(); i++) {
            for (int k = 0; k < instance->getNdays(); ++k) {
                for (int j = 0; j < instance->getSlotsperday(); ++j) {
                    solutionTime[k][j][i] = cplex.getValue(lectureTime[k][j][i]);

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
    * @param cplex
    */
    void switchSolution(IloCplex cplex) {
        removeSolution();
        switchSolutionRoom(cplex);
        switchSolutionTime(cplex);

    }

    /**
     * Switch solution room
     * Updates the solution room structure with new data
     * @Requires delete the previous found solution
     * @param cplex
     */
    void switchSolutionRoom(IloCplex cplex) {
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = cplex.getValue(roomLecture[i][j]);
                if (cplex.getValue(roomLecture[i][j]) != 0) {
                    instance->getClasses()[j]->setSolRoom(i);
                }
            }

        }
    }




};


#endif //PROJECT_ILPEXECUTER_H