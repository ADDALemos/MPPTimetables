//
// Created by Alexandre Lemos on 09/07/2019.
//

#ifndef PROJECT_CLASSBYROOM_H
#define PROJECT_CLASSBYROOM_H


#include <ostream>
#include "Class.h"
#include "Room.h"

class ClassbyRoom {
    int id=-1;
    std::vector<Class *> classes;
    std::map<Class *, int> map;
    Room* rooms;

public:

    int numberofClasses() { return classes.size(); }

    std::vector<Class *> getClasses() { return classes; }

    Room* getRooms() { return rooms; }

    friend std::ostream &operator<<(std::ostream &os, const ClassbyRoom &room) {
        os << "id: " << room.id << " classes: " << room.classes.size() << " map: " << room.map.size() << " rooms: " << room.rooms->getId();
        return os;
    }

    int getClusterID() { return id; }

    void addClass(Class *c, int p) {
        classes.push_back(c);
        map.insert(std::pair<Class *, int>(c, p));
    }

    void addRoom(Room* r) { rooms = r; }

    ClassbyRoom(int id, Room* r) : id(id), rooms(r) {

    }

    ClassbyRoom(int id, Class *c, int p, Room* r) : id(id), rooms(r) {
        addClass(c, p);
    }

    int getP(Class *c) {
        return map[c];

    }


};


#endif //PROJECT_CLUSTER_H
