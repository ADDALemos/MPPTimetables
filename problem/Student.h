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

    const bool containsClass(Class *c) const {
        for (int i = 0; i < getClasses().size(); ++i) {
            if (getClasses()[i]->getId() == c->getId())
                return true;
        }
        return false;
    }

    void addClass(Class *c) {
        Student::classes.push_back(c);
    }

    Class *getClass(int c) {
        return Student::classes[c];
    }

    bool isEnrolled(int idClass) {
        for (int i = 0; i < classes.size(); ++i) {
            if (classes[i]->getId() == idClass)
                return true;
        }
        return false;
    }

public:
    Student(int id, std::vector<Course *, std::allocator<Course *>> courses) : id(id), course(courses) {

    }
};


#endif //PROJECT_STUDENT_H
