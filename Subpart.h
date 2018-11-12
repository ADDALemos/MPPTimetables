//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_SUBPART_H
#define TIMETABLER_SUBPART_H

#include "Class.h"
#include <vector>


class Subpart {
    std::string id;
    std::vector<Class *> classes;
public:
    const std::string &getId() const {
        return id;
    }

    void setId(const std::string &id) {
        Subpart::id = id;
    }

    const std::vector<Class *, std::allocator<Class *>> &getClasses() const {
        return classes;
    }

    void setClasses(const std::vector<Class *, std::allocator<Class *>> &classes) {
        Subpart::classes = classes;
    }

public:
    Subpart(std::string id, std::vector<Class *, std::allocator<Class *>> classV) : id(id), classes(classV) {

    }
};


#endif //TIMETABLER_SUBPART_H
