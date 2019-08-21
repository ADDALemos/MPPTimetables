#ifndef IL_STD
#define IL_STD
#endif

#include <ilconcert/iloenv.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include <list>
#include "problem/Lecture.h"
#include "problem/Room.h"
#include "problem/ClusterStudents.h"
#include "problem/distribution.h"
#include "rapidXMLParser/rapidxml.hpp"
#include "rapidXMLParser/rapidxml_print.hpp"
#include "problem/ClusterClassesbyRoom.h"
#include "problem/DistributionPenalty.h"
#include "problem/Limits.h"
#include "problem/WithLimit.h"
#include "problem/Instance.h"
#include "solver/ILPExecuter.h"
#include "randomGenerator/Perturbation.h"
#include "utils/Stats.h"
#include "utils/TimeUtil.h"
#include "solver/CplexSimple.h"
#include "problem/Time.h"
#include "problem/Curriculum.h"


using namespace rapidxml;

Instance *readInputXML(std::string filename);


void readOutputXML(std::string filename, Instance *instance);

void writeOutputXML(std::string filename, Instance *instance, double time);

void readPerturbations(std::string filename, Instance *instance);

void addPossibleRooms(Class *c, Instance *instance);


void printClusterofStudents(Instance *instance);

void printCurricular();

bool mergeStudent = true;
bool mergeRoom = false;
bool roomTime = !mergeRoom;

bool warm = false;
bool optRoom = false;
bool optTime = false;
bool optStu = false;

IloEnv env; //CPLEX execution
IloModel model = IloModel(env);
std::map<int, std::vector<IloBoolVar>> cplexMap;
std::vector<Curriculum*> problem;

std::map<int, std::vector<distribution *> *> classConst;
std::map<int, std::vector<distribution *> *> classSoft;

IloExpr costTime = IloNumExpr(env);

IloExpr costRoom = IloNumExpr(env);

