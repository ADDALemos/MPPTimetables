//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef PROJECT_GRASP_H
#define PROJECT_GRASP_H


#include <fstream>
#include "../problem/Instance.h"
#include <climits>

class LocalSearch {
    int MAX_ITERATIONS = 5;
    Instance *instance;
    std::vector<Solution*> tabu;
    int best = INT_MAX;
    int currentV = INT_MAX;
    Solution** current;
    std::ofstream seedFile;
    int sizeRCL;

public:
    LocalSearch(int MAX_ITERATIONS, double rcl, Instance *instance);

    void GRASP();

    void LNS();

    LocalSearch(Instance *pInstance);

private:

    void init();

    bool eval();

    void store();

    void printStatus(int ite);

    void printFinal();

    void Greedy();

    void Local();

    int isAllocable(int lectureID, std::string week, std::string day, int start, int duration, int roomID);

    double cost(int lectureID, int roomID, int i, int i1);

    unsigned int seedHandler();

    bool assign(Solution*);

    int getGAP() const;


    bool tryswampLectures(int l1, int l2, int d1, int t1, int le1, int d2, int t2, int le2);

    void swampLectures(int lect1, int lect2, int day, int start, int lenght, int solDay, int solStart, int getLenght);

    int getGAPStored();
};


#endif //PROJECT_GRASP_H
