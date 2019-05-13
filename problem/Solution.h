//
// Created by alexa on 5/10/2019.
//

#ifndef MPPTIMETABLES_SOLUTION_H
#define MPPTIMETABLES_SOLUTION_H


#include <string>

class Solution {
    //Solution
    int lecture =-1;
    int duration =-1;
    int roomID = -1;
    int start = -1;
    std::string room = "-1";
    std::string week = "-1";
    std::string days = "-1";

public:
    int getDuration() const {
        return duration;
    }

    void setDuration(int duration) {
        Solution::duration = duration;
    }
    std::string getSolDays() const {
        return days;
    }

    int getSolRoom() const {
        return roomID;
    }

    int getSolStart() const {
        return start;
    }

    std::string getSolWeek() const {
        return week;
    }

    void setSolRoom(int roomID) {
        Solution::roomID=roomID;
    }

    int getLecture() const {
        return lecture;
    }

    void setLecture(int lecture) {
        Solution::lecture = lecture;
    }

    Solution(int start, int roomID, std::string room,std::string week, std::string day):start(start),roomID(roomID),
    room(room), week(week), days(day){

    }

    Solution(int lecture, int start, int roomID, std::string week, std::string day, int duration) : duration(duration),
                                                                                                    lecture(lecture),
                                                                                                    start(start), roomID(roomID), week(week), days(day){

    }

    Solution(int start, char *day) :start(start),days(day){

    }

    Solution(int start, std::string week, std::string day) :start(start),week(week),days(day) {

    }

    Solution(int roomID, std::string room, char *day, int start):roomID(roomID),room(room),days(day),start(start) {

    }
};


#endif //MPPTIMETABLES_SOLUTION_H
