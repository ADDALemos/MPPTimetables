//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_GUROBI1_H
#define PROJECT_GUROBI1_H

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
#include <assert.h>     /* assert */

class Gurobi1 : public ILPExecuter {
    GRBVar **timetable;
    GRBVar **room;
    GRBVar ***sameRoom;
    GRBVar ***sameTime;


    StudentSectioning *studentSectioning;

public:
    Gurobi1(Instance *i) {
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
        model->write("/Volumes/MAC/ClionProjects/timetabler/" + instance->getName() + ".lp");
        model->optimize();
        store();
        if (instance->getStudent().size() > 0)
            studentSectioning->save();

    }


    void definedAuxVar() {
        timetable = new GRBVar *[instance->getNumClasses()];
        room = new GRBVar *[instance->getNumClasses()];
        for (int c = 0; c < instance->getNumClasses(); ++c) {
            instance->getClasses()[c]->setOrderID(c);
            GRBLinExpr oneEach = 0;
            timetable[c] = new GRBVar[instance->getClasses()[c]->getLectures().size()];
            for (int p = 0; p < instance->getClasses()[c]->getLectures().size(); ++p) {
                timetable[c][p] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                                "timetable" + itos(instance->getClasses()[c]->getId()) + "_" +
                                                instance->getClasses()[c]->getLectures()[p]->getWeeks() + "_" +
                                                instance->getClasses()[c]->getLectures()[p]->getDays() + "_" +
                                                itos(instance->getClasses()[c]->getLectures()[p]->getStart()) + "_" +
                                                itos(instance->getClasses()[c]->getLectures()[p]->getLenght()));
                oneEach += timetable[c][p];
            }
            model->addConstr(oneEach == 1);
            oneEach = 0;
            room[c] = new GRBVar[instance->getClasses()[c]->getPossibleRooms().size()];
            for (int p = 0; p < instance->getClasses()[c]->getPossibleRooms().size(); ++p) {
                //instance->getClasses()[c]->getPossibleRoom(p)->setOrderId(p);
                room[c][p] = model->addVar(0.0, 1.0, 0.0, GRB_BINARY,
                                           "room" + itos(instance->getClasses()[c]->getPossibleRoom(p).getId()) + "_" +
                                           itos(instance->getClasses()[c]->getId()));
                oneEach += room[c][p];
            }
            model->addConstr(oneEach == 1);
        }
        sameRoomVar();
        sameTimeVar();

