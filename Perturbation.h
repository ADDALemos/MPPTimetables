//
// Created by Alexandre Lemos on 17/12/2018.
//

#ifndef PROJECT_PERTURBATION_H
#define PROJECT_PERTURBATION_H

static const int NUMBER_OF_COURSES = 4;

#include "problem/Instance.h"
#include<climits>
#include <random>
#include <sstream>

class Perturbation {
    std::ofstream seedFile;


public:
/**
 * Random increase/decrease in the number of attending students
 * @requires This method must be used after randomClassSelection is performed
 * @param classNumber total number of classes
 * @param change maximum/minimum value of increase/decrease in the number of new students
 * @param increase increase or decrease the number of students
 * @param factor random factor
 * @return std::vector<std::pair<int, int>> classes perturbed and the respective increase value
 */
    std::vector<std::pair<int, int>, std::allocator<std::pair<int, int>>>
    randomEnrolment(unsigned int classNumber, int change, bool increase, double factor) {
        unsigned int t = seedHandler();
        std::set<int> classes = randomClassSelection(classNumber, factor, t);
        std::default_random_engine generator(t);
        std::uniform_int_distribution<int> distribution(0, change - 1);
        std::vector<std::pair<int, int>> number;
        for (std::set<int>::iterator it = classes.begin(); it != classes.end(); ++it) {
            if (increase)
                number.push_back(std::pair<int, int>(*it, distribution(generator)));
            else
                number.push_back(std::pair<int, int>(*it, -distribution(generator)));

        }
        return number;


    }


/**
 * Random increase/decrease in the number of attending students
 * @requires This method must be used after randomClassSelection is performed
 * @param classNumber total number of classes
 * @param mean for normal distribution
 * @param Standard deviation
 * @param factor random factor
 * @return std::vector<std::pair<int, int>> classes perturbed and the respective increase value
 */
    std::vector<std::pair<int, int>, std::allocator<std::pair<int, int>>>
    randomEnrolment(unsigned int classNumber, double mean, double deviation, double factor) {
        unsigned int t = seedHandler();
        std::set<int> classes = randomClassSelection(classNumber, factor, t);
        std::default_random_engine generator;
        std::normal_distribution<double> distribution(mean, deviation);
        std::vector<std::pair<int, int>> number;
        for (std::set<int>::iterator it = classes.begin(); it != classes.end(); ++it) {
            number.push_back(std::pair<int, int>(*it, floor(distribution(generator))));
        }
        return number;


    }

/**
 * Random selection of the number classes with wrong assigments
 * @param i problem instance
 * @param factor random factor
 * @return set of classes to perturbed
 */
    void randomClassSelection(Instance *i, double factor) {
        unsigned int t = seedHandler();
        std::set<int> p = randomClassSelection(i->getNumClasses(), factor, t);
        for (int j = 0; j < p.size(); ++j) {
            if (i->isIncorrectAssignment(j))
                j--;
            else {
                i->incorrectAssignment(j);
                std::cout << "Assigment" << j << std::endl;
            }

        }
    }

/**
 * Random selection of the number classes where the number of students will increase
 * @param classNumber total number of classes
 * @param factor random factor
 * @param t seed value
 * @return set of classes to perturbed
 */

