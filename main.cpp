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
#include <mach/notify.h>
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
#include "problem/ConstraintShort.h"
#include<mach/mach.h>


using namespace rapidxml;

Instance *readInputXML(std::string filename);

void readOutputXML(std::string filename, Instance *instance);

void writeOutputXML(std::string filename, Instance *instance, double time);

void readPerturbations(std::string filename, Instance *instance);

void addPossibleRooms(Class *c, Instance *instance);

void writeXML(Instance *instance, int version, Curriculum *cur);

void printClusterofStudents(Instance *instance);

void printCurricular();

std::string constraint(Class *idClassesDist, Class *idClassesDist1, int p, int p1);

void readSAT();

bool mergeStudent = true;
bool mergeRoom = false;
bool roomTime = !mergeRoom;

bool warm = false;
bool optRoom = false;
bool optTime = false;
bool optStu = false;

int max = 0;


std::vector<Curriculum *> problem;


std::string costTime = " ";
std::string costRoom = " ";
std::string oneEachG = " ";

void printRAM() {
    vm_size_t page_size;
    mach_port_t mach_port;
    mach_msg_type_number_t count;
    vm_statistics64_data_t vm_stats;

    mach_port = mach_host_self();
    count = sizeof(vm_stats) / sizeof(natural_t);
    if (KERN_SUCCESS == host_page_size(mach_port, &page_size) &&
        KERN_SUCCESS == host_statistics64(mach_port, HOST_VM_INFO,
                                          (host_info64_t) &vm_stats, &count)) {
        long long free_memory = (int64_t) vm_stats.free_count * (int64_t) page_size;

        long long used_memory = ((int64_t) vm_stats.active_count +
                                 (int64_t) vm_stats.inactive_count +
                                 (int64_t) vm_stats.wire_count) * (int64_t) page_size;
        printf("free memory: %lldused memory: %lld", free_memory, used_memory);
    }
    struct task_basic_info t_info;
    mach_msg_type_number_t t_info_count = TASK_BASIC_INFO_COUNT;

    if (KERN_SUCCESS != task_info(mach_task_self(),
                                  TASK_BASIC_INFO, (task_info_t) &t_info,
                                  &t_info_count)) {
        std::exit(-1);
    }
    std::cout << "resident_size: " << t_info.resident_size << std::endl;
    std::cout << "virtual_size: " << t_info.virtual_size << std::endl;

}


