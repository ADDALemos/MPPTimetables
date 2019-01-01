//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_ILPEXECUTER_H
#define PROJECT_ILPEXECUTER_H
#ifndef IL_STD
#define IL_STD
#endif

#include <ilconcert/iloenv.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <exception>
#include <stdlib.h>
#include "Instance.h"

class ILPExecuter {
    IloEnv env; //CPLEX execution
    IloModel model = IloModel(env);
    Instance *instance;

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

    void oneLectureRoom() {
        try {
            for (int j = 0; j <
                            instance->getClasses().size(); j++) {
                IloNumExpr temp = IloNumExpr(env);
                for (int i = 0; i < instance->getRooms().size(); i++) {
                    temp += roomLecture[i][j];
                }
                model.add(temp == 1);
            }
        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }

    }

    void oneLectureRoomConflict() {
        try {
            for (int i = 0; i < instance->getRooms().size(); i++) {
                for (int d = 0; d < instance->getNdays(); d++) {
                    for (int k = 0; k < instance->getSlotsperday(); k++) {
                        IloNumExpr temp(env);
                        for (int j = 0; j < instance->getClasses().size(); j++) {
                            temp += lectureTime[d][k][j] * roomLecture[i][j];
                        }
                        model.add(1 >= temp);
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
                if (instance->isRoomBlocked(i + 1)) {
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
                        if (instance->isRoomBlockedbyDay(i + 1, d + 1)) {
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

    void loadOutput() {
            for (int j = 0; j < instance->getSlotsperday(); ++j) {
                for (int k = 0; k < instance->getClasses().size(); ++k) {
                    int d = 0;
                    for (auto c : instance->getClasses()[k]->getSolDays()) {
                        if (c == '1') {
                            if (instance->getClasses()[k]->getSolStart() <= j &&
                                (instance->getClasses()[k]->getSolStart() + instance->getClasses()[k]->getLenght() >
                                 j))
                                solutionTime[d][j][k] = 1;
                            else
                                solutionTime[d][j][k] = 0;
                        } else if (c == '0')
                            solutionTime[d][j][k] = 0;

                        d++;
                    }


                }

            }


        for (int i = 0; i < instance->getRooms().size(); ++i) {
            for (int k = 0; k < instance->getClasses().size(); ++k) {
                if (instance->getClasses()[k]->getSolRoom() == i)
                    solutionRoom[i][k] = 1;
                else
                    solutionRoom[i][k] = 0;

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

    double run(bool mpp) {
        std::cout << "Original Solution" << std::endl;
        printRoomSolution();
        printsolutionTime();
        IloCplex cplex(model);
        cplex.setParam(IloCplex::TiLim, 100.000);
        if (mpp)
            warmStart(cplex);
        saveEncoding();
        if (cplex.solve()) {
            std::cout << "solved" << std::endl;
            // printdistanceToSolutionRooms(cplex);
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

    Instance *getInstance() const {
        return instance;
    }

    void setInstance(Instance *instance) {
        ILPExecuter::instance = instance;
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

public:

    int **getSolutionRoom() const {
        return solutionRoom;
    }

    int ***getSolutionTime() const {
        return solutionTime;
    }

    /**
     * Initialize the solutions structures: solutionTime and solutionTime
     */
    void createSol() {
        solutionTime = new int **[instance->getNdays()];
        for (int i = 0; i < instance->getNdays(); i++) {
            solutionTime[i] = new int *[instance->getSlotsperday()];
            for (int k = 0; k < instance->getSlotsperday(); ++k) {
                solutionTime[i][k] = new int[instance->getClasses().size()];
                for (int j = 0; j < instance->getClasses().size(); ++j) {
                    solutionTime[i][k][j] = 0;
                }

            }

        }
        solutionRoom = new int *[instance->getRooms().size()];
        for (int i = 0; i < instance->getRooms().size(); i++) {
            solutionRoom[i] = new int[instance->getClasses().size()];
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = 0;
            }

        }
    }


private:
    int **solutionRoom;
    int ***solutionTime;

    /**
     * Print the current value of the solutionTime
     */
    void printsolutionTime() {

        std::cout << "d t l" << std::endl;
        for (int i = 0; i < instance->getClasses().size(); i++) {
            int k = 0;
            for (char &c :instance->getClasses()[i]->getDays()) {
                if (c != '0')
                    for (int j = 0; j < instance->getClasses()[i]->getLenght(); ++j) {

                        if (solutionTime[k][instance->getClasses()[i]->getStart() + j][i] != 0) {
                            std::cout << k << " " << instance->getClasses()[i]->getStart() + j << " " << i << std::endl;
                        }

                    }
                k++;

            }

        }


    }

    /**
     * Remove old solution
     * TODO: Memory management
     */
    void removeSolution() {
//        delete roomLecture;
        //      delete lectureTime;
        createSol();
    }

    /**
     * Switch solution time
     * Updates the solution time structure with new data
     * @Requires delete the previous found solution
     * @param cplex
     */

    void switchSolutionTime(IloCplex cplex) {
        for (int i = 0; i < instance->getClasses().size(); i++) {
            int k = 0;
            for (char &c :instance->getClasses()[i]->getDays()) {
                if (c != '0')
                    for (int j = 0; j < instance->getClasses()[i]->getLenght(); ++j) {
                        solutionTime[k][instance->getClasses()[i]->getStart() + j][i] = cplex.getValue(
                                lectureTime[k][instance->getClasses()[i]->getStart() + j][i]);
                        if (solutionTime[k][instance->getClasses()[i]->getStart() + j][i] != 0) {
                            instance->getClasses()[i]->setSolution(instance->getClasses()[i]->getStart(),
                                                                   strdup(std::to_string(k).c_str()));
                        }

                    }
                k++;

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
                    instance->getClasses()[j]->setSolRoom(i + 1);
                }
            }

        }
    }


    /**
     * Print the current value of the solutionRoom
     */
    void printRoomSolution() {
        std::cout << "room solution" << std::endl;
        std::cout << "r c" << std::endl;
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                if (solutionRoom[i][j] != 0) {
                    std::cout << i << " " << j << " " << solutionRoom[i][j] << std::endl;
                }
            }

        }
    }


};


#endif //PROJECT_ILPEXECUTER_H