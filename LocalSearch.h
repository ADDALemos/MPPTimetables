//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef TIMETABLER_GRASP_H
#define TIMETABLER_GRASP_H


static const int sizeWindow = 2;

#include <fstream>
#include "problem/Instance.h"
#include <climits>
#include <cfloat>
#include <random>
#include <chrono>
#include <ostream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstring>
#include <map>
#include <chrono>
#include <set>
#include <random>
#include <sstream>
#include<limits.h>

extern void printTime();
extern double getTimeSpent();

class LocalSearch {
protected:
    int maxCons = 0;
    int MAX_ITERATIONS = 5;
    int time;
    int alreadyDone = 0;
    Instance *instance;
    std::vector<Solution*> tabu;
    unsigned long long int best = ULLONG_MAX;
    unsigned long long int currentV = ULLONG_MAX;
    Solution** current;
    std::ofstream seedFile;
    int sizeRCL;
    bool **stu;
    bool **stuSub;
    int sizeRCLS;
    int totalNassigment;
    std::vector<int> problems;
    bool isRoomProblem = false;

public:

    bool ** getStu(){ return stu;}

    LocalSearch(Instance *instance): instance(instance){
        stu = new bool *[instance->getStudent().size()];
        stuSub = new bool *[instance->getStudent().size()];
        int i = 0;
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {
            stu[i] = new bool[instance->getClasses().size()];
            stuSub[i] = new bool[it->second.getNumbSubpart()];
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                stu[i][j] = 0;
            }
            for (int j = 0; j < it->second.getNumbSubpart(); ++j) {
                stuSub[i][j] = 0;
            }
            totalNassigment += it->second.getNumbSubpart();
            i++;
        }
    }


    void LNS() {
        init();
        Local();
        printFinal();


    }





protected:

    void allRandom() {
        int a = 0;
        while (a < instance->getClasses().size()) {
            std::default_random_engine generatorC(seedHandler());
            std::uniform_int_distribution<int> distributionC(0,
                                                             instance->getClasses().size() -
                                                             1);
            int c = distributionC(generatorC);
            std::default_random_engine generatorR(seedHandler());
            std::uniform_int_distribution<int> distributionR(0,
                                                             instance->getClasses()[c]->getPossibleRooms().size() -
                                                             1);
            std::default_random_engine generatorT(seedHandler());
            std::uniform_int_distribution<int> distributionT(0,
                                                             instance->getClasses()[c]->getLectures().size() -
                                                             1);
            int t = distributionT(generatorT);
            int r =
                    distributionR(generatorR);
            if (instance->getClasses()[c]->getPossibleRooms().size() > 0) {
                if (isAllocable(c,
                                instance->getClasses()[c]->getLectures()[t]->getWeeks(),
                                instance->getClasses()[c]->getLectures()[t]->getDays(),
                                instance->getClasses()[c]->getLectures()[t]->getStart(),
                                instance->getClasses()[c]->getLectures()[t]->getLenght(),
                                instance->getClasses()[c]->getPossibleRoomPair(
                                        r).first->getId()) == 0) {
                    current[c] = new Solution(c, instance->getClasses()[c]->getLectures()[t]->getStart(),
                                              instance->getClasses()[c]->getPossibleRoomPair(
                                                      r).first->getId(),
                                              instance->getClasses()[c]->getLectures()[t]->getWeeks(),
                                              instance->getClasses()[c]->getLectures()[t]->getDays(),
                                              instance->getClasses()[c]->getLectures()[t]->getLenght(), 0, 0);
                } else {
                    currentV += INT_MAX;
                }
            } else {
                if (isAllocable(c,
                                instance->getClasses()[c]->getLectures()[t]->getWeeks(),
                                instance->getClasses()[c]->getLectures()[t]->getDays(),
                                instance->getClasses()[c]->getLectures()[t]->getStart(),
                                instance->getClasses()[c]->getLectures()[t]->getLenght(),
                                -1) == 0) {
                    current[c] = new Solution(c, instance->getClasses()[c]->getLectures()[t]->getStart(),
                                              -1,
                                              instance->getClasses()[c]->getLectures()[t]->getWeeks(),
                                              instance->getClasses()[c]->getLectures()[t]->getDays(),
                                              instance->getClasses()[c]->getLectures()[t]->getLenght(), 0, 0);
                } else {
                    currentV += INT_MAX;
                }
            }
            a++;
        }


    }


    bool eval() {
        return best > currentV;

    }

    LocalSearch(int MAX_ITERATIONS, double rcl, Instance *instance, int seconds) : MAX_ITERATIONS(
            MAX_ITERATIONS), time(seconds), instance(instance) {
        instance->setMethod("LocalSearch");
        init();
        totalNassigment = 0;
        sizeRCL = instance->getClasses().size() * rcl;
        sizeRCLS = instance->getStudent().size() * rcl;


    }

    void init() {
        current = new Solution *[instance->getClasses().size()];
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            current[instance->getClasses()[i]->getOrderID()] = nullptr;
        }
        currentV = 0;
        tabu.clear();
    }
