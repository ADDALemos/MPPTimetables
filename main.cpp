#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include "problem/Lecture.h"

#include "problem/Room.h"
#include "problem/Course.h"
#include "problem/distribution.h"
#include "rapidXMLParser/rapidxml.hpp"
#include "rapidXMLParser/rapidxml_print.hpp"
#include "problem/DistributionRequired.h"
#include "problem/DistributionPenalty.h"
#include "problem/Limits.h"
#include "problem/WithLimit.h"
#include "problem/Instance.h"
#include "solver/ILPExecuter.h"
#include "solver/GurobiExecuter.h"
#include "refactor/Perturbation.h"


Instance *readInputXML(std::string filename);

Instance *readInputXML2007(std::string filename);


void readOutputXML(std::string filename, Instance *instance);

void writeOutputXML(std::string filename, Instance *instance, double time);

void readPerturbations();

//TODO: Actually write help
void help() {
    std::cout << "For help press -- h" << std::endl <<
              "Program execution ./program -o OriginalProblem.xml [-s OriginalSolution.xml] [-p  Perturbations.xml] [-c Cost function]"
              << std::endl <<
              "The Problem instance should be encoding with ITC-2019 format;" << std::endl <<
              "Without the file OriginalSolution.xml the program will first solve the first problem and then solve the MPP;"
              << std::endl <<
              "It is possible provide the perturbations in file or use random generated default;"//TODO: Random Perturbations ?
                      "It is possible to choose the cost function from the ones shown bellow:" << std::endl <<
              " -c 1 The Hamming distance;" << std::endl <<
              " -c 2 The Hamming distance weighted with the number of students which scheduled changed;" << std::endl <<
              " -c 3 The number of students seated." << std::endl;
    std::exit(0);
} /* displays help */


using namespace rapidxml;
bool quiet = false; //Print info
int main(int argc, char **argv) {
    clock_t tStart = clock();
    if (argc < 3)
        help();


    if (!quiet) std::cout << "Starting Reading File: " << argv[1] << std::endl;
    Instance *instance = readInputXML(argv[1]);
    if (!quiet) std::cout << "Starting Reading File: " << argv[2] << std::endl;
    readOutputXML(argv[2], instance);
    if (!quiet) std::cout << "Generating Perturbations based on the file: " << std::endl;
    readPerturbations();
    if (!quiet) std::cout << "Generating ILP model" << std::endl;
    ILPExecuter *runner = new GurobiExecuter();
    runner->setInstance(instance);
    runner->definedRoomLecture();
    runner->definedLectureTime();
    runner->oneLectureperSlot();
    runner->saveEncoding();
    std::exit(42);

    //runner->roomClose();
    //runner->slotClose();
    runner->teacher();
    //runner->roomClosebyDay();
    //runner->assignmentInvalid();
    runner->oneLectureRoom();
    runner->oneLectureperSlot();
    // runner->slackStudent();
    runner->studentConflictSolution();
    runner->oneLectureRoomConflict();

    if (!quiet) std::cout << "Add optimization: GapStudentsTimetable" << std::endl;
    //runner->optimizeGapStudentsTimetable();

    //runner->optimizeRoomUsage();


    //runner->optimizeSeatedStudents();

    if (!quiet) std::cout << "Running ILP solver" << std::endl;
    double v = runner->run(true);
    //int **sol = runner->getSolutionRoom();
    //runner->getSolutionTime();

    std::exit(42);
    writeOutputXML("/Volumes/MAC/ClionProjects/timetabler/data/output/wbg-fal10Out.xml", instance,
                   (double) (clock() - tStart) / CLOCKS_PER_SEC);
    runner = new GurobiExecuter();
    runner->setInstance(instance);
    runner->definedRoomLecture();
    runner->definedLectureTime();
    runner->oneLectureRoom();
    //runner->slackStudent();
    runner->studentConflict();
    //runner->constraintSeatedStudents(v);
//    runner->distanceToSolution(sol, nullptr, false);
    runner->run(false);


    printf("Time taken: %.2fs\n", (double) (clock() - tStart) / CLOCKS_PER_SEC);


    return 0;
}