int main(int argc, char **argv) {


    //if (!quiet)
    std::cout << "Starting Reading File: "
              << "/Volumes/MAC/ClionProjects/timetabler/data/input/ITC-2019/tg-fal17_0.xml" << std::endl;

    printRAM();

    Instance *instance = readInputXML("/Volumes/MAC/ClionProjects/timetabler/tg-fal17_0.xml");
    instance->setCompact(false);
    /*readSAT();
    writeOutputXML("/Volumes/MAC/ClionProjects/timetabler/data/output/ITC-2019/" + instance->getName() + ".xml",
                   instance, getTimeSpent());
    /*for (int k = 0; k < problem.size(); ++k) {
        writeXML(instance, k, problem[k]);
    }
    std::exit(1);*/
    std::cout << max << std::endl;
    printRAM();

    int i = 0;
    std::cout << instance->getName() << " " << problem.size() << std::endl;
    for (Curriculum *c: problem) {
        std::string nome = "/Volumes/MAC/ClionProjects/timetabler/tg-fal17" + std::to_string(0) + ".opb";
        std::ofstream file(nome);
        file << "min: " << costRoom << " + " << costTime << " ;" << std::endl;
        file << oneEachG;


        printRAM();
        //IloNumExpr t;
        for (auto *clu: c->getPClass()) {
            Class *idClassesDist, *idClassesDist1;
            for (int y = 0; y < clu->getRange().size(); ++y) {
                for (int ci = 1; ci < clu->getRange().at(y)->getClasses().size(); ++ci) {
                    idClassesDist = clu->getRange().at(y)->getClasses()[ci - 1];
                    idClassesDist1 = clu->getRange().at(y)->getClasses()[ci];//
                    if (clu->getRange().at(y)->getType().compare("MaxDayLoad") == 0) {
                        for (int w = 0; w < instance->getNweek(); ++w) {
                            for (int d = 0; d < instance->getNdays(); ++d) {
                                for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                                    if (idClassesDist->getPossiblePairLecture(p)->getWeeks()[w] == '1'
                                        && idClassesDist->getPossiblePairLecture(p)->getDays()[d] == '1') {
                                        /*t += var[vector[c]->getOrderID()][p] *
                                             vector[c]->getPossiblePairLecture(p)->getLenght();*/
                                    }
                                }
                            }
                            //  model.add(t <= l);

                        }


                    } else if (clu->getRange().at(y)->getType().compare("WorkDay") == 0) {
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
                                        clu->getRange().at(y)->getParameter1()) {
                                        file << constraint(idClassesDist, idClassesDist1, p, p1);
                                    }
                                }


                            }
                        }


                    } else if (clu->getRange().at(y)->getType().compare("DifferentDays") ==
                               0) {
                        for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                            for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                for (int d = 0; d < instance->getNdays(); ++d) {
                                    if (idClassesDist->getPossiblePairLecture(p)->getDays()[d] ==
                                        idClassesDist1->getPossiblePairLecture(p1)->getDays()[d] &&
                                        idClassesDist->getPossiblePairLecture(p)->getDays()[d] == '1') {
                                        file << constraint(idClassesDist, idClassesDist1, p, p1);


                                    }

                                }


                            }
                        }
                    } else if (clu->getRange().at(y)->getType().compare("SameDays") == 0) {
                        for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                            for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                for (int d = 0; d < instance->getNdays(); ++d) {
                                    if (stringcontains(idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                       idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                       instance->getNdays()) ==
                                        1) { ;
                                    } else {
                                        file << constraint(idClassesDist, idClassesDist1, p, p1);


                                    }

                                }


                            }
                        }
                    } else if (clu->getRange().at(y)->getType().compare("SameAttendees") == 0) {
                        std::cout<<idClassesDist->getId()<<" "<<idClassesDist1->getId()<<std::endl;
                        for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                            for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                int travel = 0;
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
                                                idClassesDist1->getPossiblePairRoom(p1)->getId())->getTravel(
                                                idClassesDist->getPossiblePairRoom(p)->getId());
                                }

                                if (idClassesDist->getPossiblePairLecture(p)->getEnd() + travel <=
                                    idClassesDist1->getPossiblePairLecture(p1)->getStart()
                                    || idClassesDist1->getPossiblePairLecture(p1)->getEnd() +
                                       travel <= idClassesDist->getPossiblePairLecture(p)->getStart()
                                    || stringcompare(idClassesDist->getPossiblePairLecture(p)->getWeeks(),
                                                     idClassesDist1->getPossiblePairLecture(p1)->getWeeks(),
                                                     instance->getNweek(), false) ==
                                       1
                                    || stringcompare(idClassesDist->getPossiblePairLecture(p)->getDays(),
                                                     idClassesDist1->getPossiblePairLecture(p1)->getDays(),
                                                     instance->getNdays(), false) ==
                                       1) { ;
                                } else {
                                    if(idClassesDist->getId()==298||idClassesDist1->getId()==298)
                                        std::cout<< constraint(idClassesDist, idClassesDist1, p, p1);
                                    file << constraint(idClassesDist, idClassesDist1, p, p1);


                                }
                            }


                        }
                    } else if (clu->getRange().at(y)->getType().compare("NotOverlap") == 0) {
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
                                    file << constraint(idClassesDist, idClassesDist1, p, p1);


                                }
                            }


                        }


                    } else if (clu->getRange().at(y)->getType().compare("Precedence") == 0) {
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
                                    file << constraint(idClassesDist, idClassesDist1, p, p1);


                                }


                            }

                        }


                    } else if (clu->getRange().at(y)->getType().compare("SameTime") == 0) {
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
                                    file << constraint(idClassesDist, idClassesDist1, p, p1);

                                }
                            }


                        }
                    } else if (clu->getRange().at(y)->getType().compare("SameStart") == 0) {
                        for (int p = 0; p < idClassesDist->getPossiblePairSize(); ++p) {
                            for (int p1 = 0; p1 < idClassesDist1->getPossiblePairSize(); ++p1) {
                                if (idClassesDist1->getPossiblePairLecture(p1)->getStart() !=
                                    idClassesDist->getPossiblePairLecture(p)->getStart())
                                    file << constraint(idClassesDist, idClassesDist1, p, p1);
                            }

                        }


                    }
                }
            }


            for (Room *r: clu->getRooms()) {
                for (Time *time1: r->t) {
                    for (int con = 0; con < time1->getClassesC().size(); ++con) {

                        for (int cla = 0; cla < time1->getClassesS().size(); ++cla) {
                            if (time1->getClassesC()[con] != time1->getClassesS()[cla]) {
                                file << "+1 " << time1->getClassesC()[con] << " +1 " << time1->getClassesS()[cla]
                                     << "    <=   1;" << std::endl;

                            }

                        }

                    }
                }
            }
        }
        i++;
    }
    printRAM();


    if (!quiet) std::cout << "Solution Found: Writing output file" << std::endl;


    return 0;
}