/*void oldCurricuculum(Instance *instance, std::vector<Curriculum *, std::allocator<Curriculum *>> &sep) {
    for (int i = 0; i < instance->getClasses().size(); ++i) {
        if (sep.size() == 0) {
            std::__1::set<Room *> values;
            for (std::__1::pair<Room *, int> e :instance->getClasses()[i]->getPossibleRooms()) {
                values.insert(e.first);

            }
            Curriculum *cum =new Curriculum(instance->getClasses()[i], values);
            for (int m1 = 0; m1 < instance->getClasses()[i]->getHard().size(); ++m1) {
                for (int mc1 = 0;
                     mc1 < instance->getClasses()[i]->getHard()[m1]->getClasses().size(); ++mc1) {
                    Class * c = instance->getClass(instance->getClasses()[i]->getHard()[m1]->getClasses()[mc1]);
                    for (std::__1::pair<Room *, int> e :c->getPossibleRooms()) {
                        values.insert(e.first);
                    }
                    cum->addClass(c);
                }
            }
            sep.push_back(cum);

        }
        bool isss = true;
        for (int j = 0; j < sep.size(); ++j) {
            bool iss = false;
            for (Room *k: sep[j]->getPRoom()) {
                bool is = false;
                for (int l = 0; l < instance->getClasses()[i]->getPossibleRooms().size(); ++l) {
                    if (k->getId() ==
                        instance->getClasses()[i]->getPossibleRoom(l)->getId()) {
                        is = true;
                        break;
                    }
                }
                if (is) {
                    iss = true;
                    break;
                }
            }
            if(!iss) {
                for (int i1 = 0; i1 < sep[j]->getPClass().size(); ++i1) {
                    if (sep[j]->getPClass()[i1]->getId() == instance->getClasses()[i]->getId()) {
                        iss = true;
                        break;
                    }
                    bool is = false;
                    for (int m = 0; m < instance->getClasses()[i]->getHard().size(); ++m) {
                        for (int mc = 0; mc < instance->getClasses()[i]->getHard()[m]->getClasses().size(); ++mc) {
                            if (sep[j]->getPClass()[i1]->getId() ==
                                instance->getClasses()[i]->getHard()[m]->getClasses()[mc]) {
                                is = true;
                                break;


                            }
                            for (int m1 = 0; m1 < sep[j]->getPClass()[i1]->getHard().size(); ++m1) {
                                for (int mc1 = 0;
                                     mc1 < sep[j]->getPClass()[i1]->getHard()[m1]->getClasses().size(); ++mc1) {
                                    if (sep[j]->getPClass()[i1]->getHard()[m1]->getClasses()[mc1] ==
                                        instance->getClasses()[i]->getHard()[m]->getClasses()[mc]) {
                                        is = true;
                                        break;
                                    }
                                    if (sep[j]->getPClass()[i1]->getHard()[m1]->getClasses()[mc1] ==
                                        instance->getClasses()[i]->getId()) {
                                        is = true;
                                        break;


                                    }
                                }
                                if (is) {
                                    iss = true;
                                    break;
                                }

                            }
                            if (is) {
                                iss = true;
                                break;
                            }
                        }
                        if (is) {
                            iss = true;
                            break;
                        }

                    }
                    if (is) {
                        iss = true;
                        break;
                    }
                }
            }
            if (iss) {
                isss = false;
                std::__1::set<Room *> values;
                for (std::__1::pair<Room *, int> e :instance->getClasses()[i]->getPossibleRooms()) {
                    values.insert(e.first);

                }
                sep[j]->addClass(instance->getClasses()[i]);
                for (int m1 = 0; m1 < instance->getClasses()[i]->getHard().size(); ++m1) {
                    for (int mc1 = 0;
                         mc1 < instance->getClasses()[i]->getHard()[m1]->getClasses().size(); ++mc1) {
                        Class *c = instance->getClass(instance->getClasses()[i]->getHard()[m1]->getClasses()[mc1]);
                        for (std::__1::pair<Room *, int> e :c->getPossibleRooms()) {
                            values.insert(e.first);
                        }
                        sep[j]->addClass(c);
                    }
                }
                sep[j]->addRooms(values);
                break;
            }
        }
        if (isss) {

            std::__1::set<Room *> values;
            for (std::__1::pair<Room *, int> e :instance->getClasses()[i]->getPossibleRooms()) {
                values.insert(e.first);

            }
            sep.push_back(new Curriculum(instance->getClasses()[i], values));

        }
    }
}

void printCurriculum(const std::vector<Curriculum *, std::allocator<Curriculum *>> &sep) {
    std::__1::cout << "a" << sep.size() << " " << sep[0]->getPClass().size() << std::__1::endl;
    for (int m = 0; m < sep.size(); ++m) {
        std::__1::cout << "ClassbyRoom " << m << std::__1::endl;
        for (int i = 0; i < sep[m]->getPClass().size(); ++i) {
            std::__1::cout << sep[m]->getPClass()[i]->getId() << std::__1::endl;
        }
        std::__1::cout << "room" << std::__1::endl;
        for (Room *k: sep[m]->getPRoom()) {
            std::__1::cout << k->getId() << std::__1::endl;
        }

    }
}*/

