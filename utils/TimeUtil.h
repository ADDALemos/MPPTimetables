//
// Created by Alexandre Lemos on 07/05/2019.
//

#ifndef PROJECT_TIMEUTIL_H
#define PROJECT_TIMEUTIL_H

bool quiet = false; //Print info
clock_t tStart;

double getTimeSpent() { return (double) (clock() - tStart) / CLOCKS_PER_SEC; }


#endif //PROJECT_TIMEUTIL_H