//remove all

    void store() {
        if (eval()) {
            int v = 0;
            best = currentV;
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                if (current[instance->getClasses()[i]->getOrderID()] != nullptr) {
                    instance->getClasses()[i]->setSolution(current[instance->getClasses()[i]->getOrderID()]);
                } else {
                    v++;
                }

            }
            std::cout << "FIN" << v << std::endl;
        }

    }

    void printStatus(int ite) {
        std::cout << "Iteration: " << ite << " Best: " << best << " Current: " << currentV << std::endl;
    }

    void printFinal() {
        std::cout << "Best result: " << best << std::endl;
    }

    int getGAP() const {
        return 0;
    }


    void greedyStu() {
        if (instance->getStudent().size() > 0) {
            int maxStu = INT_MAX;
            for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
                 it != instance->getStudent().end(); ++it) {
                for (int c = 0; c < it->second.getCourse().size(); ++c) {
                    for (int conf = 0; conf < 1; ++conf) {
                        for (int part = 0;
                             part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                            Class *c1 = it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses()[0];
                            for (int cla = 0;
                                 cla < it->second.getCourse()[c]->getSubpart(
                                         conf)[part]->getClasses().size(); ++cla) {
                                Class *cla1 = it->second.getCourse()[c]->getSubpart(
                                        conf)[part]->getClasses()[cla];
                                if (maxStu > stuCost(cla1, it->second)) {
                                    maxStu = stuCost(cla1, it->second);
                                    c1 = cla1;

                                }
                            }
                            stu[it->second.getId() - 1][c1->getOrderID()] = 1;
                        }

                    }
                }


            }

        }


    }


    int checkUpdate(int maxCost, int id, int time, const std::pair<Room *, int> &room) {
        int roomID = room.first->getId();
        int costT = isAllocable(id,
                                instance->getClasses()[id]->getLectures()[time]->getWeeks(),
                                instance->getClasses()[id]->getLectures()[time]->getDays(),
                                instance->getClasses()[id]->getLectures()[time]->getStart(),
                                instance->getClasses()[id]->getLectures()[time]->getLenght(),
                                roomID);
        //std::cout<<tabu.size()<<" "<<instance->getClasses()[id]->getId()<<" "<<instance->getClasses()[costT]->getId()<<" "<<costT<<std::endl;
        if (costT != 0) {
            return costT;
        }
        //costT += instance->getClasses()[id]->getLectures()[time]->getPenalty();
        //if (maxCost > (costR)) {
        /*if (tabu.size() == sizeRCL) {
            tabu.pop_back();
        }*/
        // maxCost = costT + costR;
        tabu.push_back(new Solution(id,
                                    instance->getClasses()[id]->getLectures()[time]->getStart(),
                                    roomID,
                                    instance->getClasses()[id]->getLectures()[time]->getWeeks(),
                                    instance->getClasses()[id]->getLectures()[time]->getDays(),
                                    instance->getClasses()[id]->getLectures()[time]->getLenght(),
                                    instance->getClasses()[id]->getLectures()[time]->getPenalty(),
                                    room.second));


        //}

        return 0;


    }


    void Local2() {
        std::vector<int> remo1;
        std::vector<Solution *> oldSol;

        for (int i = 0; i < instance->getClasses().size(); ++i) {
            if (current[instance->getClasses()[i]->getOrderID()] == nullptr) {
                for (int l = 0; l < this->instance->getClasses()[i]->getPossibleRooms().size(); ++l) {
                    for (int m = 0; m < this->instance->getClasses()[i]->getLectures().size(); ++m) {
                        int remo = 0;
                        while ((remo = isAllocable(i, instance->getClasses()[i]->getLectures()[l]->getWeeks(),
                                                   instance->getClasses()[i]->getLectures()[l]->getDays(),
                                                   instance->getClasses()[i]->getLectures()[l]->getStart(),
                                                   instance->getClasses()[i]->getLectures()[l]->getLenght(),
                                                   instance->getClasses()[i]->getPossibleRoomPair(l).first->getId())) !=
                               0) {
                            remo1.push_back(remo);
                            oldSol.push_back(current[instance->getClasses()[remo]->getOrderID()]);
                            current[instance->getClasses()[remo]->getOrderID()] = nullptr;
                        }
                    }
                }

            }
        }
        int maxCost = INT_MAX;
        for (int j = 0; j < remo1.size(); ++j) {
            std::default_random_engine generatorR(seedHandler());
            std::uniform_int_distribution<int> distributionR(0,
                                                             instance->getClasses()[remo1[j]]->getPossibleRooms().size() -
                                                             1);
            std::default_random_engine generatorT(seedHandler());
            std::uniform_int_distribution<int> distributionT(0,
                                                             instance->getClasses()[remo1[j]]->getLectures().size() -
                                                             1);
            int it = 0;
            int t = distributionT(generatorT), r = distributionR(generatorR);
            while (checkUpdate(maxCost, remo1[j], t, instance->getClasses()[remo1[j]]->getPossibleRoomPair(r)) != 0 &&
                   it <= instance->getClasses()[remo1[j]]->getPossibleRooms().size() *
                         instance->getClasses()[remo1[j]]->getLectures().size()) {

                r = distributionR(generatorR);

                t = distributionT(generatorT);
                it++;

            }

        }
        while (!tabu.empty()) {
            std::default_random_engine generator(seedHandler());
            std::uniform_int_distribution<int> distribution(0, tabu.size() - 1);
            int l = distribution(generator);
            if (current[instance->getClasses()[tabu[l]->getLecture()]->getOrderID()] != nullptr) {
                tabu.erase(tabu.begin() + l);
            } else {
                int co = assign(tabu[l]);
                if (co == 0) {
                    //std::cout<<*tabu[l]<<" A"<<instance->getClasses()[tabu[l]->getLecture()]->getId()<<" "<<(instance->getClasses()[tabu[l]->getLecture()]->getPossibleRooms().size() +
                    //                                                                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures().size() == wind)<<std::endl;
                    currentV -= INT_MAX;
                }
            }
        }
    }

    //TODO::improve LNS

    void Local() {

        bool bigmove = true;
        std::cout << "LNS: " << getTimeSpent() << std::endl;
        while (bigmove) {
            bool bigSwamp = false;
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                if (current[instance->getClasses()[i]->getOrderID()] == nullptr) {

                    if (current[instance->getClasses()[i]->getOrderID()] != nullptr) {
                        break;
                    }


                }
            }


            if (!bigSwamp)
                bigmove = false;
        }
        std::cout << "LNS-END: " << getTimeSpent() << std::endl;


    }

    int
    isAllocable(int lectureID, std::string week, std::string day, int start, int duration, int roomID) {
        isRoomProblem = false;
        std::vector<int> problemsTemp;
        problemsTemp.push_back(lectureID);
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            if (current[instance->getClasses()[i]->getOrderID()] != nullptr && roomID != -1 && i != lectureID) {
                if (current[instance->getClasses()[i]->getOrderID()]->getSolRoom() == roomID) {
                    for (int j = 0; j < instance->getNweek(); ++j) {
                        if (week[j] == current[instance->getClasses()[i]->getOrderID()]->getSolWeek()[j] &&
                            week[j] == '1') {
                            for (int d = 0; d < instance->getNdays(); ++d) {
                                if (day[d] == current[instance->getClasses()[i]->getOrderID()]->getSolDays()[d] &&
                                    day[d] == '1') {
                                    if (current[instance->getClasses()[i]->getOrderID()]->getSolStart() >= start &&
                                        current[instance->getClasses()[i]->getOrderID()]->getSolStart() <
                                        start + duration) {
                                        //std::cout<<"room"<<std::endl;
                                        problemsTemp.push_back(instance->getClasses()[i]->getId());
                                        isRoomProblem = true;
                                        problems = problemsTemp;
                                        return i;
                                    } else if (
                                            start >= current[instance->getClasses()[i]->getOrderID()]->getSolStart() &&
                                            start < current[instance->getClasses()[i]->getOrderID()]->getSolStart() +
                                                    current[instance->getClasses()[i]->getOrderID()]->getDuration()) {
                                        //std::cout<<"room"<<std::endl;
                                        problemsTemp.push_back(instance->getClasses()[i]->getId());
                                        isRoomProblem = true;
                                        problems = problemsTemp;
                                        return i;
                                    }
                                }

                            }
                        }

                    }
                }
            }

        }
        if (roomID != -1) {
            for (int una = 0; una < instance->getRoom(roomID)->getSlots().size(); ++una) {
                for (int i = 0; i < instance->getNweek(); ++i) {
                    if (week[i] == instance->getRoom(roomID)->getSlots()[una].getWeeks()[i] &&
                        instance->getRoom(roomID)->getSlots()[una].getWeeks()[i] == '1') {
                        for (int d = 0; d < instance->getNdays(); ++d) {
                            if (day[d] == instance->getRoom(roomID)->getSlots()[una].getDays()[d] && day[d] == '1') {
                                if (start >= instance->getRoom(roomID)->getSlots()[una].getStart() &&
                                    start < instance->getRoom(
                                            roomID)->getSlots()[una].getStart() + instance->getRoom(
                                            roomID)->getSlots()[una].getLenght()) {
                                    isRoomProblem = true;
                                    problems = problemsTemp;
                                    return lectureID;
                                }
                                if (start + duration >= instance->getRoom(roomID)->getSlots()[una].getStart() && start <
                                                                                                                 instance->getRoom(
                                                                                                                         roomID)->getSlots()[una].getStart() +
                                                                                                                 instance->getRoom(
                                                                                                                         roomID)->getSlots()[una].getLenght()) {
                                    isRoomProblem = true;
                                    problems = problemsTemp;
                                    return lectureID;
                                }
                            }
                        }

                    }
                }

            }
        }


        //cost += instance->getClasses()[lectureID]->getPossibleRoomCost(roomID);




        return 0;
    }

    int isFirst(std::string s1, std::string s2, int size) {
        for (int i = 0; i < size; ++i) {
            if (s1[i] == '0' && s2[i] == '1')
                return 1;
            if (s1[i] == '1' && s2[i] == '0')
                return -1;
            if (s1[i] == '1' && s2[i] == '1')
                return 0;
        }
        return 0;
    }

    bool stringcompare(std::string s1, std::string s2, int size, bool same) {
        for (int i = 0; i < size; ++i) {
            if (same && s2[i] != s1[i])
                return false;
            else if (!same && s2[i] == s1[i])
                return false;
        }
        return true;

    }

    bool stringcontains(std::string s1, std::string s2, int size) {
        bool s2B = false, s1B = false;
        for (int i = 0; i < size; ++i) {
            if (s2[i] == '1' && s1[i] == '0')
                s2B = true;
            if (s1[i] == '1' && s2[i] == '0')
                s1B = true;
        }
        return !(s1B && s2B);

    }


    double cost(int lectureID, int roomID, int day, int timeS) {
        return 0;

    }


