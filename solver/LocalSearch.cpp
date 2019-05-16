//
// Created by Alexandre Lemos on 15/03/2019.
//


#include "LocalSearch.h"

void LocalSearch::LNS() {
    init();
    Local();
    printFinal();


}


void LocalSearch::GRASP() {
    init();
    Greedy();
    store();
    printStatus(0);
    Local();
    store();
    printStatus(0);
    for (int i = 1; i < MAX_ITERATIONS; i++) {
        init();
        Greedy();
        store();
        printStatus(i);
        Local();
        store();
        printStatus(i);

    }
    printFinal();

}

bool LocalSearch::eval() {
    return best > currentV;

}

LocalSearch::LocalSearch(int MAX_ITERATIONS, double rcl, Instance *instance) : MAX_ITERATIONS(MAX_ITERATIONS),
                                                                               instance(instance) {
    instance->setCompact(false);
    instance->setMethod("LocalSearch");
    init();
    totalNassigment = 0;
    sizeRCL = instance->getClasses().size() * rcl;
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
    sizeRCLS = instance->getStudent().size() * rcl;


}

void LocalSearch::init() {
    current = new Solution*[instance->getClasses().size()];
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        current[i]= nullptr;
    }
    currentV = 0;
}


void LocalSearch::store() {
    if (eval()) {
        best = currentV;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            instance->getClasses()[i]->setSolution(current[i]);
        }
    }

}

void LocalSearch::printStatus(int ite) {
    std::cout << "Iteration: " << ite << " " << best << std::endl;
}

void LocalSearch::printFinal() {
    std::cout << "Best result: " << best << std::endl;
}

int LocalSearch::getGAP() const {
    return 0;
}

void LocalSearch::Greedy() {
    int allocation = 0;
    currentV = 0;
    while (instance->getClasses().size() > allocation) {

        int maxCost = INT_MAX;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            if (current[i] == nullptr) {
                for (int time = 0; time < instance->getClasses()[i]->getLectures().size(); ++time) {
                    if (instance->getClasses()[i]->getPossibleRooms().size() > 0) {
                        for (std::pair<Room, int> room: instance->getClasses()[i]->getPossibleRooms()) {
                            maxCost = checkUpdate(maxCost, i, time, room);
                        }
                    } else {
                        std::pair<Room, int> room(Room(-1), 0);
                        maxCost = checkUpdate(maxCost, i, time, room);
                    }

                }
            }
        }

        if (!tabu.empty()) {
            std::default_random_engine generator(seedHandler());
            std::uniform_int_distribution<int> distribution(0, tabu.size() - 1);
            int l = distribution(generator);
            int co = 0;
            while ((co = assign(tabu[l])) != -1) {
                if (co != -1) {
                    allocation++;
                    currentV += co;
                    break;
                    std::cerr << "Here!! " << getTimeSpent() << " " << allocation << " " << tabu[l]->getLecture() << " "
                              << tabu[l]->getSolRoom() << std::endl;
                } else {
                    tabu.erase(tabu.begin() + l);
                    if (tabu.empty())
                        break;
                }
            }


        } else {
            std::cout << "Not FOUND " << instance->getClasses().size() << std::endl;
            currentV = -1;
            return;
        }
        tabu.clear();
    }

}

int LocalSearch::checkUpdate(int maxCost, int id, int time, const std::pair<Room, int> &room) {
    int roomID = room.first.getId();
    int costR = room.second;
    int costT;
    if ((costT = isAllocable(id,
                             instance->getClasses()[id]->getLectures()[time]->getWeeks(),
                             instance->getClasses()[id]->getLectures()[time]->getDays(),
                             instance->getClasses()[id]->getLectures()[time]->getStart(),
                             instance->getClasses()[id]->getLectures()[time]->getLenght(),
                             roomID) != -1)) {
        if (maxCost > (costT + costR)) {
            if (tabu.size() == sizeRCL) {
                tabu.pop_back();
                maxCost = costT + costR;
            }
            tabu.push_back(new Solution(id,
                                        instance->getClasses()[id]->getLectures()[time]->getStart(),
                                        roomID,
                                        instance->getClasses()[id]->getLectures()[time]->getWeeks(),
                                        instance->getClasses()[id]->getLectures()[time]->getDays(),
                                        instance->getClasses()[id]->getLectures()[time]->getLenght()));


        }
    }
    return maxCost;
}

void LocalSearch::Local() {
    bool move = true;
    while (move) {
        bool swamp = false;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            for (std::pair<Room, int> room: instance->getClasses()[i]->getPossibleRooms()) {
                if (tryswampLectures(i, room.first.getId())) {
                    current[i]->setSolRoom(room.first.getId());
                        swamp = true;
                    }

                }

            }




        if (!swamp)
            move = false;

    }
    move = true;
    while (move) {
        bool swamp = false;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            for (int time = 0; time < instance->getClasses()[i]->getLectures().size(); ++time) {
                if (tryswampLectures(i, instance->getClasses()[i]->getLectures()[time]->getWeeks(),
                                     instance->getClasses()[i]->getLectures()[time]->getDays(),
                                     instance->getClasses()[i]->getLectures()[time]->getStart(),
                                     instance->getClasses()[i]->getLectures()[time]->getLenght())) {
                    swampLectures(i, instance->getClasses()[i]->getLectures()[time]->getWeeks(),
                                  instance->getClasses()[i]->getLectures()[time]->getDays(),
                                  instance->getClasses()[i]->getLectures()[time]->getStart(),
                                  instance->getClasses()[i]->getLectures()[time]->getLenght());
                    swamp = true;
                }

            }

        }


        if (!swamp)
            move = false;

    }


}

