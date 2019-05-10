//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_CONSTRAINT_H
#define TIMETABLER_CONSTRAINT_H

enum ConstraintType {
    NotOverlap,
    Overlap,
    Precedence,
    SameAttendees,
    SameStart,
    SameTime,
    DifferentTime,
    SameDays,
    DifferentDays,
    SameWeeks,
    DifferentWeeks,
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

    virtual int getLimit() const { throw "Not Implemented"; };

    virtual void setLimit(int limit) { throw "Not Implemented"; };

    virtual int getLimit1() const { throw "Not Implemented"; };

    virtual void setLimit1(int limit1) { throw "Not Implemented"; };

};


#endif //PROJECT_CONSTRAINT_H