/**
 * Generate a seed based on the clock
 * @return seed
 */
    unsigned int seedHandler() {
        unsigned int t = std::chrono::steady_clock::now().time_since_epoch().count();
        seedFile.open("../log/seed.txt", std::ios_base::app);
        seedFile << t << std::endl;
        seedFile.close();
        return t;
    }


    int assign(Solution *t) {
        int cost = isAllocable(t->getLecture(), t->getSolWeek(), t->getSolDays(), t->getSolStart(), t->getDuration(),
                               t->getSolRoom());
        if (cost > 0)
            return cost;
        current[instance->getClasses()[t->getLecture()]->getOrderID()] = t;
        /*cost += instance->getClasses()[t->getLecture()]->getPossibleRoomCost(current[t->getLecture()]->getSolRoom())*instance->getRoomPen();
        cost +=instance->getTimePen()*current[t->getLecture()]->penTime();*/
        return 0;
    }

    int tryswampLectures(int id, const std::string &week, std::string &day, int start, int le, int pen) {
        int old = -1;
        if (current[instance->getClasses()[id]->getOrderID()] == nullptr)
            old = INT_MAX;
        else
            old = isAllocable(id, current[instance->getClasses()[id]->getOrderID()]->getSolWeek(),
                              current[instance->getClasses()[id]->getOrderID()]->getSolDays(),
                              current[instance->getClasses()[id]->getOrderID()]->getSolStart(),
                              current[instance->getClasses()[id]->getOrderID()]->getLecture(),
                              current[instance->getClasses()[id]->getOrderID()]->getSolRoom());
        int newV = isAllocable(id, week, day, start, le,
                               current[instance->getClasses()[id]->getOrderID()]->getSolRoom());
        if (old > 0)old = INT_MAX;
        if (newV > 0)newV = INT_MAX;
        /*newV += instance->getClasses()[id]->getPossibleRoomCost(current[id]->getSolRoom())*instance->getRoomPen();
        newV += instance->getTimePen()*pen;
        old += instance->getClasses()[id]->getPossibleRoomCost(current[id]->getSolRoom())*instance->getRoomPen();
        old += instance->getTimePen()*current[id]->penTime();*/
        //std::cout << old << " " << newV << " " << (old - newV) << std::endl;
        return old - newV;
    }

    void
    swampLectures(int id, const std::string &week, std::string &day, int start, int le, int pen) {
        int i = -1;
        //if (current[instance->getClasses()[id]->getOrderID()]->getSolRoom() != -1)
        //  i = instance->getClasses()[id]->getPossibleRoomCost(current[id]->getSolRoom());

        Solution *s = new Solution(id, start, current[id]->getSolRoom(), week, day, le, pen, i);
        current[instance->getClasses()[id]->getOrderID()] = s;
        //std::cout<<instance->getClasses()[id]->getId()<<" "<<*current[id]<<std::endl;
    }

    int getGAPStored() {
        int count = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int j = 0; j < instance->getNdays(); ++j) {
                for (int k = 1; k < instance->getSlotsperday(); ++k) {
                    int before = 0, after = 0;
                    for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
                        if (j == instance->getStudent(i + 1).getClasses()[l]->getSolDay()
                            && k >= instance->getStudent(i + 1).getClasses()[l]->getSolStart() &&
                            k < (instance->getStudent(i + 1).getClasses()[l]->getSolStart()
                                 + instance->getStudent(i + 1).getClasses()[l]->getLenght()))
                            after += 1;
                        if (j == instance->getStudent(i + 1).getClasses()[l]->getSolDay()
                            && (k - 1) >= instance->getStudent(i + 1).getClasses()[l]->getSolStart() &&
                            (k - 1) < (instance->getStudent(i + 1).getClasses()[l]->getSolStart()
                                       + instance->getStudent(i + 1).getClasses()[l]->getLenght()))
                            before += 1;
                    }
                    if (before != after)
                        count++;

                }
            }
        }
        return count;
    }


    int tryswampRoom(int lecture, int roomID) {
        int old;
        if (current[instance->getClasses()[lecture]->getOrderID()] != nullptr)
            if (current[instance->getClasses()[lecture]->getOrderID()]->getSolRoom() != -1)
                old = isAllocable(lecture, current[instance->getClasses()[lecture]->getOrderID()]->getSolWeek(),
                                  current[instance->getClasses()[lecture]->getOrderID()]->getSolDays(),
                                  current[instance->getClasses()[lecture]->getOrderID()]->getSolStart(),
                                  current[instance->getClasses()[lecture]->getOrderID()]->getLecture(),
                                  current[instance->getClasses()[lecture]->getOrderID()]->getSolRoom());
            else
                old = INT_MAX;
        else
            old = INT_MAX;
        int newV = isAllocable(lecture,
                               current[instance->getClasses()[lecture]->getOrderID()]->getSolWeek(),
                               current[instance->getClasses()[lecture]->getOrderID()]->getSolDays(),
                               current[instance->getClasses()[lecture]->getOrderID()]->getSolStart(),
                               current[instance->getClasses()[lecture]->getOrderID()]->getLecture(),
                               roomID);
        if (old > 0)old = INT_MAX;
        if (newV > 0)newV = INT_MAX;

        /*newV += instance->getClasses()[lecture]->getPossibleRoomCost(roomID)*instance->getRoomPen();
        newV += instance->getTimePen()* current[instance->getClasses()[lecture]->getOrderID()]->penTime();
        old += instance->getClasses()[lecture]->getPossibleRoomCost( current[instance->getClasses()[lecture]->getOrderID()]->getSolRoom())*instance->getRoomPen();
        old += instance->getTimePen()* current[instance->getClasses()[lecture]->getOrderID()]->penTime();*/
        //std::cout << old << " " << newV << " " << (old - newV) << std::endl;
        return old - newV;
    }

    int stuCost(Class *c, Student s) {
        if (c->getParent() != nullptr) {
            if (stu[s.getId() - 1][c->getOrderID()] == 1)
                return INT_MAX;
        }
        int limit = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            limit += stu[i][c->getOrderID()];
        }
        if (c->getLimit() < limit)
            return INT_MAX;
        return 0;

    }

    void removeElement() {

    }

    void conflict(int lectureID) {
        int it_max = 0;
        int block[problems.size()];
        for (int it = 0; it < problems.size(); ++it) {
            current[instance->getClass(problems[it])->getOrderID()] = nullptr;
            it_max += instance->getClass(problems[it])->getPossibleRooms().size() *
                      instance->getClass(problems[it])->getLectures().size();
            block[it] = 0;
        }
        int s = problems.size();
        int a = 0, it = 0;
        while (a < s && it < it_max) {
            a = 0;
            for (int i = 0; i < problems.size(); ++i) {
                if (assignable(problems[i], block))
                    a++;
                else {
                    current[instance->getClass(problems[i])->getOrderID()] = nullptr;
                    block[instance->getClass(problems[i])->getOrderID()]++;
                    break;
                }

            }
            it++;


        }


    }

    int stuCost(Class *c, Student s, Instance *instance) {
        if (c->getParent() != nullptr) {
            stu[s.getId() - 1][instance->getClass(c->getParent()->getId())->getOrderID()] = 1;
        }
        int limit = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            limit += stu[i][c->getOrderID()];
        }
        limit++;
        if (c->getLimit() < limit)
            return INT_MAX;
        return 0;

    }
