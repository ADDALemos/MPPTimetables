//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_ROOM_H
#define TIMETABLER_ROOM_H

#include <vector>
#include <utility>
#include <ostream>
#include <map>
#include "Unavailability.h"

class Room {
private:
    int id;
    std::string name;
    int capacity;
    bool close = false;
    std::map<int,int> travel;//room time
    std::vector<Unavailability> slots;
public:
    bool isClosebyDay(int day) {
        if (isClose())
            return true;
        for (int i = 0; i < slots.size(); ++i) {
            int k = 0;
            for (char &c :slots[i].getDays()) {
                if (c == '1') {
                    if (k == day)
                        return true;
                }
                k++;
            }

        }
        return false;
    }
    bool isClose() const {
        return close;
    }

    void setClose(bool close) {
        Room::close = close;
    }

    int getId() const {
        return id;
    }

    void setId(int id) {
        Room::id = id;
    }

    int getCapacity() const {
        return capacity;
    }

    void setCapacity(int capacity) {
        Room::capacity = capacity;
    }



    const std::vector<Unavailability, std::allocator<Unavailability>> &getSlots() const {
        return slots;
    }

    void setSlots(const std::vector<Unavailability, std::allocator<Unavailability>> &slots) {
        Room::slots = slots;
    }

    friend std::ostream &operator<<(std::ostream &os, const Room &room) {
        os << "id: " << room.id << " capacity: " << room.capacity ;
        return os;
    }

    const std::map<int, int> &getTravel() const {
        return travel;
    }

    void setTravel(const std::map<int, int> &travel) {
        Room::travel = travel;
    }

    inline bool operator<(const Room &rhs) const {
        return id < rhs.id;
    }

    Room(int i, std::string name, int i1, std::map<int, int> map,
         std::vector<Unavailability, std::allocator<Unavailability>> vector)
            : id(i), name(name), capacity(i1), travel(map), slots(vector) {

    }

    void block() {
        setClose(true);
    }

    void block(std::string day) {
        slots.push_back(Unavailability(day));
    }
};


#endif //TIMETABLER_ROOM_H
