//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_CONSTRAINT_H
#define PROJECT_CONSTRAINT_H

enum ConstraintType {
    NotOverlap,
    Overlap,
    Precedence,
    SameAttendees,
    SameStart,
    SameTime,
    SameDays,
    DifferentDays,
    SameWeeks,
    SameRoom,
    DifferentRoom,
    MaxBreaks,
    MaxBlock,
    WorkDay,
    MinGap,
    MaxDays,
    MaxDayLoad
};
class Constraint {
public:
    ConstraintType type;
    virtual void setType(std::string s) {
    }

    ConstraintType const getType() const {
        return type;
    }

};


#endif //PROJECT_CONSTRAINT_H
