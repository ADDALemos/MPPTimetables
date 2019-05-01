//
// Created by Alexandre Lemos on 06/02/2019.
//

#ifndef PROJECT_STATS_H
#define PROJECT_STATS_H


#include <iostream>
#include "../problem/Instance.h"
#include "../problem/Constraint.h"
#include "../solver/ILPExecuter.h"


void printProblemStats(Instance *instance) {
    std::cout << "*****Problem Description*****" << std::endl;
    std::cout << "Number of Days: " << instance->getNdays() << std::endl;
    std::cout << "Number of Slots: " << instance->getSlotsperday() << std::endl;
    std::cout << "Number of Students: " << instance->getStudent().size() << std::endl;
    std::cout << "Number of Courses: " << instance->getCourses().size() << std::endl;
    std::cout << "Number of Classes by Week: " << instance->getClasses().size() << std::endl;
    std::cout << "Average Lenght of Classes: " << instance->getAvLenght() << std::endl;
    std::cout << "Standard Deviation Lenght by Classes: " << instance->getSTDLenght() << std::endl;
    std::cout << "Average Enrollment of Classes: " << instance->getAvEnrollment() << std::endl;
    std::cout << "Standard Deviation Enrollment by Classes: " << instance->getSTDEnrollment() << std::endl;
    std::cout << "Number of Rooms: " << instance->getNumRoom() << std::endl;
    std::cout << "Average Capacity by Room: " << instance->getAvCapacity() << std::endl;
    std::cout << "Standard Deviation Capacity by Room: " << instance->getSTDCapacity() << std::endl;
}

void printStats(Instance *instance) {
    std::cout << "*****Timetable Description*****" << std::endl;
    std::cout << "Frequency: " << instance->frequency() << std::endl;
    std::cout << "Utilization: " << instance->utilization() << std::endl;
}

void printSolutionStats(ILPExecuter *runner) {
    std::cout << "*****Solution Description*****" << std::endl;
    std::cout << "Frequency: " << runner->frequency() << std::endl;
    std::cout << "Utilization: " << runner->utilization() << std::endl;
}


void printCorrectedStats(Instance *instance) {
    std::cout << "*****Timetable Description with Correction*****" << std::endl;
    std::cout << "Number of Slots: " << instance->actualSpace() << std::endl;
    std::cout << "Number of Days: " << instance->actualSpaceDay() << std::endl;
    std::cout << "Frequency: " << instance->frequencyCorrected() << std::endl;
    std::cout << "Utilization: " << instance->utilizationCorrected() << std::endl;
}

void printWeekStats(Instance *instance) {
    std::cout << "Number of Classes " << instance->getClasses().size() << std::endl;
    int value = 0;
    for (int i = 0; i < instance->getNweek(); ++i) {
        value += instance->getClassesWeek(i).size();
        std::cout << "Number of Classes in " << i << " week " << instance->getClassesWeek(i).size() << std::endl;
    }
    std::cout << "Number of Classes by Week: " << ((double) value / instance->getNweek()) << std::endl;
    int max = 0;
    for (int j = 0; j < instance->getClasses().size(); ++j) {
        if (instance->getClasses()[j]->getMaxWeek() > max)
            max = instance->getClasses()[j]->getMaxWeek();
    }
    std::cout << "Max number of Week in a single class: " << (double) max / instance->getNweek() << std::endl;
    std::ofstream myfile;
    myfile.open("example1.csv");
    myfile << "source, target" << std::endl;
    for (int i = 0; i < 2; ++i) {
        for (Class *c: instance->getClassesWeek(i))
            for (Class *c1: instance->getClassesWeek(i))
                if (c1->getId() != c->getId())
                    myfile << c->getId() << "," << c1->getId() << std::endl;
    }
    myfile.close();


}

