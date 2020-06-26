

#ifndef __XMLTWO_PARSER__
#define __XMLTWO_PARSER__

#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include <list>
#include <string>
#include "problem/Lecture.h"
#include "problem/Room.h"
#include "problem/ClusterStudents.h"

#include "rapidXMLParser/rapidxml.hpp"
#include "rapidXMLParser/rapidxml_print.hpp"
#include "problem/ClusterbyRoom.h"

#include "problem/Instance.h"
#include "Perturbation.h"
#include "utils/Stats.h"
#include "utils/TimeUtil.h"
#include "problem/Time.h"
#include "problem/Curriculum.h"
#include "problem/ConstraintShort.h"
#include "utils/StringUtil.h"
#include "utils/HardwareStats.h"
#include "ParserXML.h"

using namespace rapidxml;


namespace openwbo {

    class Conflict {
        int lecID1;
        int lecID2;
        int roomID1;
        int roomID2;
    public:
        Conflict(int lecID1, int lecID2, int roomID1, int roomID2) : lecID1(lecID1), lecID2(lecID2), roomID1(roomID1),
                                                                     roomID2(roomID2) {}

        int getLecID1() const {
            return lecID1;
        }

        void setLecID1(int lecID1) {
            Conflict::lecID1 = lecID1;
        }

        int getLecID2() const {
            return lecID2;
        }

        void setLecID2(int lecID2) {
            Conflict::lecID2 = lecID2;
        }

        int getRoomID1() const {
            return roomID1;
        }

        void setRoomID1(int roomID1) {
            Conflict::roomID1 = roomID1;
        }

        int getRoomID2() const {
            return roomID2;
        }

        void setRoomID2(int roomID2) {
            Conflict::roomID2 = roomID2;
        }
    };

    class State {
    public:
        int cClass;
        int time=0;
        int start;
        int end;
        State(int cClass, int start, int end) : cClass(cClass), start(start), end(end) {}
        void update(int ctime,int start, int end){ time=ctime; this->start=start; this->end=end;}
    };

    class ParserXMLTwo : public ParserXML {


    public:
        //-------------------------------------------------------------------------
        // Constructor/destructor.
        //-------------------------------------------------------------------------

        ParserXMLTwo(MaxSATFormula *maxsat_formula, bool optAlloction, bool optStud, bool optSoft) :
                ParserXML(maxsat_formula, optAlloction, optStud, optSoft) {

        }

        virtual ~ParserXMLTwo() {}

