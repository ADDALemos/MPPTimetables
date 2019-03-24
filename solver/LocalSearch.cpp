//
// Created by Alexandre Lemos on 15/03/2019.
//

#include <cfloat>
#include <random>
#include "LocalSearch.h"

void LocalSearch::LNS() {
    init();
    Local();

    printFinal();


}


void LocalSearch::GRASP() {
    init();
    Greedy();
    Local();
    printStatus(0);
    for (int i = 1; i < MAX_ITERATIONS; i++) {
        Greedy();
        if (eval()) {
            store();
        }
        Local();
        printStatus(i);
        if (eval()) {
            store();
        } else
            init();
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
    allocated = new bool[instance->getClasses().size()];
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        allocated[i] = false;
    }
}

void LocalSearch::init() {
    room = new bool *[instance->getClasses().size()];
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        room[i] = new bool[instance->getClasses()[i]->getPossibleRooms().size()];
        for (int j = 0; j < instance->getClasses()[i]->getPossibleRooms().size(); ++j) {
        //warm-start
            room[i][j] = 1;
        }
    }
    time = new bool **[instance->getNdays()];
    for (int k = 0; k < instance->getNdays(); ++k) {
        time[k] = new bool *[instance->getSlotsperday()];
        for (int i = 0; i < instance->getSlotsperday(); ++i) {
            time[k][i] = new bool[instance->getClasses().size()];
        }

    }
}


void LocalSearch::store() {
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        for (int j = 0; j < instance->getClasses()[i]->getPossibleRooms().size(); ++j) {
            if (room[i][j])
                instance->getClasses()[i]->setSolRoom(instance->getRoom(j + 1).getId());
        }
        for (int k = 0; k < instance->getNdays(); ++k) {
            for (int t = 1; t < instance->getSlotsperday(); ++t) {
                if (time[k][t] && !time[k][t - 1])
                    instance->getClasses()[i]->setSolutionTime(t, (char *) std::to_string(k).c_str());

            }

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
    int count = 0;
    for (int i = 0; i < instance->getStudent().size(); ++i) {
        for (int j = 0; j < instance->getNdays(); ++j) {
            for (int k = 1; k < instance->getSlotsperday(); ++k) {
                int before = 0, after = 0;
                for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
                    before += time[j][k][l];
                    after += time[j][k - 1][l];
                }
                if (before != after)
                    count++;

            }
        }
    }
    return count;
}