unsigned long long int
LocalSearch::isAllocable(int lectureID, std::string week, std::string day, int start, int duration, int roomID) {
    int cost = 0;
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        if (current[i] != nullptr && roomID != -1 && i != lectureID) {
            if(current[i]->getSolRoom()==roomID){
                for (int j = 0; j < instance->getNweek(); ++j) {
                    if(week[j]==current[i]->getSolWeek()[j] && week[j]=='1'){
                        for (int d = 0; d < instance->getNdays(); ++d) {
                            if(day[d]==current[i]->getSolDays()[d]&&day[d]=='1'){
                                if(current[i]->getSolStart()>=start && current[i]->getSolStart()<start+duration) {
                                    //std::cout<<instance->getClasses()[lectureID]->getId()<<" "<<instance->getClasses()[i]->getId()<<std::endl;
                                    return ULLONG_MAX;
                                } else if (start >= current[i]->getSolStart() &&
                                           start < current[i]->getSolStart() + current[i]->getDuration()) {
                                    //std::cout<<instance->getClasses()[lectureID]->getId()<<" "<<instance->getClasses()[i]->getId()<<std::endl;
                                    return ULLONG_MAX;
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
                                        roomID).getSlots()[una].getLenght())
                                return ULLONG_MAX;
                            if (start + duration >= instance->getRoom(roomID).getSlots()[una].getStart() && start <
                                                                                                            instance->getRoom(
                                                                                                                    roomID).getSlots()[una].getStart() +
                                                                                                            instance->getRoom(
                                                                                                                    roomID).getSlots()[una].getLenght())
                                return ULLONG_MAX;
                        }
                    }

                }
            }

        }

        for (int h = 0; h < instance->getClasses()[lectureID]->getHard().size(); ++h) {
            if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameAttendees) {
                for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                    if (instance->getClasses()[lectureID]->getId() !=
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                        && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                           != nullptr && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom() !=
                                         -1) {
                        if (start + duration + instance->getRoom(roomID).getTravel(current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()) <=
                            current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()
                            || current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                               current[instance->getClass(
                                       instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration() +
                               instance->getRoom(current[instance->getClass(
                                       instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()).getTravel(
                                       roomID) <= start
                            || week.compare(current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek()) ==
                               0
                            || day.compare(current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays()) ==
                               0) { ;
                        } else
                            return ULLONG_MAX;
                    }
                }
            } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameRoom) {
                for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                    if (instance->getClasses()[lectureID]->getId() !=
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                        && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                           != nullptr && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom() !=
                                         -1) {
                        std::cout << instance->getClasses()[lectureID]->getId() << " " << roomID << " "
                                  << current[instance->getClass(
                                          instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()
                                  << " " << instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()
                                  << std::endl;
                        if (roomID != current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()) {
                            return ULLONG_MAX;
                        }
                    }

                }

            } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameAttendees) {
                for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                    if (instance->getClasses()[lectureID]->getId() !=
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                        && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                           != nullptr && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom() !=
                                         -1) {
                        if (start + duration <=
                            current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()
                            || current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                               current[instance->getClass(
                                       instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration() <=
                               start
                            || week.compare(current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek()) ==
                               0
                            || day.compare(current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays()) ==
                               0) { ;
                        } else
                            return ULLONG_MAX;
                    }
                }


            }


        }
        cost += instance->getClasses()[lectureID]->getPossibleRoomCost(roomID);
    }
    return cost;
}

double LocalSearch::cost(int lectureID, int roomID, int day, int timeS) {


}


/**
 * Generate a seed based on the clock
 * @return seed
 */
unsigned int LocalSearch::seedHandler() {
    unsigned int t = std::chrono::steady_clock::now().time_since_epoch().count();
    seedFile.open("../log/seed.txt", std::ios_base::app);
    seedFile << t << std::endl;
    seedFile.close();
    return t;
}

int LocalSearch::assign(Solution *t) {
    int cost = 0;
    if ((cost = isAllocable(t->getLecture(), t->getSolWeek(), t->getSolDays(), t->getSolStart(), t->getDuration(),
                            t->getSolRoom())) == -1)
        return -1;
    current[t->getLecture()] = t;
    return cost;
}

bool LocalSearch::tryswampLectures(int id, const std::string &week, std::string &day, int start, int le) {
    return isAllocable(id, week, day, start, le, current[id]->getSolRoom()) != -1;
}

void
LocalSearch::swampLectures(int id, const std::string &week, std::string &day, int start, int le) {
    Solution *s = new Solution(id, start, current[id]->getSolRoom(), week, day, le);
    current[id] = s;
}

int LocalSearch::getGAPStored() {
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

LocalSearch::LocalSearch(Instance *pInstance) : instance(pInstance) {

}

bool LocalSearch::tryswampLectures(int lecture, int roomID) {
    return isAllocable(lecture, current[lecture]->getSolWeek(), current[lecture]->getSolDays(),
                       current[lecture]->getSolStart(), current[lecture]->getLecture(), roomID) != -1;;
}
