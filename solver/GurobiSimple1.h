//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_GUROBISIMPLE_H
#define PROJECT_GUROBISIMPLE_H

#include <gurobi_c++.h>
#include <math.h>       /* floor */
#include "GurobiAux.h"
#include <exception>
#include <thread>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "StudentSectioning.h"
#include "../utils/TimeUtil.h"
#include "../utils/StringUtil.h"
#include "StudentSectioning.h"

class GurobiSimple : public ILPExecuter {
    GRBVar ***timetable;
    StudentSectioning *studentSectioning;

public:
    GurobiSimple(Instance *i) {
        setInstance(i);
        i->setCompact(false);
        if (i->getStudent().size() > 0)
            studentSectioning = new StudentSectioning(i);

    }

    bool run2019(bool warm) override {
        if (instance->getStudent().size() > 0) {
            studentSectioning->init();
            std::cout << "init : Done " << getTimeSpent() << std::endl;
            studentSectioning->requiredClasses();
            std::cout << "classes : Done " << getTimeSpent() << std::endl;
            studentSectioning->parentChild();
            std::cout << "family : Done " << getTimeSpent() << std::endl;
            //cost = studentSectioning->conflicts(lectureTime, endTime);
            //std::cout << "conflicts : Done " << getTimeSpent() << std::endl;
            studentSectioning->limit();
            std::cout << "limit : Done " << getTimeSpent() << std::endl;
        }
        model->write("/Volumes/MAC/ClionProjects/timetabler/pu.lp");
        model->optimize();
        store();
        if (instance->getStudent().size() > 0)
            studentSectioning->save();

    }


