//
// Created by Alexandre Lemos on 12/11/2018.
//

#ifndef PROJECT_ILPEXECUTER_H
#define PROJECT_ILPEXECUTER_H
#ifndef IL_STD
#define IL_STD
#endif


#include <ilconcert/iloenv.h>
#include <ilconcert/ilomodel.h>
#include <ilcplex/ilocplex.h>
#include <exception>
#include <stdlib.h>
#include "../problem/Instance.h"
#include "../problem/Constraint.h"

class ILPExecuter {

protected:
    Instance *instance;
    int currentW = -1;

public:

    virtual void definedRoomLecture() {}


    virtual void definedLectureTime() {}

    virtual void definedAuxVar() {}

    /**
     * The lecture can only be scheduled in one slot
     */

    virtual void oneLectureperSlot() {}


    /**
     * Force lectures to be in slot n
     */


    virtual void oneLectureSlot() {}

    /**
     * A lecture can only be in one room at time
     */

    virtual void oneLectureRoom() {}

    /**
     * Ensure room r is used to lecture l
     */
    virtual void roomPreference(int r, int l) {}

    /***
     * The room can only have one lecture per slot
     */

    virtual void oneLectureRoomConflict() {}

    /**
    * Ensure Room closed cannot be used
    */
    virtual void roomClose() {}

    /**
    * Ensure Room closed in a day cannot be used
    */
    virtual void roomClosebyDay() {}


    /**
    * Ensure times lot in a day is closed cannot be used
    */
    virtual void slotClose() {}

    /**
     * One assignment, is invalid and needs to be assigned
     * to a different room or to a different time slot
     */
    virtual void assignmentInvalid() {}

    /**Teacher's conflict*/
    virtual void teacher() {}


    /** Student conflicts hard constraint based on the input model
     *
     */
    virtual void studentConflict() {}

    /** Student conflicts hard constraint based on the original solution
     *
     */
    virtual void studentConflictSolution() {}

    void loadOutput() {
        for (int j = 0; j < instance->getSlotsperday(); j++) {
            for (int k = 0; k < instance->getClasses().size(); k++) {
                int d = 0;
                for (auto c : instance->getClasses()[k]->getSolDays()) {
                    if (c == '1') {
                        if (instance->getClasses()[k]->getSolStart() <= j &&
                            (instance->getClasses()[k]->getSolStart() + instance->getClasses()[k]->getLenght() >
                             j)) {
                            solutionTime[d][j][k] = 1;
                        }
                    }
                    d++;
                }


            }

        }


        for (int i = 0; i < instance->getRooms().size(); ++i) {
            for (int k = 0; k < instance->getClasses().size(); ++k) {
                if ((instance->getClasses()[k]->getSolRoom() - 1) == i)
                    solutionRoom[i][k] = 1;
                else
                    solutionRoom[i][k] = 0;

            }

        }

    }


    virtual void saveEncoding() {}

    virtual void loadPreviousWeekSolution(int ***time, int **room) {}


    /**
     * Creates a hard constriant to the number of students.
     * All students must be seated.
     * For slack cosntraint use slackStudent()
     * @param students
     */
    virtual void constraintSeatedStudents(double students) {}

    virtual void optimizeRoomUsage() {}

    virtual void optimizeSeatedStudents() {}


    virtual void optimizeGapStudentsTimetable() {}

    virtual double run(bool mpp) {}

    /**
     * Create a hard constraint on the number of studnets seated.
     * Can be controled by the value of slack of each instance
     */

    virtual void slackStudent() {}

    Instance *getInstance() const {
        return instance;
    }

    void setInstance(Instance *instance) {
        ILPExecuter::instance = instance;
    }


    virtual void distanceToSolution(bool w) {}

    /**
     * Minimization statement distance solutions
     * TODO: Generalize the distance metric
     * @param oldRoom
     * @param oldTime
     * @param weighted
     */

    virtual void distanceToSolution(int **oldRoom, int ***oldTime, bool weighted) {}

    /**
     * Minimization statement distance between solutions: Number of students seated
     */
    virtual void minimizeDifferenceSeatedStudents() {}


    virtual void cuts() {}

    virtual bool run2019(bool warm) {
        return false;
    }

    virtual void restart() {

    }

    virtual void force() {

    }

    virtual void block() {

    }

    virtual void save() {

    }

public:


    int **getSolutionRoom() const {
        return solutionRoom;
    }

    int ***getSolutionTime() const {
        return solutionTime;
    }

