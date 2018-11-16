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
    int id;
    int limit;//limit number of students
    Class *parent;
    //Solution
    int roomID = -1;
    int start = -1;
    char *week = "0";
    char *days = "0";
    std::vector<int> student;

public:

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

    char *getSolWeek() const {
        return week;
    }

    char *getSolDays() const {
        return days;
    }

    void setSolRoom(int roomID) {
        Class::roomID = roomID;
    }

private:
    std::vector<Lecture *> lectures;
    std::map<Room, int> possibleRooms;
    ///TODO: Save solution

public:
    Class(int id, int limit, const std::vector<Lecture *, std::allocator<Lecture *>> &lectures,
          std::map<Room, int> possibleRooms) : id(id), limit(limit), lectures(lectures), possibleRooms(possibleRooms),
                                               parent(nullptr) {
    }


    friend std::ostream &operator<<(std::ostream &os, const Class &aClass) {
        os << "id: " << aClass.id << " limit: " << aClass.limit ;
        return os;
    }

    int getLenght() const {
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


    void setSolution(int start, int room, char *weeks, char *days) {
        Class::roomID = room;
        Class::start = start;
        Class::week = weeks;
        Class::days = days;

    }

    void addStudents(std::vector<int, std::allocator<int>> student) {
        Class::student = student;
    }
};


#endif //TIMETABLER_CLASS_H
