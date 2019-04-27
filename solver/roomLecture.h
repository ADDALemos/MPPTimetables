//
// Created by Alexandre Lemos on 11/03/2019.
//

#ifndef PROJECT_ROOMLECTURE_H
#define PROJECT_ROOMLECTURE_H

#include "GurobiAux.h"
#include "../problem/Instance.h"

class roomLecture {
protected:
    Instance *instance;
    int currentW;

public:
    virtual void loadPreviousWeekSolution(int **room) =0;

    roomLecture(Instance *instance, int currentW) : instance(instance), currentW(currentW) {}

    virtual void definedRoomLecture() =0;

    virtual void oneLectureRoom() {}

    virtual void roomPreference(int r, int l) {}

    virtual void roomClose() {}

    virtual void slackStudent()=0;

    virtual void warmStart(int **sol)=0;

    virtual bool isStatic()=0;

    virtual void cuts()=0;

    virtual void oneLectureRoomConflict(GRBVar **order, GRBVar **sameday)=0;

    virtual bool **getBool() { throw "Abstract Class"; }

    virtual GRBVar **getGRB() { throw "Abstract Class"; }

    virtual void force(int **room, GRBVar *le, int ***lect) {}

    virtual void setCurrrentW(int currrentW) {
        currentW = currrentW;
    }

    virtual GRBLinExpr travel(std::vector<Class *> c, GRBVar *time, GRBVar **sameday, int pen) {
        return 0;
    }

    virtual GRBLinExpr differentRoom(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        return 0;
    }

    virtual GRBLinExpr roomPen() {
        return 0;
    }





};


#endif //PROJECT_ROOMLECTURE_H
