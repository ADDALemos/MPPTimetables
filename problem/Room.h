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
#include "Time.h"

class Room {
private:
    int id = -1;
    std::string name;
    std::string type;
    int capacity;
    bool close = false;
    std::map<int,int> travel;//room time
    std::vector<Unavailability> slots;
public:
    //std::map<int,std::vector<std::pair<int, int>>> t;
    std::vector<Time*> t;


    //Legacy
    bool isClosebyDay(int day) {
        if (isClose())
            return true;
        for (unsigned int  i = 0; i < slots.size(); ++i) {
            int k = 0;
            for (const char &c :slots[i].getDays()) {
                if (c == '1') {
                    if (k == day)
                        return true;
                }
                k++;
            }

        }
        return false;
    }

    const std::string &getName() const {
        return name;
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

    const std::string &getType() const {
        return type;
    }

    void setType(const std::string &type) {
        Room::type = type;
    }

    void setCapacity(int capacity) {
        Room::capacity = capacity;
    }



    std::vector<Unavailability, std::allocator<Unavailability>> &getSlots()  {
        return slots;
    }

    void setSlots(const std::vector<Unavailability, std::allocator<Unavailability>> &slots) {
        Room::slots = slots;
    }

    friend std::ostream &operator<<(std::ostream &os, const Room &room) {
        os << "id: " << room.id << " capacity: " << room.capacity ;
        return os;
    }

    const int getTravel(int r) const {
        if (travel.find(r) != travel.end())
            return travel.at(r);
        return 0;
    }

     std::map<int, int> &getTravel()  {
        return travel;
    }

    void setTravel(const std::map<int, int> &travel) {
        Room::travel = travel;
    }

    inline bool operator<(const Room &rhs) const {
        return id < rhs.id;
    }

    Room(int i, std::string name, int i1, std::map<int, int> map,
         std::vector<Unavailability, std::allocator<Unavailability>> vector, std::string string)
            : id(i), name(name), capacity(i1), travel(map), slots(vector), type(string) {

    }

    void block() {
        setClose(true);
    }

    void block(std::string day) {
        slots.push_back(Unavailability(day));
    }

    Room(int id, char *name, int capcacity) : id(id), name(name), capacity(capcacity) {

    }

    Room(int fakeRoom) : id(fakeRoom) {

    }
};


#endif //TIMETABLER_ROOM_H
