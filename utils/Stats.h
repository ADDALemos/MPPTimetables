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


inline void printConstraintsStat(Instance *instance) {
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

    for(auto i: instance->getDist()) {
        for (int y = 0; y < i.second.size(); ++y) {
            if (i.second[y]->getWeight() == -1)
                hard++;
            else {
                soft++;
                if (i.first.compare("SameAttendees") == 0) {
                    map["SameAttendees"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("NotOverlap") == 0) {
                    map["NotOverlap"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("Overlap") == 0) {
                    map["Overlap"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameTime") == 0) {
                    map["SameTime"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentTime") == 0) {
                    map["DifferentTime"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameWeeks") == 0) {
                    map["SameWeeks"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentWeeks") == 0) {
                    map["DifferentWeeks"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameDays") == 0) {
                    map["SameDays"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentDays") == 0) {
                    map["DifferentDays"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("Precedence") == 0) {
                    map["Precedence"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameRoom") == 0) {
                    map["SameRoom"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentRoom") == 0) {
                    map["DifferentRoom"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameStart") == 0) {
                    map["SameStart"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxDays") == 0) {
                    map["MaxDays"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MinGap") == 0) {
                    map["MinGap"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("WorkDay") == 0) {
                    map["WorkDay"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxDayLoad") == 0) {
                    map["MaxDayLoad"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxBreaks") == 0) {
                    map["MaxBreaks"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxBlock") == 0) {
                    map["MaxBlock"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }

            }
        }
    }
        std::cout<<hard<<" "<<soft<<std::endl;
        for (std::map<std::string, int>::iterator p = map.begin(); p != map.end(); ++p) {
            std::cout << p->first << " " << p->second << std::endl;
        }

}


inline void printDomainSize(Instance *instance){
    int y=0,maxValue=0;
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        if(instance->getClasses()[i]->getPossiblePairSize()>maxValue)
            maxValue=instance->getClasses()[i]->getPossiblePairSize();
        y+=instance->getClasses()[i]->getPossiblePairSize();
    }
    std::cout<<(y/instance->getClasses().size())<<" "<<maxValue<<std::endl;

    std::cout<<y<<" "<<instance->getClasses().size()<<std::endl;
}

#endif //PROJECT_STATS_H
