//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_STUDENT_H
#define PROJECT_STUDENT_H


#include "Course.h"
#include "Class.h"
#include <vector>

class Student {
    int id;
    std::vector<Course *> course;
    std::vector<Class *> classes;
public:
    const std::vector<Class *, std::allocator<Class *>> &getClasses() const {
        return classes;
    }

    void setClasses(const std::vector<Class *, std::allocator<Class *>> &classes) {
        Student::classes = classes;
    }

public:
    int getId() const {
        return id;
    }

    void setId(int id) {
        Student::id = id;
    }

    const std::vector<Course *, std::allocator<Course *>> &getCourse() const {
        return course;
    }

    void setCourse(const std::vector<Course *, std::allocator<Course *>> &course) {
        Student::course = course;
    }

public:
    Student(int id, std::vector<Course *, std::allocator<Course *>> courses) : id(id), course(courses) {

    }
};


#endif //PROJECT_STUDENT_H
