//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_COURSE_H
#define TIMETABLER_COURSE_H

#include "Subpart.h"
#include <map>

class Course {

    std::string name;
    std::map<int, std::vector<Subpart>> configuratons;

public:
    const std::string &getName() const {
        return name;
    }

    void setName(const std::string &name) {
        Course::name = name;
    }

    const std::map<int, std::vector<Subpart>> &getConfiguratons() const {
        return configuratons;
    }

    void setConfiguratons(const std::map<int, std::vector<Subpart>> &configuratons) {
        Course::configuratons = configuratons;
    }


};


#endif //TIMETABLER_COURSE_H