void constraints(Instance *instance) {
    std::map<std::string, int> map;
    for (int i = 0; i < instance->getDist().size(); ++i) {
        if (instance->getDist()[i]->getType().getType() == SameAttendees) {
            if (map.find("SameAttendees") != map.end()) {
                map["SameAttendees"]++;
            } else {
                map.insert(std::pair<std::string, int>("SameAttendees", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == NotOverlap) {
            if (map.find("Overlap") != map.end()) {
                map["Overlap"]++;
            } else {
                map.insert(std::pair<std::string, int>("Overlap", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == Overlap) {
            if (map.find("Overlap") != map.end()) {
                map["Overlap"]++;
            } else {
                map.insert(std::pair<std::string, int>("Overlap", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == SameTime) {
            if (map.find("SameTime") != map.end()) {
                map["SameTime"]++;
            } else {
                map.insert(std::pair<std::string, int>("SameTime", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == DifferentTime) {
            if (map.find("DifferentTime") != map.end()) {
                map["DifferentTime"]++;
            } else {
                map.insert(std::pair<std::string, int>("DifferentTime", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == SameWeeks) {
            if (map.find("SameWeeks") != map.end()) {
                map["SameWeeks"]++;
            } else {
                map.insert(std::pair<std::string, int>("SameWeeks", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == DifferentWeeks) {
            if (map.find("DifferentWeeks") != map.end()) {
                map["DifferentWeeks"]++;
            } else {
                map.insert(std::pair<std::string, int>("DifferentWeeks", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == SameDays) {
            if (map.find("SameDays") != map.end()) {
                map["SameDays"]++;
            } else {
                map.insert(std::pair<std::string, int>("SameDays", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == DifferentDays) {
            if (map.find("DifferentDays") != map.end()) {
                map["DifferentDays"]++;
            } else {
                map.insert(std::pair<std::string, int>("DifferentDays", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == Precedence) {
            if (map.find("Precedence") != map.end()) {
                map["Precedence"]++;
            } else {
                map.insert(std::pair<std::string, int>("Precedence", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == SameRoom) {
            if (map.find("SameRoom") != map.end()) {
                map["SameRoom"]++;
            } else {
                map.insert(std::pair<std::string, int>("SameRoom", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == DifferentRoom) {
            if (map.find("DifferentRoom") != map.end()) {
                map["DifferentRoom"]++;
            } else {
                map.insert(std::pair<std::string, int>("DifferentRoom", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == SameStart) {
            if (map.find("SameStart") != map.end()) {
                map["SameStart"]++;
            } else {
                map.insert(std::pair<std::string, int>("SameStart", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == MaxDays) {
            if (map.find("MaxDays") != map.end()) {
                map["MaxDays"]++;
            } else {
                map.insert(std::pair<std::string, int>("MaxDays", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == MinGap) {
            if (map.find("MinGap") != map.end()) {
                map["MinGap"]++;
            } else {
                map.insert(std::pair<std::string, int>("MinGap", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == WorkDay) {
            if (map.find("WorkDay") != map.end()) {
                map["WorkDay"]++;
            } else {
                map.insert(std::pair<std::string, int>("WorkDay", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == MaxDayLoad) {
            if (map.find("MaxDayLoad") != map.end()) {
                map["MaxDayLoad"]++;
            } else {
                map.insert(std::pair<std::string, int>("MaxDayLoad", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == MaxBreaks) {
            if (map.find("MaxBreaks") != map.end()) {
                map["MaxBreaks"]++;
            } else {
                map.insert(std::pair<std::string, int>("MaxBreaks", 1));
            }
        } else if (instance->getDist()[i]->getType().getType() == MaxBlock) {
            if (map.find("MaxBlock") != map.end()) {
                map["MaxBlock"]++;
            } else {
                map.insert(std::pair<std::string, int>("MaxBlock", 1));
            }
        }

    }
    for (std::map<std::string, int>::iterator p = map.begin(); p != map.end(); ++p) {
        std::cout << p->first << " " << p->second << std::endl;
    }
}

#endif //PROJECT_STATS_H
