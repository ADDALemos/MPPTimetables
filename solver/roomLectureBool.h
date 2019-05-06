//
// Created by Alexandre Lemos on 11/03/2019.
//

#ifndef PROJECT_ROOMLECTUREBOOL_H
#define PROJECT_ROOMLECTUREBOOL_H


#include <gurobi_c++.h>
#include "GurobiAux.h"
#include "../solver/roomLecture.h"


class roomLectureBool : public roomLecture {
    bool **vector;
public:
    void loadPreviousWeekSolution(int **room) override {
        // left blank
    }

    void cuts() override {
        // left blank
    }

    roomLectureBool(Instance *instance, int w) : roomLecture(instance, w) {}

    void slackStudent() override {}

    void definedRoomLecture() override {
        vector = new bool *[instance->getClassesWeek(currentW).size()];
        for (int i = 0; i < instance->getClassesWeek(currentW).size(); i++) {
            vector[i] = new bool[instance->getClasses()[i]->getPossibleRooms().size()];

        }


    }

    bool isStatic() override { return true; }

    bool **getBool() override { return vector; }

    void warmStart(int **sol) override {
        for (int l = 0; l < instance->getClassesWeek(currentW).size(); ++l) {
            int rs = 0, rg = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClassesWeek(currentW)[l]->containsRoom(instance->getRoom(rs + 1))) {
                    vector[l][rg] = sol[rs][l];
                    rg++;
                }
                rs++;
            }
        }
    }

//TODO: Finish this
    void oneLectureRoomConflict(GRBVar *start, GRBVar *end, GRBVar **sameday, GRBVar **week) override {

    }

    virtual GRBLinExpr roomPen() override {
        GRBLinExpr pen = 0;
        for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int r = 0, rl = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClassesWeek(currentW)[j]->containsRoom(instance->getRoom(r + 1))) {
                    pen += vector[instance->getClassesWeek(currentW)[j]->getOrderID()][rl] *
                           instance->getClassesWeek(currentW)[j]->getPen(instance->getRoom(r + 1));
                    rl++;
                }
                r++;

            }

        }
        return pen;
    }


};


#endif //PROJECT_ROOMLECTURE_H
