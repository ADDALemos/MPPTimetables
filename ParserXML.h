

#ifndef __XML_PARSER__
#define __XML_PARSER__

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
#include "randomGenerator/Perturbation.h"
#include "utils/Stats.h"
#include "utils/TimeUtil.h"
#include "problem/Time.h"
#include "problem/Curriculum.h"
#include "problem/ConstraintShort.h"
#include "utils/StringUtil.h"

using namespace rapidxml;


namespace openwbo {
    class Block {
    public:
        Block() {}

        Block(const int &start, const int &end, std::vector<Block *, std::allocator<Block *>> vector,
              std::vector<Block *, std::allocator<Block *>> vector1) : end(end), start(start) {
            vars.insert(vars.end(), vector.begin(), vector.end());
            vars.insert(vars.end(), vector1.begin(), vector1.end());

        }

        Block(const int &start, const int &end, Block *v) : end(end), start(start) {
            add(v);
        }

        void add(Block *v) {
            vars.push_back(v);
        }

        virtual std::string getName(){
            return "erro";
        }

        int end;
        std::vector<Block *> vars;
        int start;
    };


    class Vars : public Block {
    public:
        Vars(std::string name, int start, int end) :  Block(start, end, this), varName(name) {

        }

        virtual std::string getName(){
            return varName;
        }

        std::string varName;

    };


    class ParserXML {
        bool optAlloction = true;
        bool optStud = true;
        bool optSoft = true;

    public:
        //-------------------------------------------------------------------------
        // Constructor/destructor.
        //-------------------------------------------------------------------------

        ParserXML(MaxSATFormula *maxsat_formula, bool optAlloction, bool optStud, bool optSoft) : optAlloction(
                optAlloction),
                                                                                                  optStud(optStud),
                                                                                                  optSoft(optSoft),
                                                                                                  maxsat_formula(
                                                                                                          maxsat_formula) {

        }

        virtual ~ParserXML() {}


        void parse(std::string fileName) {
            PBObjFunction *of;
//            PB *pbopt;
            if (optAlloction)
                of = new PBObjFunction();
            /*else
                pbopt = new PB();*/

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
                    int order = 0;
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
                                    Class *c = new Class(idclass, limit, orderID,
                                                         itos(idConf) + "_" + id);
                                    c->setOrderID(order);
                                    c->setCourseID(atoi(id));
                                    order++;
                                    all.push_back(c);
                                    order = all.size() - 1;
                                    PB *pb = new PB();
                                    int i = 0;
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


                                                roomsv.insert(
                                                        std::pair<Room *, int>(instance->getRoom(std::stoi(idRoom)),
                                                                               penalty));

                                            }
                                            i++;
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
                                            Lecture *l = new Lecture(lenght, start, weeks, days, penalty);
                                            lecv.push_back(l);
                                            int max = -1;


