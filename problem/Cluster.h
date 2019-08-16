//
// Created by Alexandre Lemos on 09/07/2019.
//

#ifndef PROJECT_CLUSTER_H
#define PROJECT_CLUSTER_H


#include "Class.h"
#include "Room.h"

class Cluster {
    int id;
    std::vector<Class *> classes;
    std::map<Class *, int> map;
    Room* rooms;

public:

    int numberofClasses() { return classes.size(); }

    std::vector<Class *> getClasses() { return classes; }

    Room* getRooms() { return rooms; }

    int getClusterID() { return id; }

    void addClass(Class *c, int p) {
        classes.push_back(c);
        map.insert(std::pair<Class *, int>(c, p));
    }

    void addRoom(Room* r) { rooms = r; }

    Cluster(int id, Room* r) : id(id), rooms(r) {

    }

    Cluster(int id, Class *c, int p, Room* r) : id(id), rooms(r) {
        addClass(c, p);
    }

    int getP(Class *c) {
        return map[c];

    }


};


#endif //PROJECT_CLUSTER_H
