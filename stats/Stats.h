//
// Created by Alexandre Lemos on 06/02/2019.
//

#ifndef PROJECT_STATS_H
#define PROJECT_STATS_H


#include <iostream>
#include "../problem/Instance.h"
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

#endif //PROJECT_STATS_H
