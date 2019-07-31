//
// Created by Alexandre Lemos on 09/07/2019.
//

#ifndef PROJECT_CLUSTERStudent_H
#define PROJECT_CLUSTERStudent_H


#include "Course.h"
#include "Student.h"

class ClusterStudent {
    int id;
    std::vector<Course *> course;
    std::vector<Student > student;

public:
    int getId(){ return id;}

    int numberofCourses() { return course.size(); }

    std::vector<Course *> getCourses() { return course; }

    int numberofStudent() { return student.size(); }

    std::vector<Student> getStudent() { return student; }


    void addStudent(Student s) {
        student.push_back(s);
    }


    ClusterStudent(int id, std::vector<Course *> course, Student s) : id(id), course(course) {
        student.push_back(s);

    }






};


#endif //PROJECT_CLUSTER_H