int main(int argc, char **argv) {


    if (!quiet)
        std::cout << "Starting Reading File: "
                  << "/Volumes/MAC/ClionProjects/timetabler/data/input/ITC-2019/iku-fal17.xml" << std::endl;

    Instance *instance = readInputXML("/Volumes/MAC/ClionProjects/timetabler/data/input/ITC-2019/iku-fal17.xml");
    std::cout << instance->getName() <<" "<< problem.size()<<std::endl;
    printCurricular();
    std::exit(0);

  //  std::vector<Curriculum *> sep;
 //   oldCurricuculum(instance, sep);
//    printCurriculum(sep);


    if (!quiet) printClusterofStudents(instance);


    std::cout << "PRE " << getTimeSpent() << std::endl;
    ILPExecuter *runner = new CplexSimple(instance, model, env, cplexMap);
    std::cout << "AUX " << getTimeSpent() << std::endl;
    runner->dist(true);
    std::cout << "DIST " << getTimeSpent() << std::endl;
    runner->run2019(false);
    writeOutputXML("data/output/ITC-2019/" + instance->getName() +
                   ".xml",
                   instance, getTimeSpent());

    std::exit(1);

    printProblemStats(instance);


    if (!quiet) std::cout << getTimeSpent() << std::endl;
    /*if (!quiet) std::cout << "Compacting " << std::endl;
    instance->compact();
    MIXEDCplexExecuter m = MIXEDCplexExecuter();
    m.setInstance(instance);
    m.run(false);
    std::exit(0);
    if (!quiet) std::cout << getTimeSpent() << std::endl;
    ILPExecuter*         runner = new MixedModelGurobiExecuter((bool) std::stoi(argv[6]), (bool) std::stoi(argv[7]), instance);
    genSingleShot(instance, runner, argv[4]);
             std::exit(42);  */

    //auto *s = new LocalSearch(10000000000, .6, instance, 36000000000);
    //s->GRASP();
    std::list<Class *> classesbyCost;
    for (int j = 0; j < instance->getClasses().size(); ++j) {
        /*if( instance->getClasses()[j]->getLectures().size() > 1 || instance->getClasses()[j]->getPossibleRooms().size() > 1) {
            int cost = 0;
            for (int i = 0; i < instance->getClasses()[j]->getHard().size(); ++i) {
                for (int m = 0; m < instance->getClasses()[j]->getHard()[i]->getClasses().size(); ++m) {
                    for (int k = 0; k < instance->getClasses()[j]->getLectures().size(); ++k) {
                        for (int l = 0; l < instance->getClass(
                                instance->getClasses()[j]->getHard()[i]->getClasses()[m])->getLectures().size(); ++l) {
                            if (instance->getClasses()[j]->getLectures()[k]->getStart() == instance->getClass(
                                    instance->getClasses()[j]->getHard()[i]->getClasses()[m])->getLectures()[l]->getStart())
                                cost++;
                        }
                    }
                }

            }
            instance->getClasses()[j]->setCostG(cost / instance->getClasses()[j]->getLectures().size());
            classesbyCost.push_back(instance->getClasses()[j]);
        }
        lhs->getCostG() > rhs->getCostG()*/
        classesbyCost.push_back(instance->getClasses()[j]);

    }
    std::cout << classesbyCost.size() << std::endl;
    classesbyCost.sort([](Class *lhs, Class *rhs) {
        return lhs->getPossibleRooms().size() + lhs->getLectures().size() <
               rhs->getPossibleRooms().size() + rhs->getLectures().size();
    });

    if (!quiet) std::cout << getTimeSpent() << std::endl;



    //if (!quiet) std::cout << "Starting Reading File: " << argv[2] << std::endl;
    //readOutputXML(argv[2], instance);
    //if (!quiet) std::cout << "Generating Perturbations based on the file: " << argv[3] << std::endl;
    //readPerturbations(argv[3], instance);


    if (!quiet) std::cout << "Generating ILP model" << std::endl;
    //ILPExecuter *runner;



    if (!quiet) std::cout << "Solution Found: Writing output file" << std::endl;
    writeOutputXML("/Volumes/MAC/ClionProjects/timetabler/data/output/ITC-2019/" + instance->getName() + ".xml",
                   instance, getTimeSpent());


    return 0;
}

void printCurricular() {
    for(Curriculum *c: problem){
        std::__1::cout << "New" << std::__1::endl;
        for (ClusterbyRoom* clus: c->getPClass()) {
            std::__1::cout << "C";
            for (Class* c: clus->getClasses()) {
                std::__1::cout << c->getId() << ",";
            }
            std::__1::cout << std::__1::endl;
            std::__1::cout << "R";
            for (Room* c: clus->getRooms()) {
                std::__1::cout << c->getId() << ",";
            }
            std::__1::cout << std::__1::endl;

        }
    }
}

void printClusterofStudents(Instance *instance) {
    for (int l = 0; l < instance->getClusterStudent().size(); ++l) {
        std::cout << "NEW CLUSTER " << instance->getClusterStudent()[l].getId() << std::endl;
        for (int i = 0; i < instance->getClusterStudent()[l].getStudent().size(); ++i) {
            std::cout << instance->getClusterStudent()[l].getStudent()[i].getId() << " C ";
            for (int j = 0; j < instance->getClusterStudent()[l].getCourses().size(); ++j) {
                std::cout << instance->getClusterStudent()[l].getCourses()[j]->getName() << " ";
            }
            std::cout << std::endl;

        }

    }
}


