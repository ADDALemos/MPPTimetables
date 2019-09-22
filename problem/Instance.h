//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef TIMETABLER_INSTANCE_H
#define TIMETABLER_INSTANCE_H


#include <string>
#include <ostream>
#include <cmath>
#include <set>
#include "Course.h"
#include "Room.h"
#include "Student.h"

#include "ClusterStudents.h"
#include "Curriculum.h"

class Instance {
private:
    std::vector<Curriculum *> problem;
    std::vector<ClusterStudent> clusterStudent;
    std::map<std::string, Course *> courses;
    std::map<int, Room*> rooms;
    std::map<int, Student> student;
    std::vector<Class*> classes;
    int timePen;
    int roomPen;
    int distributionPen;
    std::string method = "Pseudo-Boolean";
    int ndays = -1;
    int nweek;
    int slotsperday;
    std::string name;
    int totalNumberSteatedStudent = -1;
    int min = -1, max = -1;
    unsigned int nClasses = 0;
    double alfa = 0;//slack anyone?
    //MPP
    std::set<int> uva;
    std::set<int> incorrentAssignments;
    //PB
    std::string costTime = " ";
    std::string costRoom = " ";
    std::string oneEachG = " ";
public:
    const std::vector<Curriculum *, std::allocator<Curriculum *>> &getProblem() const {
        return problem;
    }

    void setProblem(const std::vector<Curriculum *, std::allocator<Curriculum *>> &problem) {
        Instance::problem = problem;
    }
    const std::string &getCostTime() const {
        return costTime;
    }

    void setCostTime(const std::string &costTime) {
        Instance::costTime += costTime;
    }

    const std::string &getCostRoom() const {
        return costRoom;
    }

    void setCostRoom(const std::string &costRoom) {
        Instance::costRoom += costRoom;
    }

    const std::string &getOneEachG() const {
        return oneEachG;
    }

    void setOneEachG(const std::string &oneEachG) {
        Instance::oneEachG += oneEachG;
    }



    const std::string &getMethod() const {
        return method;
    }

    void setMethod(const std::string &method) {
        Instance::method = method;
    }


    int getTotalNumberSteatedStudent() const {
        return totalNumberSteatedStudent;
    }

    void setTotalNumberSteatedStudent(int totalNumberSteatedStudent) {
        Instance::totalNumberSteatedStudent = totalNumberSteatedStudent;
    }
    const std::map<std::string, Course *> &getCourses() const {
        return courses;
    }

    friend std::ostream &operator<<(std::ostream &os, const Instance &instance) {
        os << "ndays: " << instance.ndays << " nweek: " << instance.nweek << " slotsperday: " << instance.slotsperday
           << " name: " << instance.name << " timePen: " << instance.timePen << " roomPen: "
           << instance.roomPen << " distributionPen: " << instance.distributionPen << " studentPen: "
           << instance.studentPen;
        return os;
    }

    void setCourses(std::map<std::string, Course *> courses) {
        Instance::courses = courses;
    }


    std::map<int, Room*> &getRooms() {
        return rooms;
    }

    void setRooms(const std::map<int, Room*> &rooms) {
        Instance::rooms = rooms;
    }

     std::map<int, Student> &getStudent()  {
        return student;
    }

    void setStudent(const std::map<int, Student> &student) {
        Instance::student = student;
    }

    int getTimePen() const {
        return timePen;
    }

    void setTimePen(int timePen) {
        Instance::timePen = timePen;
    }

    int getRoomPen() const {
        return roomPen;
    }

    void setRoomPen(int roomPen) {
        Instance::roomPen = roomPen;
    }

    int getDistributionPen() const {
        return distributionPen;
    }

    void setDistributionPen(int distributionPen) {
        Instance::distributionPen = distributionPen;
    }

    int getStudentPen() const {
        return studentPen;
    }

    void setStudentPen(int studentPen) {
        Instance::studentPen = studentPen;
    }

    Instance(char *string): name(string) {
    }

    Instance(std::string name, int days, int slots, int min, int max) : ndays(days), nweek(1),
                                                                        slotsperday(slots), name(name), min(min),
                                                                        max(max) {

    }

private:
    int studentPen;


public:
    Instance(int ndays, int nweek, int slotsperday, const std::string &name) : ndays(ndays), nweek(nweek),
                                                                               slotsperday(slotsperday), name(name) {}

    int getNdays() const {
        return ndays;
    }

    void setNdays(int ndays) {
        Instance::ndays = ndays;
    }

    int getNweek() const {
        return nweek;
    }

    void setNweek(int nweek) {
        Instance::nweek = nweek;
    }

    int getSlotsperday() const {
        return slotsperday;
    }

    void setSlotsperday(int slotsperday) {
        Instance::slotsperday = slotsperday;
    }

    const std::string &getName() const {
        return name;
    }

    void setName(const std::string &name) {
        Instance::name = name;
    }