void LocalSearch::Greedy() {
    std::vector<int> room;
    std::vector<int> lecture;
    std::vector<int> day;
    std::vector<int> time;
    int current = 0;

    while (instance->getClasses().size() > current) {

        double minCost = DBL_MAX;
        for (int i = 0; i < instance->getClasses().size(); ++i) {
            if (!allocated[i]) {
                for (int j = 0; j < instance->getClasses()[i]->getPossibleRooms().size(); ++j) {
                    for (int k = 0; k < instance->getNdays(); ++k) {
                        for (int m = 0; m < instance->getSlotsperday(); ++m) {
                            if (isAllocable(i, j, k, m)) {
                                if (day.size() < sizeRCL) {
                                    room.push_back(j);
                                    day.push_back(k);
                                    time.push_back(m);
                                    lecture.push_back(i);


                                } else {
                                    bool change = false;
                                    int tabu = 0;
                                    for (; tabu < room.size(); ++tabu) {
                                        minCost = cost(lecture[tabu], room[tabu], day[tabu], time[tabu]);
                                        if (i != lecture[tabu] && cost(i, j, k, m) < minCost) {
                                            change = true;
                                            break;
                                        }
                                    }
                                    if (change) {
                                        room[tabu] = j;
                                        lecture[tabu] = i;
                                        time[tabu] = m;
                                        day[tabu] = k;
                                    }

                                }
                            }

                        }

                    }


                }
            }

        }
        if (room.size() > 0) {
            std::default_random_engine generator(seedHandler());
            std::uniform_int_distribution<int> distribution(0, room.size());
            int l = distribution(generator);
            if (assign(lecture[l], room[l], day[l], time[l])) {
                allocated[lecture[l]] = true;
                current++;
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

bool LocalSearch::isAllocable(int lectureID, int roomID, int day, int times) {
    //Students
    for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
         it != instance->getStudent().end(); it++) {
        if (it->second.containsClass(instance->getClasses()[lectureID])) {
            for (int c = 0; c < it->second.getClasses().size(); ++c) {
                if (LocalSearch::time[day][times][c] == 1 &&
                    instance->getClasses()[lectureID]->getId() != instance->getClasses()[c]->getId())
                    return false;
            }
        }
    }
    //Room
    for (int j = 0; j < instance->getNumClasses(); j++) {
        if (instance->getClasses()[j]->containsRoom(instance->getRoom(roomID + 1))) {
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getRoom(roomID + 1).getId() == it->second.getId() && LocalSearch::time[day][times][j]
                    && instance->getClasses()[lectureID]->getId() != instance->getClasses()[j]->getId())
                    return false;

            }
        }
    }
    //teacher
    for (std::map<std::string, Course *>::const_iterator it = instance->getCourses().begin();
         it != instance->getCourses().end(); it++) {
        for (std::map<int, std::vector<Subpart *>>::iterator sub = it->second->getConfiguratons().begin();
             sub != it->second->getConfiguratons().end(); ++sub) {
            for (int i = 0; i < sub->second.size(); ++i) {
                int over = 0;
                for (int c = 0; c < sub->second[i]->getClasses().size(); c++) {
                    if (LocalSearch::time[day][times][c] == 1 &&
                        instance->getClasses()[lectureID]->getId() != instance->getClasses()[c]->getId())
                        over++;


                }
                if (over > sub->second[i]->getOverlap())
                    return false;
            }


        }


    }


    return true;
}

double LocalSearch::cost(int lectureID, int roomID, int day, int timeS) {
    int count = 0;
    for (int i = 0; i < instance->getStudent().size(); ++i) {
        for (int j = 0; j < instance->getNdays(); ++j) {
            for (int k = 1; k < instance->getSlotsperday(); ++k) {
                int before = 0, after = 0;
                for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
                    if (lectureID == l && day == j && k == timeS) {
                        before += 1;
                    } else if (lectureID == l && day == j && (k - 1) == timeS) {
                        after += 1;
                    } else {
                        before += time[j][k][l];
                        after += time[j][k - 1][l];
                    }
                }
                if (before != after)
                    count++;

            }
        }
    }
    return count;

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

bool LocalSearch::assign(int lectureID, int roomID, int day, int time) {
    if (!isAllocable(lectureID, roomID, day, time))
        return false;
    for (int l = 0; l < instance->getClasses().size(); ++l) {
        if (l != lectureID && instance->getClasses()[l]->containsRoom(instance->getRoom(roomID + 1))) {
            for (int d = 0; d < instance->getNdays(); ++d) {
                for (int i = 0; i < instance->getSlotsperday(); ++i) {
                    int temp = 0;
                    if (room[l][roomID]) {

                    }


                }
            }

        }
    }


    return false;
}

bool LocalSearch::tryswampLectures(int l1, int l2, int d1, int t1, int le1, int d2, int t2, int le2) {
    int oldV = 0, newV = 0;
    Class *l2c, *l1c;
    for (int i = 0; i < instance->getStudent().size(); ++i) {
        //if(instance->getStudent(i + 1).containsClass(l1c)) {
        for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
            if (t1 > 0)
                newV += (time[d1][t1 - 1][i] == 0);
            if (t1 < instance->getSlotsperday())
                newV += (time[d1][t1 + le1][i] == 0);
            if (t2 > 0)
                oldV += (time[d2][t2 - 1][i] == 0);
            if (t2 < instance->getSlotsperday())
                oldV += (time[d2][t2 + le2][i] == 0);

        }
        // } else if (instance->getStudent(i + 1).containsClass(l2c)){
        for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
            if (t1 > 0)
                newV += (time[d1][t1 - 1][i] == 0);
            if (t1 < instance->getSlotsperday())
                newV += (time[d1][t1 + le1][i] == 0);
            if (t2 > 0)
                oldV += (time[d2][t2 - 1][i] == 0);
            if (t2 < instance->getSlotsperday())
                oldV += (time[d2][t2 + le2][i] == 0);

        }

        //}

    }
    return oldV > newV;/*
    int counto = getGAP();
    int count = 0;
    for (int i = 0; i < instance->getStudent().size(); ++i) {
        for (int j = 0; j < instance->getNdays(); ++j) {
            for (int k = 1; k < instance->getSlotsperday(); ++k) {
                int before = 0, after = 0;
                for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
                    if (l1 == l || l2 == l) {
                        if (j == d1 && k >= t1 && k < (t1 + le1))
                            after += 1;
                        if (j == d1 && (k - 1) >= t1 && (k - 1) < (t1 + le1))
                            before += 1;
                        if (j == d2 && k >= t2 && k < (t2 + le2))
                            after += 1;
                        if (j == d2 && (k - 1) >= t2 && (k - 1) < (t2 + le2))
                            before += 1;

                    } else {
                        before += time[j][k][l];
                        after += time[j][k - 1][l];
                    }
                }
                if (before != after)
                    count++;

            }
        }
    }
    return counto < count;*/
}

void
LocalSearch::swampLectures(int lect1, int lect2, int day1, int start1, int lenght1, int day2, int start2, int lenght2) {
    for (int i = start1; i < (start1 + lenght1); ++i) {
        time[day1][i][lect1] = 0;
        time[day1][i][lect2] = 1;
    }
    for (int i = start2; i < (start2 + lenght2); ++i) {
        time[day2][i][lect2] = 0;
        time[day2][i][lect1] = 1;
    }
    room[lect1][instance->getClasses()[lect1]->getSolRoom()] = 0;
    room[lect1][instance->getClasses()[lect2]->getSolRoom()] = 1;
    room[lect2][instance->getClasses()[lect1]->getSolRoom()] = 1;
    room[lect2][instance->getClasses()[lect2]->getSolRoom()] = 0;

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
