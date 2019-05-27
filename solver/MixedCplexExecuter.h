//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_MIXEDCPLEXEXECUTER_H
#define PROJECT_MIXEDCPLEXEXECUTER_H

#include <ilconcert/iloenv.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "ILPExecuter.h"


class MIXEDCplexExecuter : public ILPExecuter {
    IloEnv env; //CPLEX execution
    IloModel model = IloModel(env);


    typedef IloArray<IloBoolVarArray> NumVarMatrix;// Matrix

    //Lectures Time
    IloIntVarArray startVar;
    IloIntVarArray endVar;
    IloIntVarArray tVar;
    NumVarMatrix weekVar = NumVarMatrix(env);
    NumVarMatrix dayVar = NumVarMatrix(env);

    //Room
    NumVarMatrix roomVar = NumVarMatrix(env);


    //SameWeek
    NumVarMatrix sameWeekVar = NumVarMatrix(env);
    NumVarMatrix sameDayVar = NumVarMatrix(env);
    NumVarMatrix sameRoomVar = NumVarMatrix(env);
    NumVarMatrix sameTimeVar = NumVarMatrix(env);


public:

    double run(bool mpp) {
        std::cout << env.getVersion() << std::endl;
        IloCplex cplex(model);
        cplex.setParam(IloCplex::TiLim, 100.000);
        definedAuxVar();
        definedRoomLecture();
        std::cout << "H" << std::endl;

        definedLectureTime();
        std::cout << "G" << std::endl;

        dayConst();
        std::cout << "F" << std::endl;

        sameWeek();
        std::cout << "E" << std::endl;

        sameDay();
        std::cout << "D" << std::endl;

        sameRoom();
        std::cout << "C" << std::endl;

        sameTime();
        std::cout << "B" << std::endl;
        oneLectureRoomConflict();
        //roomUnavailable();
        //timePen();
        //roomPen();
        std::cout << "A" << std::endl;

        saveEncoding();
        try {
            if (cplex.solve()) {
                std::cout << "solved" << std::endl;
                //printdistanceToSolutionRooms(cplex);
                double value = cplex.getObjValue();
                std::cout << value << std::endl;

                return value;

            } else {
                std::cout << "false" << std::endl;
            }
        } catch (IloCplex::Exception e) {
            std::cerr << e.getMessage() << std::endl;
        }

    }

    void definedAuxVar() {
    }

    void definedRoomLecture() {
        try {
            int size = instance->getNumClasses();
            for (auto i = 0; i < instance->getRooms().size(); i++) {
                roomVar.add(IloBoolVarArray(env, size));
            }

        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }
    }


    void definedLectureTime() {
        try {
            startVar = IloIntVarArray(env, instance->getNumClasses());
            endVar = IloIntVarArray(env, instance->getNumClasses());
            tVar = IloIntVarArray(env, instance->getNumClasses());
            for (int j = 0; j < instance->getNumClasses(); ++j) {
                tVar[j] = IloIntVar(env, 0, instance->getClassesWeek(currentW)[j]->getLectures().size());
                startVar[j] = IloIntVar(env, 0, instance->getSlotsperday());
                endVar[j] = IloIntVar(env, 0, instance->getSlotsperday());
            }
            int size = instance->getNumClasses();
            for (auto i = 0; i < instance->getNdays(); i++) {
                dayVar.add(IloBoolVarArray(env, size));
            }
            for (auto i = 0; i < instance->getNweek(); i++) {
                weekVar.add(IloBoolVarArray(env, size));
            }
            for (auto i = 0; i < instance->getNumClasses(); i++) {
                sameDayVar.add(IloBoolVarArray(env, size));
                sameWeekVar.add(IloBoolVarArray(env, size));
                sameRoomVar.add(IloBoolVarArray(env, size));
                sameTimeVar.add(IloBoolVarArray(env, size));

            }

        } catch (IloCplex::Exception &e) {
            std::cout << e.getMessage() << std::endl;
        }

    }

