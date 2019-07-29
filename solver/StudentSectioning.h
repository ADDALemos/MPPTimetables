//
// Created by Alexandre Lemos on 28/04/2019.
//

#ifndef PROJECT_STUDENTVAR_H
#define PROJECT_STUDENTVAR_H

#include <gurobi_c++.h>
#include "GurobiAux.h"
#include "../problem/Instance.h"
#include "../problem/Class.h"
#include "../utils/StringUtil.h"

class StudentSectioning {
    GRBVar **student;
    Instance *instance;
    std::vector<std::vector<Class *>> conflictV;
public:
    StudentSectioning(Instance *instance) : instance(instance) {}

    void init() {
        student = new GRBVar *[instance->getStudent().size()];
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            student[i] = new GRBVar[instance->getClasses().size()];//Reduce size
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                student[i][j] = model->addVar(0, 1, 0, GRB_BINARY,
                                              "stu_" + itos(i) + "_" + itos(instance->getClasses()[j]->getId()));
            }
        }
    }

    void requiredClasses() {
        int s = 0;
        int size = 0;
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {
            std::vector<Class *> temp;
            for (int c = 0; c < it->second.getCourse().size(); ++c) {
                GRBLinExpr course = 0;
                for (int conf = 0; conf < it->second.getCourse()[c]->getNumConfig(); ++conf) {
                    GRBLinExpr con = 0;
                    for (int part = 0; part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                        GRBLinExpr sub = 0;

                        for (int cla = 0;
                             cla < it->second.getCourse()[c]->getSubpart(conf)[part]->getClasses().size(); ++cla) {
                            sub += student[s][it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses()[cla]->getOrderID()];
                            temp.push_back(it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses()[cla]);

                        }
                        model->addConstr(sub <= 1);
                        con += sub;
                    }
                    model->addConstr(con - it->second.getCourse()[c]->getSubpart(conf).size() ==
                                     it->second.getCourse()[c]->getSubpart(conf).size() - con);
                    size = it->second.getCourse()[c]->getSubpart(conf).size();
                    course += con;
                }
                model->addConstr(course == size);
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
                    model->addGenConstrIndicator(student[s][instance->getClasses()[c]->getOrderID()], 1,
                                                 student[s][instance->getClasses()[c]->getParent()->getOrderID()] == 1);
                }

            }
            s++;
        }

    }

    GRBLinExpr conflicts(GRBVar **time) {
        GRBLinExpr cost = 0;
        for (int s = 0; s < conflictV.size(); ++s) {
            for (int cla = 0; cla < conflictV[s].size(); ++cla) {
                for (int cla1 = cla + 1; cla1 < conflictV[s].size(); ++cla1) {
                    Class *class1 = conflictV[s][cla];
                    Class *class2 = conflictV[s][cla1];

                    for (int t = 0; t < class1->getPossiblePairSize(); ++t) {
                        for (int t1 = 0; t1 < class2->getPossiblePairSize(); ++t1) {
                            if (check(class2->getPossiblePair(t1),
                                      class1->getPossiblePair(t))) {
                                GRBVar pen = model->addVar(0, 1, 0, GRB_BINARY,
                                                           "pen_" + itos(s) + "_" + itos(class1->getId()) +
                                                           "_" + itos(class2->getId()) + "_" + itos(t) + "_" +
                                                           itos(t1));
                                model->addGenConstrIndicator(pen, 1, time[class1->getOrderID()][t] +
                                                                     time[class2->getOrderID()][t1] +
                                                                     student[s][class2->getOrderID()] +
                                                                     student[s][class1->getOrderID()] == 4,
                                                             "SC1_" + itos(s) + "_" +
                                                             itos(class1->getId()) + "_" +
                                                             itos(class2->getId()) + "_" + itos(t) + "_" +
                                                             itos(t1));
                                model->addGenConstrIndicator(pen, 0, time[class1->getOrderID()][t] +
                                                                     time[class2->getOrderID()][t1] +
                                                                     student[s][class2->getOrderID()] +
                                                                     student[s][class1->getOrderID()] <= 3,
                                                             "SC0_" + itos(s) + "_" +
                                                             itos(class1->getId()) + "_" +
                                                             itos(class2->getId()) + "_" + itos(t) + "_" +
                                                             itos(t1));
                                cost += pen;

                            }

                        }
                    }
                }
            }
        }
        return cost;


    }

    bool check(std::pair<Room, Lecture *> p1, std::pair<Room, Lecture *> p2) {
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
            GRBLinExpr stu = 0;
            for (int i = 0; i < instance->getStudent().size(); ++i) {
                stu += student[i][instance->getClasses()[cla]->getOrderID()];
            }
            model->addConstr(stu <= instance->getClasses()[cla]->getLimit());
        }
    }

    void save() {
        for (int s = 0; s < instance->getStudent().size(); ++s) {
            for (int cla = 0; cla < instance->getClasses().size(); ++cla) {
                if (student[s][instance->getClasses()[cla]->getOrderID()].get(GRB_DoubleAttr_X) == 1) {
                    Class *c = instance->getClasses()[cla];
                    c->addStudent(instance->getStudent(s + 1).getId());
                    instance->getStudent(s + 1).addClass(c);
                }
            }
        }
    }

};


#endif //PROJECT_STUDENT_H