void readPerturbations() {/*Perturbation *p = new Perturbation();
    p->randomAddNewCurriculum(instance);
    p->randomClassSelection(instance, 0);
    p->randomEnrolmentChanges(instance, 15, false, .5);
    p->randomCloseRoom(instance, 0);
    p->randomSlotClose(instance, 0);
    p->randomCloseRoomDay(instance, 0);
    p->randomShiftChange(instance, 1, 1, 150, false);*/}


void writeOutputXML(std::string filename, Instance *instance, double time) {
    xml_document<> doc;
    xml_node<> *decl = doc.allocate_node(node_declaration);
    decl->append_attribute(doc.allocate_attribute("version", "1.0"));
    decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
    doc.append_node(decl);

    xml_node<> *root = doc.allocate_node(node_element, "solution");
    root->append_attribute(doc.allocate_attribute("name", instance->getName().c_str()));
    root->append_attribute(doc.allocate_attribute("runtime", std::to_string(time).c_str()));
    root->append_attribute(doc.allocate_attribute("cores", "4"));
    root->append_attribute(doc.allocate_attribute("technique", "ILP"));
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
                std::to_string(instance->getClasses()[c]->getSolStart()).c_str())));
        child->append_attribute(
                doc.allocate_attribute("weeks", doc.allocate_string(instance->getClasses()[c]->getSolWeek().c_str())));
        child->append_attribute(doc.allocate_attribute("room", doc.allocate_string(
                std::to_string(instance->getClasses()[c]->getSolRoom()).c_str())));
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
    std::__1::string content(buffer.str());
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
        Class *s = instance->getClass(id);
        if (roomID.find(room) == roomID.end()) {
            std::cerr << "Room does not exist: " << room << std::endl;
            std::exit(11);
        }
        s->setSolution(start, roomID[room], room, weeks, days);
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


