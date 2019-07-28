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
#include <algorithm>

class GurobiSimple : public ILPExecuter {
    GRBVar **timetable;
    //GRBVar **order;
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
            //cost =
            //std::cout << "conflicts : Done " << getTimeSpent() << std::endl;
            studentSectioning->limit();
            std::cout << "limit : Done " << getTimeSpent() << std::endl;
        }
        GRBLinExpr opt = 0;//costRoom*instance->getRoomPen()+costTime*instance->getTimePen();
        if (instance->getStudent().size() > 0)
            opt += studentSectioning->conflicts(timetable) * instance->getStudentPen();
        model->setObjective(opt, GRB_MINIMIZE);
        std::cout << getTimeSpent() << std::endl;
        model->optimize();
        model->write("/Volumes/MAC/ClionProjects/timetabler/" + instance->getName() + ".lp");
        store();
        double value = model->get(GRB_DoubleAttr_ObjVal);
        std::cout << value << std::endl;
        if (instance->getStudent().size() > 0)
            studentSectioning->save();

    }

    GRBLinExpr costTime = 0;
    GRBLinExpr costRoom = 0;


    void definedAuxVar() {
        timetable = new GRBVar *[instance->getNumClasses()];
        for (int c = 0; c < instance->getNumClasses(); ++c) {
            GRBLinExpr oneEach = 0;
            instance->getClasses()[c]->setOrderID(c);
            instance->getClasses()[c]->computeSize();
            timetable[c] = new GRBVar[instance->getClasses()[c]->getPossiblePairSize()];
            for (int p = 0; p < instance->getClasses()[c]->getPossiblePairSize(); ++p) {
                timetable[c][p] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                "timetable" +
                                                itos(instance->getClasses()[c]->getPossiblePairRoom(p).getId()) + "_" +
                                                instance->getClasses()[c]->getPossiblePairLecture(p)->getWeeks() + "_" +
                                                instance->getClasses()[c]->getPossiblePairLecture(p)->getDays() + "_" +
                                                itos(instance->getClasses()[c]->getPossiblePairLecture(p)->getStart()) +
                                                "_" +
                                                itos(instance->getClasses()[c]->getPossiblePairLecture(
                                                        p)->getLenght()) + "_"
                                                + itos(instance->getClasses()[c]->getId()));
                costTime += timetable[c][p] * instance->getClasses()[c]->getPossiblePairLecture(p)->getPenalty();
                costRoom += timetable[c][p] * instance->getClasses()[c]->getPossibleRoomCost(
                        instance->getClasses()[c]->getPossiblePairRoom(p));


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
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] * vector[c]->getPossiblePairLecture(p)->getStart();
                }
                GRBLinExpr side2 = 0;
                for (int p = 0; p < vector[c1]->getPossiblePairSize(); ++p) {
                    side2 += timetable[vector[c1]->getOrderID()][p] * vector[c1]->getPossiblePairLecture(p)->getStart();
                }
                model->addConstr(side1 == side2,
                                 "sameStart_" + itos(vector[c]->getId()) + "_" + itos(vector[c1]->getId()));

            }
        }

    }

    void precedence(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                GRBLinExpr side1 = 0;
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                        if (isFirst(vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                    vector[c1]->getPossiblePairLecture(p1)->getWeeks(), instance->getNweek()) == -1);
                        else if (isFirst(vector[c]->getPossiblePairLecture(p)->getDays(),
                                         vector[c1]->getPossiblePairLecture(p1)->getDays(), instance->getNdays()) ==
                                 -1 && isFirst(vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                               vector[c1]->getPossiblePairLecture(p1)->getWeeks(),
                                               instance->getNweek()) == 0);
                        else if (isFirst(vector[c]->getPossiblePairLecture(p)->getDays(),
                                         vector[c1]->getPossiblePairLecture(p1)->getDays(), instance->getNdays()) ==
                                 0 && isFirst(vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                              vector[c1]->getPossiblePairLecture(p1)->getWeeks(),
                                              instance->getNweek()) == 0 &&
                                 vector[c]->getPossiblePairLecture(p)->getEnd() <=
                                 vector[c1]->getPossiblePairLecture(p1)->getEnd());
                        else
                            model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                             timetable[vector[c1]->getOrderID()][p1] <= 1,
                                             "prec" + itos(vector[c1]->getId()) + "_" +
                                             itos(vector[c]->getId()) + "_" +
                                             itos(p) + "_" +
                                             itos(p1));


                    }

                }

            }

        }
    }


    void sameTime(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {

                        if (vector[c]->getPossiblePairLecture(p)->getStart() <=
                            vector[c1]->getPossiblePairLecture(p1)->getStart()
                            && vector[c1]->getPossiblePairLecture(p1)->getEnd() <=
                               vector[c]->getPossiblePairLecture(p)->getEnd()) { ;
                        } else if (vector[c1]->getPossiblePairLecture(p1)->getStart() <=
                                   vector[c]->getPossiblePairLecture(p)->getStart()
                                   && vector[c]->getPossiblePairLecture(p)->getEnd() <=
                                      vector[c1]->getPossiblePairLecture(p1)->getEnd()) { ;
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

    void diffTime(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                        if (vector[c]->getPossiblePairLecture(p)->getEnd() <=
                            vector[c1]->getPossiblePairLecture(p1)->getStart() ||
                            vector[c1]->getPossiblePairLecture(p1)->getEnd() <=
                            vector[c]->getPossiblePairLecture(p)->getStart()) { ;
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
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] *
                             vector[c]->getPossiblePairRoom(p).getId();
                }
                GRBLinExpr side2 = 0;
                for (int p = 0; p < vector[c1]->getPossiblePairSize(); ++p) {
                    side2 +=
                            timetable[vector[c1]->getOrderID()][p] *
                            vector[c1]->getPossiblePairRoom(p).getId();
                }
                model->addConstr(side1 == side2,
                                 "sameRoom_" + itos(vector[c]->getId()) + "_" + itos(vector[c1]->getId()));
            }


        }


    }

    void sameAttendees(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                        if (vector[c]->getId() != vector[c1]->getId()) {
                            int travel = 0;
                            if (vector[c]->getPossiblePairRoom(p).getId() != -1 &&
                                vector[c1]->getPossiblePairRoom(p1).getId() != -1) {
                                if (instance->getRoom(vector[c]->getPossiblePairRoom(p).getId()).getTravel(
                                        vector[c1]->getPossiblePairRoom(p1).getId()) > 0)
                                    travel = instance->getRoom(vector[c]->getPossiblePairRoom(p).getId()).getTravel(
                                            vector[c1]->getPossiblePairRoom(p1).getId());
                                else
                                    travel = instance->getRoom(
                                            vector[c1]->getPossiblePairRoom(p1).getId()).getTravel(
                                            vector[c]->getPossiblePairRoom(p).getId());
                            }

                            if (vector[c]->getPossiblePairLecture(p)->getEnd() + travel <=
                                vector[c1]->getPossiblePairLecture(p1)->getStart()
                                || vector[c1]->getPossiblePairLecture(p1)->getEnd() +
                                   travel <= vector[c]->getPossiblePairLecture(p)->getStart()
                                || stringcompare(vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                                 vector[c1]->getPossiblePairLecture(p1)->getWeeks(),
                                                 instance->getNweek(), false) ==
                                   1
                                || stringcompare(vector[c]->getPossiblePairLecture(p)->getDays(),
                                                 vector[c1]->getPossiblePairLecture(p1)->getDays(),
                                                 instance->getNdays(), false) ==
                                   1) { ;
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
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {

                        if (vector[c]->getId() !=
                            vector[c1]->getId()) {
                            if (stringcompare(vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                              vector[c1]->getPossiblePairLecture(p1)->getWeeks(),
                                              instance->getNweek(), false) ==
                                1
                                || stringcompare(vector[c]->getPossiblePairLecture(p)->getDays(),
                                                 vector[c1]->getPossiblePairLecture(p1)->getDays(),
                                                 instance->getNdays(), false) ==
                                   1 || vector[c]->getPossiblePairLecture(p)->getEnd() <=
                                        vector[c1]->getPossiblePairLecture(p1)->getStart() ||
                                vector[c1]->getPossiblePairLecture(p1)->getEnd() <=
                                vector[c]->getPossiblePairLecture(p)->getStart()) { ;
                            } else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "NotOverlap_" + itos(vector[c1]->getId()) + "_" +
                                                 itos(vector[c]->getId()));
                            }
                        }


                    }
                }


            }
        }


    }

    void overlap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {

                        if (vector[c]->getId() !=
                            vector[c1]->getId()) {
                            if (stringcompare(vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                              vector[c1]->getPossiblePairLecture(p1)->getWeeks(),
                                              instance->getNweek(), false) ==
                                1
                                || stringcompare(vector[c]->getPossiblePairLecture(p)->getDays(),
                                                 vector[c1]->getPossiblePairLecture(p1)->getDays(),
                                                 instance->getNdays(), false) ==
                                   1 || vector[c]->getPossiblePairLecture(p)->getEnd() <=
                                        vector[c1]->getPossiblePairLecture(p1)->getStart() ||
                                vector[c1]->getPossiblePairLecture(p1)->getEnd() <=
                                vector[c]->getPossiblePairLecture(p)->getStart()) {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "NotOverlap_" + itos(vector[c1]->getId()) + "_" +
                                                 itos(vector[c]->getId()));
                            }
                        }


                    }
                }


            }
        }


    }


    void diffDays(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                            for (int i = 0; i < instance->getNdays(); ++i) {
                                if (vector[c]->getPossiblePairLecture(p)->getDays()[i] ==
                                    vector[c1]->getPossiblePairLecture(p1)->getDays()[i] &&
                                    vector[c1]->getPossiblePairLecture(p1)->getDays()[i] == '1')
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
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                            if (stringcontains(vector[c1]->getPossiblePairLecture(p1)->getDays(),
                                               vector[c]->getPossiblePairLecture(p)->getDays(),
                                               instance->getNdays()) ==
                                1) { ;
                            } else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "samedays_" + itos(vector[c]->getId()) + "_" +
                                                 itos(vector[c1]->getId()) + "_" +
                                                 vector[c1]->getPossiblePairLecture(p1)->getDays() + "_" +
                                                 vector[c]->getPossiblePairLecture(p)->getDays());


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
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                            for (int i = 0; i < instance->getNweek(); ++i) {
                                if (vector[c]->getPossiblePairLecture(p)->getWeeks()[i] ==
                                    vector[c1]->getPossiblePairLecture(p1)->getWeeks()[i] &&
                                    vector[c1]->getPossiblePairLecture(p1)->getWeeks()[i] == '1')
                                    model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                     timetable[vector[c1]->getOrderID()][p1] <= 1);

                            }


                        }
                    }
                }


            }
        }

    }

    void sameWeek(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                            for (int i = 0; i < instance->getNweek(); ++i) {
                                if (vector[c]->getPossiblePairLecture(p)->getWeeks()[i] !=
                                    vector[c1]->getPossiblePairLecture(p1)->getWeeks()[i] &&
                                    vector[c1]->getPossiblePairLecture(p1)->getWeeks()[i] == '1')
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
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                            if (stringcompare(vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                              vector[c1]->getPossiblePairLecture(p1)->getWeeks(), instance->getNweek(),
                                              false) == 1 ||
                                stringcompare(vector[c]->getPossiblePairLecture(p)->getDays(),
                                              vector[c1]->getPossiblePairLecture(p1)->getDays(), instance->getNdays(),
                                              false) == 1) { ;
                            } else {
                                if (std::max(vector[c]->getPossiblePairLecture(p)->getEnd(),
                                             vector[c1]->getPossiblePairLecture(p1)->getEnd()) -
                                    std::min(vector[c]->getPossiblePairLecture(p)->getStart(),
                                             vector[c1]->getPossiblePairLecture(p1)->getStart()) > l) {
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


    void minGap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int l) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                            if (stringcompare(vector[c1]->getPossiblePairLecture(p1)->getWeeks(),
                                              vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                              instance->getNweek(), false) == 0 ||
                                stringcompare(vector[c1]->getPossiblePairLecture(p1)->getDays(),
                                              vector[c]->getPossiblePairLecture(p)->getDays(),
                                              instance->getNdays(), false) == 0 ||
                                vector[c1]->getPossiblePairLecture(p1)->getStart() +
                                vector[c1]->getPossiblePairLecture(p1)->getLenght() + l <=
                                vector[c]->getPossiblePairLecture(p)->getStart() ||
                                vector[c]->getPossiblePairLecture(p)->getStart() +
                                vector[c]->getPossiblePairLecture(p)->getLenght() +
                                l <= vector[c1]->getPossiblePairLecture(p1)->getStart()) { ;
                            } else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "minGAP_" + itos(vector[c]->getOrderID()) + "_" +
                                                 itos(vector[c1]->getOrderID()) + "_" + itos(p) + "_" + itos(p1));


                            }


                        }
                    }
                }


            }
        }

    }


    void maxDayLoad(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, int l) {
        for (int w = 0; w < instance->getNweek(); ++w) {
            for (int d = 0; d < instance->getNdays(); ++d) {
                GRBLinExpr t = 0;
                for (int c = 0; c < vector.size(); ++c) {
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        if (vector[c]->getPossiblePairLecture(p)->getWeeks()[w] == '1'
                            && vector[c]->getPossiblePairLecture(p)->getDays()[d] == '1') {
                            t += timetable[vector[c]->getOrderID()][p] *
                                 vector[c]->getPossiblePairLecture(p)->getLenght();
                        }
                    }
                }
                model->addConstr(t <= l);

            }

        }

    }


    virtual void dist(bool hard) override {
        oneLectureRoom();
        std::cout << getTimeSpent() << std::endl;
        for (int i = 0; i < instance->getDist().size(); ++i) {
            std::vector<Class *> c;
            for (int j = 0; j < instance->getDist()[i]->getClasses().size(); ++j) {
                c.push_back(instance->getClass(instance->getDist()[i]->getClasses()[j]));
            }
            std::cout << getTimeSpent() << std::endl;
            if (instance->getDist()[i]->getPenalty() == -1) {
                if (instance->getDist()[i]->getType()->getType() == SameStart) {
                    sameStart(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameTime) {
                    sameTime(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == Precedence) {
                    precedence(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameTime) {
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
                } else if (instance->getDist()[i]->getType()->getType() == SameWeeks) {
                    sameWeek(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == WorkDay) {
                    workDay(c, instance->getDist()[i]->getPenalty(),
                            instance->getDist()[i]->getType()->getLimit());
                } else if (instance->getDist()[i]->getType()->getType() == MaxDayLoad) {
                    maxDayLoad(c, instance->getDist()[i]->getPenalty(),
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
            Room r = instance->getClassbyclusterRoom()[clu]->getRooms();
            for (int c = 0; c < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c) {
                for (int c1 = c + 1; c1 < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c1) {
                    for (int p = 0;
                         p < instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair(r).size(); ++p) {
                        for (int p1 = 0; p1 < instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair(
                                r).size(); ++p1) {

                            if (check(instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair(
                                    r)[p].second,
                                      instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair(
                                              r)[p1].second)) {
                                assert(r.getId() ==
                                       instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair(
                                               instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair(
                                                       r)[p1].first).first.getId());
                                assert(r.getId() ==
                                       instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair(
                                               instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair(
                                                       r)[p].first).first.getId());
                                model->addConstr(
                                        timetable[instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair(
                                                r)[p].first]
                                        +
                                        timetable[instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair(
                                                r)[p1].first] <= 1, "RoomConflict" + itos(r.getId()) + "_" +
                                                                    itos(instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getId())
                                                                    + "_" +
                                                                    itos(instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getId()) +
                                                                    "_" +
                                                                    itos(instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair(
                                                                            r)[p].first) + "_" +
                                                                    itos(instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair(
                                                                            r)[p1].first));


                            }

                        }
                    }

                }
            }
        }

    }

private:
    bool check(Lecture *p1, Lecture *p2) {
        for (int j = 0; j < instance->getNweek(); ++j) {
            if (p1->getWeeks()[j] == p2->getWeeks()[j] &&
                p1->getWeeks()[j] == '1') {
                for (int d = 0; d < instance->getNdays(); ++d) {
                    if (p1->getDays()[d] == p2->getDays()[d] &&
                        p1->getDays()[d] == '1') {
                        if (p1->getStart() >= p2->getStart() &&
                            p1->getStart() <
                            p2->getEnd()) {
                            return true;
                        } else if (p2->getStart() >= p1->getStart() &&
                                   p2->getStart() < p1->getEnd()) {
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
            for (int p = 0; p < instance->getClasses()[c]->getPossiblePairSize(); ++p) {
                bool active = timetable[c][p].get(GRB_DoubleAttr_X);
                if (active != 0) {
                    instance->getClasses()[c]->setSolution(new Solution(instance->getClasses()[c]->getId(),
                                                                        instance->getClasses()[c]->getPossiblePairLecture(
                                                                                p)->getStart(),
                                                                        instance->getClasses()[c]->getPossiblePairRoom(
                                                                                p).getId(),
                                                                        instance->getClasses()[c]->getPossiblePairLecture(
                                                                                p)->getWeeks(),
                                                                        instance->getClasses()[c]->getPossiblePairLecture(
                                                                                p)->getDays(),
                                                                        instance->getClasses()[c]->getPossiblePairLecture(
                                                                                p)->getLenght(), 0, 0));
                }

            }

        }


    }


};


#endif //PROJECT_ILPEXECUTER_H