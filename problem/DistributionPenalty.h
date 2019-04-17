//
// Created by Alexandre Lemos on 05/11/2018.
//

#ifndef PROJECT_DISTRIBUTIONPENALTY_H
#define PROJECT_DISTRIBUTIONPENALTY_H


#include "distribution.h"
#include "NoLimit.h"

class DistributionPenalty : public  distribution{
    int penalty;

public:
    DistributionPenalty(Constraint *pLimit, std::vector<int, std::allocator<int>> classes, int penalty) :
            distribution(classes, *pLimit), penalty(penalty) {}

    virtual int getPenalty() { return penalty; }

};


#endif //PROJECT_DISTRIBUTIONPENALTY_H