Instance *readInputXML(std::string filename) {//parent flag missing
    xml_document<> doc;
    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "File not found: " + filename << std::endl;
        std::cerr << "Method: readInputXML" << std::endl;
        std::exit(11);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::__1::string content(buffer.str());
    doc.parse<0>(&content[0]);
    xml_node<> *pRoot = doc.first_node();
    Instance *instance;
    for (const xml_attribute<> *a = pRoot->first_attribute(); a; a = a->next_attribute()) {
        if (strcmp(a->name(), "name") == 0)
            instance= new Instance(a->value());
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
                std::string id = " ";
                int capacity = -1;
                for (const xml_attribute<> *a = s->first_attribute(); a; a = a->next_attribute()) {
                    if (strcmp(a->name(), "capacity") == 0) {
                        capacity=atoi(a->value());
                    } else if (strcmp(a->name(), "id") == 0) {
                        id = a->value();
                    }
                }
                std::map<int, int> travel;
                std::vector<Unavailability> una;
                for (const xml_node<> *rs = s->first_node(); rs; rs = rs->next_sibling()) {
                    if (strcmp(rs->name(), "travel") == 0) {
                        int value=-1,room=-1;
                        for (const xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                            if (strcmp(a->name(), "room") == 0) {
                                room=atoi(a->value());
                            } else if (strcmp(a->name(), "value") == 0) {
                                value=atoi(a->value());
                            }
                        }
                        travel.insert(std::pair<int, int>(room, value));


                    } else if (strcmp(rs->name(), "unavailable") == 0) {
                        char* days,*weeks;
                        int length=-1,start=-1;
                        for (const xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                            if (strcmp(a->name(), "days") == 0) {
                                days=a->value();
                            } else if (strcmp(a->name(), "start") == 0) {
                                start=atoi(a->value());
                            } else if (strcmp(a->name(), "length") == 0) {
                                length=atoi(a->value());
                            } else if (strcmp(a->name(), "weeks") == 0) {
                                weeks=a->value();
                            }
                        }
                        Unavailability *u = new Unavailability(days, start, length, weeks);
                        //std::cout<<*u<<std::endl;
                        una.push_back(*u);
                    }


                }
                if (roomID.find(id) == roomID.end()) {
                    int size = roomID.size();
                    size++;
                    roomID.insert(std::pair<std::string, int>(id, size));
                }
                Room *r = new Room(roomID[id], id, capacity, travel, una);
                //   std::cout<<*r<<std::endl;
                //      std::cout<<r->getSlots().size()<<std::endl;
                instance->addRoom(r);
                //std::cout<<instance->getRooms().size()<<std::endl;
            }

        } else if (strcmp(n->name(), "courses") == 0) {
            for (const xml_node<> *s = n->first_node(); s; s = s->next_sibling()) {
                char *id;
                std::__1::map<int, std::__1::vector<Subpart *>> config;
                for (const xml_attribute<> *a = s->first_attribute(); a; a = a->next_attribute()) {
                    id=a->value();
                }
                for (const xml_node<> *rs = s->first_node(); rs; rs = rs->next_sibling()) {
                    int idConf = -1;

                    std::__1::vector<Subpart *> subpartvec;

                    for (const xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                        idConf = atoi(a->value());
                    }
                    for (const xml_node<> *sub = rs->first_node(); sub; sub = sub->next_sibling()) {
                        std::__1::string idsub;
                        std::__1::vector<Class *> clasv;
                        for (const xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                            idsub = a->value();
                        }
                        for (const xml_node<> *cla = sub->first_node(); cla; cla = cla->next_sibling()) {

                            int idclass = -1, limit = -1, parent = -1;
                            std::map<Room, int> roomsv;
                            std::vector<Lecture *> lecv;
                            for (const xml_attribute<> *a = cla->first_attribute(); a; a = a->next_attribute()) {
                                if (strcmp(a->name(), "id") == 0)
                                    idclass=atoi(a->value());
                                else if (strcmp(a->name(), "limit") == 0)
                                    limit = atoi(a->value());
                                else if (strcmp(a->name(), "parent") == 0)
                                    parent = atoi(a->value());
                            }
                            for (const xml_node<> *lec = cla->first_node(); lec; lec = lec->next_sibling()) {
                                if (strcmp(lec->name(), "room") == 0) {
                                    std::string idRoom = " ";
                                    int penalty = -1;
                                    for (const xml_attribute<> *a = lec->first_attribute(); a; a = a->next_attribute()) {
                                        //std::cout<<a->name()<<std::endl;
                                        if (strcmp(a->name(), "id") == 0)
                                            idRoom = a->value();
                                        else if (strcmp(a->name(), "penalty") == 0)
                                            penalty = atoi(a->value());

                                    }
                                    if (roomID.find(idRoom) == roomID.end()) {
                                        std::cerr << "Room does not exist: " << idRoom << std::endl;
                                        std::exit(11);
                                    }
                                    roomsv.insert(std::pair<Room, int>(instance->getRoom(roomID[idRoom]), penalty));
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
                                }

                                //limit=atoi(a->value());


                            }

                            Class *c = new Class(idclass, limit, lecv, roomsv);
                            if (parent != -1)
                                c->setParent(parent);
                            clasv.push_back(c);

                        }
                        Subpart *subpart = new Subpart(idsub, clasv);
                        subpartvec.push_back(subpart);

                    }
                    config.insert(std::pair<int, std::vector<Subpart *>>(idConf, subpartvec));

                }
                Course *course = new Course(id, config);
                instance->addCourse(course);

            }


        } else if (strcmp(n->name(), "distributions") == 0) {
            for (const xml_node<> *sub = n->first_node(); sub; sub = sub->next_sibling()) {
                bool isReq = false;
                std::string type;
                int penalty = -1;
                std::vector<int> c;
                int limit = -1, limit1 = -1;
                for (const xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                    if (strcmp(a->name(), "required") == 0) {
                        isReq = (atoi(a->value()) == 1);
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
                    int idClassesDist = -1;
                    for (const xml_attribute<> *a = course->first_attribute(); a; a = a->next_attribute()) {
                        idClassesDist = atoi(a->value());
                    }
                    c.push_back(idClassesDist);
                    //std::cout<<course->name()<<std::endl;
                }
                Constraint *limite;
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

                }
                if (req)
                    instance->addDistribution(req);

            }
        } else if (strcmp(n->name(), "students") == 0) {
            std::__1::map<int, Student> std;
            for (const xml_node<> *sub = n->first_node(); sub; sub = sub->next_sibling()) {
                int studentID = -1;
                for (const xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                    studentID = atoi(a->value());
                }
                std::__1::vector<Course *> c;
                for (const xml_node<> *course = sub->first_node(); course; course = course->next_sibling()) {
                    std::__1::string courseIDstd;
                    for (const xml_attribute<> *a = course->first_attribute(); a; a = a->next_attribute()) {
                        courseIDstd = a->value();
                    }
                    c.push_back(instance->getCourse(courseIDstd));

                }
                std.insert(std::pair<int, Student>(studentID, Student(studentID, c)));
            }
            instance->setStudent(std);


        }

    }
    return instance;
}


