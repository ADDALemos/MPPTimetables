//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef TIMETABLER_LocalSearchMultiShotRoom_H
#define TIMETABLER_LocalSearchMultiShotRoom_H

#include "LocalSearch.h"
#include <fstream>
#include "../problem/Instance.h"
#include <climits>
#include <cfloat>
#include <random>
#include <chrono>

extern double getTimeSpent();

class LocalSearchMultiShotRoom : public LocalSearch {

protected:
    virtual void Greedy() override {
        int allocation = 0;
        currentV = 0;
        while (instance->getClasses().size() > allocation) {

            for (int i = 0; i < instance->getClasses().size(); ++i) {
                if (current[i] == nullptr) {
                    int roomID = -1;
                    if (instance->getClasses()[i]->getPossibleRooms().size() > 0) {
                        int maxRoom = INT_MAX;
                        int c = 0;
                        for (std::pair<Room, int> room: instance->getClasses()[i]->getPossibleRooms()) {
                            if ((c = room.second) < maxRoom) {
                                maxRoom = c;
                                roomID = room.first.getId();
                            }
                        }

                    }
                    int maxCost = INT_MAX;
                    for (int time = 0; time < instance->getClasses()[i]->getLectures().size(); ++time) {
                        int costT = 0;
                        if ((costT = isAllocable(i, instance->getClasses()[i]->getLectures()[time]->getWeeks(),
                                                 instance->getClasses()[i]->getLectures()[time]->getDays(),
                                                 instance->getClasses()[i]->getLectures()[time]->getStart(),
                                                 instance->getClasses()[i]->getLectures()[time]->getLenght(),
                                                 roomID)) != -1) {
                            if (maxCost > costT) {
                                if (tabu.size() == sizeRCL) {
                                    tabu.pop_back();
                                    maxCost = costT;
                                }
                                tabu.push_back(new Solution(i,
                                                            instance->getClasses()[i]->getLectures()[time]->getStart(),
                                                            roomID,
                                                            instance->getClasses()[i]->getLectures()[time]->getWeeks(),
                                                            instance->getClasses()[i]->getLectures()[time]->getDays(),
                                                            instance->getClasses()[i]->getLectures()[time]->getLenght(),
                                                            instance->getClasses()[i]->getLectures()[time]->getPenalty(),
                                                            0));

                            }
                        }
                    }
                }
            }

            if (!tabu.empty()) {
                std::default_random_engine generator(seedHandler());
                std::uniform_int_distribution<int> distribution(0, tabu.size() - 1);
                int l = distribution(generator);
                int co = 0;
                while ((co = assign(tabu[l])) == -1) {
                    l = distribution(generator);
                }
                if (co != -1) {
                    allocation++;
                    currentV += co;

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


public:

    LocalSearchMultiShotRoom(int MAX_ITERATIONS, double rcl, Instance *instance, int seconds) : LocalSearch(
            MAX_ITERATIONS,
            rcl,
            instance,
            seconds) {

    }


};


#endif //PROJECT_GRASP_H
