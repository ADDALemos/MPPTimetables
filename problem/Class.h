//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_CLASS_H
#define TIMETABLER_CLASS_H

#include <vector>
#include <utility>
#include <ostream>
#include <iostream>
#include "Lecture.h"
#include "Room.h"
#include "distribution.h"
#include "Solution.h"
#include <stdexcept>


class Class {
    bool modified = false;
    int id = 1;
    int orderID; //easy handle for the arrays of gurobi
    std::vector<int> conv;//legal time to time slot
    std::vector<distribution*> hard;
    std::vector<distribution*> soft;
    int limit;//limit number of students
    Class *parent = nullptr;
    Solution *sol;
    std::vector<int> student;

public:

    const int getNumSlots() const {
        return lectures.size();
    }

    const std::set<int> getSlots(int min) const {
        std::set<int> temp;
        for (unsigned int i = 0; i < lectures.size(); ++i) {
            temp.insert(lectures[i]->getStart() - min);
        }
        return temp;
    }
    
    
    const std::vector<int, std::allocator<int>> &getConv() const {
        return conv;
    }
    bool isTaughtWeek(int w) {
        return getWeeks()[w] == '1';
    }

    int getOrderID() const {
        return orderID;
    }

    void setOrderID(int orderID) {
        Class::orderID = orderID;
    }

    unsigned long getSteatedStudents() {
        return (limit > student.size() ? student.size() : (unsigned long) limit);
    }

    const std::vector<int, std::allocator<int>> &getStudent() const {
        return student;
    }

    void setStudent(const std::vector<int, std::allocator<int>> &student) {
        Class::student = student;
    }

    int getSolRoom() const {
        return sol->getSolRoom();
    }

    int getSolStart() const {
        return sol->getSolStart();
    }

    std::string getSolWeek() const {
        return sol->getSolWeek();
    }

    int getMaxWeek() {
        int max = 0;
        for (int i = 0; i < lectures.size(); ++i) {
            if (max < lectures[i]->getNumbWeeks())
                max = lectures[i]->getNumbWeeks();
        }
        return max;
    }

    std::string getSolDays() const {
        return sol->getSolDays();
    }

    int getSolDay() const {
        int i = 0;
        for (char &c : getSolDays()) {
            i++;
            if (c == '1')
                return i;

        }
        return -1;
    }

    void setSolRoom(int roomID) {
        sol->setSolRoom(roomID);
    }

    Class(int id, int limit, std::vector<Lecture *, std::allocator<Lecture *>> pLecture, std::map<Room, int> map,
          int order) : id(id), limit(limit), orderID(order) {
        lectures = pLecture;
        possibleRooms = map;
    }

    Class(int id, int limit, Lecture *pLecture, std::map<Room, int> map,
          int order) : id(id), limit(limit), orderID(order) {
        lectures.push_back(pLecture);
    }

    void updateStudentEnrollment(int change) {
        if (limit + change > 0)
            limit += change;
        else
            limit = 0;
        modified = true;

    }


    Class(int id, int limit, int lenght, std::vector<int> student) : id(id), limit(limit), student(student) {
        Lecture *l = new Lecture(lenght);
        lectures.push_back(l);

    }

private:
    std::vector<Lecture *> lectures;
    std::map<Room, int> possibleRooms;

public:
    void addRoom(Room r) {
        possibleRooms.insert(std::pair<Room, int>(r, 1));
    }

    Room getFirstPossibleRoom() {
        return possibleRooms.begin()->first;
    }

    Room getPossibleRoom(int n) {
        int i = 0;
        for (std::map<Room, int>::iterator it = possibleRooms.begin(); it != possibleRooms.end(); ++it) {
            if (i == n)
                return it->first;
            i++;
        }
        throw std::out_of_range("possibleRooms::at: key not found");
    }

    bool containsRoom(Room id) {
        return possibleRooms.find(id) != possibleRooms.end();
    }

    inline bool isModified() { return modified; }
    Class(int id, int limit, const std::vector<Lecture *, std::allocator<Lecture *>> &lectures,
          std::map<Room, int> possibleRooms) : id(id), limit(limit), lectures(lectures), possibleRooms(possibleRooms),
                                               parent(nullptr) {
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
        //throw "Legacy Mode";
        return lectures[0]->getLenght();
    }

