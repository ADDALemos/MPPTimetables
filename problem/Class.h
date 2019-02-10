//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_CLASS_H
#define TIMETABLER_CLASS_H

#include <vector>
#include <utility>
#include <ostream>
#include "Lecture.h"
#include "Room.h"

class Class {
    bool modified = false;
    int id;
    int limit;//limit number of students
    int parent;

private:
    //Solution
    int roomID = -1;
    int start = -1;
    std::string room = " ";
    std::string week = "1";
    std::string days = "0";
    std::vector<int> student;

public:
    int getSteatedStudents() {
        return (limit > student.size() ? student.size() : limit);
    }

    const std::vector<int, std::allocator<int>> &getStudent() const {
        return student;
    }

    void setStudent(const std::vector<int, std::allocator<int>> &student) {
        Class::student = student;
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

    std::string getSolDays() const {
        return days;
    }

    void setSolRoom(int roomID) {
        Class::roomID = roomID;
    }

    Class(int id, int limit, Lecture *pLecture) : id(id), limit(limit) {
        lectures.push_back(pLecture);
    }

    void updateStudentEnrollment(int change) {
        if (limit + change > 0)
            limit += change;
        else
            limit = 0;
        modified = true;

    }


    Class(int limit, int lenght, std::vector<int> student) : limit(limit), student(student) {
        Lecture *l = new Lecture(lenght);
        lectures.push_back(l);

    }

private:
    std::vector<Lecture *> lectures;
    std::map<Room, int> possibleRooms;

public:
    inline bool isModified() { return modified; }
    Class(int id, int limit, const std::vector<Lecture *, std::allocator<Lecture *>> &lectures,
          std::map<Room, int> possibleRooms) : id(id), limit(limit), lectures(lectures), possibleRooms(possibleRooms),
                                               parent(-1) {
    }


    friend std::ostream &operator<<(std::ostream &os, const Class &aClass) {
        os << "id: " << aClass.id << " limit: " << aClass.limit ;
        return os;
    }

    int getLenght() const {
        if (lectures.size() == 0) {
            std::cerr << "Lecture cannot be schedule id: " << id << std::endl;
            return 0;
        }
        return lectures[0]->getLenght();
    }

    int getStart() const {
        return lectures[0]->getStart();
    }

    std::string getDays() const {
        return lectures[0]->getDays();
    }

    std::string getWeek() const {
        return lectures[0]->getWeeks();
    }

    int getId() const {
        return id;
    }

    void setId(int id) {
        Class::id = id;
    }

    int getLimit() const {
        return limit;
    }

    void setLimit(int limit) {
        Class::limit = limit;
    }

    const std::vector<Lecture *, std::allocator<Lecture *>> &getLectures() const {
        return lectures;
    }

    void setLectures(const std::vector<Lecture *, std::allocator<Lecture *>> &lectures) {
        Class::lectures = lectures;
    }

    const std::map<Room, int> &getPossibleRooms() const {
        return possibleRooms;
    }

    void
    setPossibleRooms(const std::map<Room, int> &possibleRooms) {
        Class::possibleRooms = possibleRooms;
    }




    void addStudents(std::vector<int, std::allocator<int>> student) {
        Class::student = student;
    }

    void setSolution(int start, int roomID, std::string room, std::string week, std::string day) {
        Class::start = start;
        Class::roomID = roomID;
        Class::room = room;
        Class::week = week;
        Class::days = day;
    }


    void setParent(int parent) {
        Class::parent = parent;

    }

    int getParent() const {
        return parent;
    }


    void setSolutionTime(int time, char *day) {
        Class::start = time;
        Class::days = day;
    }

    void setSolution(int time, std::string week, std::string day) {
        Class::start = time;
        Class::days = day;
        Class::week = week;

    }

    /**
     * Set Solution for ITC-2007
     */
    void setSolution(int roomID, std::string room, char *day, int time) {
        Class::start = time;
        Class::days = day;
        Class::roomID = roomID;
        Class::room = room;

    }
};


#endif //TIMETABLER_CLASS_H
