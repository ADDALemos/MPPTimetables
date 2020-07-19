//
// Created by Alexandre Lemos on 06/02/2019.
//

#ifndef PROJECT_STATS_H
#define PROJECT_STATS_H


#include <iostream>
#include "../problem/Instance.h"
#include "../MaxSATFormula.h"


inline void printMaxSATStats(Instance *instance, openwbo::MaxSATFormula *f) {
    std::cout << f->nVars() << " " << f->nCard() << " " << f->nHard() << " " << f->nSoft() << std::endl;

}

inline double getStudentsStats(Instance *instance) {
    int v =0;int v1=0;
    for (std::map<int, Student>::iterator it = instance->getStudent().begin(); it != instance->getStudent().end(); ++it) {
        std::map<int, Student>::iterator it1 = it; it1++;
        for (; it1 != instance->getStudent().end(); ++it1) {
            if(it->second.getId()!=it1->second.getId()){
                bool t = false;
                for (int c = 0; c < it->second.getCourse().size(); c++) {
                    for (int c1 = 0; c1 < it1->second.getCourse().size(); c1++) {
                        if (it->second.getCourse()[c]->getName().compare(it1->second.getCourse()[c1]->getName()) != 0) {
                            t = true;
                            break;
                        }
                    }
                    if (t)
                        break;

                }
                if (!t) {
                    v++;
                    //std::cout<<it->second.getId()<<" "<<it1->second.getId()<<std::endl;
                }
                v1++;

            }
        }
    }
    //std::cout<<v<<std::endl;
    return (float) v / instance->getStudent().size();
}


