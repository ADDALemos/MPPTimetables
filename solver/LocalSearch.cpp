//
// Created by Alexandre Lemos on 15/03/2019.
//


#include <list>
#include "LocalSearch.h"

void LocalSearch::LNS() {
    init();
    Local();
    printFinal();


}


void LocalSearch::GRASP() {
    lsDivided = new LSDivided(instance, false);
    lsDivided->init();
    greedyStu();
    printTime();
    for (int i = 0; i < MAX_ITERATIONS && getTimeSpent() <= time; i++) {
        init();
        printTime();
        Greedy();
        Greedy1();
        printTime();
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

LocalSearch::LocalSearch(int MAX_ITERATIONS, double rcl, Instance *instance, int seconds) : MAX_ITERATIONS(
        MAX_ITERATIONS),
                                                                                            instance(instance),
                                                                                            time(seconds) {
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
    current = new Solution *[instance->getClasses().size()];
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        current[instance->getClasses()[i]->getOrderID()] = nullptr;
    }
    currentV = 0;
    tabu.clear();
}


void LocalSearch::store() {
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

void LocalSearch::printStatus(int ite) {
    std::cout << "Iteration: " << ite << " Best: " << best << " Current: " << currentV << std::endl;
}

void LocalSearch::printFinal() {
    std::cout << "Best result: " << best << std::endl;
}

int LocalSearch::getGAP() const {
    return 0;
}

void LocalSearch::Greedy1() {
    int allocation = 0;
    currentV = 0;
    while ((instance->getClasses().size() - alreadyDone) != allocation) {

        int maxCost = INT_MAX;
        for (int part = 0; part < lsDivided->div.size(); part++) {
            for (std::set<int>::iterator it = lsDivided->div[part].begin(); it != lsDivided->div[part].end(); ++it) {
                int i = instance->getClass(*it)->getOrderID();
                if ((instance->getClasses()[i]->getPossibleRooms().size() +
                     instance->getClasses()[i]->getLectures().size()) > sizeWindow) {
                    if (current[instance->getClasses()[i]->getOrderID()] == nullptr) {
                        if (instance->getClasses()[i]->getPossibleRooms().size() > 0) {
                            std::default_random_engine generatorR(seedHandler());
                            std::uniform_int_distribution<int> distributionR(0,
                                                                             instance->getClasses()[i]->getPossibleRooms().size() -
                                                                             1);
                            std::default_random_engine generatorT(seedHandler());
                            std::uniform_int_distribution<int> distributionT(0,
                                                                             instance->getClasses()[i]->getLectures().size() -
                                                                             1);
                            int it = 0;
                            int t = distributionT(generatorT), r = distributionR(generatorR);
                            while (checkUpdate(maxCost, i, t, instance->getClasses()[i]->getPossibleRoomPair(r)) != 0 &&
                                   it <= instance->getClasses()[i]->getPossibleRooms().size() *
                                         instance->getClasses()[i]->getLectures().size()) {

                                r = distributionR(generatorR);

                                t = distributionT(generatorT);
                                it++;

                            }
                        } else {
                            int it = 0;
                            std::default_random_engine generatorT(seedHandler());
                            std::uniform_int_distribution<int> distributionT(0,
                                                                             instance->getClasses()[i]->getLectures().size() -
                                                                             1);
                            int t = distributionT(generatorT);
                            std::pair<Room, int> room(Room(-1), 0);
                            while (checkUpdate(maxCost, i, t, room) != 0 &&
                                   it <= instance->getClasses()[i]->getLectures().size()) {

                                t = distributionT(generatorT);
                                it++;

                            }
                        }
                    }
                }
            }


            //std::cout<<tabu.size()<<std::endl;
            if (tabu.empty()) {
                if (instance->getClasses().size() != (allocation + alreadyDone))
                    currentV += INT_MAX * (instance->getClasses().size() - (allocation + alreadyDone));
                alreadyDone += allocation;
                return;
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
                        allocation++;
                        //std::cout<<*tabu[l]<<" A"<<instance->getClasses()[tabu[l]->getLecture()]->getId()<<" "<<(instance->getClasses()[tabu[l]->getLecture()]->getPossibleRooms().size() +
                        //                                                                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures().size() == wind)<<std::endl;
                        currentV += co;
                    } else {
                        //std::cout<<"why "<<*tabu[l]<<" A"<<instance->getClasses()[tabu[l]->getLecture()]->getId()<<"B"<<instance->getClasses()[co]->getId()<<std::endl;
                        if (isRoomProblem) {
                            for (int rP = 0;
                                 rP < instance->getClasses()[tabu[l]->getLecture()]->getPossibleRooms().size(); ++rP) {
                                if (isAllocable(tabu[l]->getLecture(),
                                                tabu[l]->getSolWeek(),
                                                tabu[l]->getSolDays(),
                                                tabu[l]->getSolStart(),
                                                tabu[l]->getDuration(),
                                                instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                        rP).first.getId()) == 0) {
                                    tabu[l]->setSolRoom(
                                            instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                    rP).first.getId());
                                    assign(tabu[l]);
                                    allocation++;
                                    break;
                                }
                            }


                        } else {
                            int v = 0;
                            for (int rP = 0;
                                 rP < instance->getClasses()[tabu[l]->getLecture()]->getLectures().size(); ++rP) {
                                if ((v = isAllocable(tabu[l]->getLecture(),
                                                     instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getWeeks(),
                                                     instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getDays(),
                                                     instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getStart(),
                                                     instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getLenght(),
                                                     tabu[l]->getSolRoom())) == 0) {

                                    tabu[l] = new Solution(tabu[l]->getLecture(),
                                                           instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getStart(),
                                                           tabu[l]->getSolRoom(),
                                                           instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getWeeks(),
                                                           instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getDays(),
                                                           instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getLenght(),
                                                           instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getPenalty(),
                                                           -1);
                                    assign(tabu[l]);
                                    allocation++;
                                    break;
                                }
                            }
                            if (current[instance->getClasses()[tabu[l]->getLecture()]->getOrderID()] == nullptr) {
                                for (int rP = 0;
                                     rP <
                                     instance->getClasses()[tabu[l]->getLecture()]->getPossibleRooms().size(); ++rP) {
                                    for (int tP = 0;
                                         tP <
                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures().size(); ++tP) {
                                        if ((v = isAllocable(tabu[l]->getLecture(),
                                                             instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getWeeks(),
                                                             instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getDays(),
                                                             instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getStart(),
                                                             instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getLenght(),
                                                             instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                                     rP).first.getId())) == 0) {


                                            tabu[l] = new Solution(tabu[l]->getLecture(),
                                                                   instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getStart(),
                                                                   instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                                           rP).first.getId(),
                                                                   instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getWeeks(),
                                                                   instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getDays(),
                                                                   instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getLenght(),
                                                                   instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getPenalty(),
                                                                   -1);
                                            assign(tabu[l]);
                                            allocation++;
                                            break;
                                        }
                                    }
                                    if (current[instance->getClasses()[tabu[l]->getLecture()]->getOrderID()] !=
                                        nullptr) {
                                        break;
                                        //conflict(tabu[l]->getLecture());
                                    }

                                }

                            }
                        }
                    }


                    tabu.erase(tabu.begin() + l);
                }


            }
        }
    }

    if (instance->getClasses().size() != (allocation + alreadyDone))
        currentV += INT_MAX * (instance->getClasses().size() - (allocation + alreadyDone));
    alreadyDone += allocation;


}


