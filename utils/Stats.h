//
// Created by Alexandre Lemos on 06/02/2019.
//

#ifndef PROJECT_STATS_H
#define PROJECT_STATS_H


#include <iostream>
#include "../problem/Instance.h"

inline void printStudentsStats(Instance *instance){
    int v =0;int v1=0;
    for (std::map<int, Student>::iterator it = instance->getStudent().begin(); it != instance->getStudent().end(); ++it) {
        std::map<int, Student>::iterator it1 = it; it1++;
        for (; it1 != instance->getStudent().end(); ++it1) {
            if(it->second.getId()!=it1->second.getId()){
            bool t=false;
            for (int c= 0; c < it->second.getCourse().size(); c++) {
                for (int c1= 0; c1 < it1->second.getCourse().size(); c1++) {
                    if(it->second.getCourse()[c]->getName().compare(it1->second.getCourse()[c1]->getName())!=0){
                        t=true;
                        break;
                    }
                }
                if(t)
                    break;

            }
            if(!t) {
                v++;
                std::cout<<it->second.getId()<<" "<<it1->second.getId()<<std::endl;
            }
                v1++;

            }
        }
    }
    std::cout<<v<<std::endl;
    std::cout<<(float)v/instance->getStudent().size()<<std::endl;
}


inline void printProblemStats(Instance *instance) {
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
    std::cout << "Time option per Class: " << instance->timePerClass() << std::endl;
    std::cout << "Room option per Class: " << instance->roomPerClass() << std::endl;


}

inline void printStats(Instance *instance) {
    std::cout << "*****Timetable Description*****" << std::endl;
    std::cout << "Frequency: " << instance->frequency() << std::endl;
    std::cout << "Utilization: " << instance->utilization() << std::endl;
}




inline void printCorrectedStats(Instance *instance) {
    std::cout << "*****Timetable Description with Correction*****" << std::endl;
    std::cout << "Number of Slots: " << instance->actualSpace() << std::endl;
    std::cout << "Number of Days: " << instance->actualSpaceDay() << std::endl;
    std::cout << "Frequency: " << instance->frequencyCorrected() << std::endl;
    std::cout << "Utilization: " << instance->utilizationCorrected() << std::endl;
}


/*void constraints(Instance *instance) {
    int hard = 0, soft = 0;
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
        if (instance->getDist()[i]->getPenalty() == -1)
            hard++;
        else
            soft++;
        if (instance->getDist()[i]->getType()->getType() == SameAttendees) {
            map["SameAttendees"]++;
        } else if (instance->getDist()[i]->getType()->getType() == NotOverlap) {
            map["NotOverlap"]++;
        } else if (instance->getDist()[i]->getType()->getType() == Overlap) {
            map["Overlap"]++;
        } else if (instance->getDist()[i]->getType()->getType() == SameTime) {
            map["SameTime"]++;
        } else if (instance->getDist()[i]->getType()->getType() == DifferentTime) {
            map["DifferentTime"]++;
        } else if (instance->getDist()[i]->getType()->getType() == SameWeeks) {
            map["SameWeeks"]++;
        } else if (instance->getDist()[i]->getType()->getType() == DifferentWeeks) {
            map["DifferentWeeks"]++;
        } else if (instance->getDist()[i]->getType()->getType() == SameDays) {
            map["SameDays"]++;
        } else if (instance->getDist()[i]->getType()->getType() == DifferentDays) {
            map["DifferentDays"]++;
        } else if (instance->getDist()[i]->getType()->getType() == Precedence) {
            map["Precedence"]++;
        } else if (instance->getDist()[i]->getType()->getType() == SameRoom) {
            map["SameRoom"]++;
        } else if (instance->getDist()[i]->getType()->getType() == DifferentRoom) {
            map["DifferentRoom"]++;
        } else if (instance->getDist()[i]->getType()->getType() == SameStart) {
            map["SameStart"]++;
        } else if (instance->getDist()[i]->getType()->getType() == MaxDays) {
            map["MaxDays"]++;
        } else if (instance->getDist()[i]->getType()->getType() == MinGap) {
            map["MinGap"]++;
        } else if (instance->getDist()[i]->getType()->getType() == WorkDay) {
            map["WorkDay"]++;
        } else if (instance->getDist()[i]->getType()->getType() == MaxDayLoad) {
            map["MaxDayLoad"]++;
        } else if (instance->getDist()[i]->getType()->getType() == MaxBreaks) {
            map["MaxBreaks"]++;
        } else if (instance->getDist()[i]->getType()->getType() == MaxBlock) {
            map["MaxBlock"]++;
        }

    }
    for (std::map<std::string, int>::iterator p = map.begin(); p != map.end(); ++p) {
        std::cout << p->first << " " << p->second << std::endl;
    }
}*/

#endif //PROJECT_STATS_H
