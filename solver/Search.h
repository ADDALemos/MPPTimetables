//
// Created by Alexandre Lemos on 15/03/2019.
//

#ifndef TIMETABLER_SEARCH_H
#define TIMETABLER_SEARCH_H


#include <fstream>
#include "../problem/Instance.h"
#include <climits>
#include <cfloat>
#include <random>
#include <chrono>
#include <ostream>
#include "LSDivided.h"
#include <algorithm>

extern void printTime();

extern double getTimeSpent();

class Search {
protected:

    Instance *instance;

    std::ofstream seedFile;
    bool isRoomProblem = false;
    std::list<Class *> classesbyCost;
    int current = 0;
    Solution *prev;


public:
    Search(Instance *instance, std::list<Class *> classesbyCost, int Greedy) :
            instance(instance), classesbyCost(classesbyCost) {
        instance->setCompact(false);

    }

    void run() {
        //step1();
        step2();
    }

protected:

    void step1() {
        for (int i = 0; i < instance->getNumClasses(); ++i) {
            if (instance->getClasses()[i]->getPossibleRooms().size() == 1) {
                instance->getClasses()[i]->setSolRoom(instance->getClasses()[i]->getPossibleRoomPair(
                        0).first.getId());
                //std::cout<<instance->getClasses()[i]->getId()<<" 1 "<<*instance->getClasses()[i]->getSolution()<<std::endl;
            }
            if (instance->getClasses()[i]->getLectures().size() == 1) {
                instance->getClasses()[i]->updateSolution(instance->getClasses()[i]->getLectures()[0]->getStart(),
                                                          instance->getClasses()[i]->getLectures()[0]->getWeeks(),
                                                          instance->getClasses()[i]->getLectures()[0]->getDays(),
                                                          instance->getClasses()[i]->getLectures()[0]->getLenght());
                //std::cout<<instance->getClasses()[i]->getId()<<" 1 "<<*instance->getClasses()[i]->getSolution()<<std::endl;

            }

        }
    }

    void step2() {
        while (hasNext()) {
            Class *c = next();
            if (!assign(c))
                backtrack(c, previous());

        }
        std::cout << "End" << current << std::endl;

    }

    bool hasNext() {
        return current < classesbyCost.size();
    }

    Class *next() {
        auto it = classesbyCost.begin();
        std::advance(it, current);
        Class *c = *it;
        current++;
        return c;
    }

    Class *previous() {
        if (current - 2 < 0)
            throw ("INVALID");
        auto it = classesbyCost.begin();
        std::advance(it, current - 2);
        Class *c = *it;
        return c;
    }

    void backtrack(Class *c1, Class *c2) {
        current -= 2;
        std::cout << "Back " << c1->getId() << " to " << c2->getId() << std::endl;
        //std::cout<<c1->getLectLevel()<<" "<<c1->getId()<<" "<<c1->getRoomLevel()<<std::endl;
        std::cout << c2->getLectLevel() << " " << c2->getId() << " " << c2->getRoomLevel() << std::endl;

        c1->setTree(0, 0);
        if (c1->getLectures().size() > 1)
            c1->updateSolution(-1, "1", "1", -1);
        if (c1->getPossibleRooms().size() > 1)
            c1->setSolRoom(-1);
        if (c2->getLectures().size() > 1)
            c2->updateSolution(-1, "1", "1", -1);
        if (c2->getPossibleRooms().size() > 1)
            c2->setSolRoom(-1);


    }