void LocalSearch::Greedy() {
    int allocation = 0;
    int futureAlloc = -1;
    currentV = 0;
    int wind = 1;
    while (futureAlloc != allocation) {
        futureAlloc = 0;

        int maxCost = INT_MAX;
        bool cont = false;

        for (int part = 0; part < lsDivided->div.size(); part++) {
            for (std::set<int>::iterator it = lsDivided->div[part].begin(); it != lsDivided->div[part].end(); ++it) {
                int i = instance->getClass(*it)->getOrderID();
                if ((instance->getClasses()[i]->getPossibleRooms().size() +
                     instance->getClasses()[i]->getLectures().size()) == wind) {
                    futureAlloc++;
                    cont = true;
                    if (current[instance->getClasses()[i]->getOrderID()] == nullptr) {
                        if (instance->getClasses()[i]->getPossibleRooms().size() > 0) {
                            std::default_random_engine generatorR(seedHandler());
                            std::uniform_int_distribution<int> distributionR(0,
                                                                             instance->getClasses()[i]->getPossibleRooms().size() -
                                                                             1);
                            std::default_random_engine generatorT(seedHandler());
                            std::uniform_int_distribution<int> distributionT(0,
                                                                             instance->getClasses()[i]->getLectures().size() -
                                                                             1);
                            int it = 0;
                            int t = distributionT(generatorT), r = distributionR(generatorR);
                            while (checkUpdate(maxCost, i, t, instance->getClasses()[i]->getPossibleRoomPair(r)) != 0 &&
                                   it <= instance->getClasses()[i]->getPossibleRooms().size() *
                                         instance->getClasses()[i]->getLectures().size()) {

                                r = distributionR(generatorR);

                                t = distributionT(generatorT);
                                it++;

                            }
                            /*int remo = 0;
                            while ((remo = checkUpdate(maxCost, i, t, instance->getClasses()[i]->getPossibleRoomPair(r))) !=
                                   0) {
                                if (instance->getClasses()[i]->getId() == instance->getClasses()[remo]->getId())
                                    break;
                                current[instance->getClasses()[remo]->getOrderID()] = nullptr;
                                allocation--;
                            }*/
                        } else {
                            int it = 0;
                            std::default_random_engine generatorT(seedHandler());
                            std::uniform_int_distribution<int> distributionT(0,
                                                                             instance->getClasses()[i]->getLectures().size() -
                                                                             1);
                            int t = distributionT(generatorT);
                            std::pair<Room, int> room(Room(-1), 0);
                            while (checkUpdate(maxCost, i, t, room) != 0 &&
                                   it <= instance->getClasses()[i]->getLectures().size()) {

                                t = distributionT(generatorT);
                                it++;

                            }
                        }
                    }
                }
            }
        }

        //std::cout<<tabu.size()<<std::endl;
        if (tabu.empty()) {
            wind++;
        }
        if (tabu.empty() && wind == sizeWindow) {
            alreadyDone = allocation;
            return;
        }
        if (!cont) {
            alreadyDone = allocation;
            if (futureAlloc != allocation)
                currentV = INT_MAX * (futureAlloc - allocation);
            return;
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
                    allocation++;
                    //std::cout<<*tabu[l]<<" A"<<instance->getClasses()[tabu[l]->getLecture()]->getId()<<" "<<(instance->getClasses()[tabu[l]->getLecture()]->getPossibleRooms().size() +
                    //                                                                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures().size() == wind)<<std::endl;
                    currentV += co;
                } else {
                    //std::cout<<"why "<<*tabu[l]<<" A"<<instance->getClasses()[tabu[l]->getLecture()]->getId()<<"B"<<instance->getClasses()[co]->getId()<<std::endl;
                    if (isRoomProblem) {
                        for (int rP = 0;
                             rP < instance->getClasses()[tabu[l]->getLecture()]->getPossibleRooms().size(); ++rP) {
                            if (isAllocable(tabu[l]->getLecture(),
                                            tabu[l]->getSolWeek(),
                                            tabu[l]->getSolDays(),
                                            tabu[l]->getSolStart(),
                                            tabu[l]->getDuration(),
                                            instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                    rP).first.getId()) == 0) {
                                tabu[l]->setSolRoom(
                                        instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                rP).first.getId());
                                assign(tabu[l]);
                                allocation++;
                                break;
                            }
                        }


                    } else {
                        int v = 0;
                        for (int rP = 0;
                             rP < instance->getClasses()[tabu[l]->getLecture()]->getLectures().size(); ++rP) {
                            if ((v = isAllocable(tabu[l]->getLecture(),
                                                 instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getWeeks(),
                                                 instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getDays(),
                                                 instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getStart(),
                                                 instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getLenght(),
                                                 tabu[l]->getSolRoom())) == 0) {

                                tabu[l] = new Solution(tabu[l]->getLecture(),
                                                       instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getStart(),
                                                       tabu[l]->getSolRoom(),
                                                       instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getWeeks(),
                                                       instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getDays(),
                                                       instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getLenght(),
                                                       instance->getClasses()[tabu[l]->getLecture()]->getLectures()[rP]->getPenalty(),
                                                       -1);
                                assign(tabu[l]);
                                allocation++;
                                break;
                            }
                        }
                        if (current[instance->getClasses()[tabu[l]->getLecture()]->getOrderID()] == nullptr) {
                            for (int rP = 0;
                                 rP <
                                 instance->getClasses()[tabu[l]->getLecture()]->getPossibleRooms().size(); ++rP) {
                                for (int tP = 0;
                                     tP <
                                     instance->getClasses()[tabu[l]->getLecture()]->getLectures().size(); ++tP) {
                                    if ((v = isAllocable(tabu[l]->getLecture(),
                                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getWeeks(),
                                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getDays(),
                                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getStart(),
                                                         instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getLenght(),
                                                         instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                                 rP).first.getId())) == 0) {

                                        tabu[l] = new Solution(tabu[l]->getLecture(),
                                                               instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getStart(),
                                                               instance->getClasses()[tabu[l]->getLecture()]->getPossibleRoomPair(
                                                                       rP).first.getId(),
                                                               instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getWeeks(),
                                                               instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getDays(),
                                                               instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getLenght(),
                                                               instance->getClasses()[tabu[l]->getLecture()]->getLectures()[tP]->getPenalty(),
                                                               -1);
                                        assign(tabu[l]);
                                        allocation++;
                                        break;
                                    }
                                    if (current[instance->getClasses()[tabu[l]->getLecture()]->getOrderID()] !=
                                        nullptr) {
                                        //conflict(tabu[l]->getLecture());
                                        break;
                                    }

                                }
                            }

                        }
                    }
                }

                tabu.erase(tabu.begin() + l);
            }


        }
    }


    if (instance->getClasses().size() != allocation)
        currentV = INT_MAX * (instance->getClasses().size() - allocation);
    alreadyDone = allocation;




}