/** Function to parse data from ITC 2007 **/

Instance *readInputXML2007(std::string filename) {
    std::string name;
    xml_document<> doc;
    std::ifstream file(filename);
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::__1::string content(buffer.str());
    doc.parse<0>(&content[0]);
    xml_node<> *pRoot = doc.first_node();
    Instance *instance;
    std::map<std::string, Course *> cL;
    for (const xml_attribute<> *a = pRoot->first_attribute(); a; a = a->next_attribute()) {
        name = a->value();
    }
    int count = 0;
    for (const xml_node<> *n = pRoot->first_node(); n; n = n->next_sibling()) {
        if (strcmp("descriptor", n->name()) == 0) {
            int days = -1;
            int periods_per_day = -1;
            int min = -1, max = -1;
            for (const xml_node<> *ne = n->first_node(); ne; ne = ne->next_sibling()) {
                if (strcmp("days", ne->name()) == 0) {
                    days = atoi(ne->first_attribute()->value());
                } else if (strcmp("periods_per_day", ne->name()) == 0)
                    periods_per_day = atoi(ne->first_attribute()->value());
                else if (strcmp("daily_lectures", ne->name()) == 0) {
                    for (const xml_attribute<> *a = ne->first_attribute(); a; a = a->next_attribute()) {
                        if (strcmp("min", a->name()) == 0)
                            min = atoi(a->value());
                        else if (strcmp("max", a->name()) == 0)
                            max = atoi(a->value());
                    }
                }
            }

            instance = new Instance(name, days, periods_per_day, min, max);


        } else if (strcmp("courses", n->name()) == 0) {
            for (const xml_node<> *ne = n->first_node(); ne; ne = ne->next_sibling()) {
                int lectures = -1, min_days = -1, students = -1;
                char *teacher, *double_lectures, *id;
                for (const xml_attribute<> *a = ne->first_attribute(); a; a = a->next_attribute()) {
                    if (strcmp("id", a->name()) == 0)
                        id = a->value();
                    else if (strcmp("teacher", a->name()) == 0)
                        teacher = a->value();
                    else if (strcmp("lectures", a->name()) == 0)
                        lectures = atoi(a->value());
                    else if (strcmp("min_days", a->name()) == 0)
                        min_days = atoi(a->value());
                    else if (strcmp("students", a->name()) == 0)
                        students = atoi(a->value());
                    else if (strcmp("double_lectures", a->name()) == 0)
                        double_lectures = a->value();

                }
                Course *c = new Course(id, teacher, lectures, min_days, students, double_lectures, count);
                cL.insert(std::pair<std::string, Course *>(std::to_string(count), c));
                count += lectures;

            }
        } else if (strcmp("curricula", n->name()) == 0) {
            for (const xml_node<> *ne = n->first_node(); ne; ne = ne->next_sibling()) {
                // NoLimit *l = new NoLimit()
            }
        }
        // std::cout<<n->name()<<std::endl;
    }
    instance->setCourses(cL);

    return instance;
}


