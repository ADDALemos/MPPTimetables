//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_STUDENT_H
#define PROJECT_STUDENT_H


#include "Course.h"
#include <vector>

class Student {
    int id;
    std::vector<Course> course;

public:
    Student(int id, std::vector<Course, std::allocator<Course>> courses) : id(id), course(courses) {

    }
};


#endif //PROJECT_STUDENT_H
