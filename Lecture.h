//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_LECTURE_H
#define TIMETABLER_LECTURE_H


#include <ostream>

class Lecture {
    std::string days;
    int start;
    int lenght;
    std::string weeks;
    int penalty;
 public:
    Lecture(int lenght, int start, std::string weeks, std::string days, int penalty) : days(days), start(start),
                                                                                       lenght(lenght),
                                                                                       weeks(weeks),
                                                                                       penalty(penalty) {}

    friend std::ostream &operator<<(std::ostream &os, const Lecture &lecture) {
        os << "days: " << lecture.days << " start: " << lecture.start << " lenght: " << lecture.lenght << " weeks: "
           << lecture.weeks << " penalty: " << lecture.penalty;
        return os;
    }
};


#endif //TIMETABLER_LECTURE_H