    std::set<int> randomClassSelection(int classNumber, double factor, unsigned int t) {
        std::default_random_engine generator(t);
        std::uniform_int_distribution<int> distribution(1, classNumber);
        std::set<int> number;
        int n = 0;
        for (; number.size() < floor(factor * classNumber);) {
            n = distribution(generator);
            number.insert(n);
        }

        return number;


    }

/**
 * Update the instances with the new students
 * @param i problem instance
 * @param changeLimit maximum/minimum number of increase/decrease in the number of students.
 * @param increase increase or decrease in the number of students
 * @param factor random factor
 */
    void randomEnrolmentChanges(Instance *i, int changeLimit, bool increase, double factor) {
        std::vector<std::pair<int, int>> set = randomEnrolment(i->getNumClasses(), changeLimit, increase, factor);
        for (std::pair<int, int> pair: set) {
            i->updateStudentEnrollment(pair.first, pair.second);
        }
    }


/**
 * Update the instances with the new students
 * @param i problem instance
 * @param mean for normal distribution
 * @param Standard deviation
 * @param factor random factor
 */
    void randomEnrolmentChanges(Instance *i, double mean, double deviation, double factor) {
        std::vector<std::pair<int, int>> set = randomEnrolment(i->getNumClasses(), mean, deviation, factor);
        for (std::pair<int, int> pair: set) {
            std::cout << pair.first << " " << pair.second << std::endl;
            i->updateStudentEnrollment(pair.first, pair.second);
        }
    }

/**
 * Choose room to close
 * @param i problem instance
 * @param factor random factor
 */
    void randomCloseRoom(Instance *i, double factor) {
        unsigned int t = seedHandler();
        std::default_random_engine generator(t);
        std::uniform_int_distribution<int> distribution(1, i->getNumRoom());
        int n = 0;
        for (int size = 0; size < floor(factor * i->getNumRoom()); size++) {
            n = distribution(generator);

            i->blockRoom(n);
            std::cout << "Block room: " << n << std::endl;

        }

    }

/**
 * Choose room and a day to close it
 * @param i problem instance
 * @param factor random factor
 */
    void randomCloseRoomDay(Instance *i, double factor) {
        unsigned int t = seedHandler();
        std::default_random_engine generator(t);
        std::uniform_int_distribution<int> distribution(1, i->getNumRoom());
        int n = 0;
        for (int size = 0; size < floor(factor * i->getNumRoom()); size++) {
            n = distribution(generator);
            unsigned int t1 = seedHandler();
            std::default_random_engine generator1(t1);
            std::uniform_int_distribution<int> distribution1(1, i->getNdays());
            int d = -1;
            for (int size1 = 0; size1 < floor(factor * i->getNdays()); size1++) {
                d = distribution1(generator1);
                if (i->isRoomBlockedbyDay(n, d))
                    size1--;
                else {
                    std::string days = "";
                    for (int j = 1; j <= i->getNdays(); ++j) {
                        if (d != j)
                            days.append("0");
                        else
                            days.append("1");
                    }
                    i->blockRoombyDay(n, days);
                    std::cout << "Block room by day: " << n << " day " << days << std::endl;
                }
            }
        }
    }

/**
 * Choose a time slot to close
 * @param i problem instance
 * @param factor random factor
 */
    void randomSlotClose(Instance *i, double factor) {
        unsigned int t = seedHandler();
        std::default_random_engine generator(t);
        std::uniform_int_distribution<int> distribution1(0, i->getNumClasses() - 1);

        int n = 0, c = 0;
        for (int size = 0; size < floor(factor * i->getNumClasses()); size++) {
            c = distribution1(generator);
            std::uniform_int_distribution<int> distribution(0, i->getClasses()[c]->getLectures().size() - 1);
            n = distribution(generator);

            i->setTimeUnavailable(c, n);
            std::cout << "Block time slot:" << n << std::endl;

        }

    }

/**
 * Random increase/decrease in the number of shifts
 * @param i problem instance
 * @param factorCourse random factor for the courses
 * @param factorShift random factor for the subparts
 * @param limit increase and decrease limit
 * @param increase increase or decrease flag
 */