    void definedAuxVar() {
        timetable = new GRBVar *[instance->getNumClasses()];
        for (int c = 0; c < instance->getNumClasses(); ++c) {
            GRBLinExpr oneEach = 0;
            timetable[c] = new GRBVar[instance->getClasses()[c]->getPossiblePair().size()];
            for (int p = 0; p < instance->getClasses()[c]->getPossiblePair().size(); ++p) {
                instance->getClasses()[c]->setOrderID(c);
                timetable[c][p] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                "timetable" +
                                                itos(instance->getClasses()[c]->getPossiblePair()[p].first.getId()) +
                                                "_" +
                                                instance->getClasses()[c]->getPossiblePair()[p].second->getWeeks() +
                                                "_" +
                                                instance->getClasses()[c]->getPossiblePair()[p].second->getDays() +
                                                "_" +
                                                itos(instance->getClasses()[c]->getPossiblePair()[p].second->getStart()) +
                                                "_" +
                                                itos(instance->getClasses()[c]->getPossiblePair()[p].second->getLenght()) +
                                                "_"

                                                + itos(instance->getClasses()[c]->getId()));
                oneEach += timetable[c][p];
            }
            model->addConstr(oneEach == 1);
        }
        //orderD();
    }


    void sameStart(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                GRBLinExpr side1 = 0;
                for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] * vector[c]->getPossiblePair()[p].second->getStart();
                }
                GRBLinExpr side2 = 0;
                for (int p = 0; p < vector[c1]->getPossiblePair().size(); ++p) {
                    side2 += timetable[vector[c1]->getOrderID()][p] *
                             vector[c1]->getPossiblePair()[p].second->getStart();
                }
                model->addConstr(side1 == side2);

            }
        }

    }


    void sameTime(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                        if (vector[c]->getPossiblePair()[p].second->getLenght() ==
                            vector[c1]->getPossiblePair()[p1].second->getLenght()) {
                            std::vector<Class *> temp;
                            temp.push_back(vector[c]);
                            temp.push_back(vector[c1]);
                            sameStart(temp, penalty);
                        } else {
                            if ((vector[c]->getPossiblePair()[p].second->getStart() <=
                                 vector[c1]->getPossiblePair()[p1].second->getStart()
                                 && vector[c]->getPossiblePair()[p].second->getEnd() <=
                                    vector[c1]->getPossiblePair()[p1].second->getEnd()) ||
                                (vector[c1]->getPossiblePair()[p1].second->getStart() <=
                                 vector[c]->getPossiblePair()[p].second->getStart()
                                 && vector[c1]->getPossiblePair()[p1].second->getEnd() <=
                                    vector[c]->getPossiblePair()[p].second->getEnd())) { ;
                            } else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "sameTime" + itos(vector[c1]->getId()) + "_" +
                                                 itos(vector[c]->getId()) + "_" +
                                                 itos(p) + "_" +
                                                 itos(p1));
                            }
                        }


                    }

                }

            }
        }

    }

    void diffTime(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                        if (vector[c]->getPossiblePair()[p].second->getEnd() <=
                            vector[c1]->getPossiblePair()[p1].second->getStart() ||
                            vector[c1]->getPossiblePair()[p1].second->getEnd() <=
                            vector[c]->getPossiblePair()[p].second->getStart()) { ;
                        } else {
                            model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                             timetable[vector[c1]->getOrderID()][p1] <= 1,
                                             "diffTime" + itos(vector[c1]->getId()) + "_" +
                                             itos(vector[c]->getId()) + "_" +
                                             itos(p) + "_" +
                                             itos(p1));
                        }
                    }


                }

            }
        }

    }


    void sameRoom(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                GRBLinExpr side1 = 0;
                for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] *
                             vector[c]->getPossiblePair()[p].first.getId();
                }
                GRBLinExpr side2 = 0;
                for (int p = 0; p < vector[c1]->getPossiblePair().size(); ++p) {
                    side2 +=
                            timetable[vector[c1]->getOrderID()][p] *
                            vector[c1]->getPossiblePair()[p].first.getId();
                }
                model->addConstr(side1 == side2,
                                 "sameRoom_" + itos(vector[c]->getId()) + "_" + itos(vector[c1]->getId()));
            }


        }


    }

    void sameAttendees(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                        if (vector[c]->getId() != vector[c1]->getId()) {
                            int travel = 0;
                            if (vector[c]->getPossiblePair()[p].first.getId() != -1 &&
                                vector[c1]->getPossiblePair()[p1].first.getId() != -1) {
                                if (instance->getRoom(vector[c]->getPossiblePair()[p].first.getId()).getTravel(
                                        vector[c1]->getPossiblePair()[p1].first.getId()) > 0)
                                    travel = instance->getRoom(vector[c]->getPossiblePair()[p].first.getId()).getTravel(
                                            vector[c1]->getPossiblePair()[p1].first.getId());
                                else
                                    travel = instance->getRoom(
                                            vector[c1]->getPossiblePair()[p1].first.getId()).getTravel(
                                            vector[c]->getPossiblePair()[p].first.getId());
                            }

                            if (vector[c]->getPossiblePair()[p].second->getEnd() + travel <=
                                vector[c1]->getPossiblePair()[p1].second->getStart()
                                || vector[c1]->getPossiblePair()[p1].second->getEnd() +
                                   travel <= vector[c]->getPossiblePair()[p].second->getStart()
                                || stringcompare(vector[c]->getPossiblePair()[p].second->getWeeks(),
                                                 vector[c1]->getPossiblePair()[p1].second->getWeeks(),
                                                 instance->getNweek(), false) ==
                                   1
                                || stringcompare(vector[c]->getPossiblePair()[p].second->getDays(),
                                                 vector[c1]->getPossiblePair()[p1].second->getDays(),
                                                 instance->getNdays(), false) ==
                                   1 || vector[c]->getPossiblePair()[p].second->getEnd() <=
                                        vector[c1]->getPossiblePair()[p1].second->getStart() ||
                                vector[c1]->getPossiblePair()[p1].second->getEnd() <=
                                vector[c]->getPossiblePair()[p].second->getStart()) { ;
                            } else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "sameAttendees_" + itos(vector[c1]->getId()) + "_" +
                                                 itos(vector[c]->getId()) + "_" +
                                                 itos(p) + "_" +
                                                 itos(p1));
                            }
                        }


                    }
                }

            }
        }
    }


    void notOverlap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {

                        if (vector[c]->getId() !=
                            vector[c1]->getId()) {
                            if (stringcompare(vector[c]->getPossiblePair()[p].second->getWeeks(),
                                              vector[c1]->getPossiblePair()[p1].second->getWeeks(),
                                              instance->getNweek(), false) ==
                                1
                                || stringcompare(vector[c]->getPossiblePair()[p].second->getDays(),
                                                 vector[c1]->getPossiblePair()[p1].second->getDays(),
                                                 instance->getNdays(), false) ==
                                   1 || vector[c]->getPossiblePair()[p].second->getEnd() <=
                                        vector[c1]->getPossiblePair()[p1].second->getStart() ||
                                vector[c1]->getPossiblePair()[p1].second->getEnd() <=
                                vector[c]->getPossiblePair()[p].second->getStart()) { ;
                            } else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "NotOverlap_" + itos(vector[c1]->getId()) + "_" +
                                                 itos(vector[c]->getId()));
                            }
                        }


                    }
                }
                /*GRBVar sameweek[instance->getNweek()];
                GRBVar sameday[instance->getNdays()];

                for (int i = 0; i < instance->getNweek(); ++i) {
                    sameweek[i] = model->addVar(0, 1, 0, GRB_BINARY, "sameWeekOver_" + itos(i) + "_" +
                                                                     itos(vector[c1]->getOrderID()) + "_" +
                                                                     itos(vector[c2]->getOrderID()));
                }
                for (int i = 0; i < instance->getNdays(); ++i) {
                    sameday[i] = model->addVar(0, 1, 0, GRB_BINARY, "getNdaysOver_" + itos(i) + "_" +
                                                                     itos(vector[c1]->getOrderID()) + "_" +
                                                                     itos(vector[c2]->getOrderID()));
                }
                for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                        for (int i = 0; i < instance->getNweek(); ++i) {
                            model->addGenConstrIndicator(sameweek[i], 1,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getPossiblePair()[p].second->getWeeks()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getPossiblePair()[p1].second->getWeeks()[i] == 2);
                            model->addGenConstrIndicator(sameweek[i], 0,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getPossiblePair()[p].second->getWeeks()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getPossiblePair()[p1].second->getWeeks()[i] <= 1);
                        }
                        for (int i = 0; i < instance->getNdays(); ++i) {
                            model->addGenConstrIndicator(sameday[i], 1,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getPossiblePair()[p].second->getDays()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getPossiblePair()[p1].second->getDays()[i] == 2);
                            model->addGenConstrIndicator(sameday[i], 0,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getPossiblePair()[p].second->getDays()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getPossiblePair()[p1].second->getDays()[i] <= 1);
                        }
                    }
                }
                GRBVar weekTime = model->addVar(0, 1, 0, GRB_BINARY,
                                                            "sameWeekSum_" + itos(vector[c1]->getOrderID())
                                                            + "_" +
                                                            itos(vector[c2]->getOrderID()));
                model->addGenConstrAnd(weekTime, sameweek, instance->getNweek());

                GRBVar dayTime = model->addVar(0, 1, 0, GRB_BINARY,
                                                "sameDaySum_" + itos(vector[c1]->getOrderID())
                                                + "_" +
                                                itos(vector[c2]->getOrderID()));
                model->addGenConstrAnd(dayTime, sameday, instance->getNdays());

                GRBVar sameTime = model->addVar(0, 1, 0, GRB_BINARY,
                                                                "sameTime_" + itos(vector[c1]->getOrderID()) + "_" +
                                                                itos(vector[c2]->getOrderID()));
                //(Ci.end  ≤ Cj .start)
                std::string name =
                        "T_" + itos(vector[c1]->getOrderID()) + "_" + itos(vector[c2]->getOrderID());
                                GRBVar temp_l1_l2 = model->addVar(0, 1, 0, GRB_BINARY, name);
                                model->addGenConstrIndicator(temp_l1_l2, 1,
                                                             endTime[vector[c1]->getOrderID()] <=
                                                             lectureTime[vector[c2]->getOrderID()]);
                                model->addGenConstrIndicator(temp_l1_l2, 0,
                                                             endTime[vector[c2]->getOrderID()] >=
                                                             lectureTime[vector[c1]->getOrderID()] + 1);
                                //(Cj .end  ≤ Ci.start)
                                name = "T_" + itos(vector[c2]->getOrderID()) + "_" + itos(vector[c1]->getOrderID());
                                GRBVar temp_l2_l1 = model->addVar(0, 1, 0, GRB_BINARY, name);
                                model->addGenConstrIndicator(temp_l2_l1, 1,
                                                             endTime[vector[c2]->getOrderID()] <=
                                                             lectureTime[vector[c1]->getOrderID()]);
                                model->addGenConstrIndicator(temp_l2_l1, 0,
                                                             endTime[vector[c2]->getOrderID()] >=
                                                             lectureTime[vector[c1]->getOrderID()] + 1);

                                model->addGenConstrIndicator(sameTime, 1,
                                                             temp_l1_l2 + temp_l2_l1 >= 1);
                                model->addGenConstrIndicator(sameTime, 0,
                                                             temp_l1_l2 + temp_l2_l1 == 0);


                                model->addConstr(
                                            samedays[vector[c1]->getOrderID()][vector[c2]->getOrderID()] + /*sameTime +
                                            weekTime <= 2);*/





            }
        }


    }


    void diffDays(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                            for (int i = 0; i < instance->getNdays(); ++i) {
                                if (vector[c]->getPossiblePair()[p].second->getDays()[i] ==
                                    vector[c1]->getPossiblePair()[p1].second->getDays()[i] &&
                                    vector[c1]->getPossiblePair()[p1].second->getDays()[i] == '1')
                                    model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                     timetable[vector[c1]->getOrderID()][p1] <= 1);

                            }


                        }
                    }
                }


            }
        }

    }

    void sameDays(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                            if (stringcontains(vector[c1]->getPossiblePair()[p1].second->getDays(),
                                               vector[c]->getPossiblePair()[p].second->getDays(),
                                               instance->getNdays()) ==
                                1);
                            else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 2);

                            }


                        }
                    }


                }
            }

        }
    }


    void diffWeek(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                            for (int i = 0; i < instance->getNweek(); ++i) {
                                if (vector[c]->getPossiblePair()[p].second->getWeeks()[i] ==
                                    vector[c1]->getPossiblePair()[p1].second->getWeeks()[i] &&
                                    vector[c1]->getPossiblePair()[p1].second->getWeeks()[i] == '1')
                                    model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                     timetable[vector[c1]->getOrderID()][p1] <= 1);

                            }


                        }
                    }
                }


            }
        }

    }

    void workDay(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int l) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                            for (int i = 0; i < instance->getNweek(); ++i) {
                                if (vector[c]->getPossiblePair()[p].second->getWeeks()[i] ==
                                    vector[c1]->getPossiblePair()[p1].second->getWeeks()[i] &&
                                    vector[c1]->getPossiblePair()[p1].second->getWeeks()[i] == '1') {
                                    if (vector[c]->getPossiblePair()[p].second->getStart() >=
                                        vector[c1]->getPossiblePair()[p1].second->getStart() &&
                                        vector[c1]->getPossiblePair()[p1].second->getEnd() -
                                        vector[c]->getPossiblePair()[p].second->getStart() > l) {
                                        model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                         timetable[vector[c1]->getOrderID()][p1] <= 1);
                                    } else if (vector[c]->getPossiblePair()[p].second->getStart() <
                                               vector[c1]->getPossiblePair()[p1].second->getStart() &&
                                               vector[c]->getPossiblePair()[p].second->getEnd() -
                                               vector[c1]->getPossiblePair()[p1].second->getStart() > l) {
                                        model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                         timetable[vector[c1]->getOrderID()][p1] <= 1);
                                    }
                                }

                            }


                        }
                    }
                }


            }
        }

    }


    void minGap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int l) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePair().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePair().size(); ++p1) {
                            if (stringcompare(vector[c1]->getPossiblePair()[p1].second->getWeeks(),
                                              vector[c]->getPossiblePair()[p].second->getWeeks(),
                                              instance->getNweek(), false) == 0 ||
                                stringcompare(vector[c1]->getPossiblePair()[p1].second->getDays(),
                                              vector[c]->getPossiblePair()[p].second->getDays(),
                                              instance->getNdays(), false) == 0 ||
                                vector[c1]->getPossiblePair()[p1].second->getStart() +
                                vector[c1]->getPossiblePair()[p1].second->getLenght() + l <=
                                vector[c]->getPossiblePair()[p].second->getStart() ||
                                vector[c]->getPossiblePair()[p].second->getStart() +
                                vector[c]->getPossiblePair()[p].second->getLenght() +
                                l <= vector[c1]->getPossiblePair()[p1].second->getStart());
                            else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1);


                            }


                        }
                    }
                }


            }
        }

    }


    virtual void dist(bool hard) override {
        oneLectureRoom();
        for (int i = 0; i < instance->getDist().size(); ++i) {
            std::vector<Class *> c;
            for (int j = 0; j < instance->getDist()[i]->getClasses().size(); ++j) {
                c.push_back(instance->getClass(instance->getDist()[i]->getClasses()[j]));
            }
            if (instance->getDist()[i]->getPenalty() == -1) {
                if (instance->getDist()[i]->getType()->getType() == SameStart) {
                    sameStart(c, instance->getDist()[i]->getPenalty());
                }
                if (instance->getDist()[i]->getType()->getType() == SameTime) {
                    sameTime(c, instance->getDist()[i]->getPenalty());
                }
                if (instance->getDist()[i]->getType()->getType() == SameTime) {
                    diffTime(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameRoom) {
                    sameRoom(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameAttendees) {
                    sameAttendees(c, instance->getDist()[i]->getPenalty());
                    //th.push_back(std::thread(&GurobiSimple::sameAttendees,this,c, instance->getDist()[i]->getPenalty()));
                } else if (instance->getDist()[i]->getType()->getType() == NotOverlap) {
                    notOverlap(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameDays) {
                    sameDays(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameDays) {
                    sameDays(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == DifferentDays) {
                    diffDays(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == DifferentWeeks) {
                    diffWeek(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == WorkDay) {
                    workDay(c, instance->getDist()[i]->getPenalty(),
                            instance->getDist()[i]->getType()->getLimit());
                } else if (instance->getDist()[i]->getType()->getType() == MinGap) {
                    minGap(c, instance->getDist()[i]->getPenalty(),
                           instance->getDist()[i]->getType()->getLimit());
                }
            }

        }


    }


    /**
     * A lecture can only be in one room at time
     */

    void oneLectureRoom() {
        for (int clu = 0; clu < instance->getClassbyclusterRoom().size(); ++clu) {
            for (int c = 0; c < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c) {
                for (int c1 = c + 1; c1 < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c1) {
                    for (int p = 0;
                         p < instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair().size(); ++p) {
                        for (int p1 = 0; p1 <
                                         instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair().size(); ++p1) {
                            if (check(instance->getClasses()[c]->getPossiblePair()[p],
                                      instance->getClasses()[c1]->getPossiblePair()[p1])) {
                                model->addConstr(
                                        timetable[instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getOrderID()][p]
                                        +
                                        timetable[instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getOrderID()][p1] <=
                                        1);
                            }

                        }
                    }

                }
            }
        }

    }

private:
    bool check(std::pair<Room, Lecture *> p1, std::pair<Room, Lecture *> p2) {
        if (p1.first.getId() == -1 || p2.first.getId() == -1)
            return false;
        if (p1.first.getId() != p2.first.getId())
            return false;
        for (int j = 0; j < instance->getNweek(); ++j) {
            if (p1.second->getWeeks()[j] == p2.second->getWeeks()[j] &&
                p1.second->getWeeks()[j] == '1') {
                for (int d = 0; d < instance->getNdays(); ++d) {
                    if (p1.second->getDays()[d] == p2.second->getDays()[d] &&
                        p1.second->getDays()[d] == '1') {
                        if (p1.second->getStart() >= p2.second->getStart() &&
                            p1.second->getStart() <
                            p2.second->getStart() + p2.second->getLenght()) {
                            return true;
                        } else if (p2.second->getStart() >= p1.second->getStart() &&
                                   p2.second->getStart() < p1.second->getStart() +
                                                           p1.second->getLenght()) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    void store() {
        for (int c = 0; c < instance->getNumClasses(); c++) {
            for (int p = 0; p < instance->getClasses()[c]->getPossiblePair().size(); ++p) {
                bool active = timetable[c][p].get(GRB_DoubleAttr_X);
                if (active != 0) {
                    instance->getClasses()[c]->setSolution(new Solution(instance->getClasses()[c]->getId(),
                                                                        instance->getClasses()[c]->getPossiblePair()[p].second->getStart(),
                                                                        instance->getClasses()[c]->getPossiblePair()[p].first.getId(),
                                                                        instance->getClasses()[c]->getPossiblePair()[p].second->getWeeks(),
                                                                        instance->getClasses()[c]->getPossiblePair()[p].second->getDays(),
                                                                        instance->getClasses()[c]->getPossiblePair()[p].second->getLenght(),
                                                                        0, 0));
                }

            }

        }


    }


};


#endif //PROJECT_ILPEXECUTER_H