    /**
     * Initialize the solutions structures: solutionTime and solutionTime
     */
    void createSol() {
        solutionTime = new int **[instance->getNdays()];
        for (int i = 0; i < instance->getNdays(); i++) {
            solutionTime[i] = new int *[instance->getSlotsperday()];
            for (int k = 0; k < instance->getSlotsperday(); ++k) {
                solutionTime[i][k] = new int[instance->getClasses().size()];
                for (int j = 0; j < instance->getClasses().size(); ++j) {
                    solutionTime[i][k][j] = 0;
                }

            }

        }
        solutionRoom = new int *[instance->getRooms().size()];
        for (int i = 0; i < instance->getRooms().size(); i++) {
            solutionRoom[i] = new int[instance->getClasses().size()];
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                solutionRoom[i][j] = 0;
            }

        }
    }

    double frequency() {
        double used = 0, all = (instance->getNdays() * instance->getSlotsperday() * instance->getNumRoom());
        for (int k = 0; k < instance->getClasses().size(); ++k) {
            used += instance->getClasses()[k]->getLenght();
        }
        return 100 * used / all;
    }


    double utilization() {
        double used = 0, all = (instance->getNdays() * instance->getSlotsperday() * instance->getNumRoom() *
                                instance->getAvCapacity());
        for (int k = 0; k < instance->getClasses().size(); ++k) {
            used += instance->getClasses()[k]->getLenght() * instance->getClasses()[k]->getLimit();
        }
        return 100 * used / all;

    }

    int getCurrrentW() const {
        return currentW;
    }

    virtual void setCurrrentW(int currrentW) {
        ILPExecuter::currentW = currrentW;
    }

protected:
    int **solutionRoom;
    int ***solutionTime;

    /**
     * Print the current value of the solutionTime
     */
    void printsolutionTime() {

        std::cout << "d t l" << std::endl;
        for (int i = 0; i < instance->getClasses().size(); i++) {
            for (int d = 0; d < instance->getNdays(); d++) {
                for (int t = 0; t < instance->getSlotsperday(); t++) {
                    if (solutionTime[d][t][i] != 0) {
                        std::cout << d << " t: " << t << " l: " << instance->getClasses()[i]->getId() << std::endl;
                    }
                }

            }

        }


    }

    /**
     * Remove old solution
     * TODO: Memory management
     */
    void removeSolution() {
//        delete roomLecture;
        //      delete lectureTime;
        createSol();
    }


    /**
     * Print the current value of the solutionRoom
     */
    void printRoomSolution() {
        std::cout << "room solution" << std::endl;
        std::cout << "r c" << std::endl;
        for (int i = 0; i < instance->getRooms().size(); i++) {
            for (int j = 0; j < instance->getClasses().size(); ++j) {
                if (solutionRoom[i][j] != 0) {
                    std::cout << instance->getRoom(i + 1)->getName() << " " << instance->getClasses()[j]->getId() << " "
                              << solutionRoom[i][j] << std::endl;
                }
            }

        }
    }

    /**
     * Number of GAPS in a solution
     */

    int gapsSolution() {
        int count = 0;
        for (int i = 0; i < instance->getStudent().size(); ++i) {
            for (int j = 0; j < instance->getNdays(); ++j) {
                for (int k = 1; k < instance->getSlotsperday(); ++k) {
                    int before = 0, after = 0;
                    for (int l = 0; l < instance->getStudent(i + 1).getClasses().size(); ++l) {
                        before += solutionTime[j][k][l];
                        after += solutionTime[j][k - 1][l];
                    }
                    if (before != after)
                        count++;

                }
            }
        }
        return count;
    }


    void validator() {
        for (int l = 0; l < instance->getClasses().size(); ++l) {
            int lecture = 0;
            for (int r = 0; r < instance->getRooms().size(); ++r) {
                lecture += solutionRoom[r][l];
            }
            if (lecture != 1)
                std::cout << "Invalid solution lecture: " << instance->getClasses()[l]->getId() << std::endl;

        }
        for (int r = 0; r < instance->getRooms().size(); ++r) {
            for (int d = 0; d < instance->getNdays(); d++) {
                for (int t = 0; t < instance->getSlotsperday(); t++) {
                    int lecture = 0;
                    for (int i = 0; i < instance->getClasses().size(); i++)
                        lecture += solutionTime[d][t][i] * solutionRoom[r][i];
                    if (lecture > 1) {
                        std::cout << "!!Invalid solution room!!: " << instance->getRoom(r)->getName() << " " << d << " "
                                  << t << " lectures: ";
                        for (int i = 0; i < instance->getClasses().size(); i++)
                            if (solutionTime[d][t][i] * solutionRoom[r][i] == 1)
                                std::cout << instance->getClasses()[i]->getDay() << " "
                                          << instance->getRoom(instance->getClasses()[i]->getSolRoom())->getName() << " "
                                          << instance->getClasses()[i]->getSolStart() << " \n";
                        std::cout << std::endl;
                    }
                }
            }


        }


    }
public:



    virtual void dist(bool hard) {

    }


    virtual void roomUnavailable() {

    }


    virtual void week() {

    }
};


#endif //PROJECT_ILPEXECUTER_H