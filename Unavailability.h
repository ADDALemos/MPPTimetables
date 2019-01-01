//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_UNAVAILABILITY_H
#define TIMETABLER_UNAVAILABILITY_H


#include <ostream>

class Unavailability {
    std::string days;
    int start;
    int lenght;
    std::string weeks;
public:
    Unavailability(const std::string &days, int start, int lenght, const std::string &weeks) : days(days), start(start),
                                                                                               lenght(lenght),
                                                                                               weeks(weeks) {}

    friend std::ostream &operator<<(std::ostream &os, const Unavailability &unavailability) {
        os << "days: " << unavailability.days << " start: " << unavailability.start << " lenght: "
           << unavailability.lenght << " weeks: " << unavailability.weeks;
        return os;
    }

    std::string &getDays() {
        return days;
    }

    Unavailability(std::string day) : days(day) {


    }
};


#endif //TIMETABLER_UNAVAILABILITY_H