    void dayConst() {
        for (int k = 0; k < instance->getClassesWeek(currentW).size(); ++k) {
            for (int i = 0; i < instance->getClassesWeek(currentW)[k]->getLectures().size(); ++i) {
                for (int j = 0; j < instance->getNdays(); j++) {
                    model.add((tVar[instance->getClassesWeek(currentW)[k]->getOrderID()] == i) <=
                              (dayVar[j][instance->getClassesWeek(currentW)[k]->getOrderID()] ==
                               (instance->getClassesWeek(currentW)[k]->getLectures()[i]->getDays().at(j) == '0' ? 0
                                                                                                                : 1)));
                }
                for (int l = 0; l < instance->getNweek(); ++l) {
                    model.add((tVar[instance->getClassesWeek(currentW)[k]->getOrderID()] == i) <=
                              (weekVar[l][instance->getClassesWeek(currentW)[k]->getOrderID()] ==
                               (instance->getClassesWeek(currentW)[k]->getLectures()[i]->getWeeks().at(l) == '0' ? 0
                                                                                                                 : 1)));
                }
                model.add((tVar[instance->getClassesWeek(currentW)[k]->getOrderID()] == i) <=
                          (startVar[instance->getClassesWeek(currentW)[k]->getOrderID()] ==
                           (instance->getClassesWeek(currentW)[k]->getLectures()[i]->getStart())));
                model.add((tVar[instance->getClassesWeek(currentW)[k]->getOrderID()] == i) <=
                          (endVar[instance->getClassesWeek(currentW)[k]->getOrderID()] ==
                           (instance->getClassesWeek(currentW)[k]->getLectures()[i]->getStart() +
                            instance->getClassesWeek(currentW)[k]->getLectures()[i]->getLenght())));


            }
        }
    }


    void sameWeek() {
        for (int j1N = 0; j1N < instance->getClassesWeek(currentW).size(); ++j1N) {
            Class *j = instance->getClassesWeek(currentW)[j1N];
            for (int j2N = 0; j2N < instance->getClassesWeek(currentW).size(); ++j2N) {
                Class *j1 = instance->getClassesWeek(currentW)[j2N];
                for (int k = 0; k < instance->getNweek(); ++k) {
                    model.add(sameWeekVar[j->getOrderID()][j1->getOrderID()] ==
                              (weekVar[k][j->getOrderID()] + weekVar[k][j1->getOrderID()] == 2));
                }
            }
        }

    }


    void sameDay() {
        for (Class *j : instance->getClassesWeek(currentW)) {
            for (Class *j1 : instance->getClassesWeek(currentW)) {
                for (int k = 0; k < instance->getNdays(); ++k) {
                    model.add(sameDayVar[j->getOrderID()][j1->getOrderID()] ==
                              (dayVar[k][j->getOrderID()] + dayVar[k][j1->getOrderID()] == 2));
                }
            }
        }

    }