public:
    void stuAlloc() {
        for (std::map<int, Student>::iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {

            for (int c = 0; c < it->second.getCourse().size(); ++c) {


                for (int conf = 0; conf < it->second.getCourse()[c]->getNumConfig(); ++conf) {
                    bool fail = true;
                    Class *cla1;
                    for (int part = 0;
                         part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                        for (int cla = 0;
                             cla < it->second.getCourse()[c]->getSubpart(
                                     conf)[part]->getClasses().size(); ++cla) {
                            cla1 = it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses()[cla];

                            if (INT_MAX != stuCost(cla1, it->second, instance)) {

                                stu[it->second.getId() - 1][cla1->getOrderID()] = 1;
                                it->second.addClass(cla1);
                                cla1->addStudent(it->second.getId());
                                fail = false;
                                break;

                            } else
                                fail = true;
                        }
                        if (fail) {
                            it->second.addClass(cla1);
                            cla1->addStudent(it->second.getId());
                            break;
                        }
                    }
                    if (!fail) {
                        break;
                    }

                }
            }


        }

    }

    bool assignable(int id, int pInt[]) {
        std::default_random_engine generatorR(seedHandler());
        std::uniform_int_distribution<int> distributionR(0,
                                                         instance->getClass(id)->getPossibleRooms().size() -
                                                         1);
        std::default_random_engine generatorT(seedHandler());
        std::uniform_int_distribution<int> distributionT(0,
                                                         instance->getClass(id)->getLectures().size() -
                                                         1);
        unsigned int tP = distributionR(generatorR);
        int rP = distributionT(generatorT);
        std::cout << instance->getClass(id)->getPossibleRooms().size() << " " << tP << std::endl;


        while (isAllocable(id,
                           instance->getClass(id)->getLectures()[rP]->getWeeks(),
                           instance->getClass(id)->getLectures()[rP]->getDays(),
                           instance->getClass(id)->getLectures()[rP]->getStart(),
                           instance->getClass(id)->getLectures()[rP]->getLenght(),
                           instance->getClass(id)->getPossibleRoomPair(
                                   tP).first->getId()) != 0) {
            tP = distributionR(generatorR);
            rP = distributionT(generatorT);
        }
        if (isAllocable(id,
                        instance->getClass(id)->getLectures()[rP]->getWeeks(),
                        instance->getClass(id)->getLectures()[rP]->getDays(),
                        instance->getClass(id)->getLectures()[rP]->getStart(),
                        instance->getClass(id)->getLectures()[rP]->getLenght(),
                        instance->getClass(id)->getPossibleRoomPair(
                                tP).first->getId()) == 0) {

            assign(new Solution(id,
                                instance->getClass(id)->getLectures()[rP]->getStart(),
                                instance->getClass(id)->getPossibleRoomPair(
                                        tP).first->getId(),
                                instance->getClass(id)->getLectures()[rP]->getWeeks(),
                                instance->getClass(id)->getLectures()[rP]->getDays(),
                                instance->getClass(id)->getLectures()[rP]->getLenght(),
                                instance->getClass(id)->getLectures()[rP]->getPenalty(),
                                -1));
            return true;
        }
        /*for (int tP = 0;
                         tP < instance->getClass(id)->getPossibleRooms().size(); ++tP) {
                        for (int rP = 0;
                             rP < instance->getClass(id)->getLectures().size(); ++rP) {
                            if(pInt[id]!=rP){
                                if (isAllocable(id,
                                                     instance->getClass(id)->getLectures()[rP]->getWeeks(),
                                                     instance->getClass(id)->getLectures()[rP]->getDays(),
                                                     instance->getClass(id)->getLectures()[rP]->getStart(),
                                                     instance->getClass(id)->getLectures()[rP]->getLenght(),
                                                instance->getClass(id)->getPossibleRoomPair(
                                                        tP).first.getId()) == 0) {
    
                                    assign(new Solution(id,
                                                    instance->getClass(id)->getLectures()[rP]->getStart(),
                                                    instance->getClass(id)->getPossibleRoomPair(
                                                            tP).first.getId(),
                                                    instance->getClass(id)->getLectures()[rP]->getWeeks(),
                                                    instance->getClass(id)->getLectures()[rP]->getDays(),
                                                    instance->getClass(id)->getLectures()[rP]->getLenght(),
                                                    instance->getClass(id)->getLectures()[rP]->getPenalty(),
                                                    -1));
                                    return true;
                            }
    
                        }
    
                    }
    }*/
        return false;
    }

    std::vector<Solution *> findPossibleMove(int i) {
        std::vector<Solution *> reslut;
        for (int l = 0; l < this->instance->getClasses()[i]->getPossibleRooms().size(); ++l) {
            for (int m = 0; m < this->instance->getClasses()[i]->getLectures().size(); ++m) {
                if (this->isAllocable(i,
                                      this->instance->getClasses()[i]->getLectures()[m]->getWeeks(),
                                      this->instance->getClasses()[i]->getLectures()[m]->getDays(),
                                      this->instance->getClasses()[i]->getLectures()[m]->getStart(),
                                      this->instance->getClasses()[i]->getLectures()[m]->getLenght(),
                                      this->instance->getClasses()[i]->getPossibleRoomPair(
                                              l).first->getId()) == 0) {

                    reslut.push_back(new Solution(this->tabu[l]->getLecture(),
                                                  this->instance->getClasses()[i]->getLectures()[m]->getStart(),
                                                  this->instance->getClasses()[i]->getPossibleRoomPair(
                                                          l).first->getId(),
                                                  this->instance->getClasses()[i]->getLectures()[m]->getWeeks(),
                                                  this->instance->getClasses()[i]->getLectures()[m]->getDays(),
                                                  this->instance->getClasses()[i]->getLectures()[m]->getLenght(),
                                                  this->instance->getClasses()[i]->getLectures()[m]->getPenalty(),
                                                  -1));

                }


            }

        }
        return reslut;
    }

};


#endif //PROJECT_GRASP_H
