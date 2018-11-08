#include <iostream>
#include <fstream>
#include <vector>
#include <sstream>
#include "Lecture.h"

#include "Room.h"
#include "Course.h"
#include "distribution.h"
#include "rapid/rapidxml.hpp"
#include "Instance.h"

//#include <libxml++/libxml++.h>
//#include <libxml++/parsers/textreader.h>

using namespace rapidxml;

int main() {
    xml_document<> doc;
    std::ifstream file("/Volumes/MAC/pu-c8-spr07.xml");
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::string content(buffer.str());
    doc.parse<0>(&content[0]);
    xml_node<> *pRoot = doc.first_node();
    Instance *instance;
    std::cout<<doc.first_node()->name()<<std::endl;
    for(const rapidxml::xml_attribute<>* a = pRoot->first_attribute(); a; a = a->next_attribute()) {
        if(strcmp(a->name(),"name")==0)
            instance= new Instance(a->value());
        else if(strcmp(a->name(),"nrDays")==0)
            instance->setNdays(atoi(a->value()));
        else if(strcmp(a->name(),"slotsPerDay")==0)
            instance->setSlotsperday(atoi(a->value()));
        else if(strcmp(a->name(),"nrWeeks")==0)
            instance->setNweek(atoi(a->value()));
    }
    std::cout<<*instance<<std::endl;
    for(const rapidxml::xml_node<>* n = pRoot->first_node()
            ; n
            ; n = n->next_sibling()){
        if(strcmp(n->name(),"optimization")==0) {
            for (const rapidxml::xml_attribute<> *a = n->first_attribute(); a; a = a->next_attribute()) {
                if(strcmp(a->name(),"time")==0) {
                    instance->setTimePen(atoi(a->value()));
                } else if(strcmp(a->name(),"room")==0) {
                    instance->setRoomPen(atoi(a->value()));
                } else if(strcmp(a->name(),"distribution")==0) {
                    instance->setDistributionPen(atoi(a->value()));
                } else if(strcmp(a->name(),"student")==0) {
                    instance->setStudentPen(atoi(a->value()));
                }
            }
        } else if(strcmp(n->name(),"rooms")==0) {
            for(const rapidxml::xml_node<>* s = n->first_node(); s; s = s->next_sibling()){
                int id=-1, capacity=-1;
                for (const rapidxml::xml_attribute<> *a = s->first_attribute(); a; a = a->next_attribute()) {
                    if(strcmp(a->name(),"capacity")==0) {
                        capacity=atoi(a->value());
                    } else if(strcmp(a->name(),"id")==0) {
                        id=atoi(a->value());
                    }
                }
                std::map<int,int> travel;
                std::vector<Unavailability> una;
                for(const rapidxml::xml_node<>* rs = s->first_node(); rs; rs = rs->next_sibling()){
                    if(strcmp(rs->name(),"travel")==0) {
                        int value=-1,room=-1;
                        for (const rapidxml::xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                            if(strcmp(a->name(),"room")==0) {
                                room=atoi(a->value());
                            } else if(strcmp(a->name(),"value")==0) {
                                value=atoi(a->value());
                            }
                        }
                        travel.insert( std::pair<int,int>(room,value) );


                    } else if(strcmp(rs->name(),"unavailable")==0) {
                        char* days,*weeks;
                        int length=-1,start=-1;
                        for (const rapidxml::xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                            if(strcmp(a->name(),"days")==0) {
                                days=a->value();
                            } else if(strcmp(a->name(),"start")==0) {
                                start=atoi(a->value());
                            } else if(strcmp(a->name(),"length")==0) {
                                length=atoi(a->value());
                            } else if(strcmp(a->name(),"weeks")==0) {
                                weeks=a->value();
                            }
                        }
                        Unavailability *u =new Unavailability(days,start,length,weeks);
                        //std::cout<<*u<<std::endl;
                        una.push_back(*u);
                    }


                    }
                Room *r = new Room(id, capacity, travel, una);
             //   std::cout<<*r<<std::endl;
          //      std::cout<<r->getSlots().size()<<std::endl;
                instance->addRoom(r);
                //std::cout<<instance->getRooms().size()<<std::endl;
            }

        } else if(strcmp(n->name(), "courses")==0)   {
            for(const rapidxml::xml_node<>* s = n->first_node(); s; s = s->next_sibling()){
                char *id;
                for (const rapidxml::xml_attribute<> *a = s->first_attribute(); a; a = a->next_attribute()) {
                    id=a->value();
                }
                for(const rapidxml::xml_node<>* rs = s->first_node(); rs; rs = rs->next_sibling()){
                    int id=-1;
                    for (const rapidxml::xml_attribute<> *a = rs->first_attribute(); a; a = a->next_attribute()) {
                        id=atoi(a->value());
                    }
                    for(const rapidxml::xml_node<>* sub = rs->first_node(); sub; sub = sub->next_sibling()){
                        int idsub=-1;
                        for (const rapidxml::xml_attribute<> *a = sub->first_attribute(); a; a = a->next_attribute()) {
                            idsub=atoi(a->value());
                        }
                        for(const rapidxml::xml_node<>* cla = sub->first_node(); cla; cla = cla->next_sibling()){
                            int idclass=-1,limit=-1;
                            for (const rapidxml::xml_attribute<> *a = cla->first_attribute(); a; a = a->next_attribute()) {
                                if(strcmp(a->name(),"id")==0)
                                    idclass=atoi(a->value());
                                else
                                     limit=atoi(a->value());
                            }
                            for(const rapidxml::xml_node<>* lec = cla->first_node(); lec; lec = lec->next_sibling()){
                                if(strcmp(lec->name(),"room")==0) {
                                    for (const rapidxml::xml_attribute<> *a = lec->first_attribute(); a; a = a->next_attribute()) {
                                         //std::cout<<a->name()<<std::endl;
                                        if(strcmp(a->name(),"id")==0)
                                            ;
                                        else if(strcmp(a->name(),"penalty")==0)
                                            ;
                                    }
                                }else if(strcmp(lec->name(),"time")==0){
                                    for (const rapidxml::xml_attribute<> *a = lec->first_attribute(); a; a = a->next_attribute()) {
                                        if (strcmp(a->name(), "length") == 0) {
                                           // if(atoi(a->value())!=22&&atoi(a->value())!=15&&atoi(a->value())!=10)
                                                std::cout << a->value() << std::endl;
                                        }
                                        if (strcmp(a->name(), "start") == 0) {
                                           ;//std::cout << a->value() << std::endl;
                                        }


                                    }

                                }

                                     //limit=atoi(a->value());


                            }
                        }

                    }
                }

            }

        }

                std::cout<<n->name()<<std::endl;
    }
    std::cout<<*instance<<std::endl;

    Lecture *lecture = new Lecture();
    distribution c;
    Room * i;

    return 0;
}