    void sameRoom() {
        for (Class *j : instance->getClassesWeek(currentW)) {
            for (Class *j1 : instance->getClassesWeek(currentW)) {
                int i = 0, i1 = 0, i2 = 0;
                for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                     it != instance->getRooms().end(); it++) {
                    if (j->containsRoom(instance->getRoom(i + 1))) {
                        if (j1->containsRoom(instance->getRoom(i + 1))) {
                            model.add(sameRoomVar[j->getOrderID()][j1->getOrderID()] ==
                                      (roomVar[i1][j->getOrderID()] + roomVar[i2][j1->getOrderID()] == 2));
                            model.add(sameRoomVar[j1->getOrderID()][j->getOrderID()] ==
                                      (roomVar[i1][j->getOrderID()] + roomVar[i2][j1->getOrderID()] == 2));
                            i2++;
                        }
                        i1++;
                    }
                    i++;
                }
            }
        }

    }

    void sameTime() {
        for (Class *j : instance->getClassesWeek(currentW)) {
            for (Class *j1 : instance->getClassesWeek(currentW)) {
                model.add(sameTimeVar[j->getOrderID()][j1->getOrderID()] ==
                          ((endVar[j->getOrderID()] >= startVar[j1->getOrderID()] + 1) +
                           (endVar[j1->getOrderID()] >= startVar[j->getOrderID()] + 1) == 2));

                model.add(sameTimeVar[j1->getOrderID()][j->getOrderID()] ==
                          ((endVar[j->getOrderID()] >= startVar[j1->getOrderID()] + 1) +
                           (endVar[j1->getOrderID()] >= startVar[j->getOrderID()] + 1) == 2));

            }
        }

    }

    void oneLectureRoomConflict() {
        for (Class *j : instance->getClassesWeek(currentW)) {
            for (Class *j1 : instance->getClassesWeek(currentW)) {
                model.add(
                        sameRoomVar[j->getOrderID()][j1->getOrderID()] + sameDayVar[j->getOrderID()][j1->getOrderID()] +
                        sameWeekVar[j->getOrderID()][j1->getOrderID()] +
                        sameTimeVar[j->getOrderID()][j1->getOrderID()] <= 3);

            }
        }

    }


    /*virtual IloNumExpr roomPen()   {
        IloNumExpr pen =  IloNumExpr(env);
        for (int j = 0; j < instance->getClassesWeek(currentW).size(); j++) {
            int r = 0, rl = 0;
            for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                 it != instance->getRooms().end(); it++) {
                if (instance->getClassesWeek(currentW)[j]->containsRoom(instance->getRoom(r + 1))) {
                    pen += roomVar[rl][instance->getClassesWeek(currentW)[j]->getOrderID()] *
                           instance->getClassesWeek(currentW)[j]->getPen(instance->getRoom(r + 1));
                    rl++;
                }
                r++;

            }

        }
        return pen;

    }*/

    /**
     * roomUnavailable
     * @param week
     * @param day
     * @param lecture
     */
    virtual void roomUnavailable() {
        for (int c = 0; c < instance->getClasses().size(); ++c) {
            int rV = 0;
            for (std::pair<Room, int> r :instance->getClasses()[c]->getPossibleRooms()) {
                for (int s = 0; s < r.first.getSlots().size(); ++s) {
                    if (instance->getRoom(rV + 1).getSlots()[s].getWeeks().size() > 0 &&
                        instance->getRoom(rV + 1).getSlots()[s].getDays().size() > 0) {
                        for (int wV = 0; wV < instance->getNweek(); wV++) {
                            for (int dV = 0; dV < instance->getNdays(); dV++) {
                                model.add((weekVar[wV][instance->getClasses()[c]->getOrderID()] ==
                                           (instance->getRoom(rV + 1).getSlots()[s].getWeeks().at(wV) == '0' ? 0
                                                                                                             : 1)) +
                                          (dayVar[dV][instance->getClasses()[c]->getOrderID()] ==
                                           (instance->getRoom(rV + 1).getSlots()[s].getDays().at(
                                                   dV + instance->minDay()) == '0' ? 0 : 1)) +
                                          roomVar[rV][instance->getClasses()[c]->getOrderID()] +
                                          (r.first.getSlots()[s].getStart() + r.first.getSlots()[s].getLenght() >
                                           startVar[instance->getClasses()[c]->getOrderID()] &&
                                           r.first.getSlots()[s].getStart() <
                                           endVar[instance->getClasses()[c]->getOrderID()]) <= 3);


                            }

                        }
                    }

                }
                rV++;
            }
        }
    }