        void parse(std::string fileName) {

            PBObjFunction *of;
            if (optAlloction)
                of = new PBObjFunction();


            xml_document<> doc;
            int orderID = 0;
            std::map<int, Class *> classMap;
            std::map<Class *, ClusterbyRoom *> mapCluster;
            std::map<int, std::set<ClusterbyRoom *> *> curMap;

            std::map<int, int> classID;


            std::ifstream file(fileName);
            if (file.fail()) {
                std::cerr << "File not found: " << fileName << std::endl;
                std::cerr << "Method: readInputXML" << std::endl;
                std::exit(11);
            }
            std::stringstream buffer;
            buffer << file.rdbuf();
            file.close();
            std::string content(buffer.str());
            doc.parse<0>(&content[0]);
            xml_node<> *pRoot = doc.first_node();
            readDescription(pRoot);
            for (const xml_node<> *n = pRoot->first_node(); n; n = n->next_sibling()) {
                if (strcmp(n->name(), "optimization") == 0) {
                    optimizationCost(n);
                } else if (strcmp(n->name(), "rooms") == 0) {
                    readRoom(n);
                } else if (strcmp(n->name(), "courses") == 0) {
                    std::vector<Class *> all;
                    for (const xml_node<> *s = n->first_node(); s; s = s->next_sibling()) {
                        char *id;
                        std::map<int, std::vector<Subpart *>> config;
                        for (const xml_attribute<> *a = s->first_attribute(); a; a = a->next_attribute()) {
                            id = a->value();
                        }
                        for (const xml_node<> *rs = s->first_node(); rs; rs = rs->next_sibling()) {
                            int idConf = -1;

                            std::vector<Subpart *> subpartvec;

                            for (const xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                                idConf = atoi(a->value());
                            }
                            for (const xml_node<> *sub = rs->first_node(); sub; sub = sub->next_sibling()) {
                                std::string idsub;
                                std::vector<Class *> clasv;
                                for (const xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                                    idsub = a->value();
                                }
                                for (const xml_node<> *cla = sub->first_node(); cla; cla = cla->next_sibling()) {

                                    int idclass = -1, limit = -1, parent = -1;
                                    std::map<Room *, int> roomsv;
                                    std::vector<Lecture *> lecv;
                                    for (const xml_attribute<> *a = cla->first_attribute(); a; a = a->next_attribute()) {
                                        if (strcmp(a->name(), "id") == 0)
                                            idclass = atoi(a->value());
                                        else if (strcmp(a->name(), "limit") == 0)
                                            limit = atoi(a->value());
                                        else if (strcmp(a->name(), "parent") == 0)
                                            parent = atoi(a->value());
                                    }
                                    std::map<int,std::vector<Lit>> hour;
                                    std::map<std::string,std::vector<Lit>> day;
                                    std::map<std::string,std::vector<Lit>> week;
                                    Class *c = new Class(idclass, limit, orderID,
                                                         itos(idConf) + "_" + id);
                                    c->setCourseID(atoi(id));
                                    all.push_back(c);
                                    std::set<std::pair<int,int>> hours;
                                    std::set<std::string> daysSet;
                                    std::set<std::string> weeksSet;

                                    PB *room = new PB();
                                    PB *time = new PB();

                                    int rIDO = 0;
                                    int max = 0;
                                    for (const xml_node<> *lec = cla->first_node(); lec; lec = lec->next_sibling()) {
                                        if (strcmp(lec->name(), "room") == 0) {
                                            std::string idRoom = " ";
                                            int penalty = -1;
                                            int rID = -1;
                                            for (const xml_attribute<> *a = lec->first_attribute(); a; a = a->next_attribute()) {
                                                if (strcmp(a->name(), "id") == 0) {
                                                    idRoom = a->value();
                                                    rID = atoi(a->value());
                                                } else if (strcmp(a->name(), "penalty") == 0)
                                                    penalty = atoi(a->value());

                                            }
                                            if (instance->getRooms().find(std::stoi(idRoom)) ==
                                                instance->getRooms().end()) {
                                                std::cerr << "Room does not exist: " << idRoom << " " << id << " "
                                                          << idclass
                                                          << std::endl;
                                                std::exit(11);
                                            } else {

                                                room->addProduct(mkLit(getVariableID(
                                                        "r_" + std::to_string(orderID) + "_" + idRoom)),
                                                                 1);
                                                if (penalty != 0 && optAlloction)
                                                    of->addProduct(mkLit(getVariableID(
                                                            "r_" + std::to_string(orderID) + "_" +
                                                            idRoom)),
                                                                   instance->getRoomPen() * penalty);
                                                instance->getRoom(std::stoi(idRoom))->c.push_back(orderID);


                                                roomsv.insert(
                                                        std::pair<Room *, int>(instance->getRoom(std::stoi(idRoom)),
                                                                               rIDO));
                                                rIDO++;

                                            }
                                        } else if (strcmp(lec->name(), "time") == 0) {
                                            int lenght = -1, start = -1, penalty = -1;
                                            std::string weeks, days;
                                            for (const xml_attribute<> *a = lec->first_attribute(); a; a = a->next_attribute()) {

                                                if (strcmp(a->name(), "length") == 0) {
                                                    lenght = atoi(a->value());
                                                } else if (strcmp(a->name(), "start") == 0) {
                                                    start = atoi(a->value());
                                                } else if (strcmp(a->name(), "weeks") == 0) {
                                                    weeks = a->value();
                                                } else if (strcmp(a->name(), "days") == 0) {
                                                    days = a->value();
                                                } else if (strcmp(a->name(), "penalty") == 0) {
                                                    penalty = atoi(a->value());
                                                }


                                            }
                                            vec <Lit> *l1 = new vec<Lit>;
                                            l1->push(mkLit(getVariableID(
                                                    "h_" + std::to_string(orderID) + "_" + std::to_string(start))));
                                            l1->push(
                                                    ~mkLit(getVariableID(
                                                            "t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                            maxsat_formula->addHardClause(*l1);
                                            delete l1;

                                            if(hour.find(start)!=hour.end()){
                                                hour[start].push_back(mkLit(getVariableID("t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                            } else {
                                                std::vector<Lit> temp;
                                                temp.push_back(~mkLit(getVariableID(
                                                        "h_" + std::to_string(orderID) + "_" + std::to_string(start))));
                                                temp.push_back(mkLit(getVariableID("t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                                hour.insert(std::pair<int,std::vector<Lit>>(start,temp));
                                            }
                                            l1 = new vec<Lit>;
                                            l1->push(mkLit(getVariableID(
                                                    "w_" + std::to_string(orderID) + "_" + weeks)));
                                            l1->push(
                                                    ~mkLit(getVariableID(
                                                            "t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                            maxsat_formula->addHardClause(*l1);
                                            delete l1;

                                            if(week.find(weeks)!=week.end()){
                                                week[weeks].push_back(mkLit(getVariableID("t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                            } else {
                                                std::vector<Lit> temp;
                                                temp.push_back(~mkLit(getVariableID(
                                                        "w_" + std::to_string(orderID) + "_" + weeks)));
                                                temp.push_back(mkLit(getVariableID("t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                                week.insert(std::pair<std::string,std::vector<Lit>>(weeks,temp));
                                            }

                                            l1 = new vec<Lit>;
                                            l1->push(mkLit(getVariableID(
                                                    "d_" + std::to_string(orderID) + "_" + days)));
                                            l1->push(
                                                    ~mkLit(getVariableID(
                                                            "t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                            maxsat_formula->addHardClause(*l1);
                                            delete l1;

                                            if(day.find(days)!=day.end()){
                                                day[days].push_back(mkLit(getVariableID("t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                            } else {
                                                std::vector<Lit> temp;
                                                temp.push_back(~mkLit(getVariableID(
                                                        "d_" + std::to_string(orderID) + "_" + days)));
                                                temp.push_back(mkLit(getVariableID("t_" + std::to_string(orderID) + "_" + std::to_string(max))));
                                                day.insert(std::pair<std::string,std::vector<Lit>>(days,temp));
                                            }

                                            hours.insert(std::pair<int,int>(start,start+lenght));
                                            daysSet.insert(days);
                                            weeksSet.insert(weeks);


                                            Lecture *l = new Lecture(lenght, start, weeks, days, penalty);
                                            lecv.push_back(l);
                                            l->setOrderID(max);



                                            time->addProduct(mkLit(getVariableID(
                                                    "t_" + std::to_string(orderID) + "_" + std::to_string(max))), 1);





                                            if (penalty != 0 && optAlloction)
                                                of->addProduct(mkLit(getVariableID(
                                                        "t_" + std::to_string(orderID) + "_" + std::to_string(max))),
                                                               instance->getTimePen() * penalty);


                                            if (roomsv.size() > 0) {
                                                for (std::map<Room *, int>::iterator j = roomsv.begin();
                                                     j != roomsv.end(); ++j) {

                                                    std::string week1 = l->getWeeks();
                                                    std::string day1 = l->getDays();
                                                    int startTime = l->getStart();
                                                    int duration = l->getLenght();
                                                    bool isNotAddableTime = false;
                                                    for (int una = 0; una < j->first->getSlots().size(); ++una) {
                                                        for (int weeki = 0; weeki < instance->getNweek(); ++weeki) {
                                                            if (week1[weeki] ==
                                                                j->first->getSlots()[una].getWeeks()[weeki] &&
                                                                j->first->getSlots()[una].getWeeks()[weeki] == '1') {
                                                                for (int d = 0; d < instance->getNdays(); ++d) {
                                                                    if (day1[d] ==
                                                                        j->first->getSlots()[una].getDays()[d] &&
                                                                        day1[d] == '1') {

                                                                        if (startTime >=
                                                                            j->first->getSlots()[una].getStart() &&
                                                                            startTime <
                                                                            j->first->getSlots()[una].getStart() +
                                                                            j->first->getSlots()[una].getLenght()) {

                                                                            isNotAddableTime = true;
                                                                        } else if (
                                                                                j->first->getSlots()[una].getStart() >=
                                                                                startTime &&
                                                                                j->first->getSlots()[una].getStart() <
                                                                                startTime + duration) {


                                                                            isNotAddableTime = true;

                                                                        }
                                                                    }
                                                                    if (isNotAddableTime)
                                                                        break;

                                                                }
                                                            }
                                                            if (isNotAddableTime)
                                                                break;
                                                        }
                                                        if (isNotAddableTime)
                                                            break;

                                                    }
                                                    vec <Lit> *l1 = new vec<Lit>;
                                                    l1->push(~mkLit(getVariableID(
                                                            "y_" + std::to_string(orderID) + "_" +
                                                            std::to_string(max) + "_" +
                                                            std::to_string(j->first->getId()))));
                                                    l1->push(mkLit(getVariableID(
                                                            "r_" + std::to_string(orderID) + "_" +
                                                            std::to_string(j->first->getId()))));
                                                    maxsat_formula->addHardClause(*l1);
                                                    delete l1;
                                                    l1 = new vec<Lit>;
                                                    l1->push(~mkLit(getVariableID(
                                                            "y_" + std::to_string(orderID) + "_" +
                                                            std::to_string(max) + "_" +
                                                            std::to_string(j->first->getId()))));
                                                    l1->push(mkLit(getVariableID(
                                                            "t_" + std::to_string(orderID) + "_" +
                                                            std::to_string(max))));
                                                    maxsat_formula->addHardClause(*l1);
                                                    delete l1;
                                                    l1 = new vec<Lit>;
                                                    l1->push(mkLit(getVariableID(
                                                            "y_" + std::to_string(orderID) + "_" +
                                                            std::to_string(max) + "_" +
                                                            std::to_string(j->first->getId()))));
                                                    l1->push(~mkLit(getVariableID(
                                                            "t_" + std::to_string(orderID) + "_" +
                                                            std::to_string(max))));
                                                    l1->push(~mkLit(getVariableID(
                                                            "r_" + std::to_string(orderID) + "_" +
                                                            std::to_string(j->first->getId()))));
                                                    maxsat_formula->addHardClause(*l1);

                                                    delete l1;

                                                    if (isNotAddableTime) {
                                                        vec <Lit> *l1 = new vec<Lit>;
                                                        l1->push(~mkLit(getVariableID(
                                                                "y_" + std::to_string(orderID) + "_" +
                                                                std::to_string(max) + "_" +
                                                                std::to_string(j->first->getId()))));


                                                        maxsat_formula->addHardClause(*l1);
                                                        delete l1;
                                                    }
                                                }
                                            }


                                            max++;


                                        }


                                    }

                                    c->setPossibleRooms(roomsv);
                                    c->setLectures(lecv);
                                    c->setHours(hours);
                                    c->setWeeks(weeksSet);
                                    c->setDays(daysSet);
                                    c->setHour(hour);
                                    c->setWeek(week);
                                    c->setDay(day);


                                    classMap.insert(std::pair<int, Class *>(orderID, c));
                                    classID.insert(std::pair<int, int>(idclass, orderID));
                                    orderID++;
                                    if (parent != -1)
                                        c->setParent(classMap[classID[parent]]);
                                    clasv.push_back(c);
                                    time->addRHS(1);


                                    PB *time2 = new PB(time->_lits, time->_coeffs, time->_rhs, true);
                                    if (roomsv.size() != 0) {
                                        room->addRHS(1);
                                        PB *room2 = new PB(room->_lits, room->_coeffs, room->_rhs, true);
                                        maxsat_formula->addPBConstraint(room);
                                        maxsat_formula->addPBConstraint(room2);
                                        delete room2;
                                    }


                                    maxsat_formula->addPBConstraint(time);
                                    maxsat_formula->addPBConstraint(time2);
                                    delete time;
                                    delete time2;
                                    delete room;


                                }
                                Subpart *subpart = new Subpart(idsub, clasv);
                                subpartvec.push_back(subpart);

                            }
                            config.insert(std::pair<int, std::vector<Subpart *>>(idConf, subpartvec));

                        }
                        Course *course = new Course(id, config);
                        instance->addCourse(course);

                    }
                    instance->setClasses(all);
                   


                } else if (strcmp(n->name(), "distributions") == 0) {
                    readConstraints(mapCluster, curMap, n);
                } else if (strcmp(n->name(), "students") == 0) {
                    readStudents(n);
                }


            }
            file.close();
            if (optAlloction) {
                maxsat_formula->addObjFunction(of);
                delete of;
            }


        }


        void createCurriculum(std::map<int, std::set<ClusterbyRoom *> *> &curMap) const {
            std::vector<Curriculum *> problem;
            //printRoomCluster();
            int count = 1;
            for (auto it = curMap.begin(); it != curMap.end(); ++it) {
                if (problem.size() == 0)
                    problem.push_back(new Curriculum(*it->second, problem.size()));
                else {
                    bool test = false;
                    for (int i = 0; i < problem.size(); ++i) {
                        for (ClusterbyRoom *c:problem[i]->getPClass()) {
                            for (ClusterbyRoom *c1: *it->second) {
                                for (Class *aClass: c->getClasses()) {
                                    for (Class *aClass1: c1->getClasses()) {
                                        if (aClass->getId() == aClass1->getId()) {
                                            test = true;
                                        } else if (test && aClass->getId() != aClass1->getId()) {
                                            c->addClass(aClass1);
                                        }
                                    }
                                    if (test) {
                                        for (Room *aRoom:c->getRooms()) {
                                            for (Room *aRoom1: c1->getRooms()) {
                                                if (aRoom->getId() != aRoom1->getId()) {
                                                    c->addRoom(aRoom1);

                                                }

                                            }
                                        }
                                        break;
                                    }

                                }

                            }
                            if (!test) {
                                for (ClusterbyRoom *c1: *it->second) {
                                    for (Room *aRoom:c->getRooms()) {
                                        for (Room *aRoom1: c1->getRooms()) {
                                            if (aRoom->getId() == aRoom1->getId()) {
                                                for (Class *aClass1: c1->getClasses()) {
                                                    c->addClass(aClass1);
                                                }
                                                for (Room *aRoom2:c1->getRooms()) {
                                                    if (aRoom2->getId() != aRoom->getId()) {
                                                        c->addRoom(aRoom2);

                                                    }

                                                }


                                                test = true;
                                                break;
                                            }
                                        }
                                        if (test)
                                            break;

                                    }
                                }
                            } else
                                break;
                        }
                        if (test)
                            break;


                    }


                    if (!test)
                        problem.push_back(new Curriculum(*it->second, problem.size()));
                }
                count++;

            }
            if (count < instance->getClasses().size()) {
                for (ClusterbyRoom *c1: cluster) {
                    bool t = false;
                    for (Class *aClass1: c1->getClasses()) {
                        if (curMap.find(aClass1->getId()) != curMap.end()) {
                            t = true;
                            break;
                        }
                    }
                    if (t)
                        continue;
                    t = false;
                    for (int i = 0; i < problem.size(); ++i) {
                        for (ClusterbyRoom *c:problem[i]->getPClass()) {
                            for (Room *aRoom:c->getRooms()) {
                                for (Room *aRoom1: c1->getRooms()) {
                                    if (aRoom->getId() == aRoom1->getId()) {
                                        for (Class *aClass1: c1->getClasses()) {
                                            c->addClass(aClass1);
                                        }
                                        for (Room *aRoom2:c1->getRooms()) {
                                            if (aRoom2->getId() != aRoom->getId()) {
                                                c->addRoom(aRoom2);

                                            }

                                        }
                                        t = true;
                                        break;
                                    }
                                }
                                if (t)
                                    break;

                            }
                            if (t)
                                break;
                        }
                        if (t)
                            break;
                    }
                    if (!t)
                        problem.push_back(new Curriculum(c1, problem.size()));


                }
            }
            instance->setProblem(problem);
        }

        void readStudents(const xml_node<> *n) {
            std::map<int, Student> std;
            int idc = 0;
            bool stuSame = false;
            std::vector<ClusterStudent *> clusterStudent;
            for (const xml_node<> *sub = n->first_node(); sub; sub = sub->next_sibling()) {
                int studentID = -1;
                for (const xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                    studentID = atoi(a->value());
                }

                std::vector<Course *> c;
                int minimo = INT_MAX;
                for (const xml_node<> *course = sub->first_node(); course; course = course->next_sibling()) {
                    std::string courseIDstd = "";
                    for (const xml_attribute<> *a = course->first_attribute(); a; a = a->next_attribute()) {
                        courseIDstd = a->value();
                    }
                    Course *course1 = instance->getCourse(courseIDstd);
                    course1->setNumbofstudents(course1->getNumbofstudents());
                    if (course1->getMinLimit() < minimo)
                        minimo = course1->getMinLimit();
                    if (stuSame) {
                        for (Course *course2:c) {
                            if (pairCourse.find(course2->getName() + courseIDstd) == pairCourse.end() &&
                                pairCourse.find(courseIDstd + course2->getName()) == pairCourse.end()) {
                                pairCourse.insert(std::pair<std::string, std::pair<Course *, Course *>>(
                                        course2->getName() + courseIDstd,
                                        std::pair<Course *, Course *>(course2, course1)));
                            }
                        }
                    }
                    stuSame = true;
                    c.push_back(course1);
                }
                std::pair<int, Student> s = std::pair<int, Student>(studentID, Student(studentID, c));
                std.insert(s);


                bool temp = false;
                if (clusterStudent.size() == 0)
                    clusterStudent.push_back(new ClusterStudent(idc++, c, s.second, minimo));
                else {
                    for (int i = 0; i < clusterStudent.size(); ++i) {
                        bool t = false;


                        if (c.size() != clusterStudent[i]->getCourses().size())
                            continue;
                        for (int c0 = 0; c0 < c.size(); ++c0) {
                            t = false;
                            for (int c1 = 0; c1 < clusterStudent[i]->getCourses().size(); ++c1) {
                                if (clusterStudent[i]->getCourses()[c1]->getName().compare(
                                        c[c0]->getName()) == 0) {
                                    t = true;
                                    break;
                                }
                            }
                            if (!t)
                                break;

                        }
                        if (t) {
                            temp = true;
                            clusterStudent[i]->addStudent(s.second);
                            if (clusterStudent[i]->getMin() > minimo)
                                clusterStudent[i]->setMin(minimo);
                        }
                    }
                    if (!temp)
                        clusterStudent.push_back(new ClusterStudent(idc++, c, s.second, minimo));


                }
            }
            std::vector < ClusterStudent * > clusterStudent1;
            for (ClusterStudent* cs:clusterStudent) {
                int li=cs->getStudent().size();
                bool change=false;
                for (Course* c: cs->getCourses()) {
                    if(li>MDC(c->getMinLimit(),c->getNumbofstudents())){
                        li=MDC(c->getMinLimit(),c->getNumbofstudents());
                        change=true;
                    }

                }
                if(change){
                    int i=0;
                    ClusterStudent *csn= nullptr;
                    for (Student s: cs->getStudent()) {
                        if(i<li && i!=0){
                            csn->addStudent(s);
                            i++;
                        } else{
                            if(csn!= nullptr) {
                                assert(csn->getCourses().size()==cs->getCourses().size());
                                clusterStudent1.push_back(csn);
                            }
                            csn=new ClusterStudent(clusterStudent1.size(),cs->getCourses(),s,li);
                        }
                        
                    }
                    if(csn!= nullptr) {
                        clusterStudent1.push_back(csn);
                    }
                } else {
                    clusterStudent1.push_back(cs);
                }

            }


            instance->setStudentCluster(clusterStudent);
            instance->setStudent(std);
        }

        void readConstraints(std::map<Class *, ClusterbyRoom *> mapCluster,
                             std::map<int, std::set<ClusterbyRoom *> *> &curMap, const xml_node<> *n) const {
            std::map<std::string, std::vector<ConstraintShort *>> hard;

            for (const xml_node<> *sub = n->first_node(); sub; sub = sub->next_sibling()) {
                std::stringstream constSTR;
                bool isReq = false;
                std::string type;
                int penalty = -1;
                std::vector<Class *> c1;
                std::set<ClusterbyRoom *> *c = new std::set<ClusterbyRoom *>();
                int limit = -1, limit1 = -1;
                for (const xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                    if (strcmp(a->name(), "required") == 0) {
                        isReq = (strcmp(a->value(), "true") == 0);
                    } else if (strcmp(a->name(), "penalty") == 0) {
                        penalty = atoi(a->value());
                    } else if (strcmp(a->name(), "type") == 0) {
                        type = a->value();
                        long size;
                        std::string rest;
                        if ((size = type.find("(")) != std::string::npos) {
                            rest = type.substr(size + 1, std::string::npos);
                            type = type.substr(0, size);
                            if ((size = rest.find(",")) != std::string::npos) {
                                limit1 = atoi(rest.substr(size + 1, std::string::npos - 1).c_str());
                            }
                            limit = atoi(rest.substr(0, size).c_str());
                        }
                    }

                }
                Class *idClassesDist;
                for (const xml_node<> *course = sub->first_node(); course; course = course->next_sibling()) {
                    idClassesDist = instance->getClass(atoi(course->first_attribute()->value()));
                    c->insert(mapCluster[idClassesDist]);
                    c1.push_back(idClassesDist);
                    if (curMap.find(idClassesDist->getId()) != curMap.end()) {
                        for (ClusterbyRoom *r: *c)
                            curMap[idClassesDist->getId()]->insert(r);
                        c = curMap[idClassesDist->getId()];
                    } else {
                        curMap.insert(std::pair<int,
                                std::set<ClusterbyRoom *> *>(
                                idClassesDist->getId(), c));
                    }
                }



                if (hard.find(type) != hard.end()) {
                    hard[type].push_back(new ConstraintShort(type, penalty, c1, limit, limit1));
                } else {
                    std::vector<ConstraintShort *> t;
                    t.push_back(new ConstraintShort(type, penalty, c1, limit, limit1));
                    hard.insert(std::pair<std::string, std::vector<ConstraintShort *>>(type, t));
                }


            }
            instance->setDist(hard);
        }

        void readRoom(const xml_node<> *n) const {
            for (const xml_node<> *s = n->first_node(); s; s = s->next_sibling()) {
                std::string id = " ", type = " ";
                int capacity = -1;
                for (const xml_attribute<> *a = s->first_attribute(); a; a = a->next_attribute()) {
                    if (strcmp(a->name(), "capacity") == 0) {
                        capacity = atoi(a->value());
                    } else if (strcmp(a->name(), "id") == 0) {
                        id = a->value();
                    } else if (strcmp(a->name(), "type") == 0) {
                        type = a->value();
                    }
                }
                std::map<int, int> travel;
                std::vector<Unavailability> una;
                for (const xml_node<> *rs = s->first_node(); rs; rs = rs->next_sibling()) {
                    if (strcmp(rs->name(), "travel") == 0) {
                        int value = -1, room = -1;
                        for (const xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                            if (strcmp(a->name(), "room") == 0) {
                                room = atoi(a->value());
                            } else if (strcmp(a->name(), "value") == 0) {
                                value = atoi(a->value());
                            }
                        }
                        travel.insert(std::pair<int, int>(room, value));


                    } else if (strcmp(rs->name(), "unavailable") == 0) {
                        char *days, *weeks;
                        int length = -1, start = -1;
                        for (const xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                            if (strcmp(a->name(), "days") == 0) {
                                days = a->value();
                            } else if (strcmp(a->name(), "start") == 0) {
                                start = atoi(a->value());
                            } else if (strcmp(a->name(), "length") == 0) {
                                length = atoi(a->value());
                            } else if (strcmp(a->name(), "weeks") == 0) {
                                weeks = a->value();
                            }
                        }
                        una.push_back(Unavailability(days, start, length, weeks));
                    }


                }

                instance->addRoom(new Room(stoi(id), id, capacity, travel,
                                           una, type));
            }
        }

        void optimizationCost(const xml_node<> *n) const {
            for (const xml_attribute<> *a = n->first_attribute(); a; a = a->next_attribute()) {
                if (strcmp(a->name(), "time") == 0) {
                    instance->setTimePen(atoi(a->value()));
                } else if (strcmp(a->name(), "room") == 0) {
                    instance->setRoomPen(atoi(a->value()));
                } else if (strcmp(a->name(), "distribution") == 0) {
                    instance->setDistributionPen(atoi(a->value()));
                } else if (strcmp(a->name(), "student") == 0) {
                    instance->setStudentPen(atoi(a->value()));
                }
            }
        }

        void readDescription(const xml_node<> *pRoot) {
            for (const xml_attribute<> *a = pRoot->first_attribute(); a; a = a->next_attribute()) {
                if (strcmp(a->name(), "name") == 0)
                    instance = new Instance(a->value());
                else if (strcmp(a->name(), "nrDays") == 0)
                    instance->setNdays(atoi(a->value()));
                else if (strcmp(a->name(), "slotsPerDay") == 0)
                    instance->setSlotsperday(atoi(a->value()));
                else if (strcmp(a->name(), "nrWeeks") == 0)
                    instance->setNweek(atoi(a->value()));
            }
        }

        void printRoomCluster() const {
            for (ClusterbyRoom *c1: cluster) {
                std::cout << "New Room" << c1->getClusterID() << std::endl;
                for (Class *aClass1: c1->getClasses()) {
                    std::cout << aClass1->getId() << std::endl;
                }
                std::cout << "Room" << std::endl;
                for (Room *aRoom: c1->getRooms()) {
                    std::cout << "R " << aRoom->getId() << std::endl;
                }
            }

        }

        Instance *getInstance() const {
            return instance;
        }

        /*Same day time and week aux variables*/
        void aux() {
            for (Class *c : instance->getClasses()) {
                for(std::pair<int,std::vector<Lit>> t: c->getHour()){
                    vec<Lit> *lit = new vec<Lit>;
                    for(Lit l : t.second)
                        lit->push(l);
                    maxsat_formula->addHardClause(*lit);
                    delete lit;
                }
                for(std::pair<std::string,std::vector<Lit>> t: c->getWeek()){
                    vec<Lit> *lit = new vec<Lit>;
                    for(Lit l : t.second)
                        lit->push(l);
                    maxsat_formula->addHardClause(*lit);
                    delete lit;
                }
                for(std::pair<std::string,std::vector<Lit>> t: c->getDay()){
                    vec<Lit> *lit = new vec<Lit>;
                    for(Lit l : t.second)
                        lit->push(l);
                    maxsat_formula->addHardClause(*lit);
                    delete lit;
                }

            }

        }
        
        /*Room conflicts*/

        void room() {
            for (std::pair<int, Room *> pairr: instance->getRooms()) {
                Room *r = pairr.second;
                for (int classes = 0; classes < r->c.size(); ++classes) {
                    for (int classes1 = classes+1; classes1 < r->c.size(); ++classes1) {
                        Class * idClassesDist = instance->getClasses()[r->c[classes]];
                        Class *idClassesDist1 = instance->getClasses()[r->c[classes1]];
                        vec <Lit> *l1 = new vec<Lit>();
                        vec <Lit> *l2 = new vec<Lit>();
                        if(idClassesDist1->getId()>idClassesDist->getId()) {
                            l1->push(~mkLit(getVariableID(
                                    "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                            l2->push(~mkLit(getVariableID(
                                    "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                        } else {
                            l1->push(~mkLit(getVariableID(
                                    "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                            l2->push(~mkLit(getVariableID(
                                    "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                        }
                        for (std::string day : idClassesDist->getDays()) {
                            for (std::string day1 : idClassesDist1->getDays()) {
                                if(!stringcompare(day, day1, instance->getNdays(), false)) {


                                    l1->push(mkLit(getVariableID(
                                            "d_" + std::to_string(idClassesDist->getOrderID()) +
                                            "_" + day)));
                                    l2->push(mkLit(getVariableID(
                                            "d_" + std::to_string(idClassesDist1->getOrderID()) +
                                            "_" + day1)));
                                    vec <Lit> *l = new vec<Lit>();

                                    if(idClassesDist1->getId()>idClassesDist->getId())
                                        l->push(mkLit(getVariableID(
                                                "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                                "_" + std::to_string(idClassesDist1->getOrderID()))));
                                    else
                                        l->push(mkLit(getVariableID(
                                                "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                                "_" + std::to_string(idClassesDist->getOrderID()))));
                                    l->push(~mkLit(getVariableID(
                                            "d_" + std::to_string(idClassesDist1->getOrderID()) +
                                            "_" + day1)));
                                    l->push(~mkLit(getVariableID(
                                            "d_" + std::to_string(idClassesDist->getOrderID()) +
                                            "_" + day)));
                                    maxsat_formula->addHardClause(*l);
                                    delete l;

                                }

                            }

                        }
                        if(l1->size()>0)
                            maxsat_formula->addHardClause(*l1);
                        delete l1;
                        if(l2->size()>0)
                            maxsat_formula->addHardClause(*l2);
                        delete l2;
                        l1 = new vec<Lit>();
                        l2 = new vec<Lit>();
                        if(idClassesDist1->getId()>idClassesDist->getId()) {
                            l1->push(~mkLit(getVariableID(
                                    "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                            l2->push(~mkLit(getVariableID(
                                    "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                        } else {
                            l1->push(~mkLit(getVariableID(
                                    "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                            l2->push(~mkLit(getVariableID(
                                    "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                        }
                        for (std::string week : idClassesDist->getWeeks()) {
                            for (std::string week1 : idClassesDist1->getWeeks()) {
                                if(!stringcompare(week, week1, instance->getNweek(), false)) {
                                    l1->push(mkLit(getVariableID(
                                            "w_" + std::to_string(idClassesDist->getOrderID()) +
                                            "_" + week)));
                                    l2->push(mkLit(getVariableID(
                                            "w_" + std::to_string(idClassesDist1->getOrderID()) +
                                            "_" + week1)));

                                    vec <Lit> *l = new vec<Lit>();
                                    if(idClassesDist1->getId()>idClassesDist->getId())
                                        l->push(mkLit(getVariableID(
                                                "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                                "_" + std::to_string(idClassesDist1->getOrderID()))));
                                    else
                                        l->push(mkLit(getVariableID(
                                                "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                                "_" + std::to_string(idClassesDist->getOrderID()))));
                                    l->push(~mkLit(getVariableID(
                                            "w_" + std::to_string(idClassesDist1->getOrderID()) +
                                            "_" + week1)));
                                    l->push(~mkLit(getVariableID(
                                            "w_" + std::to_string(idClassesDist->getOrderID()) +
                                            "_" + week)));
                                    maxsat_formula->addHardClause(*l);
                                    delete l;

                                }

                            }

                        }
                        if(l1->size()>1)
                            maxsat_formula->addHardClause(*l1);
                        delete l1;
                        if(l2->size()>1)
                            maxsat_formula->addHardClause(*l2);
                        delete l2;
                        for (std::pair<int,int> time : idClassesDist->getHours()) {
                            for (std::pair<int, int> time1 : idClassesDist1->getHours()) {
                                if (time.second <= time1.first
                                    || time1.second <= time.first
                                        ) { ;
                                } else {

                                        vec <Lit> *l = new vec<Lit>();
                                        l->push(~mkLit(getVariableID(
                                                "h_" + std::to_string(idClassesDist->getOrderID()) + "_" +
                                                std::to_string(time.first))));
                                        l->push(~mkLit(getVariableID(
                                                "h_" + std::to_string(idClassesDist1->getOrderID()) + "_" +
                                                std::to_string(time1.first))));
                                        if (idClassesDist1->getId() > idClassesDist->getId()) {
                                            l->push(~mkLit(getVariableID(
                                                    "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                                            l->push(~mkLit(getVariableID(
                                                    "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                                        } else {
                                            l->push(~mkLit(getVariableID(
                                                    "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                                            l->push(~mkLit(getVariableID(
                                                    "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                                        }
                                    l->push(~mkLit(getVariableID(
                                            "r_" + std::to_string(idClassesDist->getOrderID()) +
                                            "_" + std::to_string(r->getId()))));
                                    l->push(~mkLit(getVariableID(
                                            "r_" + std::to_string(idClassesDist1->getOrderID()) +
                                            "_" + std::to_string(r->getId()))));
                                        maxsat_formula->addHardClause(*l);
                                        delete l;


                                    }


                            }
                        }

                    }

                }
            }

        }
        /*Distribution contraints*/

        void genConstraint() {
            Class *idClassesDist, *idClassesDist1;
            //printRoomCluster();
            if (instance->getDist().find("MaxBlock") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["MaxBlock"].size(); ++y) {
                    block(instance->getDist()["MaxBlock"].at(y)->getClasses(),
                          instance->getDist()["MaxBlock"].at(y)->getParameter1(),
                          instance->getDist()["MaxBlock"].at(y)->getParameter2(),
                          instance->getDist()["MaxBlock"].at(y)->getWeight(),y);
                }
            }
            if(instance->getDist().find("MaxDayLoad") != instance->getDist().end()){
                    for (int y = 0; y < instance->getDist()["MaxDayLoad"].size(); ++y) {
                        if( instance->getDist()["MaxDayLoad"].at(y)->getWeight()==-1) {
                            std::map<std::string, std::vector<std::string>> dayV;
                        std::map<std::string, int> len;
                        for (int ci = 0;
                             ci < instance->getDist()["MaxDayLoad"].at(y)->getClasses().size(); ++ci) {
                            idClassesDist = instance->getDist()["MaxDayLoad"].at(y)->getClasses()[ci];
                            for (std::string d : idClassesDist->getDays()) {

                                if (dayV.find(d) != dayV.end()) {
                                    dayV[d].push_back("d_" + std::to_string(idClassesDist->getOrderID()) + "_" + d);
                                    len[d] = idClassesDist->getLectures()[0]->getLenght();


                                } else {
                                    len.insert(std::pair<std::string, int>(d,
                                                                           idClassesDist->getLectures()[0]->getLenght()));
                                    std::vector<std::string> t;
                                    t.push_back("d_" + std::to_string(idClassesDist->getOrderID()) + "_" + d);
                                    dayV.insert(std::pair<std::string, std::vector<std::string> >(d, t));
                                }
                            }
                        }
                        for (std::pair<std::string, std::vector<std::string>> p: dayV) {
                            PB *pb = new PB();
                            int s = 0;
                            for (std::string d: p.second) {
                                pb->addProduct(mkLit(getVariableID(d)), len[p.first]);
                                s++;
                            }
                            if (s >= instance->getDist()["MaxDayLoad"].at(y)->getParameter1() &&
                                instance->getDist()["MaxDayLoad"].at(y)->getParameter1() >= 1) {
                                pb->_sign = true;
                                pb->_rhs = instance->getDist()["MaxDayLoad"].at(y)->getParameter1();
                                maxsat_formula->addPBConstraint(pb);
                                delete pb;
                            }

                        }


                    }
                }
            }

            if(instance->getDist().find("MaxDays") != instance->getDist().end()){
                for (int y = 0; y < instance->getDist()["MaxDays"].size(); ++y) {
                    if (instance->getDist()["MaxDays"].at(y)->getWeight() == -1) {

                        std::vector<std::pair<int,std::string>> temp[instance->getNdays()];
                        PB *pb = new PB();
                        for (int ci = 0;
                             ci < instance->getDist()["MaxDays"].at(y)->getClasses().size(); ++ci) {
                            idClassesDist = instance->getDist()["MaxDays"].at(y)->getClasses()[ci];
                            for (std::string d : idClassesDist->getDays()) {
                                int di = 0;
                                for (char &c : d) {
                                    if (c == '1') {
                                        temp[di].push_back(std::pair<int, std::string>(idClassesDist->getOrderID(), d));
                                        vec <Lit> *t = new vec<Lit>();
                                        t->push(mkLit(
                                                getVariableID("dm_" + std::to_string(y) + "_" + std::to_string(di))));
                                        t->push(~mkLit(getVariableID(
                                                "d_" + std::to_string(idClassesDist->getOrderID()) + "_" + d)));
                                        maxsat_formula->addHardClause(*t);
                                        delete t;
                                    }
                                    di++;
                                }

                            }
                        }

                            for (int di = 0; di < instance->getNdays(); ++di) {
                                pb->addProduct(
                                        mkLit(getVariableID("dm_" + std::to_string(y) + "_" + std::to_string(di))),
                                        1);
                                vec <Lit> *t = new vec<Lit>();
                                t->push(~mkLit(getVariableID("dm_" + std::to_string(y) + "_" + std::to_string(di))));
                                for (std::pair<int,std::string> p1 : temp[di]) {
                                    t->push(mkLit(
                                            getVariableID(
                                                    "d_" + std::to_string(p1.first) + "_" + p1.second)));
                                }
                                maxsat_formula->addHardClause(*t);
                                delete t;

                            }

                        if (instance->getNdays() >= instance->getDist()["MaxDays"].at(y)->getParameter1() &&
                            instance->getDist()["MaxDays"].at(y)->getParameter1() >= 1) {

                            pb->_sign = true;
                            pb->_rhs = instance->getDist()["MaxDays"].at(y)->getParameter1();
                            maxsat_formula->addPBConstraint(pb);
                            delete pb;
                        }



                    }
                }

            }


            if (instance->getDist().find("SameAttendees") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameAttendees"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["SameAttendees"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 < instance->getDist()["SameAttendees"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameAttendees"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameAttendees"].at(y)->getClasses()[ci1];
                            vec <Lit> *l1 = new vec<Lit>();
                            vec <Lit> *l2 = new vec<Lit>();
                            if(idClassesDist1->getId()>idClassesDist->getId()) {
                                l1->push(~mkLit(getVariableID(
                                        "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                        "_" + std::to_string(idClassesDist1->getOrderID()))));
                                l2->push(~mkLit(getVariableID(
                                        "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                        "_" + std::to_string(idClassesDist1->getOrderID()))));
                            } else {
                                l1->push(~mkLit(getVariableID(
                                        "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                        "_" + std::to_string(idClassesDist->getOrderID()))));
                                l2->push(~mkLit(getVariableID(
                                        "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                        "_" + std::to_string(idClassesDist->getOrderID()))));
                            }
                            for (std::string day : idClassesDist->getDays()) {
                                for (std::string day1 : idClassesDist1->getDays()) {
                                    if(!stringcompare(day, day1, instance->getNdays(), false)) {


                                        l1->push(mkLit(getVariableID(
                                                "d_" + std::to_string(idClassesDist->getOrderID()) +
                                                "_" + day)));
                                        l2->push(mkLit(getVariableID(
                                                "d_" + std::to_string(idClassesDist1->getOrderID()) +
                                                "_" + day1)));
                                        vec <Lit> *l = new vec<Lit>();

                                        if(idClassesDist1->getId()>idClassesDist->getId())
                                            l->push(mkLit(getVariableID(
                                                    "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                                        else
                                            l->push(mkLit(getVariableID(
                                                    "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                                        l->push(~mkLit(getVariableID(
                                                "d_" + std::to_string(idClassesDist1->getOrderID()) +
                                                "_" + day1)));
                                        l->push(~mkLit(getVariableID(
                                                "d_" + std::to_string(idClassesDist->getOrderID()) +
                                                "_" + day)));
                                        maxsat_formula->addHardClause(*l);
                                        delete l;

                                    }

                                }

                            }
                            if(l1->size()>0)
                                maxsat_formula->addHardClause(*l1);
                            delete l1;
                            if(l2->size()>0)
                                maxsat_formula->addHardClause(*l2);
                            delete l2;
                            l1 = new vec<Lit>();
                            l2 = new vec<Lit>();
                            if(idClassesDist1->getId()>idClassesDist->getId()) {
                                l1->push(~mkLit(getVariableID(
                                        "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                        "_" + std::to_string(idClassesDist1->getOrderID()))));
                                l2->push(~mkLit(getVariableID(
                                        "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                        "_" + std::to_string(idClassesDist1->getOrderID()))));
                            } else {
                                l1->push(~mkLit(getVariableID(
                                        "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                        "_" + std::to_string(idClassesDist->getOrderID()))));
                                l2->push(~mkLit(getVariableID(
                                        "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                        "_" + std::to_string(idClassesDist->getOrderID()))));
                            }
                            for (std::string week : idClassesDist->getWeeks()) {
                                for (std::string week1 : idClassesDist1->getWeeks()) {
                                    if(!stringcompare(week, week1, instance->getNweek(), false)) {
                                        l1->push(mkLit(getVariableID(
                                                "w_" + std::to_string(idClassesDist->getOrderID()) +
                                                "_" + week)));
                                        l2->push(mkLit(getVariableID(
                                                "w_" + std::to_string(idClassesDist1->getOrderID()) +
                                                "_" + week1)));

                                        vec <Lit> *l = new vec<Lit>();
                                        if(idClassesDist1->getId()>idClassesDist->getId())
                                            l->push(mkLit(getVariableID(
                                                    "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist1->getOrderID()))));
                                        else
                                            l->push(mkLit(getVariableID(
                                                    "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                                    "_" + std::to_string(idClassesDist->getOrderID()))));
                                        l->push(~mkLit(getVariableID(
                                                "w_" + std::to_string(idClassesDist1->getOrderID()) +
                                                "_" + week1)));
                                        l->push(~mkLit(getVariableID(
                                                "w_" + std::to_string(idClassesDist->getOrderID()) +
                                                "_" + week)));
                                        maxsat_formula->addHardClause(*l);
                                        delete l;

                                    }

                                }

                            }
                            if(l1->size()>1)
                                maxsat_formula->addHardClause(*l1);
                            delete l1;
                            if(l2->size()>1)
                                maxsat_formula->addHardClause(*l2);
                            delete l2;
                            if(idClassesDist->getPossibleRooms().size()==0||
                               idClassesDist1->getPossibleRooms().size()==0){
                                for (std::pair<int,int> time : idClassesDist->getHours()) {
                                    for (std::pair<int,int> time1 : idClassesDist1->getHours()) {
                                        if (time.second  <= time1.first
                                            || time1.second  <= time.first
                                                ) { ;
                                        } else {


                                            vec <Lit> *l = new vec<Lit>();
                                            l->push(~mkLit(getVariableID(
                                                    "h_" + std::to_string(idClassesDist->getOrderID()) + "_" + std::to_string(time.first))));
                                            l->push(~mkLit(getVariableID(
                                                    "h_" + std::to_string(idClassesDist1->getOrderID()) + "_" + std::to_string(time1.first))));
                                            if(idClassesDist1->getId()>idClassesDist->getId()) {
                                                l->push(~mkLit(getVariableID(
                                                        "sd_" + std::to_string(idClassesDist->getOrderID()) +
                                                        "_" + std::to_string(idClassesDist1->getOrderID()))));
                                                l->push(~mkLit(getVariableID(
                                                        "sw_" + std::to_string(idClassesDist->getOrderID()) +
                                                        "_" + std::to_string(idClassesDist1->getOrderID()))));
                                            } else {
                                                l->push(~mkLit(getVariableID(
                                                        "sd_" + std::to_string(idClassesDist1->getOrderID()) +
                                                        "_" + std::to_string(idClassesDist->getOrderID()))));
                                                l->push(~mkLit(getVariableID(
                                                        "sw_" + std::to_string(idClassesDist1->getOrderID()) +
                                                        "_" + std::to_string(idClassesDist->getOrderID()))));
                                            }
                                            int w = 0;
                                            if ((w = instance->getDist()["SameAttendees"].at(y)->getWeight()) ==
                                                -1) {
                                                maxsat_formula->addHardClause(*l);
                                                delete l;
                                            } else {
                                                maxsat_formula->addSoftClause(w * instance->getDistributionPen(), *l);
                                                delete l;
                                            }
                                        }

                                    }

                                }
                            } else {

                                for (std::pair<Room *, int> pair1: idClassesDist->getPossibleRooms()) {
                                    for (std::pair<Room *, int> pair2: idClassesDist1->getPossibleRooms()) {
                                        int travel = 0;
                                        if (pair1.first->getId() ==
                                            pair2.first->getId() &&
                                            pair2.first->getId() != -1)
                                            continue;
                                        if (pair1.first->getId() != -1 &&
                                            pair2.first->getId() != -1) {
                                            if (instance->getRoom(
                                                    pair1.first->getId())->getTravel(
                                                    pair2.first->getId()) > 0)
                                                travel = instance->getRoom(
                                                        pair1.first->getId())->getTravel(
                                                        pair2.first->getId());
                                            else
                                                travel = instance->getRoom(
                                                        pair2.first->getId())->getTravel(
                                                        pair1.first->getId());
                                        }

                                        for (std::pair<int, int> time : idClassesDist->getHours()) {
                                            for (std::pair<int, int> time1 : idClassesDist1->getHours()) {
                                                if (time.second + travel <= time1.first
                                                    || time1.second + travel <= time.first
                                                        ) { ;
                                                } else {


                                                    vec <Lit> *l = new vec<Lit>();
                                                    l->push(~mkLit(getVariableID(
                                                            "r_" + std::to_string(idClassesDist->getOrderID()) +
                                                            "_" + std::to_string(pair1.first->getId()))));
                                                    l->push(~mkLit(getVariableID(
                                                            "r_" + std::to_string(idClassesDist1->getOrderID()) +
                                                            "_" + std::to_string(pair2.first->getId()))));
                                                    l->push(~mkLit(getVariableID(
                                                            "h_" + std::to_string(idClassesDist->getOrderID()) +
                                                            "_" + std::to_string(time.first))));
                                                    l->push(~mkLit(getVariableID(
                                                            "h_" + std::to_string(idClassesDist1->getOrderID()) +
                                                            "_" + std::to_string(time1.first))));
                                                    if (idClassesDist1->getId() > idClassesDist->getId()) {
                                                        l->push(~mkLit(getVariableID(
                                                                "sd_" +
                                                                std::to_string(idClassesDist->getOrderID()) +
                                                                "_" +
                                                                std::to_string(idClassesDist1->getOrderID()))));
                                                        l->push(~mkLit(getVariableID(
                                                                "sw_" +
                                                                std::to_string(idClassesDist->getOrderID()) +
                                                                "_" +
                                                                std::to_string(idClassesDist1->getOrderID()))));
                                                    } else {
                                                        l->push(~mkLit(getVariableID(
                                                                "sd_" +
                                                                std::to_string(idClassesDist1->getOrderID()) +
                                                                "_" +
                                                                std::to_string(idClassesDist->getOrderID()))));
                                                        l->push(~mkLit(getVariableID(
                                                                "sw_" +
                                                                std::to_string(idClassesDist1->getOrderID()) +
                                                                "_" +
                                                                std::to_string(idClassesDist->getOrderID()))));
                                                    }
                                                    int w = 0;
                                                    if ((w = instance->getDist()["SameAttendees"].at(y)->getWeight()) ==
                                                        -1) {
                                                        maxsat_formula->addHardClause(*l);
                                                        delete l;
                                                    } else {

                                                        maxsat_formula->addSoftClause(w * instance->getDistributionPen(), *l);
                                                        delete l;
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
            if (instance->getDist().find("NotOverlap") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["NotOverlap"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["NotOverlap"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["NotOverlap"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["NotOverlap"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["NotOverlap"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    if (stringcompare(idClassesDist->getLectures()[p]->getWeeks(),
                                                      idClassesDist1->getLectures()[p1]->getWeeks(),
                                                      instance->getNweek(), false) ==
                                        1
                                        || stringcompare(idClassesDist->getLectures()[p]->getDays(),
                                                         idClassesDist1->getLectures()[p1]->getDays(),
                                                         instance->getNdays(), false) ==
                                           1 || idClassesDist->getLectures()[p]->getEnd() <=
                                                idClassesDist1->getLectures()[p1]->getStart() ||
                                        idClassesDist1->getLectures()[p1]->getEnd() <=
                                        idClassesDist->getLectures()[p]->getStart()) { ;

                                    } else {
                                        int w = 0;
                                        if ((w = instance->getDist()["NotOverlap"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);


                                    }
                                }


                            }
                        }
                    }
                }

            }
            if (instance->getDist().find("Overlap") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["Overlap"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["Overlap"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["Overlap"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["Overlap"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["Overlap"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    if (stringcompare(idClassesDist->getLectures()[p]->getWeeks(),
                                                      idClassesDist1->getLectures()[p1]->getWeeks(),
                                                      instance->getNweek(), false) ==
                                        1
                                        || stringcompare(idClassesDist->getLectures()[p]->getDays(),
                                                         idClassesDist1->getLectures()[p1]->getDays(),
                                                         instance->getNdays(), false) ==
                                           1 || idClassesDist->getLectures()[p]->getEnd() <=
                                                idClassesDist1->getLectures()[p1]->getStart() ||
                                        idClassesDist1->getLectures()[p1]->getEnd() <=
                                        idClassesDist->getLectures()[p]->getStart()) {
                                        int w = 0;
                                        if ((w = instance->getDist()["Overlap"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);
                                    }
                                }


                            }
                        }
                    }
                }
            }

            if (instance->getDist().find("DifferentTime") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["DifferentTime"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["DifferentTime"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["DifferentTime"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["DifferentTime"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["DifferentTime"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {

                                    if (idClassesDist->getLectures()[p]->getStart() <=
                                        idClassesDist1->getLectures()[p1]->getStart()
                                        && idClassesDist1->getLectures()[p1]->getEnd() <=
                                           idClassesDist->getLectures()[p]->getEnd()) {
                                        int w = 0;
                                        if ((w = instance->getDist()["DifferentTime"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);
                                    } else if (idClassesDist1->getLectures()[p1]->getStart() <=
                                               idClassesDist->getLectures()[p]->getStart()
                                               && idClassesDist->getLectures()[p]->getEnd() <=
                                                  idClassesDist1->getLectures()[p1]->getEnd()) {
                                        int w = 0;
                                        if ((w = instance->getDist()["DifferentTime"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);
                                    }
                                }


                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("SameTime") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameTime"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["SameTime"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameTime"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameTime"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameTime"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {

                                    if (idClassesDist->getLectures()[p]->getStart() <=
                                        idClassesDist1->getLectures()[p1]->getStart()
                                        && idClassesDist1->getLectures()[p1]->getEnd() <=
                                           idClassesDist->getLectures()[p]->getEnd()) { ;
                                    } else if (idClassesDist1->getLectures()[p1]->getStart() <=
                                               idClassesDist->getLectures()[p]->getStart()
                                               && idClassesDist->getLectures()[p]->getEnd() <=
                                                  idClassesDist1->getLectures()[p1]->getEnd()) { ;
                                    } else {
                                        int w = 0;
                                        if ((w = instance->getDist()["SameTime"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);

                                    }
                                }


                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("SameRoom") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameRoom"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["SameRoom"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameRoom"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameRoom"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameRoom"].at(y)->getClasses()[ci1];
                            for (std::pair<Room *, int> r1: idClassesDist->getPossibleRooms()) {
                                for (std::pair<Room *, int> r2: idClassesDist1->getPossibleRooms()) {
                                    if (r1.first->getId() != r2.first->getId()) {
                                        int w = 0;
                                        if ((w = instance->getDist()["SameRoom"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, r1.first->getId(), r2.first->getId(), true);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, r1.first->getId(), r2.first->getId(), w,true);

                                    }
                                }


                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("DifferentRoom") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["DifferentRoom"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["DifferentRoom"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["DifferentRoom"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["DifferentRoom"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["DifferentRoom"].at(y)->getClasses()[ci1];
                            for (std::pair<Room *, int> r1: idClassesDist->getPossibleRooms()) {
                                for (std::pair<Room *, int> r2: idClassesDist1->getPossibleRooms()) {
                                    if (r1.first->getId() == r2.first->getId()) {
                                        int w = 0;
                                        if ((w = instance->getDist()["DifferentRoom"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, r1.first->getId(), r2.first->getId(), true);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, r1.first->getId(), r2.first->getId(), w,true);

                                    }
                                }


                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("DifferentDays") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["DifferentDays"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["DifferentDays"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["DifferentDays"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["DifferentDays"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["DifferentDays"].at(
                                    y)->getClasses()[ci1];

                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    for (int d = 0; d < instance->getNdays(); ++d) {
                                        if (idClassesDist->getLectures()[p]->getDays()[d] ==
                                            idClassesDist1->getLectures()[p1]->getDays()[d] &&
                                            idClassesDist->getLectures()[p]->getDays()[d] == '1') {
                                            int w = 0;
                                            if ((w = instance->getDist()["DifferentDays"].at(y)->getWeight()) == -1)
                                                constraint(idClassesDist, idClassesDist1, p, p1);
                                            else
                                                constraintSoft(idClassesDist, idClassesDist1, p, p1, w);
                                            break;


                                        }

                                    }


                                }
                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("MinGap") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["MinGap"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["MinGap"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["MinGap"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["MinGap"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["MinGap"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {

                                    if (stringcompare(idClassesDist1->getLectures()[p1]->getWeeks(),
                                                      idClassesDist->getLectures()[p]->getWeeks(),
                                                      instance->getNweek(), false) == 1 ||
                                        stringcompare(idClassesDist1->getLectures()[p1]->getDays(),
                                                      idClassesDist->getLectures()[p]->getDays(),
                                                      instance->getNdays(), false) == 1 ||
                                        (idClassesDist1->getLectures()[p1]->getEnd() +
                                         instance->getDist()["MinGap"].at(y)->getParameter1()) <=
                                        idClassesDist->getLectures()[p]->getStart() ||
                                        (idClassesDist->getLectures()[p]->getEnd() +
                                         instance->getDist()["MinGap"].at(y)->getParameter1()) <=
                                        idClassesDist1->getLectures()[p1]->getStart()) { ;
                                    } else {
                                        int w = 0;
                                        if ((w = instance->getDist()["MinGap"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);

                                    }
                                }
                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("SameDays") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameDays"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["SameDays"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameDays"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameDays"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameDays"].at(y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    if (stringcontains(idClassesDist1->getLectures()[p1]->getDays(),
                                                       idClassesDist->getLectures()[p]->getDays(),
                                                       instance->getNdays()) ==
                                        0) {
                                        int w = 0;
                                        if ((w = instance->getDist()["SameDays"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);


                                    }

                                }


                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("SameWeeks") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameWeeks"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["SameWeeks"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameWeeks"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameWeeks"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameWeeks"].at(y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    if (stringcontains(idClassesDist1->getLectures()[p1]->getWeeks(),
                                                       idClassesDist->getLectures()[p]->getWeeks(),
                                                       instance->getNweek()) ==
                                        0) {
                                        int w = 0;
                                        if ((w = instance->getDist()["SameWeeks"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);


                                    }

                                }


                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("DifferentWeeks") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["DifferentWeeks"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["DifferentWeeks"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["DifferentWeeks"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["DifferentWeeks"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["DifferentWeeks"].at(y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    for (int we = 0; we < instance->getNweek(); ++we) {
                                        if (idClassesDist1->getLectures()[p1]->getWeeks()[we]==
                                            idClassesDist->getLectures()[p]->getWeeks()[we]&&
                                                idClassesDist->getLectures()[p]->getWeeks()[we]==
                                            '1') {
                                            int w = 0;
                                            if ((w = instance->getDist()["DifferentWeeks"].at(y)->getWeight()) == -1)
                                                constraint(idClassesDist, idClassesDist1, p, p1);
                                            else
                                                constraintSoft(idClassesDist, idClassesDist1, p, p1, w);


                                        }

                                    }


                                }


                            }
                        }
                    }
                }
            }
            if (instance->getDist().find("SameStart") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameStart"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["SameStart"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameStart"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameStart"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameStart"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    if (idClassesDist1->getLectures()[p1]->getStart() !=
                                        idClassesDist->getLectures()[p]->getStart()) {
                                        int w = 0;
                                        if ((w = instance->getDist()["SameStart"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);
                                    }
                                }

                            }
                        }
                    }
                }


            }
            if (instance->getDist().find("Precedence") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["Precedence"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["Precedence"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 < instance->getDist()["Precedence"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["Precedence"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["Precedence"].at(
                                    y)->getClasses()[ci1];

                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    if (isFirst(idClassesDist->getLectures()[p]->getWeeks(),
                                                idClassesDist1->getLectures()[p1]->getWeeks(),
                                                instance->getNweek()) == -1 ||
                                        (isFirst(idClassesDist->getLectures()[p]->getWeeks(),
                                                 idClassesDist1->getLectures()[p1]->getWeeks(),
                                                 instance->getNweek()) == 0 &&
                                         isFirst(idClassesDist->getLectures()[p]->getDays(),
                                                 idClassesDist1->getLectures()[p1]->getDays(),
                                                 instance->getNdays()) ==
                                         -1) || (isFirst(idClassesDist->getLectures()[p]->getWeeks(),
                                                         idClassesDist1->getLectures()[p1]->getWeeks(),
                                                         instance->getNweek()) == 0 &&
                                                 isFirst(idClassesDist->getLectures()[p]->getDays(),
                                                         idClassesDist1->getLectures()[p1]->getDays(),
                                                         instance->getNdays()) == 0 &&
                                                 idClassesDist->getLectures()[p]->getEnd() <=
                                                 idClassesDist1->getLectures()[p1]->getStart())) { ;
                                    } else {
                                        int w = 0;
                                        if ((w = instance->getDist()["Precedence"].at(y)->getWeight()) == -1)
                                            constraint(idClassesDist, idClassesDist1, p, p1);
                                        else
                                            constraintSoft(idClassesDist, idClassesDist1, p, p1, w);
                                    }


                                }

                            }
                        }
                    }



                }
            }
            if (instance->getDist().find("WorkDay") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["WorkDay"].size(); ++y) {
                    for (int ci = 0;
                         ci < instance->getDist()["WorkDay"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["WorkDay"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["WorkDay"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["WorkDay"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getLectures().size(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getLectures().size(); ++p1) {
                                    if (stringcompare(idClassesDist->getLectures()[p]->getWeeks(),
                                                      idClassesDist1->getLectures()[p1]->getWeeks(),
                                                      instance->getNweek(),
                                                      false) == 1 ||
                                        stringcompare(idClassesDist->getLectures()[p]->getDays(),
                                                      idClassesDist1->getLectures()[p1]->getDays(),
                                                      instance->getNdays(),
                                                      false) == 1) { ;
                                    } else {
                                        if (std::max(idClassesDist->getLectures()[p]->getEnd(),
                                                     idClassesDist1->getLectures()[p1]->getEnd()) -
                                            std::min(idClassesDist->getLectures()[p]->getStart(),
                                                     idClassesDist1->getLectures()[p1]->getStart()) >
                                            instance->getDist()["WorkDay"].at(y)->getParameter1()) {
                                            int w = 0;
                                            if ((w = instance->getDist()["WorkDay"].at(y)->getWeight()) == -1)
                                                constraint(idClassesDist, idClassesDist1, p, p1);
                                            else
                                                constraintSoft(idClassesDist, idClassesDist1, p, p1, w);
                                        }
                                    }


                                }
                            }
                        }
                    }
                }
            }


        }

        /* Hard constraints*/
        void constraint(Class *idClassesDist, Class *idClassesDist1, int p, int p1, bool isRoom = false) {
            std::string vname = "t_";
            std::string vpart, vpart1;


            if (isRoom) {//Constraint over the rooms 
                vname = "r_";
                vpart = std::to_string(p);
                vpart1 = std::to_string(p1);
            } else {// Contraint over time
                vpart = std::to_string(idClassesDist->getLectures()[p]->getOrderId());
                vpart1 = std::to_string(idClassesDist1->getLectures()[p1]->getOrderId());
            }
            vec <Lit> *l = new vec<Lit>();
            l->push(~mkLit(getVariableID(
                    vname + std::to_string(idClassesDist->getOrderID()) + "_" + vpart)));
            l->push(~mkLit(getVariableID(
                    vname + std::to_string(idClassesDist1->getOrderID()) + "_" + vpart1)));

            maxsat_formula->addHardClause(*l);
            delete l;

        }
        
        /*Add soft cnstraints*/

        void constraintSoft(Class *idClassesDist, Class *idClassesDist1, int p, int p1, int pen, bool isRoom = 0) {
            if (optSoft) {

                std::string vname = "t_";
                std::string vpart, vpart1;
                if (isRoom) {
                    vname = "r_";
                    vpart = std::to_string(p);
                    vpart1 = std::to_string(p1);
                } else {
                    vpart = std::to_string(idClassesDist->getLectures()[p]->getOrderId());
                    vpart1 = std::to_string(idClassesDist1->getLectures()[p1]->getOrderId());
                }
                vec <Lit> *l = new vec<Lit>();
                l->push(~mkLit(getVariableID(
                        vname + std::to_string(idClassesDist->getOrderID()) + "_" + vpart)));
                l->push(~mkLit(getVariableID(
                        vname + std::to_string(idClassesDist1->getOrderID()) + "_" + vpart1)));


                maxsat_formula->addSoftClause(pen * instance->getDistributionPen(), *l);
                delete l;
            }
        }


        
        /*Aux var for students conflicts*/


        void sameTime() {
            for (std::pair<std::string, std::pair<Course *, Course *>> pair: pairCourse) {
                for (Class *aClass:pair.second.first->getClasses()) {
                    for (Class *aClass1:pair.second.second->getClasses()) {
                        Class *bClass0 = (aClass->getId() > aClass1->getId() ? aClass1 : aClass);
                        Class *bClass1 = (aClass->getId() < aClass1->getId() ? aClass : aClass1);

                        if (pairConf.find(std::to_string(bClass0->getId()) + "_" + std::to_string(bClass1->getId())) !=
                            pairConf.end()) {
                            for (std::pair<int, int> conf: pairConf[std::to_string(bClass0->getId()) + "_" +
                                                                    std::to_string(bClass1->getId())]) {
                                vec <Lit> l;
                                vec <Lit> literal;


                                l.push(~mkLit(getVariableID(bClass0->getKey(bClass0->getPossiblePairRoom(conf.first),
                                                                            bClass0->getPossiblePairLecture(
                                                                                    conf.first)))));
                                l.push(mkLit(getVariableID("sameTimeperSlot_" + std::to_string(bClass0->getId()) + "_"
                                                           + std::to_string(bClass1->getId()) + "_" +
                                                           std::to_string(conf.first) + "-" +
                                                           std::to_string(conf.second))));
                                maxsat_formula->addHardClause(l);
                                vec <Lit> l1;
                                l1.push(~mkLit(getVariableID(bClass1->getKey(bClass1->getPossiblePairRoom(conf.second),
                                                                             bClass1->getPossiblePairLecture(
                                                                                     conf.second)))));
                                l1.push(mkLit(getVariableID("sameTimeperSlot_" + std::to_string(bClass0->getId()) + "_"
                                                            + std::to_string(bClass1->getId()) + "_" +
                                                            std::to_string(conf.first) + "-" +
                                                            std::to_string(conf.second))));
                                maxsat_formula->addHardClause(l1);
                                literal.push(
                                        ~mkLit(getVariableID("sameTimeperSlot_" + std::to_string(bClass0->getId()) + "_"
                                                             + std::to_string(bClass1->getId()) + "_" +
                                                             std::to_string(conf.first) + "-" +
                                                             std::to_string(conf.second))));
                                literal.push(mkLit(getVariableID("sameTime" + std::to_string(bClass0->getId()) + "_"
                                                                 + std::to_string(bClass1->getId()))));
                                maxsat_formula->addHardClause(literal);

                            }

                        }

                    }
                }
            }


        }
        
        /*Studnet sectionning */


       void genStudents(MaxSATFormula *mf) {
           maxsat_formula = mf;
            requiredClasses();
            limit();
            parentChild();
            if (optStud) {
                conflicts();
            }
        }

        void requiredClasses() {
            for (int s = 0; s < instance->getClusterStudent().size(); ++s) {
                std::vector<std::vector<Class *>> classes;
                for (int c = 0; c < instance->getClusterStudent()[s]->getCourses().size(); ++c) {
                    vec <Lit> l0;
                    for (int conf = 0;
                         conf < instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig(); ++conf) {
                        if (instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig() > 1)
                            l0.push(mkLit(getVariableID(
                                    "conf" + std::to_string(s) + "_" + std::to_string(conf) + "_" +
                                    instance->getClusterStudent()[s]->getCourses()[c]->getName())));
                        std::vector<Class *> partV;
                        for (int part = 0; part < instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                conf).size(); ++part) {
                            vec <Lit> l1;
                            if (instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig() > 1)
                                l1.push(~mkLit(getVariableID(
                                        "conf" + std::to_string(s) + "_" + std::to_string(conf) + "_" +
                                        instance->getClusterStudent()[s]->getCourses()[c]->getName())));
                            for (int cla = 0;
                                 cla < instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                         conf)[part]->getClasses().size(); ++cla) {
                                if (instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                        conf)[part]->getClasses()[cla]->getLimit() == 0)
                                    continue;
                                for (int cla1 = cla + 1;
                                     cla1 < instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                             conf)[part]->getClasses().size(); ++cla1) {
                                    if (instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                            conf)[part]->getClasses()[cla1]->getLimit() == 0)
                                        continue;
                                    vec <Lit> l;
                                    if (instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig() > 1)
                                        l.push(~mkLit(getVariableID(
                                                "conf" + std::to_string(s) + "_" + std::to_string(conf) + "_" +
                                                instance->getClusterStudent()[s]->getCourses()[c]->getName())));

                                    l.push(~mkLit(getVariableID(
                                            "stu_" + std::to_string(s) + "_" + std::to_string(
                                                    instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                                            conf)[part]->getClasses()[cla]->getId()))));
                                    l.push(~mkLit(getVariableID(
                                            "stu_" + std::to_string(s) + "_" + std::to_string(
                                                    instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                                            conf)[part]->getClasses()[cla1]->getId()))));
                                    maxsat_formula->addHardClause(l);
                                }
                                partV.push_back(instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                        conf)[part]->getClasses()[cla]);
                                l1.push(mkLit(getVariableID(
                                        "stu_" + std::to_string(s) + "_" + std::to_string(
                                                instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                                        conf)[part]->getClasses()[cla]->getId()))));
                                instance->getClusterStudent()[s]->setClassesID(
                                        instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                                conf)[part]->getClasses()[cla]->getId(),
                                        "stu_" + std::to_string(s) + "_" + std::to_string(
                                                instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                                        conf)[part]->getClasses()[cla]->getId()));


                            }
                            classes.push_back(partV);
                            maxsat_formula->addHardClause(l1);


                        }


                    }
                    if (instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig() > 1)
                        maxsat_formula->addHardClause(l0);


                }
                instance->getClusterStudent()[s]->setClasses(classes);
            }


        }

        int MDC(int a, int b) const {

            if (b == 0) return a;

            return MDC(b, a % b);
        }

        void limit() {
            for (int cla = 0; cla < instance->getClasses().size(); ++cla) {
                if (instance->getClasses()[cla]->getLimit() != 0) {
                    PB *pb = new PB();
                    bool t = false;
                    int stuNumb = 0;
                    for (int i = 0; i < instance->getClusterStudent().size(); ++i) {
                        if (instance->getClusterStudent()[i]->getClassesID(
                                instance->getClasses()[cla]->getId()).compare("Empty") != 0) {
                            t = true;
                            pb->addProduct(mkLit(getVariableID(
                                    instance->getClusterStudent()[i]->getClassesID(
                                            instance->getClasses()[cla]->getId()))),
                                           instance->getClusterStudent()[i]->getStudent().size());
                            stuNumb += instance->getClusterStudent()[i]->getStudent().size();
                        }
                    }
                    if (t && instance->getClasses()[cla]->getLimit() <= stuNumb &&
                        instance->getClasses()[cla]->getLimit() >= 1) {
                        pb->_sign = true;
                        pb->addRHS(instance->getClasses()[cla]->getLimit());

                        maxsat_formula->addPBConstraint(pb);
                    }
                    delete pb;
                }

            }
        }


        void parentChild() {
            for (int i = 0; i < instance->getClusterStudent().size(); ++i) {
                for (int c = 0; c < instance->getClasses().size(); ++c) {
                    if (instance->getClasses()[c]->getLimit() != 0) {
                        if (instance->getClasses()[c]->getParent() != nullptr) {
                            vec <Lit> l;
                            l.push(~mkLit(getVariableID(instance->getClusterStudent()[i]->getClassesID(
                                    instance->getClasses()[c]->getId()))));
                            l.push(mkLit(getVariableID(instance->getClusterStudent()[i]->getClassesID(
                                    instance->getClasses()[c]->getParent()->getId()))));
                            maxsat_formula->addHardClause(l);
                        }

                    }

                }
            }

        }
        /* Studnet Conflicts*/

        void conflicts() {

            for (int i = 0; i < instance->getClusterStudent().size(); ++i) {
                for (int it = 0; it < instance->getClusterStudent()[i]->getClasses().size(); ++it) {
                    for (int it1 = it + 1; it1 < instance->getClusterStudent()[i]->getClasses().size(); ++it1) {
                        for (int c = 0; c < instance->getClusterStudent()[i]->getClasses()[it].size(); ++c) {
                            if (instance->getClusterStudent()[i]->getClasses()[it][c]->getLimit() == 0)
                                continue;
                            for (int c1 = 0; c1 < instance->getClusterStudent()[i]->getClasses()[it1].size(); ++c1) {


                                if (instance->getClusterStudent()[i]->getClasses()[it1][c1]->getLimit() == 0)
                                    continue;
                                if (instance->getClusterStudent()[i]->getClasses()[it1][c1]->getSubconfcour().compare(
                                        instance->getClusterStudent()[i]->getClasses()[it][c]->getSubconfcour()) == 0)
                                    continue;
                                int bClass0 = (instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId() >
                                               instance->getClusterStudent()[i]->getClasses()[it][c]->getId()
                                               ? instance->getClusterStudent()[i]->getClasses()[it][c]->getId()
                                               : instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId());
                                int bClass1 = (instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId() <
                                               instance->getClusterStudent()[i]->getClasses()[it][c]->getId()
                                               ? instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId()
                                               : instance->getClusterStudent()[i]->getClasses()[it][c]->getId());

                                vec <Lit> l;
                                l.push(~mkLit(getVariableID(instance->getClusterStudent()[i]->getClassesID(
                                        instance->getClusterStudent()[i]->getClasses()[it][c]->getId()))));
                                l.push(~mkLit(getVariableID(instance->getClusterStudent()[i]->getClassesID(
                                        instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId()))));
                                l.push(~mkLit(getVariableID("sameTime" + std::to_string(bClass0) + "_"
                                                            + std::to_string(bClass1))));
                                maxsat_formula->addSoftClause(instance->getStudentPen() *
                                                              instance->getClusterStudent()[i]->getStudent().size(),
                                                              l);

                            }
                        }
                    }


                }

            }


        }

        /*Block generation */



        std::vector<int> combination;
        std::vector<std::set<int>> com;
        void print(const std::vector<int>& v) {
            for (int i = 0; i < v.size(); ++i) { std::cout << v[i] << " "; }
            std::cout<<std::endl;
        }

        void go(int offset, int k, std::set<int> times) {
            if (k == 0) {
                std::set<int> tSet;
                for (int i = 0; i < combination.size(); ++i) {
                    tSet.insert(combination[i]);
                }
                com.push_back(tSet);


                //print(combination);
                return;
            }
            for (int i = offset; i <= (times.size() - k); ++i) {
                combination.push_back(*std::next(times.begin(), i));
                go(i+1, k-1,times);
                combination.pop_back();
            }
        }

        void block(std::vector<Class *> cont, int R, int S, double weight,int b) {
            std::set<int> times;
            int len=0;
            for(Class *c: cont){
                for (std::pair<int,int> ti : c->getHours()) {
                    int time = ti.first;
                    vec<Lit> temp;
                    len=ti.second-ti.first;
                    temp.push(~mkLit(getVariableID("h_"+std::to_string(c->getOrderID())+"_"+
                    std::to_string(time))));
                    temp.push(mkLit(getVariableID("MaxBlocktemp_"+std::to_string(b)+"_"+
                                                   std::to_string(time))));

                    maxsat_formula->addHardClause(temp);
                    temp.clear();
                    times.insert(time);

                }
            }

            vec<Lit> *block= new vec<Lit>;
            if(cont.size()>times.size())
                go(0, times.size(),times);
            else
                go(0, cont.size(),times);

            for (std::set<int> timev: com) {
                block->clear();
                std::cout<<"new"<<std::endl;
                bool isBlock= false;
                int size=0;
                int start=0,first=0;
                for (int time: timev) {

                    if (isBlock) {
                        if (start + len + S < time) {
                            size = len;
                            start = time;
                            first=time;
                            std::cout<<"new"<<std::endl;
                            block->clear();
                            std::cout<<("MaxBlocktemp_" + std::to_string(b) + "_" +
                                       std::to_string(time))<<std::endl;


                            block->push(~mkLit(getVariableID("MaxBlocktemp_" + std::to_string(b) + "_" +
                                                             std::to_string(time))));

                        } else {
                            size = time - first;
                            std::cout<<size<<std::endl;

                            start=time;
                            std::cout<<("MaxBlocktemp_" + std::to_string(b) + "_" +
                                        std::to_string(time))<<std::endl;

                           block->push(~mkLit(getVariableID("MaxBlocktemp_" + std::to_string(b) + "_" +
                                                             std::to_string(time))));

                            if (size+S > R) {
                                std::cout<<"push"<<std::endl;
                                maxsat_formula->addHardClause(*block);
                            }
                        }


                    } else {
                        block->clear();
                        std::cout<<("MaxBlocktemp_" + std::to_string(b) + "_" +
                                    std::to_string(time))<<std::endl;

                        block->push(~mkLit(getVariableID("MaxBlocktemp_" + std::to_string(b) + "_" +
                                                         std::to_string(time))));
                        isBlock = true;
                        size = len;
                        start = time;
                        first=time;
                    }

                }
            }

        }


        /*MPP method: invalid time disruption*/

        void una() {
            for (std::pair<int, std::set<int>> una :instance->getTimeUnavailable()) {
                for (int tuna: una.second) {
                    vec <Lit> l;
                    l.push(~mkLit(getVariableID(
                            "h_" + std::to_string(una.first) + "_" +
                            std::to_string(instance->getClasses()[una.first]->getLectures()[tuna]->getStart()))));
                    maxsat_formula->addHardClause(l);

                }

            }
        }
        
        /* Optinal method: slack on class room capacity*/


        void slackStudent() {
            for (int l = 0; l <
                            instance->getNumClasses(); l++) {
                int j = 0;
                for (std::map<int, Room *>::const_iterator it = instance->getRooms().begin();
                     it != instance->getRooms().end(); it++) {
                    if (instance->getClasses()[l]->containsRoom(instance->getRoom(j + 1))) {
                        PB *p = new PB();
                        p->_sign = true;
                        p->addProduct(
                                mkLit(getVariableID("r_" + std::to_string(instance->getClasses()[l]->getOrderID()) +
                                                    "_" + std::to_string(it->first))), it->second->getCapacity());
                        p->addRHS(instance->getClasses()[l]->getLimit() -
                                  instance->getClasses()[l]->getLimit() * instance->getAlfa());
                        maxsat_formula->addPBConstraint(p);
                        delete p;

                    }


                    j++;
                }
            }
        }

        /***
   * The current distance of the solution with the old solution
   * The distantes is base on the weighted Hamming distance
   */


        void distanceToSolutionLectures() {
            PBObjFunction *of = new PBObjFunction();

            for (int j = 0; j < instance->getNumClasses(); ++j) {
                for (int lec = 0; lec < instance->getClasses()[j]->getLectures().size(); ++lec) {
                    if (instance->getClasses()[j]->getLectures()[lec]->getStart() !=
                        instance->getClasses()[j]->getSolStart())
                        of->addProduct(mkLit(getVariableID(
                                "h_" + std::to_string(instance->getClasses()[j]->getOrderID()) + "_" +
                                std::to_string(instance->getClasses()[j]->getLectures()[lec]->getStart()))),
                                       1);
                    if (stringcompare(instance->getClasses()[j]->getLectures()[lec]->getWeeks(),
                                      instance->getClasses()[j]->getSolWeek(), instance->getNweek(), true) == 0)
                        of->addProduct(mkLit(getVariableID(
                                "w_" + std::to_string(instance->getClasses()[j]->getOrderID()) + "_" +
                                instance->getClasses()[j]->getLectures()[lec]->getWeeks())),
                                       1);
                    if (stringcompare(instance->getClasses()[j]->getLectures()[lec]->getDays(),
                                      instance->getClasses()[j]->getSolDays(), instance->getNdays(), true) == 0)
                        of->addProduct(mkLit(getVariableID(
                                "d_" + std::to_string(instance->getClasses()[j]->getOrderID()) + "_" +
                                instance->getClasses()[j]->getLectures()[lec]->getDays())),
                                       1);


                }
                for (std::pair<Room *, int> pair1: instance->getClasses()[j]->getPossibleRooms()) {
                    if (instance->getClasses()[j]->getSolRoom() != pair1.first->getId())
                        of->addProduct(mkLit(getVariableID(
                                "r_" + std::to_string(instance->getClasses()[j]->getOrderID()) + "_" +
                                std::to_string(pair1.first->getId()))),
                                       1);

                }

            }

            maxsat_formula->addObjFunction(of);


        }

        void closeroom() {
            std::cout << "b1" << std::endl;

            for (std::pair<int, Room *> p: instance->getRooms()) {
                std::cout << "b2" << std::endl;

                if (instance->isRoomBlocked(p.first)) {
                    unsigned int t = std::chrono::steady_clock::now().time_since_epoch().count();
                    std::default_random_engine generator(t);
                    std::uniform_int_distribution<int> distribution(0, p.second->c.size() - 1);
                    int i = 0;
                    int v = distribution(generator);
                    std::cout << "b3" << std::endl;

                    for (int id: p.second->c) {
                        std::cout << "b4" << std::endl;

                        if (i == v) {
                            std::cout << "a" << std::endl;
                            vec <Lit> l;
                            l.push(~mkLit(getVariableID(
                                    "r_" + std::to_string(id) + "_" +
                                    std::to_string(p.second->getId()))));
                            maxsat_formula->addHardClause(l);
                            break;
                        }
                        i++;
                    }

                }
            }


        }

    protected:

        int currentClass=0;
        int currentT=0;
        int classCount=0;
        std::list<State *> *queue;



        std::map<int, std::vector<std::pair<Class *, Lecture *>>> times;//time class Lecture


    };


} // namespace openwbo

#endif // __PB_PARSER__

/*****************************************************************************/