    void addRoom(Room *pRoom) {
        rooms.insert(std::pair<int, Room*>(pRoom->getId(), pRoom));

    }

    Room* getRoom(int roomID) {
        if (rooms.find(roomID) != rooms.end())
            return rooms.at(roomID);
        else {
            std::cerr << "Room does not exist: " << roomID << std::endl;
            std::exit(11);
        }

    }

    void addCourse(Course *pCourse) {
        courses.insert(std::pair<std::string, Course *>(pCourse->getName(), pCourse));

    }



    Course *getCourse(std::string courseID) {
        if (courses.find(courseID) != courses.end())
            return courses.at(courseID);
        else {
            std::cerr << "Course does not exist: " << courseID << std::endl;
            std::exit(11);
            return nullptr;
        }
    }

    void setNumClasses() {
        nClasses = computeNumClasses();

    }

    unsigned int getNumClasses() {
        return nClasses;
    }

    double timePerClass() {
        int c = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            c += getClasses()[i]->getLectures().size();
        }
        return c / getClasses().size();
    }

    double roomPerClass() {
        int c = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            c += getClasses()[i]->getPossibleRooms().size();
        }
        return c / getClasses().size();
    }

    std::vector<Class *> getClasses() {
        return classes;
    }

    Class *getClass(int i) {
        for (int j = 0; j < classes.size(); ++j) {
            if (classes[j]->getId() == i)
                return classes[j];
        }
        return nullptr;
    }

    void setClasses(std::vector<Class *> result) {
        classes=result;
        nClasses=classes.size();
    }

    unsigned int computeNumClasses() {
        classes = computeClasses();
        return classes.size();
    }

    std::vector<Class *> computeClasses() {
        std::vector<Class *> result;
        for (std::map<std::string, Course *>::iterator i = courses.begin(); i != courses.end(); ++i) {
            std::vector<Class *> temp = (*i).second->getClasses();
            result.insert(result.end(), temp.begin(), temp.end());
        }
        return result;

    }







    std::vector<Subpart *> getSubparts() {
        std::vector<Subpart *> result;
        for (std::map<std::string, Course *>::iterator i = courses.begin(); i != courses.end(); ++i) {
            std::vector<Subpart *> temp = (*i).second->getSubpart();
            result.insert(result.end(), temp.begin(), temp.end());
        }
        return result;

    }

    double getAlfa() {
        return alfa;
    }

    Class *getClassbyId(int id) {
        for (int j = 0; j < getClasses().size(); ++j) {
            if (getClasses()[j]->getId() == id)
                return getClasses()[j];
        }

        std::cerr << "Class does not exist: " << id << std::endl;
        std::exit(11);

    }

    Class *getClassbyOrder(int order) {
        return getClasses()[order];

    }

    Student &getStudent(int id) {
        if (student.find(id) != student.end())
            return student.at(id);
        else {
            std::cerr << "Student does not exist: " << id << std::endl;
            std::exit(11);
        }
    }

    void updateStudentEnrollment(int classes, int change) {
        for (std::map<std::string, Course *>::iterator it = courses.begin(); it != courses.end(); ++it) {
            if (it->second->getClass(classes) != nullptr) {
                it->second->getClass(classes)->updateStudentEnrollment(change);
                break;
            }
        }

    }

    int getNumRoom() {
        return rooms.size();
    }

    void blockRoom(int roomID) {
        rooms.at(roomID)->block();

    }

    bool isRoomBlocked(int roomID) {
        if (rooms.find(roomID) != rooms.end())
            return rooms.at(roomID)->isClose();
        else {
            std::cerr << "Room does not exist: " << roomID << std::endl;
            std::exit(11);
        }
    }

    void blockRoombyDay(int roomID, std::string day) {
        rooms.at(roomID)->block(day);

    }

    bool isRoomBlockedbyDay(int roomID, int day) {
        return rooms.at(roomID)->isClosebyDay(day);
    }

    bool isTimeUnavailable(int slot) {
        return uva.find(slot) != uva.end();
    }

    void setTimeUnavailable(int time) {
        uva.insert(time);
    }

    void incorrectAssignment(int classID) {
        incorrentAssignments.insert(classID);
    }

    bool isIncorrectAssignment(int classID) {
        return incorrentAssignments.find(classID) != incorrentAssignments.end();

    }

    int averageCoursesperStudent() {
        int course = 0;
        for (auto it = std::begin(student); it != std::end(student); ++it) {
            course += it->second.getCourse().size();
        }
        return course;
    }

    int averageClassesperStudent() {
        int classes = 0;
        for (auto it = std::begin(student); it != std::end(student); ++it) {
            classes += it->second.getClasses().size();
        }
        return classes;
    }

    int averageSubpartsperCourse() {
        int sub = 0;
        for (auto it = std::begin(courses); it != std::end(courses); ++it) {
            sub += it->second->getConfiguratons().size();
        }
        return sub;
    }

    void findOverlapConstraints() {
        for (std::map<std::string, Course *>::iterator i = courses.begin(); i != courses.end(); ++i) {
            i->second->findOverlapConstraints();
        }

    }

    /**
     * Average Room Capacity
     */
    double getAvCapacity() {
        double d = 0;
        for (int i = 1; i <= getNumRoom(); ++i) {
            d += getRoom(i)->getCapacity();
        }
        return d / getNumRoom();
    }

    /**
     * Standard Deviation Room Capacity
     * @return standard deviation
     */

    double getSTDCapacity() {
        int mean = getAvCapacity();
        double d = 0;
        for (int i = 1; i <= getNumRoom(); ++i) {
            d += std::pow(getRoom(i)->getCapacity() - mean, 2);
        }
        return std::sqrt(d / (getNumRoom() - 1));
    }

    /**
     * Average Lectures Lenght
     */
    double getAvLenght() {
        double d = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            d += getClasses()[i]->getLenght();
        }
        return d / getClasses().size();
    }

    /**
     * Standard Deviation Lectures Lenght
     * @return standard deviation
     */

    double getSTDLenght() {
        double mean = getAvLenght();
        double d = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            d += std::pow(getClasses()[i]->getLenght() - mean, 2);
        }
        return std::sqrt(d / (getClasses().size() - 1));
    }

    /**
     * Average Lectures Lenght
     */
    double getAvEnrollment() {
        double d = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            d += getClasses()[i]->getLimit();
        }
        return d / getClasses().size();
    }

    /**
     * Standard Deviation Lectures Lenght
     * @return standard deviation
     */

    double getSTDEnrollment() {
        int mean = getAvEnrollment();
        double d = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            d += std::pow(getClasses()[i]->getLimit() - mean, 2);
        }
        return std::sqrt(d / (getClasses().size() - 1));
    }

    /*
     * Number of students enrolled
     */

    double frequency() {
        double used = 0, all = (getNdays() * getSlotsperday() * getNumRoom());
        for (int k = 0; k < getClasses().size(); ++k) {
            used += getClasses()[k]->getLenght();
        }
        return 100 * used / all;
    }


    double utilization() {
        double used = 0, all = (getNdays() * getSlotsperday() * getNumRoom() *
                                getAvCapacity());
        for (int k = 0; k < getClasses().size(); ++k) {
            used += getClasses()[k]->getLenght() * getClasses()[k]->getLimit();
        }
        return 100 * used / all;

    }

    Subpart *getSubpart(int subpartID) {
        if (getSubparts().size() <= subpartID) {
            std::cerr << "Subpart does not exist: " << subpartID << std::endl;
            std::exit(11);
        }
        return getSubparts()[subpartID];
    }

    int maxTimeSlot() {
        int max = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            if ((getClasses()[i]->getMaxStart() + getClasses()[i]->getLenght()) > max) {
                max = (getClasses()[i]->getMaxStart() + getClasses()[i]->getLenght());
            }
        }
        return max;
    }

    int minTimeSlot() {
        int min = maxTimeSlot();
        for (int i = 0; i < getClasses().size(); ++i) {
            if (getClasses()[i]->getMinStart() < min) {
                min = getClasses()[i]->getMinStart();
            }
        }
        return min;
    }

    int actualSpace() {
        return maxTimeSlot() - minTimeSlot() + 1;
    }

    int maxDay() {
        int max = 0;
        for (int i = 0; i < getClasses().size(); ++i) {
            if (getClasses()[i]->getMaxDays() > max)
                max = getClasses()[i]->getMaxDays();
        }
        return max;
    }

    int minDay() {
        int min = maxDay();
        for (int i = 0; i < getClasses().size(); ++i) {
            if (getClasses()[i]->getMinDays() < min)
                min = getClasses()[i]->getMinDays();
        }
        return min;
    }

    int actualSpaceDay() {
        return maxDay() - minDay() + 1;
    }

    double frequencyCorrected() {
        double used = 0, all = (actualSpaceDay() * actualSpace() * getNumRoom());
        for (int k = 0; k < getClasses().size(); ++k) {
            used += getClasses()[k]->getLenght();
        }
        return 100 * used / all;
    }


    double utilizationCorrected() {
        double used = 0, all = (actualSpaceDay() * actualSpace() * getNumRoom() *
                                getAvCapacity());
        for (int k = 0; k < getClasses().size(); ++k) {
            used += getClasses()[k]->getLenght() * getClasses()[k]->getLimit();
        }
        return 100 * used / all;

    }

    void compact() {
        slotsperday = actualSpace();
        ndays = actualSpaceDay();
    }

    void setStudentCluster(std::vector<ClusterStudent, std::allocator<ClusterStudent>> clusterStudent) {
        Instance::clusterStudent=clusterStudent;
    }

    std::vector<ClusterStudent> getClusterStudent(){
        return clusterStudent;
    }
};


#endif //TIMETABLER_INSTANCE_H