                                            if (roomsv.size() == 0) {


                                                max = c->setPossiblePair(
                                                        new Room(-1),
                                                        l, "x_" + std::to_string(order) + "_");
                                                pb->addProduct(mkLit(getVariableID(
                                                        "x_" + std::to_string(order) + "_" + std::to_string(max))), 1);
                                                //std::cout<<c->getId()<<" x_" +std::to_string(order)+"_"+std::to_string(max)<<std::endl;




                                                //oneEach += " +1 " + std::to_string(max);
                                                if (penalty != 0 && optAlloction)
                                                    of->addProduct(mkLit(getVariableID(
                                                            "x_" + std::to_string(order) + "_" + std::to_string(max))),
                                                                   instance->getTimePen() * penalty);
                                                /*else if (penalty != 0)
                                                    pbopt->addProduct(mkLit(getVariableID("x_" +std::to_string(order)+"_"+std::to_string(c->getPossiblePairSize()-1))),
                                                                      instance->getTimePen() * penalty);*/



                                            } else {
                                                for (std::map<Room *, int>::iterator j = roomsv.begin();
                                                     j != roomsv.end(); ++j) {

                                                    std::string week = l->getWeeks();
                                                    std::string day = l->getDays();
                                                    int startTime = l->getStart();
                                                    int duration = l->getLenght();
                                                    bool isNotAddableTime = false;
                                                    for (int una = 0; una < j->first->getSlots().size(); ++una) {
                                                        for (int weeki = 0; weeki < instance->getNweek(); ++weeki) {
                                                            if (week[weeki] ==
                                                                j->first->getSlots()[una].getWeeks()[weeki] &&
                                                                j->first->getSlots()[una].getWeeks()[weeki] == '1') {
                                                                for (int d = 0; d < instance->getNdays(); ++d) {
                                                                    if (day[d] ==
                                                                        j->first->getSlots()[una].getDays()[d] &&
                                                                        day[d] == '1') {

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
                                                    if (!isNotAddableTime) {
                                                        max = c->setPossiblePair(
                                                                j->first,
                                                                l, "x_" + std::to_string(order) + "_");
                                                        //std::cout<<c->getId()<<" x_" +std::to_string(order)+"_"+std::to_string(max)<<std::endl;

                                                        pb->addProduct(mkLit(getVariableID(
                                                                "x_" + std::to_string(order) + "_" + std::to_string(max)
                                                                       )),
                                                                       1);
                                                        //std::cout<<idclass<<" "<<std::to_string(max)<<std::endl;


                                                        if (penalty != 0 && optAlloction)
                                                            of->addProduct(
                                                                    mkLit(getVariableID(
                                                                            "x_" + std::to_string(order) + "_" +
                                                                            std::to_string(max))),
                                                                    instance->getTimePen() * penalty);
                                                        /*else if(penalty != 0)
                                                            pbopt->addProduct(
                                                                    mkLit(getVariableID("x_" + std::to_string(max))),
                                                                    instance->getTimePen() * penalty);*/

                                                        if (j->second != 0 && optAlloction)
                                                            of->addProduct(
                                                                    mkLit(getVariableID(
                                                                            "x_" + std::to_string(order) + "_" +
                                                                            std::to_string(max))),
                                                                    instance->getRoomPen() * j->second);
                                                        /*else if(j->second != 0)
                                                            pbopt->addProduct(
                                                                    mkLit(getVariableID("x" + std::to_string(max))),
                                                                    instance->getRoomPen() * j->second);*/




                                                        bool is = true;
                                                        for (int ti = 0; ti < j->first->t.size(); ++ti) {
                                                            if (j->first->t[ti]->getWeek().compare(l->getWeeks()) ==
                                                                0 &&
                                                                j->first->t[ti]->getDay().compare(l->getDays()) == 0 &&
                                                                l->getStart() == j->first->t[ti]->getStart() &&
                                                                l->getEnd() == j->first->t[ti]->getEnd()) {
                                                                j->first->t[ti]->addC(
                                                                        "x_" + std::to_string(order) + "_" +
                                                                        std::to_string(max), idclass);
                                                                is = false;
                                                            }
                                                        }
                                                        if (is) {

                                                            j->first->t.push_back(
                                                                    new Time(l->getStart(), l->getEnd(),
                                                                             l->getWeeks(),
                                                                             l->getDays(),
                                                                             "x_" + std::to_string(order) + "_" +
                                                                             std::to_string(max), idclass));
                                                        }


                                                    }

                                                }
                                            }
                                            if (times.find(start) != times.end()) {
                                                for (Time *time1: times[start]) {
                                                    Time *ntime = new Time(l->getStart(), l->getEnd(),
                                                                           l->getWeeks(),
                                                                           l->getDays(),
                                                                           "x_" + std::to_string(order) + "_" +
                                                                           std::to_string(max), idclass, max);
                                                    if (time1->check(ntime, instance->getNweek(),
                                                                     instance->getNdays())) {
                                                        int aClass0 = (c->getId() > time1->getClassesCid()[0]
                                                                       ? time1->getClassesCid()[0] : c->getId());
                                                        int aClass1 = (c->getId() < time1->getClassesCid()[0]
                                                                       ? c->getId() : time1->getClassesCid()[0]);
                                                        int v0 = (c->getId() > time1->getClassesCid()[0]
                                                                  ? time1->getClassesMax()[0] : max);
                                                        int v1 = (c->getId() < time1->getClassesCid()[0] ? max
                                                                                                         : time1->getClassesMax()[0]);
                                                        if (pairConf.find(std::to_string(aClass0) + "_" +
                                                                          std::to_string(aClass1)) != pairConf.end())
                                                            pairConf[std::to_string(aClass0) + "_" +
                                                                     std::to_string(aClass1)].push_back(
                                                                    std::pair<int, int>(v0, v1));
                                                        else {
                                                            std::vector<std::pair<int, int>> temp;
                                                            temp.push_back(std::pair<int, int>(v0, v1));
                                                            pairConf.insert(
                                                                    std::pair<std::string, std::vector<std::pair<int, int>>>(
                                                                            std::to_string(aClass0) + "_" +
                                                                            std::to_string(aClass1), temp));

                                                        }

                                                        //        std::map<std::string,std::vector<std::pair<int, int>>> pairConf;

                                                    }


                                                }
                                            } else {
                                                std::vector<Time *> temp;
                                                temp.push_back(new Time(l->getStart(), l->getEnd(),
                                                                        l->getWeeks(),
                                                                        l->getDays(),
                                                                        "x_" + std::to_string(order) + "_" +
                                                                        std::to_string(max), idclass, max));
                                                times.insert(std::pair<int, std::vector<Time *>>(l->getStart(), temp));
                                            }

                                        }


                                    }

                                    c->setPossibleRooms(roomsv);
                                    c->setLectures(lecv);

                                    classMap.insert(std::pair<int, Class *>(orderID, c));
                                    classID.insert(std::pair<int, int>(idclass, orderID));
                                    orderID++;
                                    if (parent != -1)
                                        c->setParent(classMap[classID[parent]]);
                                    clasv.push_back(c);
                                    pb->addRHS(1);


                                    PB *p2 = new PB(pb->_lits, pb->_coeffs, pb->_rhs, true);
                                    maxsat_formula->addPBConstraint(pb);
                                    maxsat_formula->addPBConstraint(p2);
                                    delete pb;
                                    delete p2;


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
                    std::set<Room *> temp;
                    for (std::pair<Room *, int> t: all[0]->getPossibleRooms()) {
                        temp.insert(t.first);
                    }
                    cluster.push_back(new ClusterbyRoom(0, temp, all[0]));
                    mapCluster.insert(std::pair<Class *, ClusterbyRoom *>(all[0], cluster[0]));
                    for (int k = 1; k < all.size(); ++k) {
                        bool test = false;

                        for (int i = 0; i < cluster.size(); ++i) {
                            if (cluster[i]->getRooms().size() == 0 && all[k]->getPossibleRooms().size() == 0) {
                                test = true;
                                cluster[i]->addClass(all[k]);
                                mapCluster.insert(std::pair<Class *, ClusterbyRoom *>(all[k], cluster[i]));
                                continue;
                            } else {
                                for (Room *j:cluster[i]->getRooms()) {
                                    if (all[k]->findRoom(j)) {
                                        test = true;
                                        cluster[i]->addClass(all[k]);
                                        if (all[k]->getPossibleRooms().size() > 0) {
                                            for (std::pair<Room *, int> it : all[k]->getPossibleRooms()) {
                                                cluster[i]->addRoom(it.first);
                                            }
                                        }
                                        mapCluster.insert(std::pair<Class *, ClusterbyRoom *>(all[k], cluster[i]));
                                        break;
                                    }

                                }
                            }
                            if (test) {
                                break;
                            }


                        }
                        if (!test) {
                            ClusterbyRoom *clu = nullptr;
                            if (all[k]->getPossibleRooms().size() > 0) {
                                std::set<Room *> temp1;
                                for (std::pair<Room *, int> it : all[k]->getPossibleRooms()) {
                                    temp1.insert(it.first);
                                }
                                clu = new ClusterbyRoom(cluster.size(), temp1, all[k]);
                            } else {
                                clu = new ClusterbyRoom(cluster.size(), all[k]);
                            }
                            cluster.push_back(clu);

                            mapCluster.insert(std::pair<Class *, ClusterbyRoom *>(all[k], clu));
                        }

                    }


                } else if (strcmp(n->name(), "distributions") == 0) {
                    readConstraints(mapCluster, curMap, n);
                } else if (strcmp(n->name(), "students") == 0) {
                    readStudents(n);
                }


            }
            file.close();
            createCurriculum(curMap);
            if (optAlloction) {
                maxsat_formula->addObjFunction(of);
                delete of;
            } /*else {
                pbopt->_sign=true;
                pbopt->_rhs=3290;
                maxsat_formula->addPBConstraint(pbopt);

            }*/


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
                        /*std::cout<<clusterStudent[i]->getStudent().size()<<std::endl;
                        std::cout<<clusterStudent[i]->getMin()<<std::endl;*/

                        if (c.size() != clusterStudent[i]->getCourses().size() ||
                            (clusterStudent[i]->getStudent().size() + 1) >
                            MDC(clusterStudent[i]->getMin(), clusterStudent[i]->getStudent().size() + 1)
                            || (clusterStudent[i]->getStudent().size() + 1) >
                               MDC(minimo, clusterStudent[i]->getStudent().size() + 1))
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
                //if (isReq) {
                for (ClusterbyRoom *clusterbyRoom: *c) {
                    clusterbyRoom->setRange(new ConstraintShort(type, penalty, c1, limit, limit1));
                    break;
                }


                if (hard.find(type) != hard.end()) {
                    hard[type].push_back(new ConstraintShort(type, penalty, c1, limit, limit1));
                } else {
                    std::vector<ConstraintShort *> t;
                    t.push_back(new ConstraintShort(type, penalty, c1, limit, limit1));
                    hard.insert(std::pair<std::string, std::vector<ConstraintShort *>>(type, t));
                }

                /*} else {
                    if (soft.find(type) != soft.end()) {
                        soft[type].push_back(new ConstraintShort(type, penalty, c1, limit, limit1));
                    } else {
                        std::vector<ConstraintShort *> t;
                        t.push_back(new ConstraintShort(type, penalty, c1, limit, limit1));
                        soft.insert(std::pair<std::string, std::vector<ConstraintShort *>>(type, t));
                    }

                }*/

            }
            instance->setDist(hard);
            //instance->setSoft(soft);
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