void readPerturbations(std::string filename, Instance *instance) {
    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "File not found: " + filename << std::endl;
        std::cerr << "Method: readPerturbations" << std::endl;
        std::exit(11);
    }
    std::string perturbation;
    Perturbation *p = new Perturbation();
    double percentage, mean, std;
    while (file >> perturbation >> percentage) {
        percentage = percentage / 100;
        if (std::strcmp(perturbation.c_str(), "Preference_Time") == 0) {
            p->randomTime(instance, percentage, true);
        } else if (strcmp(perturbation.c_str(), "Invalid_Time") == 0) {
            p->randomSlotClose(instance, percentage);
        } else if (strcmp(perturbation.c_str(), "Preference_Room") == 0) {
            //p->randomRoom(instance, percentage, true);
        } else if (strcmp(perturbation.c_str(), "Invalid_Room") == 0) {
            p->randomCloseRoom(instance, percentage);
        } else if (strcmp(perturbation.c_str(), "Overlap") == 0) {
            p->randomOverlap(instance, percentage);
        } else if (strcmp(perturbation.c_str(), "Modify_Enrolments") == 0) {
            file >> mean >> std;
            p->randomEnrolmentChanges(instance, mean, std, percentage);
        } else if (strcmp(perturbation.c_str(), "Modify_N_Lectures") == 0) {
            file >> mean >> std;
            p->randomShiftChange(instance, percentage, mean, std);
            instance->setNumClasses();
        } else if (strcmp(perturbation.c_str(), "Curriculum") == 0) {
            file >> mean >> std;
            p->addNewCurriculum(instance, percentage, mean, std);
        }


    }
}


void writeOutputXML(std::string filename, Instance *instance, double time) {
    xml_document<> doc;
    xml_node<> *decl = doc.allocate_node(node_declaration);
    decl->append_attribute(doc.allocate_attribute("version", "1.0"));
    decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
    doc.append_node(decl);

    xml_node<> *root = doc.allocate_node(node_element, "solution");
    root->append_attribute(doc.allocate_attribute("name", instance->getName().c_str()));
    root->append_attribute(doc.allocate_attribute("runtime", std::to_string(time).c_str()));
    root->append_attribute(
            doc.allocate_attribute("cores", std::to_string(std::thread::hardware_concurrency()).c_str()));
    root->append_attribute(doc.allocate_attribute("technique", instance->getMethod().c_str()));
    root->append_attribute(doc.allocate_attribute("author", "Alexandre Lemos"));
    root->append_attribute(doc.allocate_attribute("institution", "INESC-ID"));
    root->append_attribute(doc.allocate_attribute("country", "Portugal"));
    doc.append_node(root);
    xml_node<> *child;
    for (int c = 0; c < instance->getClasses().size(); c++) {
        child = doc.allocate_node(node_element, "class");
        child->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                std::to_string(instance->getClasses()[c]->getId()).c_str())));
        child->append_attribute(
                doc.allocate_attribute("days", doc.allocate_string(instance->getClasses()[c]->getSolDays().c_str())));

        child->append_attribute(doc.allocate_attribute("start", doc.allocate_string(
                std::to_string(instance->getClasses()[c]->getSolStart() +
                               (instance->isisCompact() ? instance->minTimeSlot() : 0)).c_str())));
        child->append_attribute(
                doc.allocate_attribute("weeks", doc.allocate_string(instance->getClasses()[c]->getSolWeek().c_str())));
        if (instance->getClasses()[c]->getSolRoom() != -1)
            child->append_attribute(doc.allocate_attribute("room", doc.allocate_string(
                    instance->getRoom(instance->getClasses()[c]->getSolRoom())->getName().c_str())));
        for (int s = 0; s < instance->getClasses()[c]->getStudent().size(); ++s) {
            xml_node<> *student = doc.allocate_node(node_element, "student");
            student->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                    std::to_string(instance->getClasses()[c]->getStudent()[s]).c_str())));
            child->append_node(student);
        }
        root->append_node(child);

    }

    std::ofstream file_stored(filename);
    //print(std::cout, doc, 0);
    file_stored << doc;

    file_stored.close();
    doc.clear();

}

//Room mapping
std::map<std::string, int> roomID;

