//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_NOLIMIT_H
#define PROJECT_NOLIMIT_H

const enum ConstraintType {
    NotOverlap;Prencedence; SameAttendees;SameStart; SameTime;SameDays;SameWeeks;SameRoom;
};

class NoLimit {
    ConstraintType type;

};


#endif //PROJECT_NOLIMIT_H
