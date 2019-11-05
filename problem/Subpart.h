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
    int overlap = 1;
public:
    int getOverlap() const {
        return overlap;
    }
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

    //TODO: implemet id before use
    void newShift(int numberNewShifts, int id) {
        for (int i = 0; i < numberNewShifts; ++i) {
            classes.push_back(
                    new Class(id++, averageClassLimit(), averageLectureLenght(), classes[0]->getStudent(),"UNUSED"));
        }

    }

    void deleteShift(int numberNewShifts) {
        if (numberNewShifts > classes.size())
            numberNewShifts = classes.size();
        for (int i = 0; i < numberNewShifts; ++i) {
            classes.erase(classes.begin() + i);
        }
    }

    int averageClassLimit() {
        int sum = 0;
        for (int i = 0; i < classes.size(); ++i) {
            sum += classes[i]->getLimit();
        }
        sum = sum / classes.size();
        return sum;

    }

    int averageLectureLenght() {
        int sum = 0;
        for (int i = 0; i < classes.size(); ++i) {
            sum += classes[i]->getLenght();
        }
        sum = sum / classes.size();
        return sum;
    }

    Class *getFirstParent() {
        return classes[0]->getParent();
    }

    bool existsParentClass(int id) {
        for (int i = 0; i < classes.size(); ++i) {
            if (classes[i]->getId() == id)
                return true;
        }
        return false;
    }

    /**
     * How many shift of this type are overlaped in the original solution
     */
    void isOverlapped() {
        int max = 1;
        std::map<int, int> map;
        for (Class *c: classes) {
            if (map.find(c->getSolStart()) == map.end())
                map.insert(std::pair<int, int>(c->getSolStart(), 1));
            else
                map[c->getSolStart()]++;
            if (max < map[c->getSolStart()])
                max = map[c->getSolStart()];

        }
        overlap = max;
    }


    void changeShift(int amount, int id) {
        std::cout << "Modified number of lectures by: " << amount << std::endl;
        if (amount < 0)
            deleteShift(amount);
        else
            newShift(amount, id++);

    }
};


#endif //TIMETABLER_SUBPART_H
