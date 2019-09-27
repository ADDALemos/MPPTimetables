

#ifndef __XML_WRITE__
#define __XML_WRITE__

#include <iostream>
#include <fstream>
#include <vector>
#include <codecvt>
#include <sstream>
#include <set>
#include <list>
#include <string>
#include <thread>
#include "problem/Lecture.h"
#include "problem/Room.h"
#include "problem/ClusterStudents.h"

#include "rapidXMLParser/rapidxml.hpp"
#include "rapidXMLParser/rapidxml_print.hpp"
#include "problem/ClusterbyRoom.h"

#include "problem/Instance.h"
#include "randomGenerator/Perturbation.h"

#include "problem/Time.h"
#include "problem/Curriculum.h"
#include "problem/ConstraintShort.h"
#include "utils/TimeUtil.h"

using namespace rapidxml;



static void writeXMLInput(Instance *instance, int version, Curriculum *cur) {
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

static void writeXMLOutput(std::string filename, Instance *instance) {

    xml_document<> doc;
    xml_node<> *decl = doc.allocate_node(node_declaration);
    decl->append_attribute(doc.allocate_attribute("version", "1.0"));
    decl->append_attribute(doc.allocate_attribute("encoding", "utf-8"));
    doc.append_node(decl);

    xml_node<> *root = doc.allocate_node(node_element, "solution");
    root->append_attribute(doc.allocate_attribute("name", instance->getName().c_str()));
    std::cout<<getTimeSpent()<<std::endl;
    root->append_attribute(doc.allocate_attribute("runtime", "0"));
    root->append_attribute(
            doc.allocate_attribute("cores", "0"));//std::to_string(std::thread::hardware_concurrency()).c_str()));
    root->append_attribute(doc.allocate_attribute("technique", instance->getMethod().c_str()));
    root->append_attribute(doc.allocate_attribute("author", "Alexandre Lemos, Pedro T. Monteiro, Ines Lynce"));
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
    file_stored.imbue(std::locale("en_US.UTF-8"));

    //print(std::cout, doc, 0);
    file_stored << doc;

    file_stored.close();
    doc.clear();

}


#endif // __PB_PARSER__

/*****************************************************************************/
