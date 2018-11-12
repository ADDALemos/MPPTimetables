//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_COURSE_H
#define TIMETABLER_COURSE_H

#include "Subpart.h"
#include <map>

class Course {

    std::string name;
    std::map<int, std::vector<Subpart *>> configuration;

public:
    const std::string &getName() const {
        return name;
    }

    void setName(const std::string &name) {
        Course::name = name;
    }

    const std::map<int, std::vector<Subpart *>> &getConfiguratons() const {
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


};


#endif //TIMETABLER_COURSE_H
