//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef TIMETABLER_LocalSearchMultiShot_H
#define TIMETABLER_LocalSearchMultiShot_H

#include "LocalSearch.h"
#include <fstream>
#include "../problem/Instance.h"
#include <climits>
#include <cfloat>
#include <random>
#include <chrono>

extern double getTimeSpent();

class LocalSearchMultiShot : public LocalSearch {

protected:
    virtual void Greedy() override {
        int allocation = 0;
        currentV = 0;
        while (instance->getClasses().size() > allocation) {

            int maxCost = INT_MAX;
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                if (current[i] == nullptr) {
                    for (int time = 0; time < instance->getClasses()[i]->getLectures().size(); ++time) {
                        maxCost = checkUpdate(maxCost, i, time);
                    }
                }
            }

            if (!tabu.empty()) {
                std::default_random_engine generator(seedHandler());
                std::uniform_int_distribution<int> distribution(0, tabu.size() - 1);
                int l = distribution(generator);
                int co = 0;
                if ((co = assign(tabu[l])) != -1) {
                    allocation++;
                    currentV += co;
                    int id = tabu[l]->getLecture();
                    if (instance->getClasses()[id]->getPossibleRooms().size() > 0) {
                        int maxRoom = INT_MAX;
                        int roomID = -1;
                        for (std::pair<Room, int> room: instance->getClasses()[id]->getPossibleRooms()) {
                            if (isAllocable(id, tabu[l]->getSolWeek(),
                                            tabu[l]->getSolDays(),
                                            tabu[l]->getSolStart(),
                                            tabu[l]->getDuration(), room.first.getId()) <
                                maxRoom && isAllocable(id, tabu[l]->getSolWeek(),
                                                       tabu[l]->getSolDays(),
                                                       tabu[l]->getSolStart(),
                                                       tabu[l]->getDuration(), room.first.getId()) != -1) {
                                roomID = room.first.getId();
                                maxRoom = isAllocable(id, tabu[l]->getSolWeek(),
                                                      tabu[l]->getSolDays(),
                                                      tabu[l]->getSolStart(),
                                                      tabu[l]->getDuration(), room.first.getId());
                            }
                        }
                        if (roomID != -1)
                            current[tabu[l]->getLecture()]->setSolRoom(roomID);
                        else
                            currentV += 10000000;

                    }
                    /*if(instance->getStudent().size()>0) {
                        int totalAss = 0;
                        int maxStu = INT_MAX;
                        std::vector<int> sub;
                        std::vector<Class *> co;
                        std::vector<Student> s;
                        while (totalAss < totalNassigment) {
                                for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
                                     it != instance->getStudent().end(); ++it) {
                                    for (int c = 0; c < it->second.getCourse().size(); ++c) {
                                        for (int conf = 0; conf < it->second.getCourse()[c]->getNumConfig(); ++conf) {
                                            for (int part = 0;
                                                 part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                                                if(stuSub[it->second.getId()-1][part]==0) {
                                                    for (int cla = 0;
                                                         cla < it->second.getCourse()[c]->getSubpart(
                                                                 conf)[part]->getClasses().size(); ++cla) {
                                                        Class *cla1 = it->second.getCourse()[c]->getSubpart(
                                                                conf)[part]->getClasses()[cla];
                                                        if (maxStu > stuCost(cla1, it->second) &&
                                                            stuCost(cla1, it->second) != -1) {
                                                            if (s.size() == sizeRCLS) {
                                                                s.pop_back();
                                                                sub.pop_back();
                                                                co.pop_back();
                                                            }
                                                            sub.push_back(part);
                                                            maxStu = stuCost(cla1, it->second);
                                                            co.push_back(cla1);
                                                            s.push_back(it->second);
                                                        }
                                                    }
                                                }

                                            }
                                        }



                                }

                            }
                            if (!s.empty()) {
                                std::default_random_engine generator(seedHandler());
                                std::uniform_int_distribution<int> distribution(0, s.size() - 1);
                                int l = distribution(generator);
                                if (stuCost(co[l], s[l]) != -1) {
                                    stu[s[l].getId()-1][co[l]->getOrderID()]=1;
                                    stuSub[s[l].getId()-1][s[l].getId()-1]=1;
                                    totalAss++;
                                    s[l].addClass(co[l]);
                                }
                            }
                            s.clear();co.clear();sub.clear();

                        }
                    }*/
                } else {
                    std::cerr << "There " << getTimeSpent() << std::endl;
                }


            } else {
                std::cout << "Not FOUND" << std::endl;
                currentV = -1;
                return;
            }
            tabu.clear();
        }
    }

    /*virtual void Greedy() override {
        int allocation = 0;
        currentV = 0;
        while (instance->getClasses().size() > allocation) {

            int maxCost = INT_MAX;
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                if (current[i] == nullptr) {
                    for (int time = 0; time < instance->getClasses()[i]->getLectures().size(); ++time) {
                        maxCost = checkUpdate(maxCost, i, time);
                    }
                }
            }

            if (!tabu.empty()) {
                std::default_random_engine generator(seedHandler());
                std::uniform_int_distribution<int> distribution(0, tabu.size() - 1);
                int l = distribution(generator);
                int co = 0;
                if ((co = assign(tabu[l])) != -1) {
                    allocation++;
                    currentV += co;
                    int id = tabu[l]->getLecture();
                    if (instance->getClasses()[id]->getPossibleRooms().size() > 0) {
                        int maxRoom = INT_MAX;
                        int roomID = -1;
                        for (std::pair<Room, int> room: instance->getClasses()[id]->getPossibleRooms()) {
                            if (isAllocable(id, tabu[l]->getSolWeek(),
                                            tabu[l]->getSolDays(),
                                            tabu[l]->getSolStart(),
                                            tabu[l]->getDuration(), room.first.getId()) <
                                maxRoom && isAllocable(id, tabu[l]->getSolWeek(),
                                                       tabu[l]->getSolDays(),
                                                       tabu[l]->getSolStart(),
                                                       tabu[l]->getDuration(), room.first.getId()) != -1) {
                                roomID = room.first.getId();
                                maxRoom = isAllocable(id, tabu[l]->getSolWeek(),
                                                      tabu[l]->getSolDays(),
                                                      tabu[l]->getSolStart(),
                                                      tabu[l]->getDuration(), room.first.getId());
                            }
                        }
                        if (roomID != -1)
                            current[tabu[l]->getLecture()]->setSolRoom(roomID);
                        else
                            currentV += 10000000;

                    }
                    /*if(instance->getStudent().size()>0) {
                        int totalAss = 0;
                        int maxStu = INT_MAX;
                        std::vector<int> sub;
                        std::vector<Class *> co;
                        std::vector<Student> s;
                        while (totalAss < totalNassigment) {
                                for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
                                     it != instance->getStudent().end(); ++it) {
                                    for (int c = 0; c < it->second.getCourse().size(); ++c) {
                                        for (int conf = 0; conf < it->second.getCourse()[c]->getNumConfig(); ++conf) {
                                            for (int part = 0;
                                                 part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                                                if(stuSub[it->second.getId()-1][part]==0) {
                                                    for (int cla = 0;
                                                         cla < it->second.getCourse()[c]->getSubpart(
                                                                 conf)[part]->getClasses().size(); ++cla) {
                                                        Class *cla1 = it->second.getCourse()[c]->getSubpart(
                                                                conf)[part]->getClasses()[cla];
                                                        if (maxStu > stuCost(cla1, it->second) &&
                                                            stuCost(cla1, it->second) != -1) {
                                                            if (s.size() == sizeRCLS) {
                                                                s.pop_back();
                                                                sub.pop_back();
                                                                co.pop_back();
                                                            }
                                                            sub.push_back(part);
                                                            maxStu = stuCost(cla1, it->second);
                                                            co.push_back(cla1);
                                                            s.push_back(it->second);
                                                        }
                                                    }
                                                }

                                            }
                                        }



                                }

                            }
                            if (!s.empty()) {
                                std::default_random_engine generator(seedHandler());
                                std::uniform_int_distribution<int> distribution(0, s.size() - 1);
                                int l = distribution(generator);
                                if (stuCost(co[l], s[l]) != -1) {
                                    stu[s[l].getId()-1][co[l]->getOrderID()]=1;
                                    stuSub[s[l].getId()-1][s[l].getId()-1]=1;
                                    totalAss++;
                                    s[l].addClass(co[l]);
                                }
                            }
                            s.clear();co.clear();sub.clear();

                        }
                    }
                } else {
                    std::cerr << "There " << getTimeSpent() << std::endl;
                }


            } else {
                std::cout << "Not FOUND" << std::endl;
                currentV = -1;
                return;
            }
            tabu.clear();
        }
    }*/

    int stuCost(Class *c, Student s) {
        if (c->getParent() != nullptr) {
            if (stu[s.getId() - 1][c->getOrderID()] == 1)
                return -1;
        }
        int limit = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            limit += stu[i][c->getOrderID()];
        }
        if (c->getLimit() < limit)
            return -1;

    }


    int checkUpdate(int maxCost, int id, int time) override {
        int costT = 0;
        if ((costT = instance->getClasses()[id]->getLectures()[time]->getPenalty()) != -1) {
            if (maxCost > costT) {
                if (tabu.size() == sizeRCL) {
                    tabu.pop_back();
                    maxCost = costT;
                }
                tabu.push_back(new Solution(id,
                                            instance->getClasses()[id]->getLectures()[time]->getStart(),
                                            -1,
                                            instance->getClasses()[id]->getLectures()[time]->getWeeks(),
                                            instance->getClasses()[id]->getLectures()[time]->getDays(),
                                            instance->getClasses()[id]->getLectures()[time]->getLenght(),
                                            instance->getClasses()[id]->getLectures()[time]->getPenalty(), 0));

            }
        }
        return costT;
    }

public:

    LocalSearchMultiShot(int MAX_ITERATIONS, double rcl, Instance *instance) : LocalSearch(MAX_ITERATIONS, rcl,
                                                                                           instance) {

    }


};


#endif //PROJECT_GRASP_H
