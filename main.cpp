#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include <set>
#include <list>
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
#include "randomGenerator/Perturbation.h"
#include "utils/Stats.h"
#include "utils/TimeUtil.h"
#include "solver/Gurobi1.h"
#include "solver/GurobiSimple.h"
#include "solver/IntegerModelGurobiExecuter.h"
#include "solver/BinaryModelGurobiExecuter.h"
#include <thread>
#include "solver/MixedModelGurobiExecuter.h"
#include "solver/MIXEDCplexExecuter.h"
#include "solver/LocalSearch.h"
#include "solver/Search.h"
#include "solver/LocalSearchMultiShot.h"
#include "solver/LocalSearchMultiShotRoom.h"
#include "solver/LSDivided.h"


using namespace rapidxml;

Instance *readInputXML(std::string filename);

Instance *readInputXML2007(std::string filename);

Instance *readOutputXML2007(std::string filename, Instance *instance);

void genWeekly(Instance *instance, ILPExecuter *runner, char *string);

void genSingleShot(Instance *instance, ILPExecuter *runner, char *string);

void readOutputXML(std::string filename, Instance *instance);

void writeOutputXML(std::string filename, Instance *instance, double time);

void readPerturbations(std::string filename, Instance *instance);

void addPossibleRooms(Class *c, Instance *instance);


bool cuts = false;
bool warm = false;
bool opt = false;


std::map<int, std::vector<distribution *> *> classConst;
std::map<int, std::vector<distribution *> *> classSoft;