    int getLenght(int t, int min) const {
        for (int i = 0; i < lectures.size(); ++i) {
            if (lectures[i]->getStart() - min == t)
                return lectures[i]->getLenght();
        }
        return 0;
    }
    int getStart() const {
        throw "Legacy Mode";
        return lectures[0]->getStart();
    }


    int getMaxStart() const {
        int max = 0;
        for (int i = 0; i < lectures.size(); ++i) {
            if ((lectures[i]->getStart() + lectures[i]->getLenght()) > max) {
                max = (lectures[i]->getStart() + lectures[i]->getLenght());
            }
        }
        return max;
    }

    int getMinStart() const {
        int min = getMaxStart();
        for (int i = 0; i < lectures.size(); ++i) {
            if (lectures[i]->getStart() < min) {
                min = lectures[i]->getStart();
            }
        }
        return min;
    }

    int getMaxDays() const {
        int max = 0;
        for (int i = 0; i < lectures.size(); ++i) {
            int j = 0;
            for (char &c : lectures[i]->getDays()) {
                if (c == '1' && j > max)
                    max = j;
                j++;
            }
        }
        return max;
    }

    int getMinDays() const {
        int min = getMaxDays();
        for (int i = 0; i < lectures.size(); ++i) {
            int j = 0;
            for (char &c : lectures[i]->getDays()) {
                if (c == '1' && j < min)
                    min = j;
                j++;
            }
        }
        return min;
    }

    int getDay() {
        throw "Legacy Mode";
        int i = 0;
        for (char &c : getDays()) {
            i++;
            if (c == '1')
                return i;

        }
        return -1;
    }

    std::string getDays() {
        throw "Legacy Mode";
        return lectures[0]->getDays();
    }

    /*std::string getWeek() {
        throw "Legacy Mode";
        return lectures[0]->getWeeks();
    }*/

    std::string getWeeks() const {
        std::string week = lectures[0]->getWeeks();
        for (int i = 0; i < lectures.size(); ++i) {
            int d = 0;
            for (const char &c :  lectures[i]->getWeeks()) {
                if (week[d] == '0' && c == '1') {
                    week[d] = '1';
                }
                d++;
            }

        }
        return week;
    }

    int getId() {
        return id;
    }

    void setId(int id) {
        Class::id = id;
    }

    int getLimit() const {
        return (limit > 0 ? limit : 1);
    }

    void setLimit(int limit) {
        Class::limit = (limit > 0 ? limit : 1);
    }

    const std::vector<Lecture *, std::allocator<Lecture *>> &getLectures() const {
        return lectures;
    }

    void setLectures(const std::vector<Lecture *, std::allocator<Lecture *>> &lectures) {
        Class::lectures = lectures;
    }

    std::map<Room, int> getPossibleRooms() {
        return possibleRooms;
    }

    int getPen(Room r) {
        return possibleRooms[r];
    }

    bool isActive(int w) {
        if (w != -1)
            return getWeeks()[w] == '1';
        return true;
    }

    void
    setPossibleRooms(const std::map<Room, int> &possibleRooms) {
        Class::possibleRooms = possibleRooms;
    }

    void converter() {
        for (int i = 0; i < lectures.size(); ++i) {
            conv.push_back(lectures[i]->getStart() - i);
        }
    }


    void addStudents(std::vector<int, std::allocator<int>> student) {
        Class::student = student;
    }

    void setSolution(int start, int roomID, std::string room, std::string week, std::string day) {
        sol= new Solution(start,roomID,room,week,day);
    }


    void setParent(Class *parent) {
        Class::parent = parent;

    }

    Class *getParent() const {
        return parent;
    }


    void setSolutionTime(int time, char *day) {
        sol = new Solution(time, day);
    }

    void setSolution(int time, std::string week, std::string day) {
        sol = new Solution(time,week, day);

    }

    void setSolution(Solution* s) {
        sol = s;

    }

    /**
     * Set Solution for ITC-2007
     */
    void setSolution(int roomID, std::string room, char *day, int time) {
        sol = new Solution(roomID,room, day,time);

    }

    void addStudent(int id) {
        Class::student.push_back(id);
    }
};


#endif //TIMETABLER_CLASS_H
