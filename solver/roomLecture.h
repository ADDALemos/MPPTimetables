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
public:
    roomLecture(Instance *instance) : instance(instance) {}

    virtual void definedRoomLecture() =0;

    virtual void oneLectureRoom() {}

    virtual void roomPreference(int r, int l) {}

    virtual void slackStudent()=0;

    virtual void warmStart(int **sol)=0;

    virtual bool isStatic()=0;

    bool **getBool() {}

    GRBVar **getGRB() {}


};


#endif //PROJECT_ROOMLECTURE_H