        void genConstraint() {
            //printRoomCluster();
            if (instance->getDist().find("MaxBlock") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["MaxBlock"].size(); ++y) {
                    block(instance->getDist()["MaxBlock"].at(y)->getClasses(),
                          instance->getDist()["MaxBlock"].at(y)->getParameter1(),
                          instance->getDist()["MaxBlock"].at(y)->getParameter2(),
                          instance->getDist()["MaxBlock"].at(y)->getWeight());
                }
            }
            if (instance->getDist().find("SameAttendees") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameAttendees"].size(); ++y) {
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["SameAttendees"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 < instance->getDist()["SameAttendees"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameAttendees"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameAttendees"].at(y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    int travel = 0;
                                    if (idClassesDist->getPossiblePairRoom(p)->getId() ==
                                        idClassesDist1->getPossiblePairRoom(p1)->getId() &&
                                        idClassesDist1->getPossiblePairRoom(p1)->getId() != -1)
                                        continue;
                                    if (idClassesDist->getPossiblePairRoom(p)->getId() != -1 &&
                                        idClassesDist1->getPossiblePairRoom(p1)->getId() != -1) {
                                        if (instance->getRoom(
                                                idClassesDist->getPossiblePairRoom(p)->getId())->getTravel(
                                                idClassesDist1->getPossiblePairRoom(p1)->getId()) > 0)
                                            travel = instance->getRoom(
                                                    idClassesDist->getPossiblePairRoom(p)->getId())->getTravel(
                                                    idClassesDist1->getPossiblePairRoom(p1)->getId());
                                        else
                                            travel = instance->getRoom(
                                                    idClassesDist1->getPossiblePairRoom(
                                                            p1)->getId())->getTravel(
                                                    idClassesDist->getPossiblePairRoom(p)->getId());
                                    }

                                    if (idClassesDist->getPossiblePairLecture(p)->getEnd() + travel <=
                                        idClassesDist1->getPossiblePairLecture(p1)->getStart()
                                        || idClassesDist1->getPossiblePairLecture(p1)->getEnd() +
                                           travel <= idClassesDist->getPossiblePairLecture(p)->getStart()
                                        || stringcompare(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                         idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                         instance->getNweek(), false)
                                        || stringcompare(idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                         idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                         instance->getNdays(), false)) { ;
                                    } else {
                                        int w = 0;
                                        if ((w = instance->getDist()["SameAttendees"].at(y)->getWeight()) == -1)
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
            if (instance->getDist().find("NotOverlap") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["NotOverlap"].size(); ++y) {
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["NotOverlap"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["NotOverlap"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["NotOverlap"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["NotOverlap"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (stringcompare(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                      idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                      instance->getNweek(), false) ==
                                        1
                                        || stringcompare(idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                         idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                         instance->getNdays(), false) ==
                                           1 || idClassesDist->getPossiblePairLecture(p)->getEnd() <=
                                                idClassesDist1->getPossiblePairLecture(p1)->getStart() ||
                                        idClassesDist1->getPossiblePairLecture(p1)->getEnd() <=
                                        idClassesDist->getPossiblePairLecture(p)->getStart()) { ;
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
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["Overlap"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["Overlap"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["Overlap"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["Overlap"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (stringcompare(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                      idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                      instance->getNweek(), false) ==
                                        1
                                        || stringcompare(idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                         idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                         instance->getNdays(), false) ==
                                           1 || idClassesDist->getPossiblePairLecture(p)->getEnd() <=
                                                idClassesDist1->getPossiblePairLecture(p1)->getStart() ||
                                        idClassesDist1->getPossiblePairLecture(p1)->getEnd() <=
                                        idClassesDist->getPossiblePairLecture(p)->getStart()) {
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
            if (instance->getDist().find("SameTime") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameTime"].size(); ++y) {
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["SameTime"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameTime"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameTime"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameTime"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {

                                    if (idClassesDist->getPossiblePairLecture(p)->getStart() <=
                                        idClassesDist1->getPossiblePairLecture(p1)->getStart()
                                        && idClassesDist1->getPossiblePairLecture(p1)->getEnd() <=
                                           idClassesDist->getPossiblePairLecture(p)->getEnd()) { ;
                                    } else if (idClassesDist1->getPossiblePairLecture(p1)->getStart() <=
                                               idClassesDist->getPossiblePairLecture(p)->getStart()
                                               && idClassesDist->getPossiblePairLecture(p)->getEnd() <=
                                                  idClassesDist1->getPossiblePairLecture(p1)->getEnd()) { ;
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
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["SameRoom"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameRoom"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameRoom"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameRoom"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (idClassesDist->getPossiblePairRoom(p)->getId() !=
                                        idClassesDist1->getPossiblePairRoom(p1)->getId()) {
                                        int w = 0;
                                        if ((w = instance->getDist()["SameRoom"].at(y)->getWeight()) == -1)
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
            if (instance->getDist().find("DifferentDays") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["DifferentDays"].size(); ++y) {
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["DifferentDays"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["DifferentDays"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["DifferentDays"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["DifferentDays"].at(
                                    y)->getClasses()[ci1];

                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    for (int d = 0; d < instance->getNdays(); ++d) {
                                        if (idClassesDist->getPossiblePairLecture(p)->getDays()[d] ==
                                            idClassesDist1->getPossiblePairLecture(p1)->getDays()[d] &&
                                            idClassesDist->getPossiblePairLecture(p)->getDays()[d] == '1') {
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
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["MinGap"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["MinGap"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["MinGap"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["MinGap"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {

                                    if (stringcompare(idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                      idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                      instance->getNweek(), false) == 1 ||
                                        stringcompare(idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                      idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                      instance->getNdays(), false) == 1 ||
                                        (idClassesDist1->getPossiblePairLecture(p1)->getEnd() +
                                         instance->getDist()["MinGap"].at(y)->getParameter1()) <=
                                        idClassesDist->getPossiblePairLecture(p)->getStart() ||
                                        (idClassesDist->getPossiblePairLecture(p)->getEnd() +
                                         instance->getDist()["MinGap"].at(y)->getParameter1()) <=
                                        idClassesDist1->getPossiblePairLecture(p1)->getStart()) { ;
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
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["SameDays"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameDays"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameDays"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameDays"].at(y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (stringcontains(idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                       idClassesDist->getPossiblePairLecture(p)->getDays(),
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
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["SameWeeks"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameWeeks"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameWeeks"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameWeeks"].at(y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (stringcontains(idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                       idClassesDist->getPossiblePairLecture(p)->getWeeks(),
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
            if (instance->getDist().find("SameStart") != instance->getDist().end()) {
                for (int y = 0; y < instance->getDist()["SameStart"].size(); ++y) {
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["SameStart"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["SameStart"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["SameStart"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["SameStart"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (idClassesDist1->getPossiblePairLecture(p1)->getStart() !=
                                        idClassesDist->getPossiblePairLecture(p)->getStart()) {
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
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["Precedence"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["Precedence"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["Precedence"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["Precedence"].at(
                                    y)->getClasses()[ci1];

                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (isFirst(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                instance->getNweek()) == -1);
                                    else if (isFirst(idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                     idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                     instance->getNdays()) ==
                                             -1 && isFirst(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                           idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                           instance->getNweek()) == 0);
                                    else if (isFirst(idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                     idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                     instance->getNdays()) ==
                                             0 && isFirst(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                          idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                          instance->getNweek()) == 0 &&
                                             idClassesDist->getPossiblePairLecture(p)->getEnd() <=
                                             idClassesDist1->getPossiblePairLecture(p1)->getEnd());
                                    else {
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
                    Class *idClassesDist, *idClassesDist1;
                    for (int ci = 0;
                         ci < instance->getDist()["WorkDay"].at(y)->getClasses().size(); ++ci) {
                        for (int ci1 = ci + 1;
                             ci1 <
                             instance->getDist()["WorkDay"].at(y)->getClasses().size(); ++ci1) {

                            idClassesDist = instance->getDist()["WorkDay"].at(y)->getClasses()[ci];
                            idClassesDist1 = instance->getDist()["WorkDay"].at(
                                    y)->getClasses()[ci1];
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    if (stringcompare(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                      idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                      instance->getNweek(),
                                                      false) == 1 ||
                                        stringcompare(idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                      idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                      instance->getNdays(),
                                                      false) == 1) { ;
                                    } else {
                                        if (std::max(idClassesDist->getPossiblePairLecture(p)->getEnd(),
                                                     idClassesDist1->getPossiblePairLecture(p1)->getEnd()) -
                                            std::min(idClassesDist->getPossiblePairLecture(p)->getStart(),
                                                     idClassesDist1->getPossiblePairLecture(p1)->getStart()) >
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


            for (std::pair<int, Room *> pairr: instance->getRooms()) {
                Room *r = pairr.second;
                for (int timei = 0; timei < r->t.size(); ++timei) {
                    Time *time1 = r->t[timei];
                    for (int timei1 = timei + 1; timei1 < r->t.size(); ++timei1) {
                        Time *time2 = r->t[timei1];
                        if (time1->check(time2, instance->getNweek(),
                                         instance->getNdays())) {
                            for (int con = 0; con < time1->getClassesC().size(); ++con) {
                                for (int cla = 0;
                                     cla < time2->getClassesC().size(); ++cla) {
                                    if (time1->getClassesC()[con] !=
                                        time2->getClassesC()[cla]) {
                                        PB *pb = new PB();
                                        pb->addProduct(mkLit(getVariableID(

                                                time1->getClassesC()[con])),
                                                       1);
                                        pb->addProduct(mkLit(getVariableID(

                                                time2->getClassesC()[cla])),
                                                       1);
                                        pb->_sign = true;
                                        pb->addRHS(1);

                                        maxsat_formula->addPBConstraint(pb);
                                        delete pb;
                                    }
                                }
                            }
                        }
                    }

                    for (int con = 0; con < time1->getClassesC().size(); ++con) {
                        for (int cla = con + 1; cla < time1->getClassesC().size(); ++cla) {
                            if (time1->getClassesC()[con] != time1->getClassesC()[cla]) {
                                PB *pb = new PB();
                                pb->addProduct(
                                        mkLit(getVariableID(time1->getClassesC()[con])),
                                        1);
                                pb->addProduct(
                                        mkLit(getVariableID(time1->getClassesC()[cla])),
                                        1);
                                pb->_sign = true;
                                pb->addRHS(1);


                                maxsat_formula->addPBConstraint(pb);
                                delete pb;

                            }

                        }

                    }

                }
            }

        }


        void constraint(Class *idClassesDist, Class *idClassesDist1, int p, int p1) {
            vec <Lit> l;
            l.push(~mkLit(getVariableID(
                    idClassesDist->getKey(idClassesDist->getPossiblePairRoom(p),
                                          idClassesDist->getPossiblePairLecture(p)))));
            l.push(~mkLit(getVariableID(
                    idClassesDist1->getKey(idClassesDist1->getPossiblePairRoom(p1),
                                           idClassesDist1->getPossiblePairLecture(
                                                   p1)))));

            maxsat_formula->addHardClause(l);

        }

        void constraintSoft(Class *idClassesDist, Class *idClassesDist1, int p, int p1, int pen) {
            if (optSoft) {
                vec <Lit> l;
                l.push(~mkLit(getVariableID(
                        idClassesDist->getKey(idClassesDist->getPossiblePairRoom(p),
                                              idClassesDist->getPossiblePairLecture(p)))));
                l.push(~mkLit(getVariableID(
                        idClassesDist1->getKey(idClassesDist1->getPossiblePairRoom(p1),
                                               idClassesDist1->getPossiblePairLecture(
                                                       p1)))));

                maxsat_formula->addSoftClause(pen * instance->getDistributionPen(), l);
            }
        }


        int getVariableID(std::string varName) {
            char *cstr = new char[varName.length() + 1];
            strcpy(cstr, varName.c_str());
            int id = maxsat_formula->varID(cstr);
            if (id == var_Undef)
                id = maxsat_formula->newVarName(cstr);
            delete[] cstr;
            return id;
        }


        void sameTime() {
            for (std::pair<std::string, std::pair<Course *, Course *>> pair: pairCourse) {
                for (Class *aClass:pair.second.first->getClasses()) {
                    for (Class *aClass1:pair.second.second->getClasses()) {
                        vec <Lit> literal;
                        Class *bClass0 = (aClass->getId() > aClass1->getId() ? aClass1 : aClass);
                        Class *bClass1 = (aClass->getId() < aClass1->getId() ? aClass : aClass1);

                        if (pairConf.find(std::to_string(bClass0->getId()) + "_" + std::to_string(bClass1->getId())) !=
                            pairConf.end()) {
                            for (std::pair<int, int> conf: pairConf[std::to_string(bClass0->getId()) + "_" +
                                                                    std::to_string(bClass1->getId())]) {
                                vec <Lit> l;

                                l.push(~mkLit(getVariableID(bClass0->getKey(bClass0->getPossiblePairRoom(conf.first),
                                                                            bClass0->getPossiblePairLecture(
                                                                                    conf.first)))));
                                l.push(~mkLit(getVariableID("sameTimeperSlot_" + std::to_string(bClass0->getId()) + "_"
                                                            + std::to_string(bClass1->getId()) + "_" +
                                                            std::to_string(conf.first) + "-" +
                                                            std::to_string(conf.second))));
                                maxsat_formula->addHardClause(l);
                                vec <Lit> l1;
                                l1.push(~mkLit(getVariableID(bClass1->getKey(bClass1->getPossiblePairRoom(conf.second),
                                                                             bClass1->getPossiblePairLecture(
                                                                                     conf.second)))));
                                l1.push(~mkLit(getVariableID("sameTimeperSlot_" + std::to_string(bClass0->getId()) + "_"
                                                             + std::to_string(bClass1->getId()) + "_" +
                                                             std::to_string(conf.first) + "-" +
                                                             std::to_string(conf.second))));
                                maxsat_formula->addHardClause(l1);
                                literal.push(
                                        mkLit(getVariableID("sameTimeperSlot_" + std::to_string(bClass0->getId()) + "_"
                                                            + std::to_string(bClass1->getId()) + "_" +
                                                            std::to_string(conf.first) + "-" +
                                                            std::to_string(conf.second))));

                            }

                        }
                        if (literal.size() > 0) {
                            literal.push(~mkLit(getVariableID("sameTime" + std::to_string(bClass0->getId()) + "_"
                                                              + std::to_string(bClass1->getId()))));
                            maxsat_formula->addHardClause(literal);
                        }
                    }
                }
            }




        }


        void genStudents() {
            requiredClasses();
            limit();
            parentChild();
            if (optStud) {
                sameTime();
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
                        if(instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig()>1)
                            l0.push(mkLit(getVariableID(
                                "conf" + std::to_string(s) + "_" + std::to_string(conf) + "_" +
                                instance->getClusterStudent()[s]->getCourses()[c]->getName())));
                        std::vector<Class *> partV;
                        for (int part = 0; part < instance->getClusterStudent()[s]->getCourses()[c]->getSubpart(
                                conf).size(); ++part) {
                            vec <Lit> l1;
                            if(instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig()>1)
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
                                    if(instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig()>1)
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
                        //


                    }
                    if(instance->getClusterStudent()[s]->getCourses()[c]->getNumConfig()>1)
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
                            //for (Student s: instance->getClusterStudent()[i]->getStudent()) {
                            pb->addProduct(mkLit(getVariableID(
                                    instance->getClusterStudent()[i]->getClassesID(
                                            instance->getClasses()[cla]->getId()))),
                                           instance->getClusterStudent()[i]->getStudent().size());
                            stuNumb += instance->getClusterStudent()[i]->getStudent().size();
                            //}
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


        void conflicts() {

            for (int i = 0; i < 1; ++i) {
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
                                int bClass0 = (instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId() > instance->getClusterStudent()[i]->getClasses()[it][c]->getId() ? instance->getClusterStudent()[i]->getClasses()[it][c]->getId()
                                                                                     : instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId());
                                int bClass1 = (instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId() < instance->getClusterStudent()[i]->getClasses()[it][c]->getId() ? instance->getClusterStudent()[i]->getClasses()[it1][c1]->getId() : instance->getClusterStudent()[i]->getClasses()[it][c]->getId());

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



        void block(std::vector<Class *> cont, int R, int S, double weight) {
            std::list<Block *> blocks = genPair(cont);
            for (Block *b :blocks) {
                Block * blocks1 = makeBlock(S, b);
                if(blocks1->vars.size()>R){
                    vec <Lit> l;
                    for (Block *b1 :blocks1->vars) {
                        std::cout<<b1->vars[0]->getName()<<std::endl;
                        l.push(~mkLit(getVariableID((b1->vars[0]->getName()))));
                    }
                    if(weight==-1)
                        maxsat_formula->addHardClause(l);
                    else
                        maxsat_formula->addSoftClause(blocks1->vars.size()-R,l);
                }

            }

        }

        std::list<Block *> genPair(std::vector<Class *> cont) {
            std::list<Block *> blocks;
            for (Class *c :cont) {
                if (blocks.size() != 0) {
                    for (std::list<Block *>::iterator it = blocks.begin(); it != blocks.end(); ++it) {
                        for (int i = 0; i < c->getPossiblePairSize(); ++i) {
                            (*it)->add(new Vars(c->getKey(
                                    c->getPossiblePair(
                                            i).first,
                                    c->getPossiblePair(
                                            i).second), c->getPossiblePair(
                                    i).second->getStart(), c->getPossiblePair(
                                    i).second->getEnd()));
                        }
                    }

                } else {
                    for (int i = 0; i < c->getPossiblePairSize(); ++i) {
                        blocks.push_back(new Vars(c->getKey(
                                c->getPossiblePair(
                                        i).first,
                                c->getPossiblePair(
                                        i).second), c->getPossiblePair(
                                i).second->getStart(), c->getPossiblePair(
                                i).second->getEnd()));
                    }
                }

            }
            return blocks;

        }

        Block * makeBlock(int S, Block * block) {
            bool change = true;
            while (change) {
                change = false;
                for (int i=0; i< block->vars.size();i++) {
                    for (int y=i+1;y<block->vars.size();y++) {
                        if (merge(block->vars[i],i, block->vars[y], y,S,block))
                            change = true;

                    }

                }
            }
            return block;
        }

        bool merge(Block *b1, int b1i,Block *b2,int b2i, int S, Block* blocks) {
            if ((b2->start - b1->end < S && b1->end < b2->start) ||
                (b1->start - b2->end < S && b2->end < b1->start)) {
                blocks->vars.erase(blocks->vars.begin()+b1i);
                if(b1i<b2i)
                    b2i--; //correct pointer
                blocks->vars.erase(blocks->vars.begin()+b2i);
                blocks->vars.push_back(
                        new Block(std::min(b2->start, b1->start), std::max(b2->end, b1->end), b1->vars, b2->vars));
                return true;
            }
            return false;
        }

    protected:

        Instance *instance;

        std::map<std::string, std::pair<Course *, Course *>> pairCourse;
        std::map<std::string, std::vector<std::pair<int, int>>> pairConf;
        std::map<int, std::vector<Time *>> times;


        MaxSATFormula *maxsat_formula;
        std::vector<ClusterbyRoom *> cluster;
    };


} // namespace openwbo

#endif // __PB_PARSER__

/*****************************************************************************/