void readSAT() {
    std::ifstream infile("/Volumes/MAC/ClionProjects/timetabler/a.txt");
    std::string line;
    bool temp = false;
    while (std::getline(infile, line)) {
        std::istringstream iss(line);
        std::string a, b;
        while (iss >> a) {
            if (temp) {
                if (a[0] != '-') {
                    bool find = false;
                    for (Curriculum *c: problem) {
                        for (auto *clu: c->getPClass()) {
                            for (auto *cla: clu->getClasses()) {
                                for (int i = 0; i < cla->getPossiblePairSize(); ++i) {
                                    if (cla->getKey(cla->getPossiblePair(i).first, cla->getPossiblePair(i).second) ==
                                        a) {
                                        cla->setSolution(cla->getPossiblePair(i).second->getStart(),
                                                         cla->getPossiblePair(i).first->getId(),
                                                         cla->getPossiblePair(i).first->getName(),
                                                         cla->getPossiblePair(i).second->getWeeks(),
                                                         cla->getPossiblePair(i).second->getDays());
                                        std::cout << a << " " << cla->getId() << std::endl;
                                        find = true;
                                        break;
                                    }

                                }
                                if (find)
                                    break;

                            }
                            if (find)
                                break;
                        }
                        if (find)
                            break;


                    }

                }

            }
            if (a == "v")
                temp = true;
        }

    }

}

std::string constraint(Class *idClassesDist, Class *idClassesDist1, int p, int p1) {
    return "+1 " + idClassesDist->getKey(idClassesDist->getPossiblePairRoom(p),
                                         idClassesDist->getPossiblePairLecture(p)) + " +1 "
           + idClassesDist1->getKey(idClassesDist1->getPossiblePairRoom(p1),
                                    idClassesDist1->getPossiblePairLecture(p1)) + "  <=  1;\n";
}

