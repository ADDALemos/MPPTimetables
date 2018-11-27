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
    int ndays = -1;
    double alfa = 1;
    int nweek;
    int slotsperday;
    std::string name;
    int totalNumberSteatedStudent = -1;
    int min = -1, max = -1;


private:
    std::map<std::string, Course *> courses;
    std::vector<distribution *> dist;
    std::map<int, Room> rooms;
    std::map<int, Student> student;
    int timePen;
    int roomPen;
    int distributionPen;
public:
    int getTotalNumberSteatedStudent() const {
        return totalNumberSteatedStudent;
    }

    void setTotalNumberSteatedStudent(int totalNumberSteatedStudent) {
        Instance::totalNumberSteatedStudent = totalNumberSteatedStudent;
    }
    const std::map<std::string, Course *> &getCourses() const {
        return courses;
    }

    friend std::ostream &operator<<(std::ostream &os, const Instance &instance) {
        os << "ndays: " << instance.ndays << " nweek: " << instance.nweek << " slotsperday: " << instance.slotsperday
           << " name: " << instance.name << " timePen: " << instance.timePen << " roomPen: "
           << instance.roomPen << " distributionPen: " << instance.distributionPen << " studentPen: "
           << instance.studentPen;
        return os;
    }

    void setCourses(const std::map<std::string, Course *> &courses) {
        Instance::courses = courses;
    }

    const std::vector<distribution *> &getDist() const {
        return dist;
    }

    void setDist(const std::vector<distribution *> &dist) {
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

    Instance(std::string name, int days, int slots, int min, int max) : ndays(days), nweek(1),
                                                                        slotsperday(slots), name(name), min(min),
                                                                        max(max) {

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

    Room getRoom(int roomID) {
        return rooms.at(roomID);

    }

    void addCourse(Course *pCourse) {
        std::pair<std::string, Course *> *p = new std::pair<std::string, Course *>(pCourse->getName(), pCourse);
        courses.insert(*p);

    }

    void addDistribution(distribution *pConstraint) {
        dist.push_back(pConstraint);
    }

    Course *getCourse(std::string courseID) {
        return courses.at(courseID);
    }

    unsigned int getNumClasses() {
        unsigned int size = 0;
        for (std::map<std::string, Course *>::iterator i = courses.begin(); i != courses.end(); ++i) {
            size += (*i).second->getNumClasses();
        }
        return size;
    }

    std::vector<Class *> getClasses() {
        std::vector<Class *> result;
        for (std::map<std::string, Course *>::iterator i = courses.begin(); i != courses.end(); ++i) {
            std::vector<Class *> temp = (*i).second->getClasses();
            result.insert(result.end(), temp.begin(), temp.end());
        }
        return result;

    }

    double getAlfa() {
        return alfa;
    }

    Class *getClass(int id) {
        for (std::map<std::string, Course *>::iterator i = courses.begin(); i != courses.end(); ++i) {
            std::vector<Class *> temp = (*i).second->getClasses();
            for (int j = 0; j < temp.size(); ++j) {
                if (temp[j]->getId() == id)
                    return temp[j];
            }
        }
        return nullptr;
    }

    Student &getStudent(int id) {
        return student.at(id);
    }
};


#endif //TIMETABLER_INSTANCE_H
