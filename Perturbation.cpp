//
// Created by Alexandre Lemos on 17/12/2018.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <map>
#include <chrono>
#include <set>
#include <random>
#include <sstream>
#include "Perturbation.h"
#include "Instance.h"

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
Perturbation::randomEnrolment(unsigned int classNumber, int change, bool increase, double factor) {
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
 * Random selection of the number classes where the number of students will increase
 * @param classNumber total number of classes
 * @param factor random factor
 * @param t seed value
 * @return set of classes to perturbed
 */

std::set<int> Perturbation::randomClassSelection(int classNumber, double factor, unsigned int t) {

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
void Perturbation::randomEnrolmentChanges(Instance *i, int changeLimit, bool increase, double factor) {
    std::vector<std::pair<int, int>> set = randomEnrolment(i->getNumClasses(), changeLimit, increase, factor);
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
void Perturbation::randomCloseRoom(Instance *i, double factor) {
    unsigned int t = seedHandler();
    std::default_random_engine generator(t);
    std::uniform_int_distribution<int> distribution(1, i->getNumRoom());
    int n = 0;
    for (int size = 0; size < floor(factor * i->getNumRoom()); size++) {
        n = distribution(generator);
        if (i->isRoomBlocked(n))
            size--;
        else {
            i->blockRoom(n);
            std::cout << "Block room:" << n << std::endl;
        }
    }

}

/**
 * Choose a time slot to close
 * @param i problem instance
 * @param factor random factor
 */
void Perturbation::randomSlotClose(Instance *i, double factor) {
    unsigned int t = seedHandler();
    std::default_random_engine generator(t);
    std::uniform_int_distribution<int> distribution(1, i->getSlotsperday() * i->getNdays());
    int n = 0;
    for (int size = 0; size < floor(factor * i->getSlotsperday() * i->getNdays()); size++) {
        n = distribution(generator);
        if (i->isTimeUnavailable(n))
            size--;
        else {
            i->setTimeUnavailable(n);
            std::cout << "Block time slot:" << n << std::endl;
        }
    }

}

/**
 * Genarete a seed based on the clock
 * @return seed
 */
unsigned int Perturbation::seedHandler() {
    unsigned int t = std::chrono::steady_clock::now().time_since_epoch().count();
    seedFile.open("seed.txt", std::ios_base::app);
    seedFile << t << std::__1::endl;
    return t;
}

