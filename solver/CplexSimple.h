//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_CPLEXSIMPLE_H
#define PROJECT_CPLEXSIMPLE_H

#include <ilconcert/iloenv.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <math.h>       /* floor */
#include <exception>
#include <thread>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../utils/TimeUtil.h"
#include "../utils/StringUtil.h"
#include <algorithm>


class CplexSimple : public ILPExecuter {
    IloEnv env; //CPLEX execution
    IloModel model = IloModel(env);
    std::vector<std::vector<Class *>> conflictV;

    typedef IloArray <IloBoolVarArray> NumVarMatrix;// Matrix

    //Lectures
    NumVarMatrix timetable = NumVarMatrix(env);
    NumVarMatrix student = NumVarMatrix(env);

public:
    CplexSimple(Instance *i) {
        setInstance(i);
        i->setCompact(false);


    }

    bool run2019(bool warm) override {
        if (instance->getStudent().size() > 0) {
            init();
            ////std::cout << "init : Done " << getTimeSpent() << std::endl;
            requiredClasses();
            //std::cout << "classes : Done " << getTimeSpent() << std::endl;
            parentChild();
            //std::cout << "family : Done " << getTimeSpent() << std::endl;
            //cost =
            ////std::cout << "conflicts : Done " << getTimeSpent() << std::endl;
            limit();
            std::cout << "limit : Done " << getTimeSpent() << std::endl;
        }

        IloExpr opt(env);//costRoom*instance->getRoomPen()+costTime*instance->getTimePen();
        if (instance->getStudent().size() > 0)
            opt += conflicts() * instance->getStudentPen();
        model.add(IloMinimize(env, opt));
        std::cout << getTimeSpent() << std::endl;
        IloCplex cplex(model);
        //cplex.setParam(IloCplex::TiLim, 100.000);
        //cplex.exportModel("/Volumes/MAC/ClionProjects/timetabler/nome.lp");

        if (cplex.solve()) {
            std::cout << "solved" << std::endl;
            //printdistanceToSolutionRooms(cplex);
            double value = cplex.getObjValue();
            //std::cout << value << std::endl;
            save(cplex);
            if (instance->getStudent().size() > 0)
                saveStu(cplex);
        }


    }

    IloExpr costTime=  IloNumExpr(env);;

    IloExpr costRoom=  IloNumExpr(env);;


    void definedAuxVar() {
        for (int c = 0; c < instance->getNumClasses(); ++c) {
            IloNumExpr oneEach =  IloNumExpr(env);;
            instance->getClasses()[c]->setOrderID(c);
            instance->getClasses()[c]->computeSize();
            timetable.add(IloBoolVarArray(env, instance->getClasses()[c]->getPossiblePairSize()));
            for (int p = 0; p < instance->getClasses()[c]->getPossiblePairSize(); ++p) {
                costTime += timetable[c][p] * instance->getClasses()[c]->getPossiblePairLecture(p)->getPenalty();
                costRoom += timetable[c][p] * instance->getClasses()[c]->getPossibleRoomCost(
                        instance->getClasses()[c]->getPossiblePairRoom(p));


                oneEach += timetable[c][p];
            }
            model.add(oneEach == 1);
        }
    }


