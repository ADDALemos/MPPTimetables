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

    virtual int getLimit() const {};

    virtual void setLimit(int limit) {};

    virtual int getLimit1() const {};

    virtual void setLimit1(int limit1) {};

};


#endif //PROJECT_CONSTRAINT_H