    bool assign(Class *c) {
        std::cout << c->getLectLevel() << " " << c->getId() << " " << c->getRoomLevel() << std::endl;
        if (c->getLectures().size() == 1) {
            c->updateSolution(c->getLectures()[0]->getStart(), c->getLectures()[0]->getWeeks(),
                              c->getLectures()[0]->getDays(), c->getLectures()[0]->getLenght());

        }
        if (c->getPossibleRooms().size() == 1) {
            c->setSolRoom(c->getPossibleRoomPair(0).first.getId());
        }
        if (c->getLectures().size() > 1 && c->getPossibleRooms().size() > 1) {
            for (int j = c->getRoomLevel(); j < c->getPossibleRooms().size(); ++j) {
                for (int i = c->getLectLevel(); i < c->getLectures().size(); ++i) {
                    if (check(c, i, j)) {
                        c->setTree(i + 1, j + 1);
                        c->setSolRoom(c->getPossibleRoomPair(j).first.getId());
                        c->updateSolution(c->getLectures()[i]->getStart(), c->getLectures()[i]->getWeeks(),
                                          c->getLectures()[i]->getDays(), c->getLectures()[i]->getLenght());
                        return true;
                    }
                }
            }
            return false;
        } else if (c->getPossibleRooms().size() > 1) {
            for (int i = c->getRoomLevel(); i < c->getPossibleRooms().size(); ++i) {
                if (check(c, 0, i)) {
                    c->setTree(0, i + 1);
                    c->setSolRoom(c->getPossibleRoomPair(i).first.getId());
                    return true;
                }

            }
            return false;
        } else if (c->getLectures().size() > 1) {
            for (int i = c->getLectLevel(); i < c->getLectures().size(); ++i) {
                if (check(c, i, 0)) {
                    c->setTree(i + 1, 0);
                    c->updateSolution(c->getLectures()[i]->getStart(), c->getLectures()[i]->getWeeks(),
                                      c->getLectures()[i]->getDays(), c->getLectures()[i]->getLenght());
                    return true;
                }
            }
            return false;
        }
        return true;


    }

