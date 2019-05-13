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

    //Local();
    printStatus(0);
    for (int i = 1; i < MAX_ITERATIONS; i++) {
        init();
        Greedy();
        store();
        //  Local();
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
    init();
    sizeRCL = instance->getClasses().size() * rcl;

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
            if ((co = assign(tabu[l])) != -1) {
                allocation++;
                currentV += co;
                std::cerr << "Here!! " << getTimeSpent() << std::endl;

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

int LocalSearch::checkUpdate(int maxCost, int id, int time, const std::pair<Room, int> &room) {
    int roomID = room.first.getId();
    int costR = room.second;
    int costT;
    if ((costT = isAllocable(instance->getClasses()[id]->getId(),
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
            for (int i1 = i + 1; i1 < instance->getClasses().size(); ++i1) {
                if (instance->getClasses()[i]->getLenght() == instance->getClasses()[i1]->getLenght()) {
                    if (tryswampLectures(i, i1, instance->getClasses()[i]->getSolDay() - 1,
                                         instance->getClasses()[i]->getSolStart(),
                                         instance->getClasses()[i]->getLenght(),
                                         instance->getClasses()[i1]->getSolDay() - 1,
                                         instance->getClasses()[i1]->getSolStart(),
                                         instance->getClasses()[i1]->getLenght())) {
                        swampLectures(i, i1, instance->getClasses()[i]->getSolDay() - 1,
                                      instance->getClasses()[i]->getSolStart(), instance->getClasses()[i]->getLenght(),
                                      instance->getClasses()[i1]->getSolDay() - 1,
                                      instance->getClasses()[i1]->getSolStart(),
                                      instance->getClasses()[i1]->getLenght());
                        swamp = true;
                    }

                }

            }

        }


        if (!swamp)
            move = false;

    }

}

int LocalSearch::isAllocable(int lectureID, std::string week, std::string day, int start, int duration, int roomID) {
    int cost =0 ;
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        if (current[i] != nullptr && roomID != -1) {
            if(current[i]->getSolRoom()==roomID){
                for (int j = 0; j < instance->getNweek(); ++j) {
                    if(week[j]==current[i]->getSolWeek()[j] && week[j]=='1'){
                        for (int d = 0; d < instance->getNdays(); ++d) {
                            if(day[d]==current[i]->getSolDays()[d]&&day[d]=='1'){
                                if(current[i]->getSolStart()>=start && current[i]->getSolStart()<start+duration) {
                                    return -1;
                                }
                                if(start>=current[i]->getSolStart() && start<current[i]->getSolStart() + current[i]->getDuration()) {
                                    return -1;
                                }
                            }

                        }
                    }

                }
            }
        }

    }
    //cost+=;
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

bool LocalSearch::tryswampLectures(int l1, int l2, int d1, int t1, int le1, int d2, int t2, int le2) {
    return true;
}

void
LocalSearch::swampLectures(int lect1, int lect2, int day1, int start1, int lenght1, int day2, int start2, int lenght2) {


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