    void randomShiftChange(Instance *i, double factorCourse, double factorShift, int limit, bool increase) {
        unsigned int t = seedHandler();
        std::default_random_engine generator(t);
        std::normal_distribution<float> distribution(1, i->getCourses().size() - 1);//numb of courses not classes
        int course = 0;
        for (int size = 0; size < floor(factorCourse * i->getCourses().size()); size++) {
            course = distribution(generator);
            unsigned int tShift = seedHandler();
            std::default_random_engine generatorShift(tShift);
            std::uniform_int_distribution<int> distributionShift(0,
                                                                 i->getCourse(std::to_string(course))->getNumShifts() -
                                                                 1);
            int shift = 0;
            for (int size1 = 0;
                 size1 < floor(factorShift * i->getCourse(std::to_string(course))->getNumShifts()); size1++) {
                shift = distributionShift(generatorShift);
                unsigned int tAmount = seedHandler();
                std::default_random_engine generatorAmount(tAmount);
                std::uniform_int_distribution<int> distributionAmount(1, limit);
                int amount = distributionAmount(generatorAmount);
                if (increase)
                    i->getCourse(std::to_string(course))->newShift(shift, amount, i->getNumClasses());
                else
                    i->getCourse(std::to_string(course))->deleteShift(shift, amount);
                std::cout << "Shift: courseID " << course << " SubpartID:" << shift << " number:" << amount
                          << std::endl;


            }
        }
    }


/**
 * Random increase/decrease in the number of shifts with normal distribution
 * @param i problem instance
 * @param factorCourse random factor for the courses
 * @param factorShift random factor for the subparts
 * @param median
 * @param Standard deviation
 * @param increase increase or decrease flag
 */

    void randomShiftChange(Instance *i, double factorCourse, double standard, double mean) {
        unsigned int t = seedHandler();
        std::default_random_engine generator(t);
        std::normal_distribution<float> distribution(1, i->getSubparts().size() - 1);//numb of shifts not classes
        int subpart = 0;
        for (int size = 0; size < floor(factorCourse * i->getSubparts().size()); size++) {
            subpart = distribution(generator);
            unsigned int t = seedHandler();
            std::default_random_engine generatorShift(t);
            std::normal_distribution<float> distributionShift(mean, standard);
            int amount = distributionShift(generatorShift);
            i->getSubpart(subpart)->changeShift(amount, i->getNumClasses());
            std::cout << "Shift: subpartID " << subpart << " number:" << amount << std::endl;
        }
    }


/**
 *
 */
    void addNewCurriculum(Instance *i, double lecture, double lenght, double student) {
        std::vector<Subpart *> vector;
        for (int j = 0; j < lecture; ++j) {
            std::vector<Class *> cl;
            Class *c;// = new Class(i->getClasses().size() + 1, student, new Lecture(lenght), std::map<Room, int>(), 0);
            cl.push_back(c);
            std::ostringstream oss;
            oss << " " << (i->getClasses().size() + 1);
            std::string var = oss.str();
            Subpart *s = new Subpart(oss.str(), cl);
            vector.push_back(s);
        }
        std::map<int, std::vector<Subpart *>> ss;
        ss.insert(std::pair<int, std::vector<Subpart *>>(0, vector));
        char *id = (char *) ((i->getCourses().size() + 1) + '0');
        Course *c = new Course(id, ss);
        i->addCourse(c);
    }


    void randomOverlap(Instance *i, double factor) {
        unsigned int t = seedHandler();
        std::default_random_engine generator(t);
        std::uniform_int_distribution<int> distribution(1, i->getClasses().size() - 1);
        std::vector<std::pair<int, int>> l;
        int class1 = distribution(generator);
        for (int size = 0; size < floor(factor * i->getClasses().size()) * 2; size++) {
            l.push_back(std::pair<int, int>(class1, distribution(generator)));
        }

    }


/**
 * Generate a seed based on the clock
 * @return seed
 */
    unsigned int seedHandler() {
        unsigned int t = std::chrono::steady_clock::now().time_since_epoch().count();
        seedFile.open("../log/seed.txt", std::ios_base::app);
        seedFile << t << std::endl;
        seedFile.close();
        return t;
    }


    void randomRoom(Instance *pInstance, double prob, bool isPreference) {

    }
};


#endif //PROJECT_PERTUBATION_H
