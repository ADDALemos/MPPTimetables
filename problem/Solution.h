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
    int penaltyTime = 0;
    int penaltyRoom = 0;

public:
    friend std::ostream &operator<<(std::ostream &os, const Solution &s) {
        os << " id " << s.lecture << " start: " << s.start << " duration: " << s.duration
           << " days: " << s.days << " week: " << s.week << " room: " << s.roomID
           << " penaltyTime: " << s.penaltyTime << " penaltyRoom: " << s.penaltyRoom;
        return os;
    }

    int penRoom() const {
        return penaltyRoom;
    }

    int penTime() const {
        return penaltyTime;
    }
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

    void updateSolution(int time, std::string week, std::string day, int duration) {
        start = time;
        this->week = week;
        this->days = day;
        this->duration = duration;
    }

    void updateSolution(int roomID, std::string room) {
        this->room=room;
        this->roomID=roomID;
    }

    Solution(int start, int roomID, std::string room,std::string week, std::string day):roomID(roomID),start(start),
    room(room), week(week), days(day){

    }

    Solution(int lecture, int start, int roomID, std::string week, std::string day, int duration, int penT, int penR)
            : lecture(lecture), duration(duration), roomID(roomID),
              start(start),  week(week), days(day), penaltyTime(penT), penaltyRoom(penR) {

    }


    Solution(int start, char *day) :start(start),days(day){

    }

    Solution(int start, std::string week, std::string day) :start(start),week(week),days(day) {

    }

    Solution(int roomID, std::string room, char *day, int start):roomID(roomID),start(start),room(room),days(day) {

    }
};


#endif //MPPTIMETABLES_SOLUTION_H
