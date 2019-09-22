//
// Created by Alexandre Lemos on 09/07/2019.
//

#ifndef PROJECT_CLUSTERBYROOM_H
#define PROJECT_CLUSTERBYROOM_H


#include "Class.h"
#include "Room.h"
#include "ConstraintShort.h"

class ClusterbyRoom {
    int id;
    std::set<Class *> classes;
    std::vector<ConstraintShort*> range;
    std::set<Room*> rooms;

public:

    void setRange(ConstraintShort* newRange){ range.push_back(newRange);}

    std::vector<ConstraintShort*>  getRange(){ return range;}

    int numberofClasses() { return classes.size(); }

    std::set<Class *> getClasses() { return classes; }

    std::set<Room*> getRooms() { return rooms; }

    int getClusterID() { return id; }

    void addClass(Class *c) {
        classes.insert(c);
    }

    void addRoom(Room* r, Class *c) { rooms.insert(r);
    }

    void addRoom(Room* r) { rooms.insert(r);
    }

    ClusterbyRoom(int id, std::set<Room*> r,Class * c):rooms(r){ classes.insert(c);

    }

    ClusterbyRoom(int id, std::set<Room*> r) : id(id), rooms(r) {

    }

    ClusterbyRoom(int id, Class *c, Room* r) : id(id) {
        rooms.insert(r);
    }



};


#endif //PROJECT_CLUSTER_H
