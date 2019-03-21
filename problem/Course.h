//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_COURSE_H
#define TIMETABLER_COURSE_H

#include "Subpart.h"
#include <map>

class Course {

    std::string name;
    std::string teacherID;
    std::map<int, std::vector<Subpart *>> configuration;

public:
    const std::string &getName() const {
        return name;
    }

    void setName(const std::string &name) {
        Course::name = name;
    }

    std::map<int, std::vector<Subpart *>> &getConfiguratons() {
        return configuration;
    }

    void setConfiguratons(const std::map<int, std::vector<Subpart *>> &configuratons) {
        Course::configuration = configuratons;
    }


    Course(char *id, std::map<int, std::vector<Subpart *, std::allocator<Subpart *>>> config) : name(id),
                                                                                                configuration(config) {

    }

    std::vector<Class *> getClasses() {
        std::vector<Class *> result;
        for (std::map<int, std::vector<Subpart *>>::iterator i = configuration.begin();
             i != configuration.end(); i++) {
            for (int j = 0; j < (*i).second.size(); ++j) {
                std::vector<Class *> temp = (*i).second[j]->getClasses();
                result.insert(result.end(), temp.begin(), temp.end());
            }
        }
        return result;

    }

    std::vector<Class *> getClassesWeek(int w) {
        std::vector<Class *> result;
        for (std::map<int, std::vector<Subpart *>>::iterator i = configuration.begin();
             i != configuration.end(); i++) {
            for (int j = 0; j < (*i).second.size(); ++j) {
                std::vector<Class *> temp = (*i).second[j]->getClassesWeek(w);
                result.insert(result.end(), temp.begin(), temp.end());
            }
        }
        return result;

    }

    std::vector<Subpart *> getSubpart() {
        std::vector<Subpart *> result;
        for (std::map<int, std::vector<Subpart *>>::iterator i = configuration.begin();
             i != configuration.end(); i++) {
            std::vector<Subpart *> temp = (*i).second;
            result.insert(result.end(), temp.begin(), temp.end());

        }
        return result;

    }


    int getNumClasses() {
        unsigned int size = 0;
        for (std::map<int, std::vector<Subpart *>>::iterator i = configuration.begin();
             i != configuration.end(); i++) {
            for (int j = 0; j < (*i).second.size(); ++j) {
                size += (*i).second[j]->getClasses().size();

            }
        }
        return size;
    }


    Course(char *id, char *teacherID, int numLect, int min_days, int student, char *doubleLec, int count) : name(id),
                                                                                                            teacherID(
                                                                                                                    teacherID) {
        Subpart *p = new Subpart(std::to_string(count));
        std::vector<Class *> list;
        for (int i = 0; i < numLect; ++i) {
            Lecture *l = new Lecture(count, doubleLec);
            Class *c = new Class(count, student, l);
            list.push_back(c);
            count++;
        }
        p->setClasses(list);
        std::vector<Subpart *> sub;
        sub.push_back(p);
        configuration.insert(std::pair<int, std::vector<Subpart *>>(0, sub));

    }

    Class *getClass(int id) {
        for (std::map<int, std::vector<Subpart *>>::iterator it = configuration.begin();
             it != configuration.end(); ++it) {
            for (int i = 0; i < it->second.size(); ++i) {
                if (it->second[i]->getClass(id) != nullptr)
                    return it->second[i]->getClass(id);
            }
        }
        return nullptr;
    }

    int getNumShifts() {
        return configuration.begin()->second.size();
    }

    void newShift(int subpart, int numberNewShifts) {
        configuration.begin()->second[subpart]->newShift(numberNewShifts);
        int subpartID = subpart;
        while ((subpart = findParentSubpart(subpartID)) != -1) {
            configuration.begin()->second[subpart]->newShift(numberNewShifts);
            //configuration.begin()->second[subpartID]->setParent(subpart);TODO:parent part
            subpartID = subpart;
        }

    }

    void deleteShift(int subpartID, int amount) {
        configuration.begin()->second[subpartID]->deleteShift(amount);

    }

    int findParentSubpart(int subpart) {
        if (configuration.begin()->second[subpart]->getFirstParent() == -1)
            return -1;
        for (auto it = configuration.begin(); it != configuration.end(); ++it) {
            for (int i = 0; i < it->second.size(); ++i) {
                if (it->second[i]->existsParentClass(configuration.begin()->second[subpart]->getFirstParent()))
                    return i;

            }

        }
        return -1;
    }

    void findOverlapConstraints() {
        for (std::map<int, std::vector<Subpart *>>::iterator it = configuration.begin();
             it != configuration.end(); ++it) {
            for (int i = 0; i < it->second.size(); ++i) {
                it->second[i]->isOverlapped();
            }
        }

    }

    /**
     * Add solution for old encodings
     */
    void addSol(int lectureID, int idRoom, std::string room, char *day, int slot) {
        (configuration.begin()->second)[0]->getClass(lectureID)->setSolution(idRoom, room, day, slot);

    }

};


#endif //TIMETABLER_COURSE_H
