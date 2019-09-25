

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


    class ParserXML {

    public:
        //-------------------------------------------------------------------------
        // Constructor/destructor.
        //-------------------------------------------------------------------------

        ParserXML(MaxSATFormula *maxsat_formula) : maxsat_formula(maxsat_formula){

        }

        virtual ~ParserXML() {}


        void parse(std::string fileName) {
            PBObjFunction *of = new PBObjFunction();
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
                                                         idsub + "_" + itos(idConf) + "_" + id);
                                    c->setOrderID(order);
                                    c->setCourseID(atoi(id));
                                    order++;
                                    all.push_back(c);
                                    PB *p = new PB();
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


                                            if (roomsv.size() == 0) {


                                                c->setPossiblePair(
                                                        new Room(-1),
                                                        l, max);
                                                p->addProduct(mkLit(getVariableID("x"+std::to_string(max))), 1);
                                                //oneEach += " +1 " + std::to_string(max);
                                                if (penalty != 0)
                                                    of->addProduct(mkLit(getVariableID("x"+std::to_string(max))), instance->getTimePen() * penalty);
                                                max++;


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
                                                        p->addProduct(mkLit(getVariableID("x"+std::to_string(max))), 1);


                                                        if (penalty != 0)
                                                            of->addProduct(mkLit(getVariableID("x"+std::to_string(max))), instance->getTimePen() * penalty);

                                                        if (j->second != 0)
                                                            of->addProduct(mkLit(getVariableID("x"+std::to_string(max))), instance->getRoomPen() * j->second);
                                                        max++;


                                                        c->setPossiblePair(
                                                                j->first,
                                                                l, max);
                                                        bool is = true;
                                                        for (int ti = 0; ti < j->first->t.size(); ++ti) {
                                                            if (j->first->t[ti]->checkWD(l, instance->getNweek(),
                                                                                         instance->getNdays()) &&
                                                                l->getStart() == j->first->t[ti]->getStart() &&
                                                                l->getEnd() == j->first->t[ti]->getEnd()) {
                                                                j->first->t[ti]->addC(max, idclass);
                                                                is = false;
                                                            }
                                                        }
                                                        if (is) {
                                                            j->first->t.push_back(
                                                                    new Time(l->getStart(), l->getEnd(),
                                                                             l->getWeeks(),
                                                                             l->getDays(), max, idclass));
                                                        }
                                                    }

                                                }
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
                                    p->addRHS(1);
                                    PB *p2 = new PB(p->_lits, p->_coeffs, p->_rhs, true);
                                    maxsat_formula->addPBConstraint(p);
                                    maxsat_formula->addPBConstraint(p2);
                                    delete p;
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
                            if(all[k]->getPossibleRooms().size()>0) {
                                std::set<Room *> temp1;
                                for (std::pair<Room *, int> it : all[k]->getPossibleRooms()) {
                                    temp1.insert(it.first);
                                }
                                clu =new ClusterbyRoom(cluster.size(), temp1, all[k]);
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

            maxsat_formula->addObjFunction(of);
            delete of;




        }

        void createCurriculum(std::map<int, std::set<ClusterbyRoom *> *> &curMap) const {
            std::vector<Curriculum *> problem;
            //printRoomCluster();
            for (auto it = curMap.begin(); it != curMap.end(); ++it) {
                if (problem.size() == 0)
                    problem.push_back(new Curriculum(*it->second));
                else {
                    bool test = false;
                    for (int i = 0; i < problem.size(); ++i) {
                        for (ClusterbyRoom *c:problem[i]->getPClass()) {
                            for (ClusterbyRoom *c1: *it->second) {
                                for (Class *aClass: c->getClasses()) {
                                    for (Class *aClass1: c1->getClasses()) {
                                        if (aClass->getId() == aClass1->getId()) {
                                            test = true;
                                            break;
                                        }
                                    }
                                    if (test)
                                        break;

                                }
                                if (!test) {
                                    for (Room *aRoom:c->getRooms()) { //TODO:ADDclases
                                        for (Room *aRoom1: c1->getRooms()) {
                                            if (aRoom->getId() == aRoom1->getId()) {
                                                test = true;
                                                break;
                                            }
                                        }
                                        if (test)
                                            break;

                                    }
                                } else
                                    break;
                            }
                            if (test)
                                break;
                        }
                        if (test)
                            break;


                    }


                    if (!test)
                        problem.push_back(new Curriculum(*it->second));
                }

            }
            instance->setProblem(problem);
        }

        void readStudents(const xml_node<> *n) const {
            std::map<int, Student> std;
            int idc = 0;
            std::vector<ClusterStudent> clusterStudent;
            for (const xml_node<> *sub = n->first_node(); sub; sub = sub->next_sibling()) {
                int studentID = -1;
                for (const xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                    studentID = atoi(a->value());
                }
                std::vector<Course *> c;
                for (const xml_node<> *course = sub->first_node(); course; course = course->next_sibling()) {
                    std::string courseIDstd = "";
                    for (const xml_attribute<> *a = course->first_attribute(); a; a = a->next_attribute()) {
                        courseIDstd = a->value();
                    }
                    c.push_back(instance->getCourse(courseIDstd));
                }
                std::pair<int, Student> s = std::pair<int, Student>(studentID, Student(studentID, c));
                std.insert(s);


                bool temp = false;
                if (clusterStudent.size() == 0)
                    clusterStudent.push_back(ClusterStudent(idc++, c, s.second));
                for (int i = 0; i < clusterStudent.size(); ++i) {
                    bool t = false;
                    if (c.size() != clusterStudent[i].getCourses().size())
                        continue;
                    for (int c0 = 0; c0 < c.size(); ++c0) {
                        t = false;
                        for (int c1 = 0; c1 < clusterStudent[i].getCourses().size(); ++c1) {
                            if (clusterStudent[i].getCourses()[c1]->getName().compare(
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
                        clusterStudent[i].addStudent(s.second);
                    }
                }
                if (!temp)
                    clusterStudent.push_back(ClusterStudent(idc++, c, s.second));


            }


            instance->setStudentCluster(clusterStudent);
            instance->setStudent(std);
        }

        void readConstraints(std::map<Class *, ClusterbyRoom *> mapCluster,
                             std::map<int, std::set<ClusterbyRoom *> *> &curMap, const xml_node<> *n) const {
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
                if (isReq) {
                    for (ClusterbyRoom *clusterbyRoom: *c) {
                        clusterbyRoom->setRange(new ConstraintShort(type, penalty, c1, limit, limit1));
                        break;
                    }

                }

            }
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
                    std::cout << aRoom->getId() << std::endl;
                }
            }

        }

        Instance *getInstance() const {
            return instance;
        }


        void constraint(){

        }

        int getVariableID(std::string varName) {
            char *cstr = new char[varName.length() + 1];
            strcpy(cstr, varName.c_str());
            int id = maxsat_formula->varID(cstr);
            if (id == var_Undef)
                id = maxsat_formula->newVarName(cstr);
            delete [] cstr;
            return id;
        }


    protected:

        Instance *instance;
        MaxSATFormula *maxsat_formula;
        int max = 0;
        std::vector<ClusterbyRoom *> cluster;
    };

} // namespace openwbo

#endif // __PB_PARSER__

/*****************************************************************************/