void LocalSearch::greedyStu() {
    if (instance->getStudent().size() > 0) {
        int totalAss = 0;
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
                        c1->addStudent(it->second.getId());
                        stu[it->second.getId() - 1][c1->getOrderID()] = 1;
                    }

                }
            }


        }

    }


}


int LocalSearch::checkUpdate(int maxCost, int id, int time, const std::pair<Room, int> &room) {
    int roomID = room.first.getId();
    int costR = room.second;
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


void LocalSearch::Local() {

    int cost = 0;
    int old = 0;
    bool bigmove = true;
    std::cout << "LNS: " << getTimeSpent() << std::endl;
    while (bigmove) {
        bool bigSwamp = false;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            if (current[instance->getClasses()[i]->getOrderID()] == nullptr) {
                for (int j = 0; j < instance->getClasses()[i]->getHard().size(); ++j) {
                    for (int k = 0; k < instance->getClasses()[i]->getHard()[j]->getClasses().size(); ++k) {
                        if (current[instance->getClass(
                                instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] != nullptr) {
                            int n = 0;
                            Solution *t = current[instance->getClass(
                                    instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()];
                            current[instance->getClass(
                                    instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] = nullptr;
                            std::vector<Solution *> p1 = findPossibleMove(i);
                            if (p1.size() == 0) {
                                current[instance->getClass(
                                        instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] = t;
                                continue;
                            }
                            std::vector<Solution *> p2 = findPossibleMove(instance->getClass(
                                    instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID());
                            for (Solution *sp1: p1) {
                                for (Solution *sp2: p2) {
                                    current[instance->getClasses()[i]->getOrderID()] = sp1;
                                    if (isAllocable(sp2->getLecture(), sp2->getSolWeek(), sp2->getSolDays(),
                                                    sp2->getSolStart(), sp2->getDuration(), sp2->getSolRoom())) {
                                        current[instance->getClass(
                                                instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] = sp2;
                                        currentV -= INT_MAX;
                                        break;
                                    }
                                }
                                if (current[instance->getClasses()[i]->getOrderID()] != nullptr &&
                                    current[instance->getClass(
                                            instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] !=
                                    nullptr)
                                    break;

                            }
                            if (current[instance->getClass(
                                    instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] == nullptr)
                                current[instance->getClass(
                                        instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] = t;


                            /*while(n<instance->getClasses()[i]->getPossibleRooms().size()*instance->getClasses()[i]->getLectures().size()) {
                                tryMove(i, n);
                                tryMove(instance->getClass(
                                        instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID(), 0);
                                if (current[instance->getClass(
                                        instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()] !=
                                    nullptr && current[instance->getClasses()[i]->getOrderID()] != nullptr) {
                                    currentV -= INT_MAX;
                                    bigSwamp = true;
                                    break;
                                }
                                current[instance->getClass(instance->getClasses()[i]->getHard()[j]->getClasses()[k])->getOrderID()]= t;
                                current[i]= nullptr;

                                n++;
                            }*/


                        }
                        if (current[instance->getClasses()[i]->getOrderID()] != nullptr) {
                            break;
                        }


                    }
                    if (current[instance->getClasses()[i]->getOrderID()] != nullptr) {
                        break;
                    }
                }
            }
        }


        if (!bigSwamp)
            bigmove = false;
    }
    std::cout << "LNS-END: " << getTimeSpent() << std::endl;


}


int
LocalSearch::isAllocable(int lectureID, std::string week, std::string day, int start, int duration, int roomID) {
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
                                } else if (start >= current[instance->getClasses()[i]->getOrderID()]->getSolStart() &&
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
                                isRoomProblem = true;
                                problems = problemsTemp;
                                return lectureID;
                            }
                            if (start + duration >= instance->getRoom(roomID).getSlots()[una].getStart() && start <
                                                                                                            instance->getRoom(
                                                                                                                    roomID).getSlots()[una].getStart() +
                                                                                                            instance->getRoom(
                                                                                                                    roomID).getSlots()[una].getLenght()) {
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

    for (int h = 0; h < instance->getClasses()[lectureID]->getHard().size(); ++h) {
        if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameAttendees) {
            if (roomID != -1) {
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
                            || stringcompare(week, current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek(),
                                             instance->getNweek(), false) ==
                               0
                            || stringcompare(day, current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                             instance->getNdays(), false) ==
                               0) {
                            /*std::cout<<
                                    instance->getClasses()[lectureID]->getId()<<" "<<instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]<<std::endl;
                            std::cout<<current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                                       current[instance->getClass(
                                               instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration() +
                                       instance->getRoom(current[instance->getClass(
                                               instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()).getTravel(
                                               roomID)<<" "<<start<<std::endl;
                            std::cout<<start + duration + instance->getRoom(roomID).getTravel(current[instance->getClass(instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()) <<" "<<
                                        current[instance->getClass(
                                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()<<std::endl;
                            std::cout<< stringcompare(day, current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                                      instance->getNdays(), false) <<" OLD "<<current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays()<<" "<<day<<std::endl;*/
                        } else {
                            //std::cout<<"SAME"<<std::endl;
                            isRoomProblem = false;
                            problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                            return current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                        }
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameRoom) {
            if (roomID != -1) {
                for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                    if (instance->getClasses()[lectureID]->getId() !=
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                        && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                           != nullptr && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom() !=
                                         -1) {
                        /*std::cout << instance->getClasses()[lectureID]->getId() << " " << roomID << " "
                                  << current[instance->getClass(
                                          instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()
                                  << " " << instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()
                                  << std::endl;*/
                        if (roomID != current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()) {
                            //std::cout<<"SAMEROOM"<<std::endl;
                            isRoomProblem = true;
                            problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                            return current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                        }
                    }

                }
            }

        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == DifferentRoom) {
            if (roomID != -1) {
                for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                    if (instance->getClasses()[lectureID]->getId() !=
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                        && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                           != nullptr && current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom() !=
                                         -1) {
                        /*std::cout << instance->getClasses()[lectureID]->getId() << " " << roomID << " "
                                  << current[instance->getClass(
                                          instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()
                                  << " " << instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()
                                  << std::endl;*/
                        if (roomID == current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolRoom()) {
                            //std::cout<<"DifROOM"<<std::endl;
                            isRoomProblem = true;
                            problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                            return current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                        }
                    }

                }
            }

        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == NotOverlap) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (start + duration <=
                        current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()
                        || current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                           current[instance->getClass(
                                   instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration() <=
                           start
                        || stringcompare(week, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek(),
                                         instance->getNweek(), false) ==
                           0
                        || stringcompare(day, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                         instance->getNdays(), false) ==
                           0) { ;
                    } else {
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        //std::cout<<"!OVER"<<std::endl;
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }


        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == Overlap) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (start < (current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                                 current[instance->getClass(
                                         instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration())
                        || current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() <
                           (start + duration)
                        || stringcompare(week, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek(),
                                         instance->getNweek(), true) ==
                           1
                        || stringcompare(day, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                         instance->getNdays(), true) ==
                           1) { ;
                    } else {
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        //std::cout<<"OVER"<<std::endl;
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }

        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameStart) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (start != current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()) {
                        //std::cout<<"start"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameTime) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (duration != current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration()) {
                        if (start <= current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()
                            && start + duration <= current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                                                   current[instance->getClass(
                                                           instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration()
                                );
                        else if (current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() <=
                                 start
                                 && current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                                    current[instance->getClass(
                                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration() <=
                                    start + duration
                                );
                        else {
                            //std::cout<<"sameTime"<<std::endl;
                            isRoomProblem = false;
                            problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                            return current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                        }
                    } else if (start != current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()) {
                        //std::cout<<"sameTime"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == DifferentTime) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                        current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration()
                        <= start);
                    else if (start + duration <= current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()
                            );
                    else {
                        //std::cout<<"difftime"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameWeeks) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (stringcontains(week, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek(),
                                       instance->getNweek()) ==
                        1);
                    else {
                        //std::cout<<"SameWeeks"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == DifferentWeeks) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (stringcompare(week, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek(),
                                      instance->getNweek(),
                                      false) ==
                        1);
                    else {
                        //std::cout<<"DifferentWeeks"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == DifferentDays) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (stringcompare(day, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                      instance->getNdays(),
                                      false) ==
                        1) {
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == SameDays) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]
                    && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                       != nullptr) {
                    if (stringcontains(day, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                       instance->getNdays()) ==
                        1);
                    else {
                        //std::cout<<"SameDays: "<<instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]<<" "<<instance->getClass(
                        //    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == Precedence) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() ==
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i]) {
                    if (i > 0) {
                        if (current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i - 1])->getOrderID()]
                            != nullptr) {
                            if (isFirst(week, current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                  1])->getOrderID()]->getSolWeek(),
                                        instance->getNweek()) == -1) {
                                //std::cout<<"Precedence"<<std::endl;
                                isRoomProblem = false;
                                problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                                return current[instance->getClass(
                                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                      1])->getOrderID()]->getLecture();
                            } else if (isFirst(week, current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                  1])->getOrderID()]->getSolWeek(),
                                             instance->getNdays()) == 0 && isFirst(day, current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                  1])->getOrderID()]->getSolDays(),
                                                                                   instance->getNdays()) == -1) {
                                //std::cout<<"Precedence"<<std::endl;
                                isRoomProblem = false;
                                problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                                return current[instance->getClass(
                                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                      1])->getOrderID()]->getLecture();
                            } else if (isFirst(week, current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                  1])->getOrderID()]->getSolWeek(),
                                             instance->getNdays()) == 0 && isFirst(day, current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                  1])->getOrderID()]->getSolDays(),
                                                                                   instance->getNdays()) == 0 &&
                                     current[instance->getClass(
                                             instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                           1])->getOrderID()]->getSolStart() +
                                     current[instance->getClass(
                                             instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                           1])->getOrderID()]->getDuration() >
                                     start) {
                                //std::cout<<"Precedence"<<std::endl;
                                isRoomProblem = false;
                                problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                                return current[instance->getClass(
                                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i -
                                                                                                      1])->getOrderID()]->getLecture();
                            }

                        }
                    } else if (i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size()) {
                        if (current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i + 1])->getOrderID()]
                            != nullptr) {
                            if (isFirst(current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                  1])->getOrderID()]->getSolWeek(),
                                        week,
                                        instance->getNweek()) == -1) {
                                isRoomProblem = false;
                                problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                                //std::cout<<"Precedence"<<std::endl;
                                return current[instance->getClass(
                                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                      1])->getOrderID()]->getLecture();
                            } else if (isFirst(current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                  1])->getOrderID()]->getSolWeek(),
                                             week,
                                             instance->getNweek()) == 0 && isFirst(current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                  1])->getOrderID()]->getSolDays(),
                                                                                   day,
                                                                                   instance->getNdays()) == -1) {
                                //std::cout<<"Precedence"<<std::endl;
                                isRoomProblem = false;
                                problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                                return current[instance->getClass(
                                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                      1])->getOrderID()]->getLecture();
                            } else if (isFirst(current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                  1])->getOrderID()]->getSolWeek(),
                                             week,
                                             instance->getNweek()) == 0 && isFirst(current[instance->getClass(
                                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                  1])->getOrderID()]->getSolDays(),
                                                                                   day,
                                                                                   instance->getNdays()) == 0 &&
                                     start + duration > current[instance->getClass(
                                             instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                           1])->getOrderID()]->getSolStart()) {
                                //std::cout<<"Precedence"<<std::endl;
                                isRoomProblem = false;
                                problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                                return current[instance->getClass(
                                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i +
                                                                                                      1])->getOrderID()]->getLecture();
                            }

                        }
                    }
                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == WorkDay) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i] && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                                                                                        != nullptr) {
                    if (stringcompare(week, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek(),
                                      instance->getNweek(), false) ==
                        0 || stringcompare(day, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                           instance->getNdays(), false) ==
                             0);
                    else if (std::max(start + duration, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                                                        current[instance->getClass(
                                                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration()) -
                             std::min(start, current[instance->getClass(
                                     instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart()) >
                             instance->getClasses()[lectureID]->getHard()[h]->getType()->getLimit()) {
                        //std::cout<<"WorkDay"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }

                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == MinGap) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i] && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                                                                                        != nullptr) {
                    if (stringcompare(week, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolWeek(),
                                      instance->getNweek(), false) ==
                        0 || stringcompare(day, current[instance->getClass(
                            instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays(),
                                           instance->getNdays(), false) ==
                             0 ||
                        start + duration + instance->getClasses()[lectureID]->getHard()[h]->getType()->getLimit() <=
                        current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() ||
                        current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolStart() +
                        current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getDuration() +
                        instance->getClasses()[lectureID]->getHard()[h]->getType()->getLimit() <= start);
                    else {
                        //std::cout<<"MinGap"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }

                }
            }
        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == MaxDays) {
            int d = 0;
            for (int i = 0; i < instance->getNdays(); ++i) {
                if (day[i] == '1')
                    d++;
            }
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i] && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                                                                                        != nullptr) {
                    for (int d = 0; d < instance->getNdays(); ++d) {
                        if (current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getSolDays()[d] ==
                            '1')
                            d++;
                    }
                    if (d > instance->getClasses()[lectureID]->getHard()[h]->getType()->getLimit()) {
                        //std::cout<<"MaxDays"<<std::endl;
                        isRoomProblem = false;
                        problemsTemp = instance->getClasses()[lectureID]->getHard()[h]->getClasses();
                        return current[instance->getClass(
                                instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]->getLecture();
                    }

                }
            }

        } else if (instance->getClasses()[lectureID]->getHard()[h]->getType()->getType() == MaxDayLoad) {
            for (int i = 0; i < instance->getClasses()[lectureID]->getHard()[h]->getClasses().size(); ++i) {
                if (instance->getClasses()[lectureID]->getId() !=
                    instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i] && current[instance->getClass(
                        instance->getClasses()[lectureID]->getHard()[h]->getClasses()[i])->getOrderID()]
                                                                                        != nullptr) {
                    for (int w = 0; w < instance->getNweek(); ++w) {
                        for (int d = 0; d < instance->getNdays(); ++d) {


                        }

                    }

                }
            }

        }

    }
    //cost += instance->getClasses()[lectureID]->getPossibleRoomCost(roomID);




    return 0;
}

