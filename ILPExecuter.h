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
                    for (int k = 0; k < instance->getClasses()[j]->getDays().length(); ++k) {
                        if (atoi(&instance->getClasses()[j]->getDays()[k]) != 0)
                            // lectureTime[k][instance->getClasses()[j]->getStart() + i][j] = 1;
                            model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 1);
                        else
                            //lectureTime[k][instance->getClasses()[j]->getStart() + i][j] = 0;
                            model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 0);

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
                        model.add(1 <= temp);
                    }
                }
            }
        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }

    }

    void studentConflict() {
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
        for (int i = 0; i < instance->getNdays(); ++i) {
            for (int j = 0; j < instance->getSlotsperday(); ++j) {
                for (int k = 0; k < instance->getClasses().size(); ++k) {
                    if (atoi(&instance->getClasses()[k]->getSolDays()[i]) != 0 &&
                        instance->getClasses()[k]->getSolStart() >= j &&
                        (instance->getClasses()[k]->getSolStart() + instance->getClasses()[k]->getLenght() <= j))
                        solutionTime[i][j][k] = 1;
                    else
                        solutionTime[i][j][k] = 0;


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



private:
//Number of seated students for optimization or constraint
    IloExpr numberSeatedStudents() {
        IloExpr temp(env);
        for (int l = 0; l < instance->getClasses().size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                for (int d = 0; d < instance->getClasses()[l]->getDays().length(); ++d) {
                    if (atoi(&instance->getClasses()[l]->getDays()[d]) != 0) {
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
                }
                j++;
            }
        }
        //std::cout<<temp<<std::endl;

        return temp;
    }

public:


    void constraintSeatedStudents(double students) {
        try {
            model.add(numberSeatedStudents() >= students);
            model.add(numberSeatedStudents() <= students);
        } catch (IloAlgorithm::CannotExtractException &e) {
            std::cout << e.getExtractables() << std::endl;
            e.end();
        }

    }

    void optimizeSeatedStudents() {
        model.add(IloMaximize(env, numberSeatedStudents()));
    }


    double run() {
        IloCplex cplex(model);
        cplex.setParam(IloCplex::TiLim, 100.000);
        cplex.exportModel("/Volumes/MAC/ClionProjects/timetabler/test1.lp");
        if (cplex.solve()) {
            std::cout << "solved" << std::endl;
            double value = cplex.getObjValue();
            std::cout << value << std::endl;
            solutionReader(cplex);
            solutionTimeReader(cplex);
            return value;

        } else {
            std::cout << "false" << std::endl;
        }

    }

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

    IloExpr distanceToSolutionRooms(int **oldRoom, bool weighted) {
        IloExpr temp(env);
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                temp += instance->getClasses()[j]->getLimit() * (oldRoom[i][j] != roomLecture[i][j]);
            }

        }
        return temp;
    }

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

    void distanceToSolution(int **oldRoom, int ***oldTime, bool weighted) {

        model.add(IloMinimize(env, distanceToSolutionRooms(oldRoom, weighted)));

    }

    void minimizeDifferenceSeatedStudents() {
        model.add(IloMinimize(env, IloAbs(numberSeatedStudents() - instance->getTotalNumberSteatedStudent())));
    }

    void warmStart(IloCplex cplex) {
        IloNumVarArray startVar(env);
        IloNumArray startVal(env);
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


    void solutionTimeReader(IloCplex cplex) {


        for (int i = 0; i < instance->getClasses().size(); i++) {
            for (int k = 0; k < instance->getClasses()[i]->getDays().length(); ++k) {
                if (instance->getClasses()[i]->getDays()[k] != 0)
                    for (int j = 0; j < instance->getClasses()[i]->getLenght(); ++j) {
                        solutionTime[k][instance->getClasses()[i]->getStart() + j][i] = cplex.getValue(
                                lectureTime[k][instance->getClasses()[i]->getStart() + j][i]);
                        if (solutionTime[k][instance->getClasses()[i]->getStart() + j][i] != 0) {
                            instance->getClass(j + 1)->setSolution(instance->getClasses()[i]->getStart(),
                                                                   strdup(std::to_string(k).c_str()));
                        }

                    }

            }

        }


    }


    void solutionReader(IloCplex cplex) {

        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = cplex.getValue(roomLecture[i][j]);
                if (solutionRoom[i][j] != 0) {
                    std::cout << i << " " << j << " " << solutionRoom[i][j] << std::endl;
                    instance->getClass(j + 1)->setSolRoom(i + 1);
                }
            }

        }
    }


};


#endif //PROJECT_ILPEXECUTER_H