inline void printStatforTable(Instance *instance) {
    //|Co|	|C|	|R|	AVG. |R_c|	AVG. |P_c|	|S|	AVG. |S_c|	Hard	Soft	MaxBreak	MaxBlock
    std::cout << instance->getCourses().size() << " " <<
              instance->getClasses().size() << " " <<
              instance->getRooms().size() << " " <<
              instance->roomPerClass() << " " <<
              instance->timePerClass() << " " <<
              instance->getStudent().size() << " " <<
              getStudentsStats(instance) << " ";

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
            }
                if (i.first.compare("SameAttendees") == 0) {
                    map["SameAttendees"] ++;std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("NotOverlap") == 0) {
                    map["NotOverlap"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("Overlap") == 0) {
                    map["Overlap"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameTime") == 0) {
                    map["SameTime"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentTime") == 0) {
                    map["DifferentTime"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameWeeks") == 0) {
                    map["SameWeeks"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentWeeks") == 0) {
                    map["DifferentWeeks"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameDays") == 0) {
                    map["SameDays"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentDays") == 0) {
                    map["DifferentDays"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("Precedence") == 0) {
                    map["Precedence"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameRoom") == 0) {
                    map["SameRoom"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("DifferentRoom") == 0) {
                    map["DifferentRoom"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("SameStart") == 0) {
                    map["SameStart"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxDays") == 0) {
                    map["MaxDays"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MinGap") == 0) {
                    map["MinGap"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("WorkDay") == 0) {
                    map["WorkDay"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxDayLoad") == 0) {
                    map["MaxDayLoad"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxBreaks") == 0) {
                    map["MaxBreaks"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }
                if (i.first.compare("MaxBlock") == 0) {
                    map["MaxBlock"]++;//std::cout<<i.second[y]->getClasses().size()<<std::endl;;
                }


        }
    }
    std::cout << hard << " " << soft << " " << map["MaxBreaks"] << " " << map["MaxBlock"] << std::endl;
    /*for (std::map<std::string, int>::iterator p = map.begin(); p != map.end(); ++p) {
        std::cout << p->first << " " << p->second << std::endl;
    }*/

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

inline static long GetBinCoeff(long N, long K) {
    // This function gets the total number of unique combinations based upon N and K.
    // N is the total number of items.
    // K is the size of the group.
    // Total number of unique combinations = N! / ( K! (N - K)! ).
    // This function is less efficient, but is more likely to not overflow when N and K are large.
    // Taken from:  http://blog.plover.com/math/choose.html
    //
    if (K > N) return 0;
    long r = 1;
    long d;
    for (d = 1; d <= K; d++)
    {
        r *= N--;
        r /= d;
    }
    return r;
}


inline  void worstCost(Instance *instance){
    int cost=0;
    for (std::pair<std::string, std::vector<ConstraintShort *>> c : instance->getDist()) {
        for (ConstraintShort * con: c.second) {
            if(con->getWeight()!=-1){
                cost+=con->getWeight()*GetBinCoeff(con->getClasses().size(),2);

            }

        }

    }
    std::cout<<cost*instance->getDistributionPen()<<std::endl;
    cost=0;
    for(Class *c: instance->getClasses()){
        cost+=c->worstCost().first*instance->getRoomPen()+c->worstCost().second*instance->getTimePen();

    }
    std::cout<<cost<<std::endl;
}

inline void blocks(Instance *instance){//        bet-fal17

    int max=0,duration=0;
    for (std::pair<std::string, std::vector<ConstraintShort *>> c : instance->getDist()) {
        if(strcmp(c.first.substr(0,8).c_str(),"MaxBlock")==0||strcmp(c.first.substr(0,8).c_str(),"MaxBreak")==0){
            for (ConstraintShort * con: c.second) {
                duration=0;
                for(Class *cl: con->getClasses()){
                    duration+=cl->getHour().size();
                    if(max<cl->getHour().size())
                        max=cl->getHour().size();


                }
                std::cout<<con->getClasses().size()<<" "<<duration<<" "<<max<<std::endl;




            }
        }
    }



    std::exit(1);


}

inline void overlpacont(Instance *instance){
    int count=0;
    for (int s = 0; s < instance->getClusterStudent().size(); ++s) {
        std::vector<std::vector<Class *>> classes;
        bool t0=false;
        for (int c = 0; c < instance->getClusterStudent()[s]->getCourses().size(); ++c) {
            bool t=true;
            for (int conf = 0;
                 conf < instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig(); ++conf) {

                for (int part = 0; part < instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                        conf).size(); ++part) {
                    bool t1=false,t2= false;

                    for (int cla = 0;
                         cla < instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                 conf)[part]->getClasses().size(); ++cla) {

                        for (int cla1 = cla + 1;
                             cla1 < instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                     conf)[part]->getClasses().size(); ++cla1) {
                            for (std::pair<std::string, std::vector<ConstraintShort *>> cs : instance->getDist()) {
                                for (ConstraintShort * con: cs.second) {
                                    for (Class *ck: con->getClasses()) {
                                        if(ck->getOrderID()==instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                                conf)[part]->getClasses()[cla]->getOrderID())
                                            t1=true;
                                        if(ck->getOrderID()==instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                                conf)[part]->getClasses()[cla1]->getOrderID())
                                            t2=true;

                                    }
                                    if(t1&&!t2||!t1&&t2)
                                        t1=t2=false;
                                }

                            }


                        }



                    }

                    t= t1&&t2;





                }
                if(!t)
                    break;

            }
            if(t)
                t0= true;



        }
        if(t0)
            count++;

    }
    std::cout<<count<<std::endl;
    std::cout<<instance->getClusterStudent().size()<<std::endl;

    std::exit(0);


}

inline void domainreduction(Instance *instance){
    int zero=0,one=0;
    for (Class *c: instance->getClasses()) {
        bool test=true;


        for (std::pair<Room*, int> p : c->getPossibleRooms()) {
            if(p.first->getSlots().size()==0||c->getFirstPossibleRoom()->getSlots().size()==0) {
                test=false;
            } else if(p.first->getSlots().size()!=c->getFirstPossibleRoom()->getSlots().size()) {
                    test=false;
                } else {
                    for (int i = 0; i < p.first->getSlots().size(); ++i) {
                        if (strcmp(c->getFirstPossibleRoom()->getSlots()[i].getDays().c_str(),
                                   p.first->getSlots()[i].getDays().c_str()) != 0
                            || strcmp(c->getFirstPossibleRoom()->getSlots()[i].getWeeks().c_str(),
                                      p.first->getSlots()[i].getWeeks().c_str()) != 0
                            || c->getFirstPossibleRoom()->getSlots()[i].getStart() != p.first->getSlots()[i].getStart()
                            || c->getFirstPossibleRoom()->getSlots()[i].getLenght() != p.first->getSlots()[i].getLenght()) {
                            test = false;
                            break;
                        }

                    }
                }

            if(!test) {
                zero++;
                break;
            }

        }
        if(test && c->getPossibleRooms().size()>0  ){
            one++;

        }


    }
    std::cout<<zero<<", "<<one<<std::endl;


}

inline void penTime(Instance *i){
    std::set<int> pen;
    for (Class *c: i->getClasses()) {
        for(Lecture *l: c->getLectures()){
            pen.insert(l->getPenalty());
        }

    }
    std::cout<<pen.size()<<std::endl;
    for(int p: pen)
        std::cout<<p<<std::endl;
    std::exit(1);
}



#endif //PROJECT_STATS_H
