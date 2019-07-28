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
                if (div.size() == 0) {
                    std::set<int> temp;
                    temp.insert(instance->getClasses()[i]->getId());
                    for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                        for (int l = 0; l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                            temp.insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);
                        }
                    }
                    div.push_back(temp);
                    isHere = true;

                }
                for (int j = 0; j < div.size(); ++j) {
                    if (div[j].find(instance->getClasses()[i]->getId()) != div[j].end()) {
                        isHere = true;
                        for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                            for (int l = 0; l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                                div[j].insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);

                            }
                        }
                    } else {
                        for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                            for (int l0 = 0;
                                 l0 < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l0) {
                                if (div[j].find(instance->getClasses()[i]->getHard()[k]->getClasses()[l0]) !=
                                    div[j].end()) {
                                    isHere = true;
                                    for (int l = 0;
                                         l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                                        div[j].insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);

                                    }
                                    break;
                                }
                            }

                        }
                        }
                    }
                if (!isHere) {
                    std::set<int> temp;
                    temp.insert(instance->getClasses()[i]->getId());
                    for (int k = 0; k < instance->getClasses()[i]->getHard().size(); ++k) {
                        for (int l = 0; l < instance->getClasses()[i]->getHard()[k]->getClasses().size(); ++l) {
                            temp.insert(instance->getClasses()[i]->getHard()[k]->getClasses()[l]);
                        }
                    }
                    div.push_back(temp);

                }
            }

            /*if (!isHere && instance->getClasses()[i]->getPossibleRooms().size() > 0) {
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

        }

    }*/
        } else {
            std::set<int> temp;
            for (int i = 0; i < instance->getClasses().size(); ++i) {
                temp.insert(instance->getClasses()[i]->getId());
            }
            div.push_back(temp);

        }
        for (int part = 0; part < div.size(); part++) {
            std::cout << "A" << part << std::endl;
            for (std::set<int>::iterator it = div[part].begin(); it != div[part].end(); ++it) {
                std::cout << part << " " << *it << std::endl;
            }

        }


    }


};


#endif //PROJECT_LSDIVIDED_H
