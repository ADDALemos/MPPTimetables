//
// Created by Alexandre Lemos on 09/07/2019.
//

#ifndef PROJECT_CLUSTERBYROOM_H
#define PROJECT_CLUSTERBYROOM_H


#include "Class.h"
#include "Room.h"

class ClusterbyRoom {
    int id;
    std::set<Class *> classes;
    std::map<Class *, int> map;
    std::set<Room*> rooms;

public:

    int numberofClasses() { return classes.size(); }

    std::set<Class *> getClasses() { return classes; }

    std::set<Room*> getRooms() { return rooms; }

    int getClusterID() { return id; }

    void addClass(Class *c, int p) {
        classes.insert(c);
        map.insert(std::pair<Class *, int>(c, p));
    }

    void addRoom(Room* r, Class *c, int p) { rooms.insert(r);        map.insert(std::pair<Class *, int>(c, p));
    }

    void addRoom(Room* r) { rooms.insert(r);
    }

    ClusterbyRoom(int id, std::set<Room*> r,Class * c):rooms(r){ classes.insert(c);

    }

    ClusterbyRoom(int id, std::set<Room*> r) : id(id), rooms(r) {

    }

    ClusterbyRoom(int id, Class *c, int p, Room* r) : id(id) {
        addClass(c, p);
        rooms.insert(r);
    }

    int getP(Class *c) {
        return map[c];

    }


};


#endif //PROJECT_CLUSTER_H
