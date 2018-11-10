//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_CONSTRAINT_H
#define PROJECT_CONSTRAINT_H

enum ConstraintType {
    NotOverlap,
    Prencedence,
    SameAttendees,
    SameStart,
    SameTime,
    SameDays,
    SameWeeks,
    SameRoom,
    MaxBreaks,
    MaxBlock,
    WorkDay,
    MinGap,
    MaxDays,
    MaxDayLoad
};
class Constraint {
public:
    virtual void setType(std::string s) {
    }

};


#endif //PROJECT_CONSTRAINT_H
