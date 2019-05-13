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
                                            instance->getClasses()[id]->getLectures()[time]->getLenght()));

            }
        }
    }

public:

    LocalSearchMultiShot(int MAX_ITERATIONS, double rcl, Instance *instance) : LocalSearch(MAX_ITERATIONS, rcl,
                                                                                           instance) {

    }


};


#endif //PROJECT_GRASP_H
