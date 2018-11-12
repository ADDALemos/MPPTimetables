//
// Created by Alexandre Lemos on 11/11/2018.
//

#ifndef PROJECT_ILP_H
#define PROJECT_ILP_H
#ifndef IL_STD
#define IL_STD
#endif

#include <ilconcert/iloenv.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <exception>

#include "Instance.h"


//Execute Cplex solver
void run();

//Initialize Room Lecture

void definedRoomLecture(Instance *instance);

//Initialize Lecture day hour

void definedLectureTime(Instance *instance);

void oneLectureSlot(Instance *instance);
void oneLectureRoom(Instance *instance);

void oneLectureRoomConflict(Instance *instance);

#endif //PROJECT_ILP_H