void readOutputXML(std::string filename, Instance *instance) {
    xml_document<> doc;
    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "File not found: " + filename << std::endl;
        std::cerr << "Method: readOutputXML" << std::endl;
        std::exit(11);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string content(buffer.str());
    doc.parse<0>(&content[0]);
    xml_node<> *pRoot = doc.first_node();
    for (const xml_attribute<> *a = pRoot->first_attribute(); a; a = a->next_attribute()) {
        if (strcmp(a->name(), "name") == 0 && strcmp(a->value(), instance->getName().c_str()) != 0) {
            std::cerr << "Instance and Solution do not match" << std::endl;
        }
    }
    int total = 0;
    for (const xml_node<> *n = pRoot->first_node(); n; n = n->next_sibling()) {
        std::string weeks = " ", days = " ", room = " ";
        int id = -1, start = -1;
        for (const xml_attribute<> *a = n->first_attribute(); a; a = a->next_attribute()) {
            if (strcmp(a->name(), "id") == 0) {
                id = atoi(a->value());
            } else if (strcmp(a->name(), "start") == 0) {
                start = atoi(a->value());
            } else if (strcmp(a->name(), "room") == 0) {
                room = a->value();
            } else if (strcmp(a->name(), "weeks") == 0) {
                weeks = a->value();
            } else if (strcmp(a->name(), "days") == 0) {
                days = a->value();
            }
        }
        Class *s = instance->getClassbyId(id);
        if (std::strcmp(room.c_str(), " ") != 0) {
            if (roomID.find(room) == roomID.end()) {
                std::cerr << "Room does not exist: " << room << std::endl;
                std::exit(11);
            }
            s->setSolution(start, roomID[room], room, weeks, days);
        } else
            s->setSolution(start, weeks, days);

        std::vector<int> student;
        for (const xml_node<> *stu = n->first_node(); stu; stu = stu->next_sibling()) {
            for (const xml_attribute<> *a = stu->first_attribute(); a; a = a->next_attribute()) {
                student.push_back(atoi(a->value()));
                instance->getStudent(atoi(a->value())).addClass(s);
            }
        }

        s->addStudents(student);
        total += s->getSteatedStudents();
    }
    instance->setTotalNumberSteatedStudent(total);
    instance->findOverlapConstraints();


}