int main(int argc, char **argv) {


    //if (!quiet) std::cout << "Starting Reading File: " << argv[1] << std::endl;
    std::map<int, int> roomCluster;
    std::vector<Cluster *> cluster;
    Instance *instance = readInputXML("/Volumes/MAC/ClionProjects/timetabler/data/input/ITC-2019/wbg-fal10.xml");
    std::cout << instance->getName() << std::endl;
    /*for (int j = 0; j < instance->getClasses().size(); ++j) {
        bool exist=false;
        std::cout<<instance->getClasses()[j]->getId()<<std::endl;
        for (int i = 0; i < instance->getClasses()[j]->getPossibleRooms().size(); ++i) {
            if(roomCluster.find(instance->getClasses()[j]->getPossibleRoom(i).getId())!=roomCluster.end())
                exist=true;
            if(exist){
                cluster[roomCluster.find(instance->getClasses()[j]->getPossibleRoom(i).getId())->second-1]->addClass(instance->getClasses()[j]);
                /*for (int r = 0; r < instance->getClasses()[j]->getPossibleRooms().size(); ++r) {
                    cluster[roomCluster.find(instance->getClasses()[j]->getPossibleRoom(i).getId())->second-1]->addRoom(
                            instance->getClasses()[j]->getPossibleRoom(r).getId());
                    roomCluster.insert(std::pair<int,int>(instance->getClasses()[j]->getPossibleRoom(i).getId(),cluster[roomCluster.find(instance->getClasses()[j]->getPossibleRoom(i).getId())->second-1]->getClusterID()));


                }
                break;
            }

        }
        if(!exist){
            Cluster* c= new Cluster(cluster.size()+1,instance->getClasses()[j]);
            for (int i = 0; i < instance->getClasses()[j]->getPossibleRooms().size(); ++i) {
                c->addRoom(instance->getClasses()[j]->getPossibleRoom(i).getId());
                roomCluster.insert(std::pair<int,int>(instance->getClasses()[j]->getPossibleRoom(i).getId(),c->getClusterID()));
            }
            cluster.push_back(c);

        }
    }*/
    for (int j = 0; j < instance->getClasses().size(); ++j) {
        for (int i = 0; i < instance->getClasses()[j]->getPossibleRooms().size(); ++i) {
            if (roomCluster.find(instance->getClasses()[j]->getPossibleRoom(i).getId()) != roomCluster.end()) {
                cluster[roomCluster.find(instance->getClasses()[j]->getPossibleRoom(i).getId())->second - 1]->addClass(
                        instance->getClasses()[j], i);
            } else {
                Cluster *c = new Cluster(cluster.size() + 1, instance->getClasses()[j], i,
                                         instance->getClasses()[j]->getPossibleRoom(i).getId());
                roomCluster.insert(
                        std::pair<int, int>(instance->getClasses()[j]->getPossibleRoom(i).getId(), c->getClusterID()));
                cluster.push_back(c);

            }

        }
    }
    instance->setClassbyclusterRoom(cluster);
    for (int j = 0; j < instance->getClasses().size(); ++j) {
        int v = 0;

        for (int k = 0; k < instance->getClasses()[j]->getLectures().size(); ++k) {

            if (instance->getClasses()[j]->getPossibleRooms().size() == 0) {
                instance->getClasses()[j]->setPossiblePair(
                        Room(-1),
                        instance->getClasses()[j]->getLectures()[k], v);
                v++;
            } else {
                for (int r = 0; r < instance->getClasses()[j]->getPossibleRooms().size(); ++r) {
                    int roomID = instance->getClasses()[j]->getPossibleRoomPair(r).first.getId();
                    std::string week = instance->getClasses()[j]->getLectures()[k]->getWeeks();
                    std::string day = instance->getClasses()[j]->getLectures()[k]->getDays();
                    int start = instance->getClasses()[j]->getLectures()[k]->getStart();
                    int duration = instance->getClasses()[j]->getLectures()[k]->getLenght();
                    bool isNotAddableTime = false;
                    for (int una = 0; una < instance->getRoom(roomID).getSlots().size(); ++una) {
                        for (int i = 0; i < instance->getNweek(); ++i) {
                            if (week[i] == instance->getRoom(roomID).getSlots()[una].getWeeks()[i] &&
                                instance->getRoom(roomID).getSlots()[una].getWeeks()[i] == '1') {
                                for (int d = 0; d < instance->getNdays(); ++d) {
                                    if (day[d] == instance->getRoom(roomID).getSlots()[una].getDays()[d] &&
                                        day[d] == '1') {

                                        if (start >= instance->getRoom(roomID).getSlots()[una].getStart() &&
                                            start < instance->getRoom(
                                                    roomID).getSlots()[una].getStart() + instance->getRoom(
                                                    roomID).getSlots()[una].getLenght()) {

                                            isNotAddableTime = true;
                                        } else if (instance->getRoom(roomID).getSlots()[una].getStart() >= start &&
                                                   instance->getRoom(
                                                           roomID).getSlots()[una].getStart() < start + duration) {


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
                        instance->getClasses()[j]->setPossiblePair(
                                instance->getClasses()[j]->getPossibleRoomPair(r).first,
                                instance->getClasses()[j]->getLectures()[k], v);
                        v++;
                        // std::cout<<instance->getClasses()[j]->getPossibleRoomPair(r).first<<" "<<instance->getClasses()[j]->getId()<<" "<<*instance->getClasses()[j]->getLectures()[k]<<std::endl;
                    }

                }
            }
        }

    }
    std::cout << "PRE " << getTimeSpent() << std::endl;
    ILPExecuter *runner = new GurobiSimple(instance);
    runner->definedAuxVar();
    std::cout << "AUX " << getTimeSpent() << std::endl;
    runner->dist(true);
    std::cout << "Run " << getTimeSpent() << std::endl;
    runner->run2019(true);
    writeOutputXML("/Volumes/MAC/ClionProjects/timetabler/data/output/ITC-2019/" + instance->getName() +
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

    auto *s = new Search(instance, classesbyCost, 0);
    s->run();
    if (!quiet) std::cout << "Solution Found: Writing output file" << std::endl;
    writeOutputXML("/Volumes/MAC/ClionProjects/timetabler/data/output/ITC-2019/" + instance->getName() +
                   ".xml",
                   instance, getTimeSpent());
    std::exit(0);

    //if (!quiet) std::cout << "Starting Reading File: " << argv[2] << std::endl;
    //readOutputXML(argv[2], instance);
    //if (!quiet) std::cout << "Generating Perturbations based on the file: " << argv[3] << std::endl;
    //readPerturbations(argv[3], instance);

    if (argc >= 7) {
        for (int i = 6; i < argc; ++i) {
            if (strcmp(argv[i], "-c") == 0)
                cuts = true;
            if (strcmp(argv[i], "-w") == 0)
                warm = true;
        }
    }
    if (!quiet) std::cout << "Generating ILP model" << std::endl;
    //ILPExecuter *runner;
    if (strcmp(argv[5], "Integer") == 0) {
        runner = new IntegerModelGurobiExecuter();
        runner->setInstance(instance);
    } else if (strcmp(argv[5], "Binary") == 0) {
        runner = new BinaryModelGurobiExecuter((bool) std::stoi(argv[6]), (bool) std::stoi(argv[7]), instance);
        if (std::stoi(argv[7]) == 1)
            genWeekly(instance, runner, argv[4]);
        else
            genSingleShot(instance, runner, argv[4]);
    } else if (strcmp(argv[5], "Mixed") == 0) {
        runner = new MixedModelGurobiExecuter((bool) std::stoi(argv[6]), (bool) std::stoi(argv[7]), instance);
        if (std::stoi(argv[7]) == 1)
            genWeekly(instance, runner, argv[4]);
        else
            genSingleShot(instance, runner, argv[4]);
    } else if (strcmp(argv[5], "GRASP") == 0) {
        LocalSearch *g = new LocalSearch(std::stoi(argv[6]), std::stoi(argv[7]), instance, 3600);
        g->GRASP();
        std::exit(42);
    } else if (strcmp(argv[5], "LNS") == 0) {
        LocalSearch *g = new LocalSearch(instance);
        g->LNS();
        std::exit(42);
    }


    if (!quiet) std::cout << "Solution Found: Writing output file" << std::endl;
    writeOutputXML("/Volumes/MAC/ClionProjects/timetabler/data/output/ITC-2019/" + instance->getName() + ".xml",
                   instance, getTimeSpent());


    return 0;
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

void genSingleShot(Instance *instance, ILPExecuter *runner, char *string) {
    runner->setCurrrentW(-1); //Comapct weeks before hand??
    if (!quiet) std::cout << "Defined Room Lect :  " << getTimeSpent() << std::endl;
    runner->definedRoomLecture();
    if (!quiet) std::cout << "Defined Room Lect : Done " << getTimeSpent() << std::endl;
    runner->definedLectureTime();
    if (!quiet) std::cout << "Defined var : Done " << getTimeSpent() << std::endl;
    runner->dayConst();
    if (!quiet) std::cout << "Days : Done " << getTimeSpent() << std::endl;
    runner->week();
    if (!quiet) std::cout << "Week : Done " << getTimeSpent() << std::endl;
    //runner->createSol();
    //if (!quiet) std::cout << "Sol: DONE " << getTimeSpent() << std::endl;
    runner->block();
    if (!quiet) std::cout << "Block var : Done " << getTimeSpent() << std::endl;
    runner->dist(true);
    if (!quiet) std::cout << "Dist : Done " << getTimeSpent() << std::endl;
    runner->oneLectureRoom();
    if (!quiet) std::cout << "Lecture : Done " << getTimeSpent() << std::endl;
    runner->oneLectureRoomConflict();
    if (!quiet) std::cout << "Room : Done " << getTimeSpent() << std::endl;
    runner->roomPen();
    if (!quiet) std::cout << "Room Pen : Done " << getTimeSpent() << std::endl;

    if (opt) {
        if (strcmp(string, "Hamming") == 0) {
            if (!quiet) std::cout << "Add optimization: Hamming Distance" << std::endl;
            runner->distanceToSolution(false);
        } else if (strcmp(string, "Weighted") == 0) {
            if (!quiet) std::cout << "Add optimization: Weighted Hamming Distance" << std::endl;
            runner->distanceToSolution(true);
        } else if (strcmp(string, "GAP") == 0) {
            if (!quiet) std::cout << "Add optimization: GAP in a students Timetable" << std::endl;
            runner->optimizeGapStudentsTimetable();
        } else {
            //dist
        }
    }
    if (!quiet) std::cout << "Execute" << std::endl;
    runner->saveEncoding();
    //runner->run2019(warm);
    //runner->save();


}

void genWeekly(Instance *instance, ILPExecuter *runner, char *string) {
    int i = 0;
    int iold = 0;
    while (i < instance->getNweek()) {
        if (i < 0) {
            if (!quiet) std::cout << "No solution found " << getTimeSpent() << std::endl;
            std::exit(0);
        }
        if (!quiet) std::cout << "Week: " << i << " " << getTimeSpent() << std::endl;

        if (iold <= i)
            instance->computeClassesWeek(i);
        if (!quiet)
            std::cout << "Computed week: " << i << " " << getTimeSpent() << std::endl;

        runner->restart();
        runner->setCurrrentW(i);
        if (!quiet) std::cout << "Defined Room Lect :  " << getTimeSpent() << std::endl;
        runner->definedRoomLecture();
        if (!quiet)
            std::cout << "Defined Room Lect : Done " << getTimeSpent() << std::endl;
        runner->definedLectureTime();
        if (!quiet) std::cout << "Defined var : Done " << getTimeSpent() << std::endl;

        if (iold > 0) {
            while (instance->classesbyWeekComparison(i - 1, i)) {
                if (!quiet) std::cout << "Compacting " << getTimeSpent() << std::endl;
                iold = i;
                i++;
            }
            if (!quiet) std::cout << "Compacting: Done " << getTimeSpent() << std::endl;
            runner->loadPreviousWeekSolution(runner->getSolutionTime(), runner->getSolutionRoom());
            if (iold > i) {
                if (!quiet) std::cout << "Force " << getTimeSpent() << std::endl;
                runner->force();
                if (!quiet) std::cout << "Force: Done " << getTimeSpent() << std::endl;
            }
        }
        if (!quiet) std::cout << "While : Done " << getTimeSpent() << std::endl;

        runner->createSol();

        if (!quiet) std::cout << "Sol: DONE " << getTimeSpent() << std::endl;


        runner->definedAuxVar();
        if (!quiet) std::cout << "Defined Auxvar : Done " << getTimeSpent() << std::endl;
        runner->block();
        if (!quiet) std::cout << "Block var : Done " << getTimeSpent() << std::endl;

        runner->oneLectureperSlot();
        if (!quiet) std::cout << "LectureperSlot : Done " << getTimeSpent() << std::endl;

        //if (cuts) runner->cuts();
        runner->dayConst();
        runner->dist(false);
        //runner->roomClose();
        //runner->slotClose();
        //runner->teacher();
        //if (!quiet) std::cout << "Teacher : Done " << getTimeSpent() << std::endl;
        //runner->assignmentInvalid();
        runner->oneLectureRoom();
        if (!quiet) std::cout << "Lecture : Done " << getTimeSpent() << std::endl;


        //runner->studentConflictSolution();
        //if (!quiet) std::cout << "Student : Done " << getTimeSpent() << std::endl;


        runner->oneLectureRoomConflict();
        if (!quiet) std::cout << "Room : Done " << getTimeSpent() << std::endl;

        runner->roomPen();
        if (!quiet) std::cout << "Room Pen : Done " << getTimeSpent() << std::endl;

        //runner->slackStudent();
        //if (!quiet) std::cout << "Slack : Done " << getTimeSpent() << std::endl;

        if (opt) {
            if (strcmp(string, "Hamming") == 0) {
                if (!quiet) std::cout << "Add optimization: Hamming Distance" << std::endl;
                runner->distanceToSolution(false);
            } else if (strcmp(string, "Weighted") == 0) {
                if (!quiet) std::cout << "Add optimization: Weighted Hamming Distance" << std::endl;
                runner->distanceToSolution(true);
            } else if (strcmp(string, "GAP") == 0) {
                if (!quiet) std::cout << "Add optimization: GAP in a students Timetable" << std::endl;
                runner->optimizeGapStudentsTimetable();
            } else {
                //dist
            }
        }
        iold = i;
        if (!quiet) std::cout << "Execute" << std::endl;
        if (runner->run2019(warm)) {
            runner->save();
            i++;
        } else {
            i--;
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
                    instance->getRoom(instance->getClasses()[c]->getSolRoom()).getName().c_str())));
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
    std::map<int, int> classID;


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
                instance->addRoom(Room(roomID[id], id, capacity, travel,
                                       una, type));
                //std::cout<<instance->getRooms().size()<<std::endl;
            }

        } else if (strcmp(n->name(), "courses") == 0) {
            for (const xml_node<> *s = n->first_node(); s; s = s->next_sibling()) {
                char *id;
                std::__1::map<int, std::__1::vector<Subpart *>> config;
                for (const xml_attribute<> *a = s->first_attribute(); a; a = a->next_attribute()) {
                    id = a->value();
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
                                    idclass = atoi(a->value());
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
                                    } else
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
                            Class *c = new Class(idclass, limit, lecv, roomsv, orderID);
                            classMap.insert(std::pair<int, Class *>(orderID, c));
                            classID.insert(std::pair<int, int>(idclass, orderID));
                            orderID++;
                            //addPossibleRooms(c, instance);
                            if (parent != -1)
                                c->setParent(classMap[classID[parent]]);
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
                    int idClassesDist = -1;
                    for (const xml_attribute<> *a = course->first_attribute(); a; a = a->next_attribute()) {
                        idClassesDist = atoi(a->value());
                    }
                    c.push_back(idClassesDist);
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
    instance->setNumClasses();
    for (auto i = classConst.begin(); i != classConst.end(); ++i) {
        instance->getClass(i->first)->setHard(*i->second);
    }
    for (auto i = classSoft.begin(); i != classSoft.end(); ++i) {
        instance->getClass(i->first)->setSoft(*i->second);
    }
    return instance;
}


/** Function to parse data from ITC 2007 **/

//TODO: READ curricular constraints

Instance *readInputXML2007(std::string filename) {
    std::string name;
    xml_document<> doc;
    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "File not found: " + filename << std::endl;
        std::cerr << "Method: readInputXML2007" << std::endl;
        std::exit(11);
    }
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
    int count = 1;
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


        } else if (strcmp("rooms", n->name()) == 0) {
            for (const xml_node<> *ne = n->first_node(); ne; ne = ne->next_sibling()) {
                int size = -1, building = -1;
                char *id;
                for (const xml_attribute<> *a = ne->first_attribute(); a; a = a->next_attribute()) {
                    if (strcmp("id", a->name()) == 0)
                        id = a->value();
                    else if (strcmp("size", a->name()) == 0)
                        size = atoi(a->value());
                    else if (strcmp("building", a->name()) == 0)
                        building = atoi(a->value());

                }
                if (roomID.find(id) == roomID.end()) {
                    int newID = roomID.size();
                    newID++;
                    roomID.insert(std::pair<std::string, int>(id, newID));
                }
                instance->addRoom(Room(roomID[id], id, size));
            }
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
                cL.insert(std::pair<std::string, Course *>(id, c));
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

Instance *readOutputXML2007(std::string filename, Instance *instance) {
    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "File not found: " + filename << std::endl;
        std::cerr << "Method: readOutputXML2007" << std::endl;
        std::exit(11);
    }
    std::string course, room;
    int day, slot;
    int lec = 1;
    while (file >> course >> room >> day >> slot) {
        char *days = new char[instance->getNdays()];
        for (int i = 0; i < instance->getNdays(); ++i) {
            if (i == day)
                days[i] = '1';
            else
                days[i] = '0';

        }
        if (roomID.find(room) == roomID.end()) {
            std::cerr << "Room does not exist: " << room << std::endl;
            std::exit(11);
        }
        instance->getCourse(course)->addSol(lec, roomID[room], room, days, slot);
        lec++;
    }

    return instance;


}

void addPossibleRooms(Class *c, Instance *instance) {
    if (c->getPossibleRooms().size() > 0) {
        for (int i = 0; i < instance->getRooms().size(); ++i) {
            if (instance->getRoom(i + 1).getType().compare(c->getFirstPossibleRoom().getType()) == 0)
                c->addRoom(instance->getRoom(i + 1));
        }
    }

}
