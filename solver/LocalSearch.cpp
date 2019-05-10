//
// Created by Alexandre Lemos on 15/03/2019.
//

#include <cfloat>
#include <random>
#include <chrono>
#include "LocalSearch.h"

void LocalSearch::LNS() {
    init();
    Local();
    printFinal();


}


void LocalSearch::GRASP() {
    init();
    Greedy();
    //Local();
    printStatus(0);
    for (int i = 1; i < MAX_ITERATIONS; i++) {
        Greedy();
        store();
      //  Local();
        printStatus(i);
        store();

    }
    printFinal();

}

bool LocalSearch::eval() {

    return getGAP() < getGAPStored();

}

LocalSearch::LocalSearch(int MAX_ITERATIONS, double rcl, Instance *instance) : MAX_ITERATIONS(MAX_ITERATIONS),
                                                                               instance(instance) {
    init();
    sizeRCL = instance->getClasses().size() * rcl;

}

void LocalSearch::init() {
    current = new Solution*[instance->getClasses().size()];
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        current[i]= nullptr;
    }
}


void LocalSearch::store() {
    if(best>currentV){
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            instance->getClasses()[i]->setSolution(current[i]);
        }
    }

}

void LocalSearch::printStatus(int ite) {
    std::cout << "Iteration: " << ite << getGAP() << std::endl;
}

void LocalSearch::printFinal() {
    std::cout << "Best result: " << getGAP() << std::endl;
}

int LocalSearch::getGAP() const {
    return 0;
}

void LocalSearch::Greedy() {
    int allocation = 0;

    while (instance->getClasses().size() > allocation) {

        double maxCost = DBL_MAX;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            if (current[i]!=nullptr) {
                for (auto j=instance->getClasses()[i]->getPossibleRooms().begin();
                j != instance->getClasses()[i]->getPossibleRooms().end(); ++j) {
                    int costR=j->second,costT;
                        for (int time = 0; time < instance->getClasses()[i]->getLectures().size(); ++time) {
                            if ((costT=isAllocable(instance->getClasses()[i]->getOrderID(),
                                    instance->getClasses()[i]->getLectures()[time]->getWeeks(),
                                                   instance->getClasses()[i]->getLectures()[time]->getDays(),
                                                   instance->getClasses()[i]->getLectures()[time]->getStart(),
                                                   instance->getClasses()[i]->getLectures()[time]->getLenght(),
                                                   j->first.getId())!=-1)) {
                                if(maxCost>(costR+costR)) {
                                    if(tabu.size()==sizeRCL) {
                                        tabu.pop_back();
                                        maxCost=costT+costR;
                                    }
                                    tabu.push_back(new Solution(instance->getClasses()[i]->getOrderID(),
                                                                 instance->getClasses()[i]->getLectures()[time]->getStart(),
                                                                 j->first.getId(),
                                                                 instance->getClasses()[i]->getLectures()[time]->getWeeks(),
                                                                 instance->getClasses()[i]->getLectures()[time]->getDays()));

                                }
                            }
                        }

                }
            }
        }

        if (tabu.empty()) {
            std::default_random_engine generator(seedHandler());
            std::uniform_int_distribution<int> distribution(0, tabu.size());
            int l = distribution(generator);
            if (assign(tabu[l])) {
                current[tabu[l]->getLecture()]=tabu[l];
                tabu.erase(tabu.begin()+l);
                allocation++;
            }


        }
    }
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
        if(current[i]!= nullptr){
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

bool LocalSearch::assign(Solution * t) {
    if (isAllocable(t->getLecture(), t->getSolWeek(),t->getSolDays(),t->getSolStart(),t->getDuration(),t->getSolRoom())==-1)
        return false;
    instance->getClasses()[t->getLecture()]->setSolution(t);
    return true;
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