    void sameStart(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                IloNumExpr side1 =  IloNumExpr(env);;
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] * vector[c]->getPossiblePairLecture(p)->getStart();
                }
                IloNumExpr side2 =  IloNumExpr(env);;
                for (int p = 0; p < vector[c1]->getPossiblePairSize(); ++p) {
                    side2 += timetable[vector[c1]->getOrderID()][p] * vector[c1]->getPossiblePairLecture(p)->getStart();
                }
                IloConstraint ctemp(side1 == side2);
                ctemp.setName(("sameStart_"+itos(vector[c1]->getId())+"_"+itos(vector[c]->getId())).c_str());
                model.add(ctemp);

            }
        }

    }

    void precedence(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                IloNumExpr side1 =  IloNumExpr(env);;
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
                            model.add(timetable[vector[c]->getOrderID()][p] +
                                      timetable[vector[c1]->getOrderID()][p1] <= 1);


                    }

                }

            }

        }
    }


    void sameTime(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                            model.add(timetable[vector[c]->getOrderID()][p] +
                                      timetable[vector[c1]->getOrderID()][p1] <= 1);
                        }
                    }


                }


            }
        }

    }

    void diffTime(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                        if (vector[c]->getPossiblePairLecture(p)->getEnd() <=
                            vector[c1]->getPossiblePairLecture(p1)->getStart() ||
                            vector[c1]->getPossiblePairLecture(p1)->getEnd() <=
                            vector[c]->getPossiblePairLecture(p)->getStart()) { ;
                        } else {
                            model.add(timetable[vector[c]->getOrderID()][p] +
                                      timetable[vector[c1]->getOrderID()][p1] <= 1);
                        }
                    }


                }

            }
        }

    }


    void sameRoom(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                IloNumExpr
                side1 =  IloNumExpr(env);;
                for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                    side1 += timetable[vector[c]->getOrderID()][p] *
                             vector[c]->getPossiblePairRoom(p).getId();
                }
                IloNumExpr
                side2 =  IloNumExpr(env);;
                for (int p = 0; p < vector[c1]->getPossiblePairSize(); ++p) {
                    side2 +=
                            timetable[vector[c1]->getOrderID()][p] *
                            vector[c1]->getPossiblePairRoom(p).getId();
                }
                model.add(side1 == side2);
            }


        }


    }

    void sameAttendees(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                                model.add(timetable[vector[c]->getOrderID()][p] +
                                          timetable[vector[c1]->getOrderID()][p1] <= 1);
                            }
                        }


                    }
                }

            }
        }
    }


    void notOverlap(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                                model.add(timetable[vector[c]->getOrderID()][p] +
                                          timetable[vector[c1]->getOrderID()][p1] <= 1);
                            }
                        }


                    }
                }


            }
        }


    }

    void overlap(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                                model.add(timetable[vector[c]->getOrderID()][p] +
                                          timetable[vector[c1]->getOrderID()][p1] <= 1);
                            }
                        }


                    }
                }


            }
        }


    }


    void diffDays(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                                    model.add(timetable[vector[c]->getOrderID()][p] +
                                              timetable[vector[c1]->getOrderID()][p1] <= 1);

                            }


                        }
                    }
                }


            }
        }

    }

    void sameDays(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                                model.add(timetable[vector[c]->getOrderID()][p] +
                                          timetable[vector[c1]->getOrderID()][p1] <= 1);


                            }


                        }
                    }


                }
            }

        }
    }


    void diffWeek(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                                    model.add(timetable[vector[c]->getOrderID()][p] +
                                              timetable[vector[c1]->getOrderID()][p1] <= 1);

                            }


                        }
                    }
                }


            }
        }

    }

    void sameWeek(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty
    ) {
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
                                    model.add(timetable[vector[c]->getOrderID()][p] +
                                              timetable[vector[c1]->getOrderID()][p1] <= 1);

                            }


                        }
                    }
                }


            }
        }

    }


    void workDay(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty,
    int l
    ) {
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
                                    model.add(timetable[vector[c]->getOrderID()][p] +
                                              timetable[vector[c1]->getOrderID()][p1] <= 1);
                                }
                            }


                        }
                    }
                }


            }
        }

    }


    void minGap(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty,
    int l
    ) {
        for (int c = 0; c < vector.size(); ++c) {
            for (int c1 = c + 1; c1 < vector.size(); ++c1) {
                if (vector[c]->getId() !=
                    vector[c1]->getId()) {
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        for (int p1 = 0; p1 < vector[c1]->getPossiblePairSize(); ++p1) {
                            if (stringcompare(vector[c1]->getPossiblePairLecture(p1)->getWeeks(),
                                              vector[c]->getPossiblePairLecture(p)->getWeeks(),
                                              instance->getNweek(), false) == 1 ||
                                stringcompare(vector[c1]->getPossiblePairLecture(p1)->getDays(),
                                              vector[c]->getPossiblePairLecture(p)->getDays(),
                                              instance->getNdays(), false) == 1 ||
                                    (vector[c1]->getPossiblePairLecture(p1)->getEnd() + l) <=
                                vector[c]->getPossiblePairLecture(p)->getStart() ||
                                    (vector[c]->getPossiblePairLecture(p)->getEnd() +
                                l )<= vector[c1]->getPossiblePairLecture(p1)->getStart()) { ;
                            } else {
                                model.add(timetable[vector[c]->getOrderID()][p] +
                                          timetable[vector[c1]->getOrderID()][p1] <= 1);


                            }


                        }
                    }
                }


            }
        }

    }


    void maxDayLoad(const std::vector<Class *, std::allocator < Class * >>

    &vector,
    int penalty,
    int l
    ) {
        for (int w = 0; w < instance->getNweek(); ++w) {
            for (int d = 0; d < instance->getNdays(); ++d) {
                IloNumExpr t =  IloNumExpr(env);;

                for (int c = 0; c < vector.size(); ++c) {
                    for (int p = 0; p < vector[c]->getPossiblePairSize(); ++p) {
                        if (vector[c]->getPossiblePairLecture(p)->getWeeks()[w] == '1'
                            && vector[c]->getPossiblePairLecture(p)->getDays()[d] == '1') {
                            t += timetable[vector[c]->getOrderID()][p] *
                                 vector[c]->getPossiblePairLecture(p)->getLenght();
                        }
                    }
                }
                model.add(t <= l);

            }

        }

    }


    virtual void dist(bool hard) override {
        oneLectureRoom();
        std::cout << getTimeSpent() << std::endl;
        for (int i = 0; i < instance->getDist().size(); ++i) {
            std::vector < Class * > c;
            for (int j = 0; j < instance->getDist()[i]->getClasses().size(); ++j) {
                c.push_back(instance->getClass(instance->getDist()[i]->getClasses()[j]));
            }
            //std::cout << getTimeSpent() << std::endl;
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


    void oneLectureRoom() {
        for (std::map<int, Room>::iterator it = instance->getRooms().begin(); it != instance->getRooms().end(); ++it) {
            Room r = it->second;//    std::map<int,std::vector<std::pair<int, int>>> t;
            for (std::map<int, std::vector<std::pair<int, int>>>::iterator it1 =
                    r.t.begin(); it1 != r.t.end(); ++it1) {
                for (int c = 0; c < it1->second.size(); ++c) {
                    for (int c1 = c + 1; c1 < it1->second.size(); ++c1) {
                            model.add(
                                    timetable[it1->second[c].first][it1->second[c].second]
                                    +
                                    timetable[it1->second[c1].first][it1->second[c1].second] <= 1);




                    }
                }
            }
        }
    }




    /**
     * A lecture can only be in one room at time
     */

    void oneLectureRoom1() {
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
                                model.add(
                                        timetable[instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c]->getPossiblePair(
                                                r)[p].first]
                                        +
                                        timetable[instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getOrderID()][instance->getClassbyclusterRoom()[clu]->getClasses()[c1]->getPossiblePair(
                                                r)[p1].first] <= 1);


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

    void save(IloCplex cplex) {
        for (int c = 0; c < instance->getNumClasses(); c++) {
            for (int p = 0; p < instance->getClasses()[c]->getPossiblePairSize(); ++p) {
                bool active = cplex.getValue(timetable[c][p]);
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


    //Stu
    void init() {
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            student.add(IloBoolVarArray(env, instance->getClasses().size()));
        }
    }

    void requiredClasses() {
        int s = 0;
        int size = 0;
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {
            std::vector < Class * > temp;
            for (int c = 0; c < it->second.getCourse().size(); ++c) {
                IloNumExpr course =  IloNumExpr(env);;
                for (int conf = 0; conf < it->second.getCourse()[c]->getNumConfig(); ++conf) {
                    IloNumExpr con =  IloNumExpr(env);;
                    for (int part = 0; part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                        IloNumExpr sub =  IloNumExpr(env);;

                        for (int cla = 0;
                             cla < it->second.getCourse()[c]->getSubpart(conf)[part]->getClasses().size(); ++cla) {
                            sub += student[s][it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses()[cla]->getOrderID()];
                            temp.push_back(it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses()[cla]);

                        }
                        IloConstraint ctemp(sub <= 1);
                        ctemp.setName(("requiredClassesSub_"+itos(s)+"_"+itos(c)+"_"+itos(conf)+"_"+itos(part)).c_str());
                        model.add(ctemp);
                        con += sub;
                    }
                    IloConstraint ctemp(con == it->second.getCourse()[c]->getSubpart(conf).size() ||
                                        0 == con);
                    ctemp.setName(("requiredClassesConf_"+itos(s)+"_"+itos(c)+"_"+itos(conf)).c_str());
                    model.add(ctemp);
                    size = it->second.getCourse()[c]->getSubpart(conf).size();
                    course += con;
                }
                IloConstraint ctemp(course == size);
                ctemp.setName(("requiredClassesCourse_"+itos(s)+"_"+itos(c)).c_str());
                model.add(ctemp);
            }
            conflictV.push_back(temp);
            s++;

        }


    }


    void parentChild() {
        int s = 0;
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {
            for (int c = 0; c < instance->getClasses().size(); ++c) {
                if (instance->getClasses()[c]->getParent() != nullptr) {
                    model.add(IloIfThen(env, student[s][instance->getClasses()[c]->getOrderID()] == 1,
                                        student[s][instance->getClasses()[c]->getParent()->getOrderID()] == 1));
                }

            }
            s++;
        }

    }

    IloNumExpr conflicts() {
        IloNumExpr cost =  IloNumExpr(env);;
        for (int s = 0; s < conflictV.size(); ++s) {
            for (int cla = 0; cla < conflictV[s].size(); ++cla) {
                for (int cla1 = cla + 1; cla1 < conflictV[s].size(); ++cla1) {
                    Class *class1 = conflictV[s][cla];
                    Class *class2 = conflictV[s][cla1];
                    if(class1->getSubconfcour().compare(class2->getSubconfcour())==0)
                        continue;
                    for (int t = 0; t < class1->getPossiblePairSize(); ++t) {
                        for (int t1 = 0; t1 < class2->getPossiblePairSize(); ++t1) {
                            if (checkStu(class2->getPossiblePair(t1),
                                         class1->getPossiblePair(t))) {
                                cost += (timetable[class1->getOrderID()][t] +
                                         timetable[class2->getOrderID()][t1] +
                                         student[s][class2->getOrderID()] +
                                         student[s][class1->getOrderID()] == 4);

                            }

                        }
                    }
                }
            }
        }
        return cost;


    }

    bool checkStu(std::pair<Room, Lecture *> p1, std::pair<Room, Lecture *> p2) {
        int travel = 0;
        if (p1.first.getId() != -1 && p2.first.getId() != -1 && p1.first.getId() != p2.first.getId()) {
            if (instance->getRoom(p1.first.getId()).getTravel(
                    p2.first.getId()) > 0)
                travel = instance->getRoom(p1.first.getId()).getTravel(
                        p2.first.getId());
            else
                travel = instance->getRoom(
                        p2.first.getId()).getTravel(
                        p1.first.getId());
        }
        for (int j = 0; j < instance->getNweek(); ++j) {
            if (p1.second->getWeeks()[j] == p2.second->getWeeks()[j] &&
                p1.second->getWeeks()[j] == '1') {
                for (int d = 0; d < instance->getNdays(); ++d) {
                    if (p1.second->getDays()[d] == p2.second->getDays()[d] &&
                        p1.second->getDays()[d] == '1') {
                        if (p1.second->getStart() >= p2.second->getStart() &&
                            p1.second->getStart() <
                            p2.second->getEnd() + travel) {
                            return true;
                        } else if (p2.second->getStart() >= p1.second->getStart() &&
                                   p2.second->getStart() < p1.second->getEnd() + travel) {
                            return true;
                        }
                    }
                }
            }
        }
        return false;
    }


    void limit() {
        for (int cla = 0; cla < instance->getClasses().size(); ++cla) {
            IloNumExpr stu =  IloNumExpr(env);;
            for (int i = 0; i < instance->getStudent().size(); ++i) {
                stu += student[i][instance->getClasses()[cla]->getOrderID()];
            }
            IloConstraint c(stu <= instance->getClasses()[cla]->getLimit());
            c.setName(("limit"+itos(instance->getClasses()[cla]->getId())).c_str());
            model.add(c);
        }
    }

    void saveStu(IloCplex cplex) {
        for (int s = 0; s < instance->getStudent().size(); ++s) {
            for (int cla = 0; cla < instance->getClasses().size(); ++cla) {
                if (cplex.getValue(student[s][instance->getClasses()[cla]->getOrderID()]) == 1) {
                    Class *c = instance->getClasses()[cla];
                    c->addStudent(instance->getStudent(s + 1).getId());
                    instance->getStudent(s + 1).addClass(c);
                }
            }
        }
    }




};


#endif //PROJECT_ILPEXECUTER_H