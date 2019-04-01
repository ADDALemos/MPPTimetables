//
// Created by Alexandre Lemos on 11/03/2019.
//

#ifndef PROJECT_GUROBIAUX_H
#define PROJECT_GUROBIAUX_H


#include <gurobi_c++.h>

GRBEnv env = GRBEnv();

GRBModel *model = new GRBModel(env);

std::string itos(int i) {
    std::stringstream s;
    s << i;
    return s.str();
}

void printError(const GRBException &e, std::string local) {
    std::cout << "Error found: " << local << std::endl;
    std::cout << "Error code = " << e.getErrorCode() << std::endl;
    std::cout << e.getMessage() << std::endl;
}

void restartModel() {
    model = new GRBModel(env);
}


#endif //PROJECT_GUROBIAUX_H
