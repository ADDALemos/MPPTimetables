//
// Created by Alexandre Lemos on 05/08/2019.
//

#ifndef PROJECT_TIME_H
#define PROJECT_TIME_H


#include <vector>
#include <string>
#include <ostream>
#include <iostream>
#include "Lecture.h"

class Time {
    int start;
    int end;
    std::string day;
    std::string week;
    std::vector<int> classesCid;
    std::vector<std::string > classesC;
public:


    void addC(std::string c,int idC){
        classesC.push_back(c);
        classesCid.push_back(idC);

    }

    int getStart() const {
        return start;
    }

    void setStart(int start) {
        Time::start = start;
    }

    int getEnd() const {
        return end;
    }

    void setEnd(int end) {
        Time::end = end;
    }





    Time(int start, int end, const std::string &week, std::string &day, std::string clID, int idC) : start(start), end(end),day(day),week(week) {
        classesC.push_back(clID);
        classesCid.push_back(idC);
    }

    const std::vector<std::string> &getClassesC() const {
        return classesC;
    }
    const std::vector<int> &getClassesCid() const {
        return classesCid;
    }

    bool checkWD(Lecture *p1, int nw, int nd) {
        for (int j = 0; j < nw; ++j) {
            if (p1->getWeeks()[j] == week[j] &&
                p1->getWeeks()[j] == '1') {
                for (int d = 0; d < nd; ++d) {
                    if (p1->getDays()[d] == day[d] &&
                        p1->getDays()[d] == '1') {
                            return true;
                    }
                }
            }
        }
        return false;
    }


    bool check(Time *p1, int nw, int nd) {
        for (int j = 0; j < nw; ++j) {
            if (p1->getWeek()[j] == week[j] &&
                p1->getWeek()[j] == '1') {
                for (int d = 0; d < nd; ++d) {
                    if (p1->getDay()[d] == day[d] &&
                        p1->getDay()[d] == '1') {
                        if (p1->getStart() >= start &&
                            p1->getStart() <
                            end) {
                            return true;
                        } else if ( start >= p1->getStart() &&
                                   start < p1->getEnd()) {
                            return true;
                        }
                    }
                }
            }
        }


        return false;
    }

    const std::string &getDay() const {
        return day;
    }

    void setDay(const std::string &day) {
        Time::day = day;
    }

    const std::string &getWeek() const {
        return week;
    }

    void setWeek(const std::string &week) {
        Time::week = week;
    }

    friend std::ostream &operator<<(std::ostream &os, const Time &time1) {
        os << "start: " << time1.start << " end: " << time1.end << " day: " << time1.day << " week: " << time1.week;
        return os;
    }

};


#endif //PROJECT_TIME_H