void printCurricular() {
    for (Curriculum *c: problem) {
        std::cout << "New" << std::endl;
        for (ClusterbyRoom *clus: c->getPClass()) {
            std::cout << "C";
            for (Class *c: clus->getClasses()) {
                std::cout << c->getId() << std::endl;
            }
            std::cout << std::endl;
            std::cout << "R";
            for (Room *c: clus->getRooms()) {
                std::cout << c->getId() << std::endl;
            }
            std::cout << std::endl;

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
    int orderID = 0;
    std::map<int, Class *> classMap;
    std::map<Class *, ClusterbyRoom *> mapCluster;
    std::map<int, std::set<ClusterbyRoom *> *> curMap;

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
                            std::string oneEach = " ";
                            c->setOrderID(order);
                            c->setCourseID(atoi(id));
                            order++;
                            all.push_back(c);
                            int i = 0;
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
                                        std::cerr << "Room does not exist: " << idRoom << " " << id << " " << idclass
                                                  << std::endl;
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
                                    Lecture *l = new Lecture(lenght, start, weeks, days, penalty);
                                    lecv.push_back(l);


                                    if (roomsv.size() == 0) {
                                        std::string old;
                                        old = "x" + std::to_string(max);
                                        max++;

                                        c->setPossiblePair(
                                                new Room(-1),
                                                l, old);
                                        oneEach = " +1 " + old;
                                        if (penalty != 0)
                                            costTime +=
                                                    " +" + std::to_string(instance->getTimePen() * penalty) + " " + old;

                                    } else {
                                        for (std::map<Room *, int>::iterator j = roomsv.begin();
                                             j != roomsv.end(); ++j) {
                                            int roomID = j->first->getId();
                                            std::string week = l->getWeeks();
                                            std::string day = l->getDays();
                                            int startTime = l->getStart();
                                            int duration = l->getLenght();
                                            bool isNotAddableTime = false;
                                            for (int una = 0; una < j->first->getSlots().size(); ++una) {
                                                for (int weeki = 0; weeki < instance->getNweek(); ++weeki) {
                                                    if (week[weeki] == j->first->getSlots()[una].getWeeks()[weeki] &&
                                                        j->first->getSlots()[una].getWeeks()[weeki] == '1') {
                                                        for (int d = 0; d < instance->getNdays(); ++d) {
                                                            if (day[d] == j->first->getSlots()[una].getDays()[d] &&
                                                                day[d] == '1') {

                                                                if (startTime >= j->first->getSlots()[una].getStart() &&
                                                                    startTime < j->first->getSlots()[una].getStart() +
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
                                                std::string old;
                                                old = "x" + std::to_string(max);
                                                max++;

                                                oneEach += " +1 " + old;

                                                if (penalty != 0)
                                                    costTime +=
                                                            " +" + std::to_string(instance->getTimePen() * penalty) +
                                                            " " + old;

                                                if (j->second != 0)
                                                    costRoom +=
                                                            " +" + std::to_string(instance->getRoomPen() * j->second) +
                                                            " " + old;

                                                c->setPossiblePair(
                                                        j->first,
                                                        l, old);
                                                if (roomTime) {
                                                    bool is = true;
                                                    for (int ti = 0; ti < j->first->t.size(); ++ti) {
                                                        if (l->getDays().compare(j->first->t[ti]->getDay()) == 0 &&
                                                            l->getWeeks().compare(j->first->t[ti]->getWeek()) == 0 &&
                                                            l->getStart() == j->first->t[ti]->getStart() &&
                                                            l->getEnd() == j->first->t[ti]->getEnd()) {
                                                            j->first->t[ti]->addS(old);
                                                            is = false;
                                                        } else if (j->first->t[ti]->check(l, instance->getNweek(),
                                                                                          instance->getNdays())) {
                                                            j->first->t[ti]->addC(old);
                                                            is = false;


                                                        }
                                                    }
                                                    if (is) {
                                                        j->first->t.push_back(
                                                                new Time(l->getStart(), l->getEnd(), l->getWeeks(),
                                                                         l->getDays(), old));
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
                            if (parent != -1)
                                c->setParent(classMap[classID[parent]]);
                            clasv.push_back(c);
                            oneEachG += oneEach + " = 1 ; \n";

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
                    /*if(all[k]->getCourseID()==(*cluster[i]->getClasses().begin())->getCourseID()) {
                        test = true;
                        cluster[i]->addClass(all[k], -1);
                        if (all[k]->getPossibleRooms().size() > 0) {
                            for (std::pair<Room *, int> it : all[k]->getPossibleRooms()) {
                                cluster[i]->addRoom(it.first);
                            }
                        }
                        mapCluster.insert(std::pair<Class *, ClusterbyRoom *>(all[k], cluster[i]));
                        break;
                    }else {*/
                    for (Room *j:cluster[i]->getRooms()) {
                        if (all[k]->getPossibleRoom(j) != -1) {
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
                    if (test) {
                        break;
                    }
                    //}

                }
                if (!test) {
                    std::set<Room *> temp1;
                    for (std::pair<Room *, int> it : all[k]->getPossibleRooms()) {
                        temp1.insert(it.first);
                    }
                    cluster.push_back(new ClusterbyRoom(cluster.size(), temp, all[k]));
                    mapCluster.insert(std::pair<Class *, ClusterbyRoom *>(all[k], cluster[cluster.size() - 1]));
                }

            }


        } else if (strcmp(n->name(), "distributions") == 0) {
            for (const xml_node<> *sub = n->first_node(); sub; sub = sub->next_sibling()) {
                std::stringstream constSTR;
                bool isReq = false;
                std::vector<ConstraintShort *> *temp = new std::vector<ConstraintShort *>();
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
                        if (test)
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


    return instance;
}

void writeXML(Instance *instance, int version, Curriculum *cur) {
    std::map<int, int> courseM;
    xml_document<> doc;
    xml_node<> *decl = doc.allocate_node(node_declaration);
    decl->append_attribute(doc.allocate_attribute("version", "1.0"));
    decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
    doc.append_node(decl);
    //nrDays="7" slotsPerDay="288" nrWeeks="14"
    xml_node<> *root = doc.allocate_node(node_element, "problem");
    root->append_attribute(doc.allocate_attribute("name", instance->getName().c_str()));
    root->append_attribute(doc.allocate_attribute("nrDays", std::to_string(instance->getNdays()).c_str()));
    root->append_attribute(doc.allocate_attribute("slotsPerDay", std::to_string(instance->getSlotsperday()).c_str()));
    root->append_attribute(doc.allocate_attribute("nrWeeks", std::to_string(instance->getNweek()).c_str()));

    doc.append_node(root);
    xml_node<> *child;
    //<optimization time="1" room="1" distribution="30" student="0"/>
    child = doc.allocate_node(node_element, "optimization");
    child->append_attribute(doc.allocate_attribute("time", std::to_string(instance->getTimePen()).c_str()));
    child->append_attribute(doc.allocate_attribute("room", std::to_string(instance->getRoomPen()).c_str()));
    child->append_attribute(
            doc.allocate_attribute("distribution", std::to_string(instance->getDistributionPen()).c_str()));
    child->append_attribute(doc.allocate_attribute("student", std::to_string(instance->getStudentPen()).c_str()));
    root->append_node(child);
    child = doc.allocate_node(node_element, "rooms");
    xml_node<> *grandchild;
    for (ClusterbyRoom *clu: cur->getPClass()) {
        for (Room *r: clu->getRooms()) {
            grandchild = doc.allocate_node(node_element, "room");
            grandchild->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                    std::to_string(r->getId()).c_str())));
            grandchild->append_attribute(doc.allocate_attribute("capacity", doc.allocate_string(
                    std::to_string(r->getCapacity()).c_str())));
            for (std::pair<int, int> t: r->getTravel()) {
                xml_node<> *travel = doc.allocate_node(node_element, "travel");
                travel->append_attribute(doc.allocate_attribute("room", doc.allocate_string(
                        std::to_string(t.first).c_str())));
                travel->append_attribute(doc.allocate_attribute("capacity", doc.allocate_string(
                        std::to_string(t.second).c_str())));
                grandchild->append_node(travel);

            }
            for (Unavailability t: r->getSlots()) {
                xml_node<> *una = doc.allocate_node(node_element, "unavailable");
                una->append_attribute(doc.allocate_attribute("days", doc.allocate_string(
                        t.getDays().c_str())));
                una->append_attribute(doc.allocate_attribute("start", doc.allocate_string(
                        std::to_string(t.getStart()).c_str())));
                una->append_attribute(doc.allocate_attribute("length", doc.allocate_string(
                        std::to_string(t.getLenght()).c_str())));
                una->append_attribute(doc.allocate_attribute("weeks", doc.allocate_string(
                        t.getWeeks().c_str())));
                grandchild->append_node(una);

            }
            child->append_node(grandchild);
        }
    }
    root->append_node(child);
    child = doc.allocate_node(node_element, "courses");
    for (ClusterbyRoom *clu: cur->getPClass()) {
        for (Class *c: clu->getClasses()) {
            if (courseM.find(c->getCourseID()) == courseM.end()) {
                courseM.insert(std::pair<int, int>(c->getCourseID(), 1));
                grandchild = doc.allocate_node(node_element, "course");
                grandchild->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                        std::to_string(c->getCourseID()).c_str())));

                xml_node<> *confi;
                for (std::pair<int, std::vector<Subpart *>> conf:instance->getCourse(
                        std::to_string(c->getCourseID()))->getConfiguratons()) {
                    confi = doc.allocate_node(node_element, "config");
                    confi->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                            std::to_string(conf.first).c_str())));
                    xml_node<> *sub;
                    int subid = 0;
                    for (Subpart *part: conf.second) {
                        sub = doc.allocate_node(node_element, "subpart");
                        sub->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                                std::to_string(subid).c_str())));
                        subid++;
                        xml_node<> *classXML;
                        for (Class *aClass: part->getClasses()) {
                            classXML = doc.allocate_node(node_element, "class");
                            classXML->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                                    std::to_string(aClass->getId()).c_str())));
                            classXML->append_attribute(doc.allocate_attribute("limit", doc.allocate_string(
                                    std::to_string(aClass->getLimit()).c_str())));
                            if (aClass->getParent() != nullptr)
                                classXML->append_attribute(doc.allocate_attribute("parent", doc.allocate_string(
                                        std::to_string(aClass->getParent()->getId()).c_str())));
                            xml_node<> *time;
                            for (std::pair<Room *, int> room: aClass->getPossibleRooms()) {
                                time = doc.allocate_node(node_element, "room");
                                time->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                                        std::to_string(room.first->getId()).c_str())));
                                time->append_attribute(doc.allocate_attribute("penalty", doc.allocate_string(
                                        std::to_string(room.second).c_str())));
                                classXML->append_node(time);
                            }
                            for (Lecture *lecture:aClass->getLectures()) {
                                // <time days="1000000" start="108" length="22" weeks="11111111111111" penalty="20"/>
                                time = doc.allocate_node(node_element, "time");
                                time->append_attribute(doc.allocate_attribute("days", doc.allocate_string(
                                        lecture->getDays().c_str())));
                                time->append_attribute(doc.allocate_attribute("start", doc.allocate_string(
                                        std::to_string(lecture->getStart()).c_str())));
                                time->append_attribute(doc.allocate_attribute("length", doc.allocate_string(
                                        std::to_string(lecture->getLenght()).c_str())));
                                time->append_attribute(doc.allocate_attribute("weeks", doc.allocate_string(
                                        lecture->getWeeks().c_str())));
                                time->append_attribute(doc.allocate_attribute("penalty", doc.allocate_string(
                                        std::to_string(lecture->getPenalty()).c_str())));
                                classXML->append_node(time);
                            }
                            sub->append_node(classXML);
                        }
                        confi->append_node(sub);


                    }
                    grandchild->append_node(confi);


                }
                child->append_node(grandchild);

            }
        }

    }
    root->append_node(child);
    child = doc.allocate_node(node_element, "distributions");
    for (ClusterbyRoom *clusterbyRoom: cur->getPClass()) {
        for (ConstraintShort *clu :clusterbyRoom->getRange()) {

            grandchild = doc.allocate_node(node_element, "distribution");
            std::stringstream name;
            if (clu->getParameter1() != -1 && clu->getParameter2() != -1) {
                name << clu->getType() << "(" << clu->getParameter1() << "," << clu->getParameter2() << ")";
            } else if (clu->getParameter1() != -1 && clu->getParameter2() == -1) {
                name << clu->getType() << "(" << clu->getParameter1() << ")";
            } else
                name << clu->getType();
            grandchild->append_attribute(doc.allocate_attribute("type", doc.allocate_string(
                    name.str().c_str())));
            if (clu->getWeight() != -1)
                grandchild->append_attribute(doc.allocate_attribute("penalty", doc.allocate_string(
                        std::to_string(clu->getWeight()).c_str())));
            else
                grandchild->append_attribute(doc.allocate_attribute("required", doc.allocate_string(
                        "true")));
            xml_node<> *classXML;
            for (Class *c: clu->getClasses()) {
                classXML = doc.allocate_node(node_element, "class");
                classXML->append_attribute(doc.allocate_attribute("id", doc.allocate_string(
                        std::to_string(c->getId()).c_str())));
                grandchild->append_node(classXML);
            }
            child->append_node(grandchild);

        }
    }
    root->append_node(child);


    //TODO:Students

    std::stringstream name;
    name << instance->getName() << "_" << version << ".xml";
    std::ofstream file_stored(name.str());
    //print(std::cout, doc, 0);
    file_stored << doc;

    file_stored.close();
    doc.clear();
}