        //orderD();
    }

    void sameRoomVar() {
        assert(instance->getClassbyclusterRoom().size() == instance->getRooms().size());
        sameRoom = new GRBVar **[instance->getClassbyclusterRoom().size()];
        for (int c = 0; c < instance->getClassbyclusterRoom().size(); ++c) {
            sameRoom[c] = new GRBVar *[instance->getNumClasses()];
            for (int cla = 0; cla < instance->getClassbyclusterRoom()[c]->numberofClasses(); ++cla) {
                sameRoom[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()] = new GRBVar[instance->getNumClasses()];
                for (int cla1 = 0; cla1 < instance->getClassbyclusterRoom()[c]->numberofClasses(); ++cla1) {
                    sameRoom[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()] = model->addVar(
                            0.0, 1.0, 0.0, GRB_BINARY,
                            "sameRoom" + itos(c) + "_" +
                            itos(instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getId()) + "_" +
                            itos(instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getId()));

                }
            }
        }

        for (int c = 0; c < instance->getClassbyclusterRoom().size(); ++c) {
            for (int cla = 0; cla < instance->getClassbyclusterRoom()[c]->numberofClasses(); ++cla) {
                for (int cla1 = 0; cla1 < instance->getClassbyclusterRoom()[c]->numberofClasses(); ++cla1) {
                    model->addGenConstrIndicator(
                            sameRoom[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()],
                            1,
                            room[instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][instance->getClassbyclusterRoom()[c]->getP(
                                    instance->getClassbyclusterRoom()[c]->getClasses()[cla])]
                            +
                            room[instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()][instance->getClassbyclusterRoom()[c]->getP(
                                    instance->getClassbyclusterRoom()[c]->getClasses()[cla1])] == 2);
                    model->addGenConstrIndicator(
                            sameRoom[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()],
                            0,
                            room[instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][instance->getClassbyclusterRoom()[c]->getP(
                                    instance->getClassbyclusterRoom()[c]->getClasses()[cla])]
                            +
                            room[instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()][instance->getClassbyclusterRoom()[c]->getP(
                                    instance->getClassbyclusterRoom()[c]->getClasses()[cla1])] <= 1);

                }
            }


        }
    }

    void sameTimeVar() {
        sameTime = new GRBVar **[instance->getClassbyclusterRoom().size()];
        for (int c = 0; c < instance->getClassbyclusterRoom().size(); ++c) {
            sameTime[c] = new GRBVar *[instance->getNumClasses()];
            for (int cla = 0; cla < instance->getClassbyclusterRoom()[c]->getClasses().size(); ++cla) {
                sameTime[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()] = new GRBVar[instance->getNumClasses()];
                for (int cla1 = cla + 1; cla1 < instance->getClassbyclusterRoom()[c]->getClasses().size(); ++cla1) {
                    sameTime[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()]
                    [instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()] = model->addVar(0.0, 1.0,
                                                                                                             0.0,
                                                                                                             GRB_BINARY,
                                                                                                             "sameTime" +
                                                                                                             itos(c) +
                                                                                                             "_" +
                                                                                                             itos(instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getId()) +
                                                                                                             "_" +
                                                                                                             itos(instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getId()));

                }
            }
        }

        for (int c = 0; c < instance->getClassbyclusterRoom().size(); ++c) {
            for (int cla = 0; cla < instance->getClassbyclusterRoom()[c]->getClasses().size(); ++cla) {
                for (int cla1 = cla + 1; cla1 < instance->getClassbyclusterRoom()[c]->getClasses().size(); ++cla1) {
                    for (int p = 0;
                         p < instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getLectures().size(); ++p) {
                        for (int p1 = 0; p1 <
                                         instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getLectures().size(); ++p1) {
                            if (check(instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getLectures()[p1],
                                      instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getLectures()[p])) {
                                model->addGenConstrIndicator(
                                        sameTime[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()],
                                        1,
                                        timetable[instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][p]
                                        +
                                        timetable[instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()][p1] ==
                                        2);
                                model->addGenConstrIndicator(
                                        sameTime[c][instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()],
                                        0,
                                        timetable[instance->getClassbyclusterRoom()[c]->getClasses()[cla]->getOrderID()][p]
                                        +
                                        timetable[instance->getClassbyclusterRoom()[c]->getClasses()[cla1]->getOrderID()][p1] <=
                                        1);

                            }
                        }

                    }

                }
            }
        }


    }


    void sameStart(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                GRBLinExpr side1 = 0;
                for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] * vector[c]->getLectures()[p]->getStart();
                }
                GRBLinExpr side2 = 0;
                for (int p = 0; p < vector[c1]->getLectures().size(); ++p) {
                    side2 += timetable[vector[c1]->getOrderID()][p] * vector[c1]->getLectures()[p]->getStart();
                }
                model->addConstr(side1 == side2);

            }
        }

    }


    void sameTimeC(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                        if (vector[c]->getLectures()[p]->getLenght() == vector[c1]->getLectures()[p1]->getLenght()) {
                            std::vector<Class *> temp;
                            temp.push_back(vector[c]);
                            temp.push_back(vector[c1]);
                            sameStart(temp, penalty);
                        } else {
                            if ((vector[c]->getLectures()[p]->getStart() <=
                                 vector[c1]->getLectures()[p1]->getStart()
                                 && vector[c]->getLectures()[p]->getEnd() <=
                                    vector[c1]->getLectures()[p1]->getEnd()) ||
                                (vector[c1]->getLectures()[p1]->getStart() <=
                                 vector[c]->getLectures()[p]->getStart()
                                 && vector[c1]->getLectures()[p1]->getEnd() <=
                                    vector[c]->getLectures()[p]->getEnd())) { ;
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
                for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                        if (vector[c]->getLectures()[p]->getEnd() <=
                            vector[c1]->getLectures()[p1]->getStart() ||
                            vector[c1]->getLectures()[p1]->getEnd() <=
                            vector[c]->getLectures()[p]->getStart()) { ;
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


    void sameRoomC(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                GRBLinExpr side1 = 0;
                /*for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] *
                             vector[c]->getLectures()[p]->getId();
                }
                GRBLinExpr side2 = 0;
                for (int p = 0; p < vector[c1]->getLectures().size(); ++p) {
                    side2 +=
                            timetable[vector[c1]->getOrderID()][p] *
                            vector[c1]->getLectures()[p]->getId();
                }
                model->addConstr(side1 == side2,
                                 "sameRoomC_" + itos(vector[c]->getId()) + "_" + itos(vector[c1]->getId()));*/
            }


        }


    }

    void sameAttendees(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                        if (vector[c]->getId() != vector[c1]->getId()) {
                            int travel = 0;
/*                            if(vector[c]->getLectures()[p].getId()!=-1&&vector[c1]->getLectures()[p1].getId()!=-1) {
                                if (instance->getRoom(vector[c]->getLectures()[p].getId()).getTravel(
                                        vector[c1]->getLectures()[p1].getId()) > 0)
                                    travel = instance->getRoom(vector[c]->getLectures()[p].getId()).getTravel(
                                            vector[c1]->getLectures()[p1].getId());
                                else
                                    travel = instance->getRoom(
                                            vector[c1]->getLectures()[p1].getId()).getTravel(
                                            vector[c]->getLectures()[p].getId());
                            }

                            if (vector[c]->getLectures()[p]->getEnd() + travel <=
                                vector[c1]->getLectures()[p1]->getStart()
                                || vector[c1]->getLectures()[p1]->getEnd() +
                                   travel <= vector[c]->getLectures()[p]->getStart()
                                || stringcompare(vector[c]->getLectures()[p]->getWeeks(),
                                                 vector[c1]->getLectures()[p1]->getWeeks(),
                                                 instance->getNweek(), false) ==
                                   1
                                || stringcompare(vector[c]->getLectures()[p]->getDays(),
                                                 vector[c1]->getLectures()[p1]->getDays(),
                                                 instance->getNdays(), false) ==
                                   1 || vector[c]->getLectures()[p]->getEnd() <=
                                        vector[c1]->getLectures()[p1]->getStart() ||
                                vector[c1]->getLectures()[p1]->getEnd() <=
                                vector[c]->getLectures()[p]->getStart()) { ;
                            } else {
                                model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                 timetable[vector[c1]->getOrderID()][p1] <= 1,
                                                 "sameAttendees_" + itos(vector[c1]->getId()) + "_" +
                                                 itos(vector[c]->getId())+ "_" +
                                                 itos(p)+ "_" +
                                                 itos(p1));
                            }
                        }*/


                        }
                    }

                }
            }
        }
    }


    void notOverlap(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {

                        if (vector[c]->getId() !=
                            vector[c1]->getId()) {
                            if (stringcompare(vector[c]->getLectures()[p]->getWeeks(),
                                              vector[c1]->getLectures()[p1]->getWeeks(),
                                              instance->getNweek(), false) ==
                                1
                                || stringcompare(vector[c]->getLectures()[p]->getDays(),
                                                 vector[c1]->getLectures()[p1]->getDays(),
                                                 instance->getNdays(), false) ==
                                   1 ||
                                vector[c]->getLectures()[p]->getEnd() <= vector[c1]->getLectures()[p1]->getStart() ||
                                vector[c1]->getLectures()[p1]->getEnd() <= vector[c]->getLectures()[p]->getStart()) { ;
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
                for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                        for (int i = 0; i < instance->getNweek(); ++i) {
                            model->addGenConstrIndicator(sameweek[i], 1,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getLectures()[p]->getWeeks()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getLectures()[p1]->getWeeks()[i] == 2);
                            model->addGenConstrIndicator(sameweek[i], 0,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getLectures()[p]->getWeeks()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getLectures()[p1]->getWeeks()[i] <= 1);
                        }
                        for (int i = 0; i < instance->getNdays(); ++i) {
                            model->addGenConstrIndicator(sameday[i], 1,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getLectures()[p]->getDays()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getLectures()[p1]->getDays()[i] == 2);
                            model->addGenConstrIndicator(sameday[i], 0,
                                                         timetable[vector[c]->getOrderID()][p] *
                                                         vector[c]->getLectures()[p]->getDays()[i] +
                                                         timetable[vector[c1]->getOrderID()][p1] *
                                                         vector[c1]->getLectures()[p1]->getDays()[i] <= 1);
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
                    for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                            for (int i = 0; i < instance->getNdays(); ++i) {
                                if (vector[c]->getLectures()[p]->getDays()[i] ==
                                    vector[c1]->getLectures()[p1]->getDays()[i] &&
                                    vector[c1]->getLectures()[p1]->getDays()[i] == '1')
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
                    for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                            if (stringcontains(vector[c1]->getLectures()[p1]->getDays(),
                                               vector[c]->getLectures()[p]->getDays(),
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
                    for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                            for (int i = 0; i < instance->getNweek(); ++i) {
                                if (vector[c]->getLectures()[p]->getWeeks()[i] ==
                                    vector[c1]->getLectures()[p1]->getWeeks()[i] &&
                                    vector[c1]->getLectures()[p1]->getWeeks()[i] == '1')
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
                    for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                            for (int i = 0; i < instance->getNweek(); ++i) {
                                if (vector[c]->getLectures()[p]->getWeeks()[i] ==
                                    vector[c1]->getLectures()[p1]->getWeeks()[i] &&
                                    vector[c1]->getLectures()[p1]->getWeeks()[i] == '1') {
                                    if (vector[c]->getLectures()[p]->getStart() >=
                                        vector[c1]->getLectures()[p1]->getStart() &&
                                        vector[c1]->getLectures()[p1]->getEnd() -
                                        vector[c]->getLectures()[p]->getStart() > l) {
                                        model->addConstr(timetable[vector[c]->getOrderID()][p] +
                                                         timetable[vector[c1]->getOrderID()][p1] <= 1);
                                    } else if (vector[c]->getLectures()[p]->getStart() <
                                               vector[c1]->getLectures()[p1]->getStart() &&
                                               vector[c]->getLectures()[p]->getEnd() -
                                               vector[c1]->getLectures()[p1]->getStart() > l) {
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
                    for (int p = 0; p < vector[c]->getLectures().size(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getLectures().size(); ++p1) {
                            if (stringcompare(vector[c1]->getLectures()[p1]->getWeeks(),
                                              vector[c]->getLectures()[p]->getWeeks(),
                                              instance->getNweek(), false) == 0 ||
                                stringcompare(vector[c1]->getLectures()[p1]->getDays(),
                                              vector[c]->getLectures()[p]->getDays(),
                                              instance->getNdays(), false) == 0 ||
                                vector[c1]->getLectures()[p1]->getStart() +
                                vector[c1]->getLectures()[p1]->getLenght() + l <=
                                vector[c]->getLectures()[p]->getStart() ||
                                vector[c]->getLectures()[p]->getStart() +
                                vector[c]->getLectures()[p]->getLenght() +
                                l <= vector[c1]->getLectures()[p1]->getStart());
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
                    //sameStart(c, instance->getDist()[i]->getPenalty());
                }
                if (instance->getDist()[i]->getType()->getType() == SameTime) {
                    //th.push_back(std::thread(&GurobiSimple::sameTime,this,c, instance->getDist()[i]->getPenalty()));
                }
                if (instance->getDist()[i]->getType()->getType() == SameTime) {
                    //diffTime(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameRoom) {
                    //sameRoomC(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameAttendees) {
                    sameAttendees(c, instance->getDist()[i]->getPenalty());
                    //th.push_back(std::thread(&GurobiSimple::sameAttendees,this,c, instance->getDist()[i]->getPenalty()));
                } else if (instance->getDist()[i]->getType()->getType() == NotOverlap) {
                    //notOverlap(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameDays) {
                    //sameDays(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == SameDays) {
                    //sameDays(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == DifferentDays) {
                    //diffDays(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == DifferentWeeks) {
                    //diffWeek(c, instance->getDist()[i]->getPenalty());
                } else if (instance->getDist()[i]->getType()->getType() == WorkDay) {
                    //workDay(c, instance->getDist()[i]->getPenalty(),
                    //             instance->getDist()[i]->getType()->getLimit());
                } else if (instance->getDist()[i]->getType()->getType() == MinGap) {
                    //minGap(c, instance->getDist()[i]->getPenalty(),
                    //           instance->getDist()[i]->getType()->getLimit());
                }
            }

        }
/*        t.join();
        for (int k = 0; k < th.size(); ++k) {
            th[k].join();
        }*/


    }


    /**
     * A lecture can only be in one room at time
     */

    void oneLectureRoom() {
        for (int clu = 0; clu < instance->getClassbyclusterRoom().size(); ++clu) {
            for (int c = 0; c < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c) {
                for (int c1 = c + 1; c1 < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c1) {
                    model->addConstr(
                            sameTime[clu][instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getOrderID()]
                            +
                            sameRoom[clu][instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getOrderID()] <=
                            1);


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
                            p2->getStart() + p2->getLenght()) {
                            return true;
                        } else if (p2->getStart() >= p1->getStart() &&
                                   p2->getStart() < p1->getStart() +
                                                    p1->getLenght()) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }

    void store() {
        for (int clu = 0; clu < instance->getClassbyclusterRoom().size(); ++clu) {
            for (int c = 0; c < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c) {
                for (int c1 = c + 1; c1 < instance->getClassbyclusterRoom()[clu]->numberofClasses(); ++c1) {
                    std::cout << instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getId() << " " <<
                              instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getId() << std::endl;
                    std::cout << "sameTime "
                              << sameTime[clu][instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getOrderID()].get(
                                      GRB_DoubleAttr_X)
                              << " SameRoom " <<
                              sameRoom[clu][instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getOrderID()].get(
                                      GRB_DoubleAttr_X) << std::endl;


                }
            }
        }
        for (int c = 0; c < instance->getNumClasses(); c++) {
            for (int p = 0; p < instance->getClasses()[c]->getLectures().size(); ++p) {
                bool active = timetable[c][p].get(GRB_DoubleAttr_X);
                if (active != 0) {
                    instance->getClasses()[c]->updateSolution(
                            instance->getClasses()[c]->getLectures()[p]->getStart(),
                            instance->getClasses()[c]->getLectures()[p]->getWeeks(),
                            instance->getClasses()[c]->getLectures()[p]->getDays(),
                            instance->getClasses()[c]->getLectures()[p]->getLenght());
                }

            }
            for (int r = 0; r < instance->getClasses()[c]->getPossibleRooms().size(); ++r) {
                bool active = room[c][r].get(GRB_DoubleAttr_X);
                if (active != 0) {
                    instance->getClasses()[c]->setSolRoom(instance->getClasses()[c]->getPossibleRoom(r).getId());
                }
            }


        }


    }


};


#endif //PROJECT_ILPEXECUTER_H