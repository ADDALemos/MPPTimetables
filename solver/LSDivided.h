//
// Created by Alexandre Lemos on 30/05/2019.
//

#ifndef PROJECT_LSDIVIDED_H
#define PROJECT_LSDIVIDED_H


class LSDivided {


public:
    std::vector<std::set<int>> div;
    Instance *instance;
    bool division;

    LSDivided(Instance *instance, bool division) : instance(instance), division(division) {}

    void init() {
        if (division) {
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                bool isHere = false;
                std::cerr << instance->getClasses()[i]->getId() << std::endl;
                for (int j = 0; j < div.size(); ++j) {
                    if (div[j].find(instance->getClasses()[i]->getId()) != div[j].end()) {
                        isHere = true;
                        for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                            for (int l = 0; l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                                div[j].insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);
                                std::cout << j << " " << instance->getClasses()[i]->getHard()[k]->getClasses()[l]
                                          << std::endl;
                            }
                        }
                    } else {
                        for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                            for (int l0 = 0; l0 < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l0) {
                                if (div[j].find(instance->getClasses()[i]->getHard()[k]->getClasses()[l0]) !=
                                    div[j].end()) {
                                    isHere = true;
                                    for (int l = 0;
                                         l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                                        div[j].insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);
                                        std::cout << j << " "
                                                  << instance->getClasses()[i]->getHard()[k]->getClasses()[l]
                                                  << std::endl;
                                    }
                                    break;
                                }
                            }

                        }
                    }

                    if (!isHere && instance->getClasses()[i]->getPossibleRooms().size() > 0) {
                        for (int r = 0; r < instance->getClasses()[i]->getPossibleRooms().size(); ++r) {
                            Room id = instance->getClasses()[i]->getPossibleRoomPair(r).first;
                            for (std::set<int>::iterator it = div[j].begin();
                                 it != div[j].end(); ++it) {
                                if (instance->getClass(*it)->getPossibleRooms().size() > 0) {
                                    if (instance->getClass(*it)->containsRoom(id)) {
                                        isHere = true;
                                        for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                                            for (int l = 0; l <
                                                            instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                                                div[j].insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);
                                                std::cout << j << " "
                                                          << instance->getClasses()[i]->getHard()[k]->getClasses()[l]
                                                          << std::endl;
                                            }
                                        }
                                    }
                                }

                            }

                        }
                        if (!isHere) {
                            for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                                for (int l = 0; l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                                    if (instance->getClass(
                                            instance->getClasses()[i]->getHard()[k]->getClasses()[l])->getPossibleRooms().size() >
                                        0) {
                                        for (int r = 0; r < instance->getClass(
                                                instance->getClasses()[i]->getHard()[k]->getClasses()[l])->getPossibleRooms().size(); ++r) {
                                            Room id = instance->getClass(
                                                    instance->getClasses()[i]->getHard()[k]->getClasses()[l])->getPossibleRoomPair(
                                                    r).first;
                                            for (std::set<int>::iterator it = div[j].begin();
                                                 it != div[j].end(); ++it) {
                                                if (instance->getClass(*it)->getPossibleRooms().size() > 0) {
                                                    if (instance->getClass(*it)->containsRoom(id)) {
                                                        isHere = true;
                                                        for (int k = 0;
                                                             k < instance->getClasses()[i]->getHard().size(); ++k) {
                                                            for (int l = 0; l <
                                                                            instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                                                                div[j].insert(
                                                                        instance->getClasses()[i]->getHard()[k]->getClasses()[l]);
                                                                std::cout << j << " "
                                                                          << instance->getClasses()[i]->getHard()[k]->getClasses()[l]
                                                                          << std::endl;
                                                            }
                                                        }
                                                    }
                                                }

                                            }
                                        }
                                    }


                                }
                            }

                        }
                    }
                }
                if (!isHere) {
                    std::set<int> temp;
                    for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                        for (int l = 0; l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                            temp.insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);
                            std::cout << instance->getClasses()[i]->getHard()[k]->getClasses()[l] << std::endl;
                        }
                    }
                    div.push_back(temp);
                }

            }
        } else {
            std::set<int> temp;
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                temp.insert(instance->getClasses()[i]->getId());
            }
            div.push_back(temp);

        }


    }


};


#endif //PROJECT_LSDIVIDED_H
