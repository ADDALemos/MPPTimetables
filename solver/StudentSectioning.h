//
// Created by Alexandre Lemos on 28/04/2019.
//

#ifndef PROJECT_STUDENTVAR_H
#define PROJECT_STUDENTVAR_H

#include <gurobi_c++.h>
#include "GurobiAux.h"
#include "../problem/Instance.h"
#include "../problem/Class.h"

class StudentSectioning {
    GRBVar **student;
    Instance *instance;
public:
    StudentSectioning(Instance *instance) : instance(instance) {}

    void init() {
        student = new GRBVar *[instance->getStudent().size()];
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            student[i] = new GRBVar[instance->getClasses().size()];//Reduce size
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                student[i][j] = model->addVar(0, 1, 0, GRB_BINARY);
            }
        }
    }

    void requiredClasses() {
        int s = 0;
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {
            for (int c = 0; c < it->second.getCourse().size(); ++c) {
                GRBLinExpr course = 0;
                for (int conf = 0; conf < it->second.getCourse()[c]->getNumConfig(); ++conf) {
                    GRBVar con[it->second.getCourse()[c]->getSubpart(conf).size()];
                    GRBVar confVAR = model->addVar(0, 1, 0, GRB_BINARY, "conf_" +
                                                                        itos(conf) + "_" + itos(c) + "_" + itos(s));
                    for (int part = 0; part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                        GRBLinExpr sub = 0;
                        GRBVar subVAR = model->addVar(0, 1, 0, GRB_BINARY, "sub_" +
                                                                           itos(part) + "_" + itos(c) + "_" + itos(s));
                        for (int cla = 0;
                             cla < it->second.getCourse()[c]->getSubpart(conf)[part]->getClasses().size(); ++cla) {
                            sub += student[s][it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses()[cla]->getOrderID()];
                        }
                        model->addGenConstrIndicator(subVAR, 1, sub == 1);
                        model->addGenConstrIndicator(subVAR, 0, sub == 0);
                        con[part] = subVAR;
                    }
                    model->addGenConstrAnd(confVAR, con, it->second.getCourse()[c]->getSubpart(conf).size());
                    course += confVAR;
                }
                model->addConstr(course == 1);
            }
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
                                                 1 == student[s][instance->getClasses()[c]->getParent()->getOrderID()]);
                }

            }
            s++;
        }

    }

    //TODO: FAR APART
    GRBLinExpr conflicts(GRBVar *start, GRBVar *end) {
        int s = 0;
        GRBLinExpr res = 0;
        for (std::map<int, Student>::const_iterator it = instance->getStudent().begin();
             it != instance->getStudent().end(); ++it) {
            for (int c = 0; c < it->second.getCourse().size(); ++c) {
                for (int conf = 0; conf < it->second.getCourse()[c]->getNumConfig(); ++conf) {
                    for (int part = 0; part < it->second.getCourse()[c]->getSubpart(conf).size(); ++part) {
                        for (int cla = 0;
                             cla < it->second.getCourse()[c]->getSubpart(conf)[part]->getClasses().size(); ++cla) {
                            for (int cla1 = cla + 1; cla1 < it->second.getCourse()[c]->getSubpart(
                                    conf)[part]->getClasses().size(); ++cla1) {
                                Class *class1 = it->second.getCourse()[c]->getSubpart(conf)[part]->getClasses()[cla];
                                Class *class2 = it->second.getCourse()[c]->getSubpart(conf)[part]->getClasses()[cla1];
                                GRBVar tempT = model->addVar(0, 1, 0, GRB_BINARY,
                                                             "conflict_" + itos(class1->getOrderID())
                                                             + "_" + itos(class2->getOrderID())
                                                             + "_" + itos(s));
                                model->addGenConstrIndicator(tempT, 1, end[class1->getOrderID()]
                                                                       <=
                                                                       start[class2->getOrderID()]);
                                model->addGenConstrIndicator(tempT, 0, end[class1->getOrderID()] + 1
                                                                       >=
                                                                       start[class2->getOrderID()]);
                                GRBVar pen = model->addVar(0, 1, 0, GRB_BINARY);
                                model->addGenConstrIndicator(pen, 1, tempT + student[s][class2->getOrderID()] +
                                                                     student[s][class1->getOrderID()] == 3);
                                model->addGenConstrIndicator(pen, 0, tempT + student[s][class2->getOrderID()] +
                                                                     student[s][class1->getOrderID()] <= 2);
                                res += pen;

                            }
                        }
                    }
                }

            }
            s++;
        }
        return res;


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