Instance *readInputXML(std::string filename) {
    xml_document<> doc;
    IloRangeArray tempModel(env);
    int orderID = 0;
    std::map<int, Class *> classMap;
    std::map<Class*,ClusterbyRoom*> mapCluster;
    std::map<int, std::set<ClusterbyRoom*>*> curMap;

    std::map<int, int> classID;
    std::vector<ClusterbyRoom *> cluster;



    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "File not found: " + filename << std::endl;
        std::cerr << "Method: readInputXML" << std::endl;
        std::exit(11);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string content(buffer.str());
    doc.parse<0>(&content[0]);
    xml_node<> *pRoot = doc.first_node();
    Instance *instance;
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
    for (const xml_node<> *n = pRoot->first_node(); n; n = n->next_sibling()) {
        if (strcmp(n->name(), "optimization") == 0) {
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
        } else if (strcmp(n->name(), "rooms") == 0) {
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
                        //std::cout<<*u<<std::endl;
                        una.push_back(Unavailability(days, start, length, weeks));
                    }


                }
                if (roomID.find(id) == roomID.end()) {
                    int size = roomID.size();
                    size++;
                    roomID.insert(std::pair<std::string, int>(id, size));
                }

                //   std::cout<<*r<<std::endl;
                //      std::cout<<r->getSlots().size()<<std::endl;
                instance->addRoom(new Room(roomID[id], id, capacity, travel,
                                           una, type));
                //std::cout<<instance->getRooms().size()<<std::endl;
            }

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
                            Class *c = new Class(idclass, limit, orderID, idsub + "_" + itos(idConf) + "_" + id);
                            IloNumExpr oneEach = IloNumExpr(env);
                            c->setOrderID(order);
                            order++;
                            all.push_back(c);
                            int i = 0;
                            int v = 0;
                            for (const xml_node<> *lec = cla->first_node(); lec; lec = lec->next_sibling()) {
                                if (strcmp(lec->name(), "room") == 0) {
                                    std::string idRoom = " ";
                                    int penalty = -1;
                                    int rID = -1;
                                    for (const xml_attribute<> *a = lec->first_attribute(); a; a = a->next_attribute()) {
                                        //std::cout<<a->name()<<std::endl;
                                        if (strcmp(a->name(), "id") == 0) {
                                            idRoom = a->value();
                                            rID = atoi(a->value());
                                        } else if (strcmp(a->name(), "penalty") == 0)
                                            penalty = atoi(a->value());

                                    }
                                    if (roomID.find(idRoom) == roomID.end()) {
                                        std::cerr << "Room does not exist: " << idRoom << std::endl;
                                        std::exit(11);
                                    } else {



                                        roomsv.insert(
                                                std::pair<Room *, int>(instance->getRoom(roomID[idRoom]), penalty));

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
                                    //std::cout<<idclass<<" "<<lenght<<" "<<start<<std::endl;
                                    Lecture *l = new Lecture(lenght, start, weeks, days, penalty);
                                    lecv.push_back(l);


                                    if (roomsv.size() == 0) {
                                        IloBoolVar classCplexVar(env);
                                        if (cplexMap.find(c->getOrderID()) != cplexMap.end()) {
                                            cplexMap[c->getOrderID()].push_back(classCplexVar);
                                        } else {
                                            std::vector<IloBoolVar> classCplex;
                                            classCplex.push_back(classCplexVar);
                                            cplexMap.insert(std::pair<int, std::vector<IloBoolVar>>(c->getOrderID(),
                                                                                                    classCplex));

                                        }
                                        c->setPossiblePair(
                                                new Room(-1),
                                                l, v);
                                        oneEach += cplexMap[c->getOrderID()][v];
                                        costTime += cplexMap[c->getOrderID()][v] *
                                                    c->getPossiblePairLecture(v)->getPenalty();
                                        costRoom += cplexMap[c->getOrderID()][v] *
                                                    c->getPossibleRoomCost(c->getPossiblePairRoom(v));
                                        v++;

                                    } else {
                                        for (std::map<Room *, int>::iterator j = roomsv.begin();
                                             j != roomsv.end(); ++j) {
                                            int roomID = j->first->getId();
                                            std::string week = l->getWeeks();
                                            std::string day = l->getDays();
                                            int start = l->getStart();
                                            int duration = l->getLenght();
                                            bool isNotAddableTime = false;
                                            for (int una = 0; una < j->first->getSlots().size(); ++una) {
                                                for (int i = 0; i < instance->getNweek(); ++i) {
                                                    if (week[i] == j->first->getSlots()[una].getWeeks()[i] &&
                                                        j->first->getSlots()[una].getWeeks()[i] == '1') {
                                                        for (int d = 0; d < instance->getNdays(); ++d) {
                                                            if (day[d] == j->first->getSlots()[una].getDays()[d] &&
                                                                day[d] == '1') {

                                                                if (start >= j->first->getSlots()[una].getStart() &&
                                                                    start < j->first->getSlots()[una].getStart() +
                                                                            j->first->getSlots()[una].getLenght()) {

                                                                    isNotAddableTime = true;
                                                                } else if (
                                                                        j->first->getSlots()[una].getStart() >= start &&
                                                                        j->first->getSlots()[una].getStart() <
                                                                        start + duration) {


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
                                                c->setPossiblePair(
                                                        j->first,
                                                        l, v);
                                                if (roomTime) {
                                                    bool is = true;
                                                    for (int ti = 0; ti < j->first->t.size(); ++ti) {
                                                        if (l->getDays().compare(j->first->t[ti]->getDay()) == 0 &&
                                                            l->getWeeks().compare(j->first->t[ti]->getWeek()) == 0 &&
                                                            l->getStart() == j->first->t[ti]->getStart() &&
                                                            l->getEnd() == j->first->t[ti]->getEnd()) {
                                                            j->first->t[ti]->addS(c->getOrderID(), v);
                                                            is = false;
                                                        } else if (j->first->t[ti]->check(l, instance->getNweek(),
                                                                                          instance->getNdays())) {
                                                            j->first->t[ti]->addC(c->getOrderID(), v);
                                                            is = false;
//                                                                std::cout<<"L1 "<<*l<<" "<<c->getOrderID()<<std::endl;


                                                        }
                                                    }
                                                    if (is) {
//                                                            std::cout<<"L "<<*l<<" "<<c->getOrderID()<<std::endl;
                                                        j->first->t.push_back(
                                                                new Time(l->getStart(), l->getEnd(), l->getWeeks(),
                                                                         l->getDays(), c->getOrderID(), v));
                                                    }
                                                    /*
                                                    for (int i = 0; i < l->getLenght(); ++i) {
                                                        if (j->first->t.find(l->getStart() + i) !=
                                                            j->first->t.end()) {
                                                            j->first->t[
                                                                    l->getStart() + i].push_back(
                                                                    std::pair<int, int>(c->getOrderID(), v));
                                                        } else {
                                                            std::vector<std::pair<int, int>> temp;
                                                            temp.push_back(std::pair<int, int>(c->getOrderID(), v));
                                                            j->first->t.insert(
                                                                    std::pair<int, std::vector<std::pair<int, int>>>(
                                                                            l->getStart() + i, temp));
                                                        }

                                                    }*/
                                                }
                                                IloBoolVar classCplexVar(env);
                                                if (cplexMap.find(c->getOrderID()) != cplexMap.end()) {
                                                    cplexMap[c->getOrderID()].push_back(classCplexVar);
                                                } else {
                                                    std::vector<IloBoolVar> classCplex;
                                                    classCplex.push_back(classCplexVar);
                                                    cplexMap.insert(
                                                            std::pair<int, std::vector<IloBoolVar>>(c->getOrderID(),
                                                                                                    classCplex));

                                                }
                                                oneEach += cplexMap[c->getOrderID()][v];
                                                costTime += cplexMap[c->getOrderID()][v] *
                                                            c->getPossiblePairLecture(v)->getPenalty();
                                                costRoom += cplexMap[c->getOrderID()][v] *
                                                            c->getPossibleRoomCost(c->getPossiblePairRoom(v));
                                                v++;
                                                // std::cout<<instance->getClasses()[j]->getPossibleRoomPair(r).first<<" "<<instance->getClasses()[j]->getId()<<" "<<*l<<std::endl;
                                            }

                                        }
                                    }

                                }


                                //limit=atoi(a->value());


                            }

                            c->setPossibleRooms(roomsv);
                            c->setLectures(lecv);

                            classMap.insert(std::pair<int, Class *>(orderID, c));
                            classID.insert(std::pair<int, int>(idclass, orderID));
                            orderID++;
                            //addPossibleRooms(c, instance);
                            if (parent != -1)
                                c->setParent(classMap[classID[parent]]);
                            clasv.push_back(c);
                            tempModel.add(oneEach == 1);

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
            std::set<Room*> temp;
            for(std::pair<Room*,int> t: all[0]->getPossibleRooms()) {
                temp.insert(t.first);
            }
            cluster.push_back(new ClusterbyRoom(0,temp,all[0]));
            mapCluster.insert(std::pair<Class*,ClusterbyRoom*>(all[0],cluster[0]));
            for (int k = 1; k < all.size(); ++k) {
                    bool test=false;
                    for (int i = 0; i <cluster.size() ; ++i) {
                        for (Room *j:cluster[i]->getRooms()) {
                            if(all[k]->getPossibleRoom(j)!=-1){
                                test=true;
                                cluster[i]->addClass(all[k],-1);
                                if(all[k]->getPossibleRooms().size()>0) {
                                    for (std::pair<Room *, int> it : all[k]->getPossibleRooms()) {
                                        cluster[i]->addRoom(it.first);
                                    }
                                }
                                mapCluster.insert(std::pair<Class*,ClusterbyRoom*>(all[k],cluster[i]));
                                break;
                            }

                        }
                        if(test){
                            break;
                        }

                    }
                if(!test){
                    std::set<Room*> temp;
                    for (std::pair<Room *, int> it : all[k]->getPossibleRooms()) {
                        temp.insert(it.first);
                    }
                    cluster.push_back(new ClusterbyRoom(cluster.size(),temp,all[k]));
                    mapCluster.insert(std::pair<Class*,ClusterbyRoom*>(all[k],cluster[cluster.size()-1]));
                }

            }




        } else if (strcmp(n->name(), "distributions") == 0) {
            for (const xml_node<> *sub = n->first_node(); sub; sub = sub->next_sibling()) {
                bool isReq = false;
                std::string type;
                int penalty = -1;
                std::set<ClusterbyRoom*> *c=new std::set<ClusterbyRoom*>();
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
                for (const xml_node<> *course = sub->first_node(); course; course = course->next_sibling()) {
                    Class *idClassesDist = instance->getClass(atoi(course->first_attribute()->value()));
                    c->insert(mapCluster[idClassesDist]);
                    if(curMap.find(idClassesDist->getId())!=curMap.end()){
                        for(ClusterbyRoom* r: *c)
                            curMap[idClassesDist->getId()]->insert(r);
                        c=curMap[idClassesDist->getId()];
                    } else {
                        curMap.insert(std::pair<int,std::set<ClusterbyRoom*>*>(idClassesDist->getId(),c));
                    }
                    Class *idClassesDist1 = nullptr;
                    if (course->next_sibling())
                        idClassesDist1 = instance->getClass(atoi(course->next_sibling()->first_attribute()->value()));
                    if (isReq && type.compare("DifferentDays") == 0) {
                        if (course->next_sibling()) {
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    for (int i = 0; i < instance->getNdays(); ++i) {
                                        if (idClassesDist->getPossiblePairLecture(p)->getDays()[i] ==
                                            idClassesDist1->getPossiblePairLecture(p1)->getDays()[i] &&
                                            idClassesDist->getPossiblePairLecture(p)->getDays()[i] == '1')
                                            tempModel.add(cplexMap[idClassesDist->getOrderID()][p] +
                                                      cplexMap[idClassesDist1->getOrderID()][p1] <= 1);

                                    }


                                }
                            }
                        }
                    } else if (isReq && type.compare("DifferentDays") == 0) {
                        if (course->next_sibling()) {
                            for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                    for (int i = 0; i < instance->getNdays(); ++i) {
                                        if (stringcontains(idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                           idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                           instance->getNdays()) ==
                                            1) { ;
                                        } else {
                                            tempModel.add(cplexMap[idClassesDist->getOrderID()][p] +
                                                      cplexMap[idClassesDist1->getOrderID()][p1] <= 1);


                                        }

                                    }


                                }
                            }
                        }
                    }


                }






               /* Constraint *limite;
                distribution *req;
                if (isReq) {
                    if (limit1 != -1) {
                        limite = new Limits(limit, limit1);
                    } else if (limit != -1) {
                        limite = new WithLimit(limit);
                    } else {
                        limite = new NoLimit();
                    }
                    limite->setType(type);
                    req = new DistributionRequired(limite, c);
                    for (int restr = 0; restr < c.size(); restr++) {
                        if (classConst.count(c[restr])) {
                            classConst[c[restr]]->push_back(req);
                        } else {
                            std::vector<distribution *> *t = new std::vector<distribution *>();
                            t->push_back(req);
                            classConst.insert(std::pair<int, std::vector<distribution *> *>(c[restr], t));
                        }
                    }
                } else {
                    if (limit1 != -1) {
                        limite = new Limits(limit, limit1);
                    } else if (limit != -1) {
                        limite = new WithLimit(limit);
                    } else {
                        limite = new NoLimit();
                    }
                    limite->setType(type);
                    req = new DistributionPenalty(limite, c, penalty);
                    for (int restr = 0; restr < c.size(); restr++) {
                        if (classSoft.count(c[restr])) {
                            classSoft[c[restr]]->push_back(req);
                        } else {
                            std::vector<distribution *> *t = new std::vector<distribution *>();
                            t->push_back(req);
                            classSoft.insert(std::pair<int, std::vector<distribution *> *>(c[restr], t));
                        }
                    }

                }
                if (req)
                    instance->addDistribution(req);*/

            }
        } else if (strcmp(n->name(), "students") == 0) {
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

                if (mergeStudent) {

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


            }
            instance->setStudentCluster(clusterStudent);
            instance->setStudent(std);


        }


    }
    for (auto it= curMap.begin(); it!=curMap.end();++it) {
        if(problem.size()==0)
            problem.push_back(new Curriculum(*it->second));
        else{
            bool test=false;
            for (int i = 0; i <problem.size() ; ++i) {
                if(problem[i]->getPClass().size()!=it->second->size())
                    continue;
                else {
                    for (ClusterbyRoom *c:problem[i]->getPClass()) {
                        for (ClusterbyRoom *c1: *it->second) {
                            if(c==c1){
                                test=true;
                                break;
                            }
                        }
                        if(test)
                            break;

                    }
                }
                if(test)
                    break;

            }
            if(!test)
                problem.push_back(new Curriculum(*it->second));
        }

    }
    model.add(tempModel);

    for (auto i = classConst.begin(); i != classConst.end(); ++i) {
        instance->getClass(i->first)->setHard(*i->second);
    }
    for (auto i = classSoft.begin(); i != classSoft.end(); ++i) {
        instance->getClass(i->first)->setSoft(*i->second);
    }
    return instance;
}


void addPossibleRooms(Class *c, Instance *instance) {
    if (c->getPossibleRooms().size() > 0) {
        for (int i = 0; i < instance->getRooms().size(); ++i) {
            if (instance->getRoom(i + 1)->getType().compare(c->getFirstPossibleRoom()->getType()) == 0)
                c->addRoom(instance->getRoom(i + 1));
        }
    }

}
