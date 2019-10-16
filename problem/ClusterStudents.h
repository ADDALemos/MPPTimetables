//
// Created by Alexandre Lemos on 09/07/2019.
//

#ifndef PROJECT_CLUSTERStudent_H
#define PROJECT_CLUSTERStudent_H


#include "Course.h"
#include "Student.h"

class ClusterStudent {
    int id;
    int minimo;
    std::vector<Course *> course;
    std::set<Student > student;
    std::map<int,std::string> classesID;

public:
    int getMin(){ return minimo;}

    void setMin(int n){  minimo=n;}

    const std::string getClassesID(int i)  {
        if(classesID.find(i)!=classesID.end())
            return classesID[i];
        return "Empty";
    }

    void setClassesID(int i, std::string name)  {
        classesID.insert(std::pair<int,std::string>(i,name));
    }

    int getId(){ return id;}

    int numberofCourses() { return course.size(); }

    std::vector<Course *> getCourses() { return course; }

    int numberofStudent() { return student.size(); }

    std::set<Student> getStudent() { return student; }


    void addStudent(Student s) {
        student.insert(s);
    }


    ClusterStudent(int id, std::vector<Course *, std::allocator<Course *>> course, Student s, int minimo)
            : id(id), course(course),minimo(minimo) {
        student.insert(s);

    }






};


#endif //PROJECT_CLUSTER_H
