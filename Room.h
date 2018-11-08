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
    int id;
    int capacity;
    std::map<int,int> travel;//room time
    std::vector<Unavailability> slots;
public:

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

    Room(int i, int i1, std::map<int, int> map, std::vector<Unavailability, std::allocator<Unavailability>> vector)
            : capacity(i1), id(i), travel(map),slots(vector) {

    }
};


#endif //TIMETABLER_ROOM_H
