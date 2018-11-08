//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_INSTANCE_H
#define TIMETABLER_INSTANCE_H


#include <string>
#include <ostream>
#include "Course.h"
#include "distribution.h"
#include "Room.h"
#include "Student.h"

class Instance {
    int ndays;
    int nweek;
    int slotsperday;
    std::string name;
    std::map<int, Course> courses;
    std::map<int, distribution> dist;
    std::map<int, Room> rooms;
    std::map<int, Student> student;
    int timePen;
    int roomPen;
    int distributionPen;
public:
    const std::map<int, Course> &getCourses() const {
        return courses;
    }

    friend std::ostream &operator<<(std::ostream &os, const Instance &instance) {
        os << "ndays: " << instance.ndays << " nweek: " << instance.nweek << " slotsperday: " << instance.slotsperday
           << " name: " << instance.name << " timePen: " << instance.timePen << " roomPen: "
           << instance.roomPen << " distributionPen: " << instance.distributionPen << " studentPen: "
           << instance.studentPen;
        return os;
    }

    void setCourses(const std::map<int, Course> &courses) {
        Instance::courses = courses;
    }

    const std::map<int, distribution> &getDist() const {
        return dist;
    }

    void setDist(const std::map<int, distribution> &dist) {
        Instance::dist = dist;
    }

    const std::map<int, Room> &getRooms() const {
        return rooms;
    }

    void setRooms(const std::map<int, Room> &rooms) {
        Instance::rooms = rooms;
    }

    const std::map<int, Student> &getStudent() const {
        return student;
    }

    void setStudent(const std::map<int, Student> &student) {
        Instance::student = student;
    }

    int getTimePen() const {
        return timePen;
    }

    void setTimePen(int timePen) {
        Instance::timePen = timePen;
    }

    int getRoomPen() const {
        return roomPen;
    }

    void setRoomPen(int roomPen) {
        Instance::roomPen = roomPen;
    }

    int getDistributionPen() const {
        return distributionPen;
    }

    void setDistributionPen(int distributionPen) {
        Instance::distributionPen = distributionPen;
    }

    int getStudentPen() const {
        return studentPen;
    }

    void setStudentPen(int studentPen) {
        Instance::studentPen = studentPen;
    }

    Instance(char *string): name(string) {

    }

private:
    int studentPen;


public:
    Instance(int ndays, int nweek, int slotsperday, const std::string &name) : ndays(ndays), nweek(nweek),
                                                                               slotsperday(slotsperday), name(name) {}

        int getNdays() const {
        return ndays;
    }

    void setNdays(int ndays) {
        Instance::ndays = ndays;
    }

    int getNweek() const {
        return nweek;
    }

    void setNweek(int nweek) {
        Instance::nweek = nweek;
    }

    int getSlotsperday() const {
        return slotsperday;
    }

    void setSlotsperday(int slotsperday) {
        Instance::slotsperday = slotsperday;
    }

    const std::string &getName() const {
        return name;
    }

    void setName(const std::string &name) {
        Instance::name = name;
    }


    void addRoom(Room *pRoom) {
        std::pair<int,Room> *p= new std::pair<int, Room>(pRoom->getId(),*pRoom);
        rooms.insert(*p);

    }
};


#endif //TIMETABLER_INSTANCE_H
