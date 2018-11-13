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
    //NumVar3Matrix lectureTime = NumVar3Matrix(env);
    int ***lectureTime;

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

    int ***staticTime() {
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

    }

    void definedLectureTime() {
        staticTime();
        int size = instance->getClasses().size();
        //lectureTime = NumVar3Matrix(env, instance->getNdays());
        for (auto i = 0; i < instance->getNdays(); i++) {
            //lectureTime[i] = NumVarMatrix(env, instance->getSlotsperday());
            for (int j = 0; j < instance->getSlotsperday(); ++j) {
                //  lectureTime[i][j] = IloBoolVarArray(env, size);
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
                            lectureTime[k][instance->getClasses()[j]->getStart() + i][j] = 1;
                            // model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 1);
                        else
                            lectureTime[k][instance->getClasses()[j]->getStart() + i][j] = 0;
                        // model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 0);

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
            model.add(numberSeatedStudents() == students);
        } catch (IloAlgorithm::CannotExtractException &e) {
            // based on the ILOG CPLEX 10.0 User’s Manual / Languages and APIs / Handling Errors
            std::cout << e << std::endl;
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
            solution(cplex);
            return value;

        }

    }

    void slackStudent() {
        for (int l = 0; l <
                        instance->getClasses().size(); l++) {
            int j = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                model.add(it->second.getCapacity() <=
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

    void distanceToSolution(int **oldRoom) {
        IloExpr temp(env);
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                temp += (oldRoom[i][j] != roomLecture[i][j]);
            }

        }
        model.add(IloMinimize(env, temp));

    }

public:

    int **getSolutionRoom() const {
        return solutionRoom;
    }

private:
    int **solutionRoom;

    void solution(IloCplex cplex) {
        solutionRoom = new int *[instance->getRooms().size()];
        for (int i = 0; i < instance->getRooms().size(); i++) {
            solutionRoom[i] = new int[instance->getClasses().size()];
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = 0;
            }

        }
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = cplex.getValue(roomLecture[i][j]);
                if (solutionRoom[i][j] != 0)
                    std::cout << i << " " << j << " " << solutionRoom[i][j] << std::endl;
            }

        }
    }


};


#endif //PROJECT_ILPEXECUTER_H
