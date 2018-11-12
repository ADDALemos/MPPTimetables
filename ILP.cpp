//
// Created by Alexandre Lemos on 11/11/2018.
//

#include "ILP.h"

IloEnv env; //CPLEX execution
IloModel model(env);

typedef IloArray<IloBoolVarArray> NumVarMatrix;// Matrix
typedef IloArray<NumVarMatrix> NumVar3Matrix;// 3d Matrix

//Lectures
NumVarMatrix roomLecture(env);
//Time
NumVar3Matrix lectureTime(env);

void definedRoomLecture(Instance *instance) {
    try {
        int size = instance->getNumClasses();
        for (auto i = 0; i < instance->getRooms().size(); i++) {
            roomLecture.add(IloBoolVarArray(env, size));
        }

    } catch (IloCplex::Exception &e) {
        std::cout << e.getMessage() << std::endl;
    }


}

void definedLectureTime(Instance *instance) {
    int size = instance->getClasses().size();
    lectureTime = NumVar3Matrix(env, instance->getNdays());
    for (auto i = 0; i < instance->getNdays(); i++) {
        lectureTime[i] = NumVarMatrix(env, instance->getSlotsperday());
        for (int j = 0; j < instance->getSlotsperday(); ++j) {
            lectureTime[i][j] = IloBoolVarArray(env, size);
        }
    }


}

void oneLectureSlot(Instance *instance) {
    try {
        for (int j = 0; j <
                        instance->getClasses().size(); j++) {
            for (int i = 0; i < instance->getClasses()[j]->getLenght(); i++) {
                for (int k = 0; k < instance->getClasses()[j]->getDays().length(); ++k) {
                    if (atoi(&instance->getClasses()[j]->getDays()[k]) != 0)
                        model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 1);
                    else
                        model.add(lectureTime[k][instance->getClasses()[j]->getStart() + i][j] == 0);

                }
            }

        }
    } catch (IloCplex::Exception &e) {
        std::cout << e.getMessage() << std::endl;
    }

}

void oneLectureRoom(Instance *instance) {
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
void run() {
    IloCplex cplex(model);
    cplex.setParam(IloCplex::TiLim, 100.000);
    cplex.exportModel("/Volumes/MAC/ClionProjects/timetabler/test.lp");
}