/*    virtual IloNumExpr travel(std::vector<Class *> c, int pen)   {
        IloNumExpr travelL = IloNumExpr(env);
        for (int cla = 0; cla < c.size(); cla++) {
            Class *l1 = c[cla];
                for (int cla1 = cla + 1; cla1 < c.size(); cla1++) {
                    Class *l2 = c[cla1];
                    if (l2->getOrderID() != l1->getOrderID()) {


                        model.add((endVar[l1->getOrderID()] + travel(l1, l2) <=
                                                     startVar[l2->getOrderID()])|| (
                                                     endVar[l2->getOrderID()] + travel(l2, l1) <=
                                                             startVar[l1->getOrderID()]) && sameDayVar[l1->getOrderID()][l2->getOrderID()]&& sameWeekVar[l1->getOrderID()][l2->getOrderID()]);

                    }

                }


        }
        return travelL;
    }

    virtual IloNumExpr travel(Class *c1, Class *c2) {
        IloNumExpr temp = IloNumExpr(env);
        int i=0;
        for (std::pair<Room,int> r1 :c1->getPossibleRooms().size()) {
            int y=0;
            for (std::pair<Room,int> r2 :c1->getPossibleRooms().size()) {
                if(r1.first.getId()==r2.first.getId())
                    temp+= (vector[c1->getOrderID()][i] + vector[c2->getOrderID()][y]==2);
                else
                temp += (vector[c1->getOrderID()][i] + vector[c2->getOrderID()][y]==2) * c2->getPossibleRoom(y).
                        getTravel(c1->getPossibleRoom(i).getId());
            }
        }
        return temp;
    }

    virtual IloNumExpr differentRoom(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b) {
        IloNumExpr result = IloNumExpr(env);
        for (int c1 = 0; c1 < vector.size(); c1++) {
            for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                int i = 0, i1 = 0, i2 = 0;
                for (std::map<int, Room>::const_iterator it = instance->getRooms().begin();
                     it != instance->getRooms().end(); it++) {
                    if (vector[c1]->containsRoom(instance->getRoom(i + 1))) {
                        if (vector[c2]->containsRoom(instance->getRoom(i + 1))) {
                            if (penalty == -1) {
                                if (b)
                                    model.add(sameRoomVar[vector[c1]->getOrderID()][vector[c2]->getOrderID()]==1);
                                else
                                    model.add(sameRoomVar[vector[c1]->getOrderID()][vector[c2]->getOrderID()]==0);
                            } else {
                                result += (b ? (sameRoomVar[vector[c1]->getOrderID()][vector[c2]->getOrderID()]==0) :
                                           (sameRoomVar[vector[c1]->getOrderID()][vector[c2]->getOrderID()]==1));
                            }

                            i2++;
                        }
                        i1++;
                    }
                    i++;

                }
            }
        }
        return result;
    }*/


    virtual void block() {

    }

    virtual void week() {
    }




    /** SameTime
    * Given classes must be taught at the same time of day, regardless of their days of week or weeks. For the classes
    * of the same length, this is the same constraint as SameStart (classes must start at the same time slot).
    * For the classes of different lengths, the shorter class can start after the longer class but must end before or
    * at the same time as the longer class. This means that
    * (Ci.start ≤ Cj.start ∧ Cj.end ≤ Ci.end) ∨ (Cj.start ≤ Ci.start ∧ Ci.end ≤ Cj.end)
    * for any two classes Ci and Cj from the constraint; Ci.end = Ci.start + Ci.length is the assigned end time slot
    * of a class Ci.
    * @param vector
    * @param penalty
    * @param b
    * @return

    virtual IloNumExpr
    sameTime(const std::vector<Class *, std::allocator<Class *>> &vector, int penalty, bool b)   {
        IloNumExpr result = IloNumExpr(env);
        for (int c1 = 0; c1 < vector.size(); ++c1) {
            for (int c2 = c1 + 1; c2 < vector.size(); ++c2) {
                if (b) {

                } else {


                }
            }
        }
        return result;
    }*/






};


#endif //PROJECT_ILPEXECUTER_H