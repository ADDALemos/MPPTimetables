//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_CLASS_H
#define TIMETABLER_CLASS_H

#include <vector>
#include <utility>
#include <ostream>
#include "Lecture.h"

class Class {
    int id;
    int limit;//limit number of students
    Class *parent;
    std::vector<Lecture> lectures;
    std::vector<std::pair<Room,int>> possibleRooms;

public:
    Class(int id, int limit, const std::vector<Lecture, std::allocator<Lecture>> &lectures,
          const std::vector<std::pair<Room, int>, std::allocator<std::pair<Room, int>>> &possibleRooms) : id(id),
                                                                                                          limit(limit),
                                                                                                          lectures(
                                                                                                                  lectures),
                                                                                                          possibleRooms(
                                                                                                                  possibleRooms),
                                                                                                            parent(nullptr){}


    friend std::ostream &operator<<(std::ostream &os, const Class &aClass) {
        os << "id: " << aClass.id << " limit: " << aClass.limit ;
        return os;
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

    const std::vector<Lecture, std::allocator<Lecture>> &getLectures() const {
        return lectures;
    }

    void setLectures(const std::vector<Lecture, std::allocator<Lecture>> &lectures) {
        Class::lectures = lectures;
    }

    const std::vector<std::pair<Room, int>, std::allocator<std::pair<Room, int>>> &getPossibleRooms() const {
        return possibleRooms;
    }

    void
    setPossibleRooms(const std::vector<std::pair<Room, int>, std::allocator<std::pair<Room, int>>> &possibleRooms) {
        Class::possibleRooms = possibleRooms;
    }


};


#endif //TIMETABLER_CLASS_H