int LocalSearch::isFirst(std::string s1, std::string s2, int size) {
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

bool LocalSearch::stringcompare(std::string s1, std::string s2, int size, bool same) {
    for (int i = 0; i < size; ++i) {
        if (same && s2[i] != s1[i])
            return false;
        else if (!same && s2[i] == s1[i])
            return false;
    }
    return true;

}

bool LocalSearch::stringcontains(std::string s1, std::string s2, int size) {
    bool s2B = false, s1B = false;
    for (int i = 0; i < size; ++i) {
        if (s2[i] == '1' && s1[i] == '0')
            s2B = true;
        if (s1[i] == '1' && s2[i] == '0')
            s1B = true;
    }
    return !(s1B && s2B);

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
    int cost = isAllocable(t->getLecture(), t->getSolWeek(), t->getSolDays(), t->getSolStart(), t->getDuration(),
                           t->getSolRoom());
    if (cost > 0)
        return cost;
    current[instance->getClasses()[t->getLecture()]->getOrderID()] = t;
    /*cost += instance->getClasses()[t->getLecture()]->getPossibleRoomCost(current[t->getLecture()]->getSolRoom())*instance->getRoomPen();
    cost +=instance->getTimePen()*current[t->getLecture()]->penTime();*/
    return 0;
}

int LocalSearch::tryswampLectures(int id, const std::string &week, std::string &day, int start, int le, int pen) {
    int old = -1;
    if (current[instance->getClasses()[id]->getOrderID()] == nullptr)
        old = INT_MAX;
    else
        old = isAllocable(id, current[instance->getClasses()[id]->getOrderID()]->getSolWeek(),
                          current[instance->getClasses()[id]->getOrderID()]->getSolDays(),
                          current[instance->getClasses()[id]->getOrderID()]->getSolStart(),
                          current[instance->getClasses()[id]->getOrderID()]->getLecture(),
                          current[instance->getClasses()[id]->getOrderID()]->getSolRoom());
    int newV = isAllocable(id, week, day, start, le, current[instance->getClasses()[id]->getOrderID()]->getSolRoom());
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
LocalSearch::swampLectures(int id, const std::string &week, std::string &day, int start, int le, int pen) {
    int i = -1;
    //if (current[instance->getClasses()[id]->getOrderID()]->getSolRoom() != -1)
    //  i = instance->getClasses()[id]->getPossibleRoomCost(current[id]->getSolRoom());

    Solution *s = new Solution(id, start, current[id]->getSolRoom(), week, day, le, pen, i);
    current[instance->getClasses()[id]->getOrderID()] = s;
    //std::cout<<instance->getClasses()[id]->getId()<<" "<<*current[id]<<std::endl;
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

int LocalSearch::tryswampRoom(int lecture, int roomID) {
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

int LocalSearch::stuCost(Class *c, Student s) {
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

}

void LocalSearch::removeElement() {

}

void LocalSearch::conflict(int lectureID) {
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

bool LocalSearch::assignable(int id, int pInt[]) {
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
                               tP).first.getId()) != 0) {
        tP = distributionR(generatorR);
        rP = distributionT(generatorT);
    }
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

std::vector<Solution *> LocalSearch::findPossibleMove(int i) {
    std::vector<Solution *> reslut;
    for (int l = 0; l < this->instance->getClasses()[i]->getPossibleRooms().size(); ++l) {
        for (int m = 0; m < this->instance->getClasses()[i]->getLectures().size(); ++m) {
            if (this->isAllocable(i,
                                  this->instance->getClasses()[i]->getLectures()[m]->getWeeks(),
                                  this->instance->getClasses()[i]->getLectures()[m]->getDays(),
                                  this->instance->getClasses()[i]->getLectures()[m]->getStart(),
                                  this->instance->getClasses()[i]->getLectures()[m]->getLenght(),
                                  this->instance->getClasses()[i]->getPossibleRoomPair(
                                          l).first.getId()) == 0) {

                reslut.push_back(new Solution(this->tabu[l]->getLecture(),
                                              this->instance->getClasses()[i]->getLectures()[m]->getStart(),
                                              this->instance->getClasses()[i]->getPossibleRoomPair(
                                                      l).first.getId(),
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
