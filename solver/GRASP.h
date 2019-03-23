//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef PROJECT_GRASP_H
#define PROJECT_GRASP_H


#include <fstream>
#include "../problem/Instance.h"

class GRASP {
    int MAX_ITERATIONS = 5;
    Instance *instance;
    bool **room;
    bool ***time;
    bool *allocated;
    std::ofstream seedFile;
    int sizeRCL;

public:
    GRASP(int MAX_ITERATIONS, double rcl, Instance *instance);

    void run();


private:

    void init();

    bool eval();

    void store();

    void printStatus(int ite);

    void printFinal();

    void Greedy();

    void Local();

    bool isAllocable(int lectureID, int roomID, int i, int i1);

    double cost(int lectureID, int roomID, int i, int i1);

    unsigned int seedHandler();

    bool assign(int lectureID, int roomID, int day, int time);

    int getGAP() const;


    bool tryswampLectures(int l1, int l2, int d1, int t1, int le1, int d2, int t2, int le2);

    void swampLectures(int lect1, int lect2, int day, int start, int lenght, int solDay, int solStart, int getLenght);

    int getGAPStored();
};


#endif //PROJECT_GRASP_H
