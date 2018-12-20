//
// Created by Alexandre Lemos on 17/12/2018.
//

#ifndef PROJECT_PERTURBATION_H
#define PROJECT_PERTURBATION_H


class Perturbation {
    std::ofstream seedFile;

    std::set<int> randomClassSelection(int classNumber, double factor, unsigned int t);

public:
    std::vector<std::pair<int, int>> randomIncreaseCapacity(int classNumber, int increase, double factor);


};


#endif //PROJECT_PERTUBATION_H
