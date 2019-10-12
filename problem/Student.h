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
    std::map<int, std::string> var;
public:
    Student() {}

    void init(){
        for (Class *c: classes) {
            var.insert(std::pair<int,std::string>(c->getId(),"stu_"+std::to_string(c->getId())+"_"+std::to_string(id)));
        }

    }

    std::string varLimit(int classID){
        return var[classID];
    }

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

    const int getNumbSubpart() const {
        int value = 0;
        for (int i = 0; i < course.size(); ++i) {
            value += course[i]->getNumShifts();
        }
        return value;
    }

    bool operator<(const Student &rhs) const {
        return id < rhs.id;
    }

    bool operator==(const Student &rhs) const {
        return id == rhs.id &&
               course == rhs.course &&
               classes == rhs.classes &&
               var == rhs.var;
    }

    bool operator!=(const Student &rhs) const {
        return !(rhs == *this);
    }

public:
    Student(int id, std::vector<Course *, std::allocator<Course *>> courses) : id(id), course(courses) {

    }
};


#endif //PROJECT_STUDENT_H
