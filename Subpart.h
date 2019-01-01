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

    Subpart(std::string id) : id(id) {

    }

    Subpart(std::string id, std::vector<Class *, std::allocator<Class *>> classV) : id(id), classes(classV) {

    }

    Class *getClass(int id) {
        for (int i = 0; i < classes.size(); ++i) {
            if (classes[i]->getId() == id)
                return classes[i];
        }
        return nullptr;
    }

    void newShift(int numberNewShifts) {
        for (int i = 0; i < numberNewShifts; ++i) {
            //classes.push_back(new Class());//TODO: Actually create a new class
        }

    }

    void deleteShift(int numberNewShifts) {
        if (numberNewShifts > classes.size())
            numberNewShifts = classes.size();
        for (int i = 0; i < numberNewShifts; ++i) {
            classes.erase(classes.begin() + i);
        }


    }
};


#endif //TIMETABLER_SUBPART_H
