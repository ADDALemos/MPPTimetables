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
    map.insert(std::pair<std::string, int>("SameAttendees", 0));
    map.insert(std::pair<std::string, int>("NotOverlap", 0));
    map.insert(std::pair<std::string, int>("Overlap", 0));
    map.insert(std::pair<std::string, int>("SameTime", 0));
    map.insert(std::pair<std::string, int>("DifferentTime", 0));
    map.insert(std::pair<std::string, int>("SameWeeks", 0));
    map.insert(std::pair<std::string, int>("DifferentWeeks", 0));
    map.insert(std::pair<std::string, int>("SameDays", 0));
    map.insert(std::pair<std::string, int>("DifferentDays", 0));
    map.insert(std::pair<std::string, int>("Precedence", 0));
    map.insert(std::pair<std::string, int>("SameRoom", 0));
    map.insert(std::pair<std::string, int>("DifferentRoom", 0));
    map.insert(std::pair<std::string, int>("SameStart", 0));
    map.insert(std::pair<std::string, int>("MaxDays", 0));
    map.insert(std::pair<std::string, int>("MinGap", 0));
    map.insert(std::pair<std::string, int>("WorkDay", 0));
    map.insert(std::pair<std::string, int>("MaxDayLoad", 0));
    map.insert(std::pair<std::string, int>("MaxBreaks", 0));
    map.insert(std::pair<std::string, int>("MaxBlock", 0));


    for (int i = 0; i < instance->getDist().size(); ++i) {
        if (instance->getDist()[i]->getType().getType() == SameAttendees) {
            map["SameAttendees"]++;
        } else if (instance->getDist()[i]->getType().getType() == NotOverlap) {
            map["NotOverlap"]++;
        } else if (instance->getDist()[i]->getType().getType() == Overlap) {
            map["Overlap"]++;
        } else if (instance->getDist()[i]->getType().getType() == SameTime) {
            map["SameTime"]++;
        } else if (instance->getDist()[i]->getType().getType() == DifferentTime) {
            map["DifferentTime"]++;
        } else if (instance->getDist()[i]->getType().getType() == SameWeeks) {
            map["SameWeeks"]++;
        } else if (instance->getDist()[i]->getType().getType() == DifferentWeeks) {
            map["DifferentWeeks"]++;
        } else if (instance->getDist()[i]->getType().getType() == SameDays) {
            map["SameDays"]++;
        } else if (instance->getDist()[i]->getType().getType() == DifferentDays) {
            map["DifferentDays"]++;
        } else if (instance->getDist()[i]->getType().getType() == Precedence) {
            map["Precedence"]++;
        } else if (instance->getDist()[i]->getType().getType() == SameRoom) {
            map["SameRoom"]++;
        } else if (instance->getDist()[i]->getType().getType() == DifferentRoom) {
            map["DifferentRoom"]++;
        } else if (instance->getDist()[i]->getType().getType() == SameStart) {
            map["SameStart"]++;
        } else if (instance->getDist()[i]->getType().getType() == MaxDays) {
            map["MaxDays"]++;
        } else if (instance->getDist()[i]->getType().getType() == MinGap) {
            map["MinGap"]++;
        } else if (instance->getDist()[i]->getType().getType() == WorkDay) {
            map["WorkDay"]++;
        } else if (instance->getDist()[i]->getType().getType() == MaxDayLoad) {
            map["MaxDayLoad"]++;
        } else if (instance->getDist()[i]->getType().getType() == MaxBreaks) {
            map["MaxBreaks"]++;
        } else if (instance->getDist()[i]->getType().getType() == MaxBlock) {
            map["MaxBlock"]++;
        }

    }
    for (std::map<std::string, int>::iterator p = map.begin(); p != map.end(); ++p) {
        std::cout << p->first << " " << p->second << std::endl;
    }
}

#endif //PROJECT_STATS_H
