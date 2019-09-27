//
// Created by Alexandre Lemos on 07/05/2019.
//

#ifndef PROJECT_TIMEUTIL_H
#define PROJECT_TIMEUTIL_H

const bool quiet = true; //Print info
const clock_t myTimeStart = clock();

inline double getTimeSpent() { return (double) (clock() - myTimeStart) / CLOCKS_PER_SEC; }

inline void printTime() { std::cout << getTimeSpent() << std::endl; }


#endif //PROJECT_TIMEUTIL_H