    bool check(Class *c, int lec, int room) {
        int roomID = -1;
        if (c->getPossibleRooms().size() > 0)
            roomID = c->getPossibleRoomPair(room).first.getId();
        std::string week = c->getLectures()[lec]->getWeeks();
        std::string day = c->getLectures()[lec]->getDays();
        int start = c->getLectures()[lec]->getStart();
        int duration = c->getLectures()[lec]->getLenght();
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            if (instance->getClasses()[i]->getSolStart() != -1 && instance->getClasses()[i]->getSolRoom() != -1 &&
                instance->getClasses()[i]->getId() != c->getId()) {
                if (instance->getClasses()[i]->getSolRoom() == roomID) {
                    for (int j = 0; j < instance->getNweek(); ++j) {
                        if (week[j] == instance->getClasses()[i]->getSolWeek()[j] &&
                            week[j] == '1') {
                            for (int d = 0; d < instance->getNdays(); ++d) {
                                if (day[d] == instance->getClasses()[i]->getSolDays()[d] &&
                                    day[d] == '1') {
                                    if (instance->getClasses()[i]->getSolStart() >= start &&
                                        instance->getClasses()[i]->getSolStart() <
                                        start + duration) {
                                        printCurrent(c, lec, room, instance->getClasses()[i]);
                                        return false;
                                    } else if (start >= instance->getClasses()[i]->getSolStart() &&
                                               start < instance->getClasses()[i]->getSolStart() +
                                                       instance->getClasses()[i]->getSolDuration()) {
                                        printCurrent(c, lec, room, instance->getClasses()[i]);
                                        return false;
                                    }
                                }

                            }
                        }

                    }
                }
            }

        }
        if (roomID != -1) {
            for (int una = 0; una < instance->getRoom(roomID).getSlots().size(); ++una) {
                for (int i = 0; i < instance->getNweek(); ++i) {
                    if (week[i] == instance->getRoom(roomID).getSlots()[una].getWeeks()[i] &&
                        instance->getRoom(roomID).getSlots()[una].getWeeks()[i] == '1') {
                        for (int d = 0; d < instance->getNdays(); ++d) {
                            if (day[d] == instance->getRoom(roomID).getSlots()[una].getDays()[d] && day[d] == '1') {
                                if (start >= instance->getRoom(roomID).getSlots()[una].getStart() &&
                                    start < instance->getRoom(
                                            roomID).getSlots()[una].getStart() + instance->getRoom(
                                            roomID).getSlots()[una].getLenght()) {
                                    /*std::cout<<instance->getRoom(
                                            roomID).getSlots()[una].getStart()<<" "<<instance->getRoom(
                                            roomID).getSlots()[una].getLenght()<<std::endl;
                                    std::cout<<start<<" "<<week<<" "<<day<<" "<<roomID<<std::endl;*/
                                    return false;
                                }
                                if (start + duration >= instance->getRoom(roomID).getSlots()[una].getStart() && start <
                                                                                                                instance->getRoom(
                                                                                                                        roomID).getSlots()[una].getStart() +
                                                                                                                instance->getRoom(
                                                                                                                        roomID).getSlots()[una].getLenght()) {
                                    /*std::cout<<instance->getRoom(
                                            roomID).getSlots()[una].getStart()<<" "<<instance->getRoom(
                                            roomID).getSlots()[una].getLenght()<<std::endl;
                                    std::cout<<start<<" "<<duration<<" "<<week<<" "<<day<<" "<<roomID<<std::endl;*/

                                    return false;
                                }
                            }
                        }

                    }
                }

            }

            for (int h = 0; h < c->getHard().size(); ++h) {
                if (c->getHard()[h]->getType()->getType() == SameAttendees) {
                    for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                        //std::cout<<"TE"<<c->getHard()[h]->getClasses()[i]<<std::endl;
                        if (c->getId() !=
                            c->getHard()[h]->getClasses()[i]
                            && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolStart()
                               != -1 && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolRoom() !=
                                        -1) {
                            if (start + duration + instance->getRoom(roomID).getTravel(instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolRoom()) <=
                                instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolStart()
                                || instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart() +
                                   instance->getClass(
                                           c->getHard()[h]->getClasses()[i])->getSolDuration() +
                                   instance->getRoom(instance->getClass(
                                           c->getHard()[h]->getClasses()[i])->getSolRoom()).getTravel(
                                           roomID) <= start
                                || stringcompare(week, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                                 instance->getNweek(), false) ==
                                   1
                                || stringcompare(day, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                 instance->getNdays(), false) ==
                                   1) { ;
                            } else {
                                printCurrent(c, lec, room, instance->getClass(
                                        c->getHard()[h]->getClasses()[i]));
                                return false;
                            }
                        }
                    }
                } else if (c->getHard()[h]->getType()->getType() == NotOverlap) {
                    for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                        if (c->getId() !=
                            c->getHard()[h]->getClasses()[i]
                            && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolStart()
                               != -1) {
                            if (start + duration <=
                                instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolStart()
                                || instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart() +
                                   instance->getClass(
                                           c->getHard()[h]->getClasses()[i])->getSolDuration() <=
                                   start
                                || stringcompare(week, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                                 instance->getNweek(), false) ==
                                   1
                                || stringcompare(day, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                 instance->getNdays(), false) ==
                                   1) { ;
                            } else {
                                printCurrent(c, lec, room, instance->getClass(
                                        c->getHard()[h]->getClasses()[i]));
                                return false;
                            }
                        }
                    }


                } else if (c->getHard()[h]->getType()->getType() == Overlap) {
                    for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                        if (c->getId() !=
                            c->getHard()[h]->getClasses()[i]
                            && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolStart()
                               != -1) {
                            if (start < (instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart() +
                                         instance->getClass(
                                                 c->getHard()[h]->getClasses()[i])->getSolDuration())
                                || instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart() <
                                   (start + duration)
                                || stringcompare(week, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                                 instance->getNweek(), true) ==
                                   1
                                || stringcompare(day, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                 instance->getNdays(), true) ==
                                   1) { ;
                            } else {
                                printCurrent(c, lec, room, instance->getClass(
                                        c->getHard()[h]->getClasses()[i]));
                                return false;
                            }
                        }
                    }

                } else if (c->getHard()[h]->getType()->getType() == SameStart) {
                    for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                        if (c->getId() !=
                            c->getHard()[h]->getClasses()[i]
                            && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolStart()
                               != -1) {
                            if (start != instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()) {
                                //std::cout<<"start"<<std::endl;
                                printCurrent(c, lec, room, instance->getClass(
                                        c->getHard()[h]->getClasses()[i]));
                                return false;
                            }
                        }
                    }
                } else if (c->getHard()[h]->getType()->getType() == WorkDay) {
                    for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                        if (c->getId() !=
                            c->getHard()[h]->getClasses()[i] && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolStart()
                                                                != -1) {
                            if (stringcompare(week, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                              instance->getNweek(), false) ==
                                1 || stringcompare(day, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                   instance->getNdays(), false) ==
                                     1);
                            else if (std::max(start + duration, instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart() +
                                                                instance->getClass(
                                                                        c->getHard()[h]->getClasses()[i])->getSolDuration()) -
                                     std::min(start, instance->getClass(
                                             c->getHard()[h]->getClasses()[i])->getSolStart()) >
                                     c->getHard()[h]->getType()->getLimit()) {
                                printCurrent(c, lec, room, instance->getClass(
                                        c->getHard()[h]->getClasses()[i]));

                                return false;
                            }

                        }
                    }
                } else if (c->getHard()[h]->getType()->getType() == SameRoom) {
                    for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                        Class *temp = instance->getClass(c->getHard()[h]->getClasses()[i]);
                        if (temp->getSolRoom() !=
                            -1) {

                            if (roomID != instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolRoom()) {
                                //std::cout<<"SAMEROOM"<<std::endl;
                                printCurrent(c, lec, room, instance->getClass(
                                        c->getHard()[h]->getClasses()[i]));
                                return false;
                            }
                        }


                    }
                } else if (c->getHard()[h]->getType()->getType() == DifferentRoom) {

                    for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                        if (c->getId() !=
                            c->getHard()[h]->getClasses()[i]
                            && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolStart()
                               != -1 && instance->getClass(
                                c->getHard()[h]->getClasses()[i])->getSolRoom() !=
                                        -1) {

                            if (roomID == instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolRoom()) {
                                //std::cout<<"DifROOM"<<std::endl;
                                printCurrent(c, lec, room, instance->getClass(
                                        c->getHard()[h]->getClasses()[i]));
                                return false;
                            }
                        }


                    }
                }
            }/* else if (c->getHard()[h]->getType()->getType() == SameStart) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i]
                                && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                   != -1) {
                                if (start != instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolStart()) {
                                    //std::cout<<"start"<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == SameTime) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i]
                                && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                   != -1) {
                                if (duration != instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolDuration()) {
                                    if (start <= instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolStart()
                                        && start + duration <= instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolStart() +
                                                              instance->getClass(
                                                                       c->getHard()[h]->getClasses()[i])->getSolDuration()
                                            );
                                    else if (instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolStart() <=
                                             start
                                             && instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolStart() +
                                                instance->getClass(
                                                        c->getHard()[h]->getClasses()[i])->getSolDuration() <=
                                                start + duration
                                            );
                                    else {
                                        //std::cout<<"sameTime"<<std::endl;
                                        printCurrent(c,lec,room,instance->getClass(
                                                c->getHard()[h]->getClasses()[i]));
                                        return false;
                                    }
                                } else if (start != instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolStart()) {
                                    //std::cout<<"sameTime"<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == DifferentTime) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i]
                                && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                   != -1) {
                                if (instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolStart() +
                                    instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolDuration()
                                    <= start);
                                else if (start + duration <= instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolStart()
                                        );
                                else {
                                    //std::cout<<"difftime"<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == SameWeeks) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i]
                                && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                   != -1) {
                                if (stringcontains(week, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                                   instance->getNweek()) ==
                                    1);
                                else {
                                    //std::cout<<"SameWeeks"<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == DifferentWeeks) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i]
                                && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                   != -1
                                   ) {
                                if (stringcompare(week, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                                  instance->getNweek(),
                                                  false) ==
                                    1);
                                else {
                                    //std::cout<<"DifferentWeeks"<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == DifferentDays) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i]
                                && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                   != -1) {
                                if (stringcompare(day, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                  instance->getNdays(),
                                                  false) ==
                                    1) {
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == SameDays) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i]
                                && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                   != -1) {
                                if (stringcontains(day, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                   instance->getNdays()) ==
                                    1);
                                else {
                                    //std::cout<<"SameDays: "<<c->getHard()[h]->getClasses()[i]<<" "<<instance->getClass(
                                    //    c->getHard()[h]->getClasses()[i])->getOrderID()<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == Precedence) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() ==
                                c->getHard()[h]->getClasses()[i]) {
                                if (i > 0) {
                                    if (instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolStart()
                                        != -1) {
                                        if (isFirst(week, instance->getClass(
                                                c->getHard()[h]->getClasses()[i -
                                                                                                              1])->getSolWeek(),
                                                    instance->getNweek()) == -1) {
                                            //std::cout<<"Precedence"<<std::endl;
                                            printCurrent(c,lec,room,instance->getClass(
                                                    c->getHard()[h]->getClasses()[i]));
                                           return false;
                                        } else if (isFirst(week, instance->getClass(
                                                c->getHard()[h]->getClasses()[i -
                                                                                                              1])->getSolWeek(),
                                                           instance->getNdays()) == 0 && isFirst(day, instance->getClass(
                                                c->getHard()[h]->getClasses()[i -
                                                                                                              1])->getSolDays(),
                                                                                                 instance->getNdays()) == -1) {
                                            //std::cout<<"Precedence"<<std::endl;
                                            printCurrent(c,lec,room,instance->getClass(
                                                    c->getHard()[h]->getClasses()[i]));
                                           return false;
                                        } else if (isFirst(week, instance->getClass(
                                                c->getHard()[h]->getClasses()[i -
                                                                                                              1])->getSolWeek(),
                                                           instance->getNdays()) == 0 && isFirst(day, instance->getClass(
                                                c->getHard()[h]->getClasses()[i -
                                                                                                              1])->getSolDays(),
                                                                                                 instance->getNdays()) == 0 &&
                                                   instance->getClass(
                                                           c->getHard()[h]->getClasses()[i -
                                                                                                                         1])->getSolStart() +
                                                   instance->getClass(
                                                           c->getHard()[h]->getClasses()[i -
                                                                                                                         1])->getSolDuration() >
                                                   start) {
                                            //std::cout<<"Precedence"<<std::endl;
                                            printCurrent(c,lec,room,instance->getClass(
                                                    c->getHard()[h]->getClasses()[i]));
                                            return false;
                                        }

                                    }
                                } else if (i < c->getHard()[h]->getClasses().size()) {
                                    if (instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolStart()
                                        != -1) {
                                        if (isFirst(instance->getClass(
                                                c->getHard()[h]->getClasses()[i +
                                                                                                              1])->getSolWeek(),
                                                    week,
                                                    instance->getNweek()) == -1) {
                                            printCurrent(c,lec,room,instance->getClass(
                                                    c->getHard()[h]->getClasses()[i]));

                                            return false;
                                        } else if (isFirst(instance->getClass(
                                                c->getHard()[h]->getClasses()[i +
                                                                                                              1])->getSolWeek(),
                                                           week,
                                                           instance->getNweek()) == 0 && isFirst(instance->getClass(
                                                c->getHard()[h]->getClasses()[i +
                                                                                                              1])->getSolDays(),
                                                                                                 day,
                                                                                                 instance->getNdays()) == -1) {
                                            //std::cout<<"Precedence"<<std::endl;
                                            printCurrent(c,lec,room,instance->getClass(
                                                    c->getHard()[h]->getClasses()[i]));

                                            return false;
                                        } else if (isFirst(instance->getClass(
                                                c->getHard()[h]->getClasses()[i +
                                                                                                              1])->getSolWeek(),
                                                           week,
                                                           instance->getNweek()) == 0 && isFirst(instance->getClass(
                                                c->getHard()[h]->getClasses()[i +
                                                                                                              1])->getSolDays(),
                                                                                                 day,
                                                                                                 instance->getNdays()) == 0 &&
                                                   start + duration > instance->getClass(
                                                           c->getHard()[h]->getClasses()[i +
                                                                                                                         1])->getSolStart()) {
                                            //std::cout<<"Precedence"<<std::endl;
                                            printCurrent(c,lec,room,instance->getClass(
                                                    c->getHard()[h]->getClasses()[i]));

                                            return false;
                                        }

                                    }
                                }
                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == WorkDay) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i] && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                                                                                    != -1) {
                                if (stringcompare(week, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                                  instance->getNweek(), false) ==
                                    0 || stringcompare(day, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                       instance->getNdays(), false) ==
                                         0);
                                else if (std::max(start + duration, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolStart() +
                                                                    instance->getClass(
                                                                            c->getHard()[h]->getClasses()[i])->getSolDuration()) -
                                         std::min(start, instance->getClass(
                                                 c->getHard()[h]->getClasses()[i])->getSolStart()) >
                                         c->getHard()[h]->getType()->getLimit()) {
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));

                                    return false;
                                }

                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == MinGap) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i] && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                                                                                    != -1) {
                                if (stringcompare(week, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolWeek(),
                                                  instance->getNweek(), false) ==
                                    0 || stringcompare(day, instance->getClass(
                                        c->getHard()[h]->getClasses()[i])->getSolDays(),
                                                       instance->getNdays(), false) ==
                                         0 ||
                                    start + duration + c->getHard()[h]->getType()->getLimit() <=
                                    instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolStart() ||
                                        instance->getClass(
                                                c->getHard()[h]->getClasses()[i])->getSolStart() +
                                                instance->getClass(
                                                        c->getHard()[h]->getClasses()[i])->getSolDuration() +
                                    c->getHard()[h]->getType()->getLimit() <= start);
                                else {
                                    //std::cout<<"MinGap"<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }

                            }
                        }
                    } else if (c->getHard()[h]->getType()->getType() == MaxDays) {
                        int d = 0;
                        for (int i = 0; i < instance->getNdays(); ++i) {
                            if (day[i] == '1')
                                d++;
                        }
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i] && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                                                                                    != -1) {
                                for (int d = 0; d < instance->getNdays(); ++d) {
                                    if (instance->getClass(
                                            c->getHard()[h]->getClasses()[i])->getSolDays()[d] ==
                                        '1')
                                        d++;
                                }
                                if (d > c->getHard()[h]->getType()->getLimit()) {
                                    //std::cout<<"MaxDays"<<std::endl;
                                    printCurrent(c,lec,room,instance->getClass(
                                            c->getHard()[h]->getClasses()[i]));
                                    return false;
                                }

                            }
                        }

                    } else if (c->getHard()[h]->getType()->getType() == MaxDayLoad) {
                        for (int i = 0; i < c->getHard()[h]->getClasses().size(); ++i) {
                            if (c->getId() !=
                                c->getHard()[h]->getClasses()[i] && instance->getClass(
                                    c->getHard()[h]->getClasses()[i])->getSolStart()
                                                                                                    != -1) {
                                for (int w = 0; w < instance->getNweek(); ++w) {
                                    for (int d = 0; d < instance->getNdays(); ++d) {


                                    }

                                }

                            }
                        }

                    }*/




        }

        return true;

    }

    void printCurrent(Class *c1, int lec, int room, Class *c2) {
        /*std::cout<<"Conflict Start"<<std::endl;
        std::cout<<c1->getLectures()[lec]->getStart()<<" "<<c1->getLectures()[lec]->getLenght()<<" "
                <<c1->getLectures()[lec]->getDays()<<" "
                <<c1->getLectures()[lec]->getWeeks()<<" ";
        if(c1->getPossibleRooms().size()>0)
            std::cout<<c1->getPossibleRoomPair(room).first.getId()<<" "<<c1->getId()<<std::endl;
        std::cout<<*c2->getSolution()<<" "<<c2->getId()<<std::endl;
        std::cout<<"Conflict End"<<std::endl;*/


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
            else if (!same && s2[i] == s1[i] && '1' == s1[i])
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


};


#endif //PROJECT_GRASP_H
