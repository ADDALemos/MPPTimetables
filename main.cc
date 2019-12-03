/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
  * MiniSat,  Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
 *           Copyright (c) 2007-2010, Niklas Sorensson
 * Open-WBO, Copyright (c) 2013-2017, Ruben Martins, Vasco Manquinho, Ines Lynce
 * Open-WBO-Inc Copyright (c) 2018  Saurabh Joshi, Prateek Kumar, Ruben Martins, Sukrut Rao
 * TT-Open-WBO-Inc Copyright (c) 2019 Alexander Nadel
 * Timetabler Copyright (c) 2019 Alexandre Lemos, Pedro T Monteiro, Ines Lynce
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include "utils/Options.h"
#include "utils/ParseUtils.h"
#include "utils/System.h"
#include <errno.h>
#include <signal.h>
#include <zlib.h>

#include <fstream>
#include <iostream>
#include <map>
#include <stdlib.h>
#include <string>
#include <vector>

#ifdef SIMP
#include "simp/SimpSolver.h"
#else

#include "core/Solver.h"

#endif

#include "Perturbation.h"
#include "MaxSAT.h"
#include "MaxTypes.h"
#include "ParserMaxSAT.h"
#include "ParserPB.h"
#include "ParserXML.h"
#include "Torc.h"

// Algorithms
#include "algorithms/Alg_LinearSU.h"
#include "algorithms/Alg_LinearSU_Clustering.h"
#include "algorithms/Alg_LinearSU_Mod.h"
#include "algorithms/Alg_MSU3.h"
#include "algorithms/Alg_OLL.h"
#include "algorithms/Alg_OLL_Mod.h"
#include "algorithms/Alg_PartMSU3.h"
#include "algorithms/Alg_WBO.h"
#include "algorithms/Alg_OBV.h"
#include "algorithms/Alg_BLS.h"


#define VER1_(x) #x
#define VER_(x) VER1_(x)
#define SATVER VER_(SOLVERNAME)
#define VER VER_(VERSION)


using NSPACE::BoolOption;
using NSPACE::IntOption;
using NSPACE::IntRange;
using NSPACE::OutOfMemoryException;
using NSPACE::StringOption;
using NSPACE::cpuTime;
using NSPACE::parseOptions;
using namespace openwbo;

//=================================================================================================

static MaxSAT *mxsolver;

void printCNF(MaxSATFormula *f, std::string s);

void printCurricular(Instance *instance);

void createSmallerInstances(Instance *);

void printClusterofStudents(Instance *instance);


void readOutputXML(std::string filename, Instance *instance);



static void SIGINT_exit(int signum) {
    mxsolver->printAnswer(_UNKNOWN_);
    exit(_UNKNOWN_);
}

#include "Test.h"
#include "ParserXMLTwo.h"
#include "WriteXML.h"
#include "LocalSearch.h"



//=================================================================================================
// Main:

int main(int argc, char **argv) {
    double initial_time = cpuTime();

    try {
#if defined(__linux__)
        fpu_control_t oldcw, newcw;
        _FPU_GETCW(oldcw);
        newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
        _FPU_SETCW(newcw);
        printf(
            "c WARNING: for repeatability, setting FPU to use double precision\n");
#endif


        BoolOption printmodel("Open-WBO", "print-model", "Print model.\n", true);

        IntOption num_tests("Test", "num_tests", "Number of tests\n", 0,
                            IntRange(0, 10000000));

        IntOption test_rhs("Test", "test_rhs",
                           "RHS for a custom encoding test\n", 0,
                           IntRange(0, 10000000));

        IntOption test_rhs2("Test", "test_rhs2",
                            "RHS for a custom encoding test for the second tree\n", 0,
                            IntRange(0, 10000000));

        IntOption test_nsoft("Test", "test_nsoft",
                             "Nsoft for a custom encoding test\n", 0,
                             IntRange(0, 10000000));

        IntOption test_join("Test", "test_join",
                            "Join for a custom encoding test\n", 0, IntRange(0, 1));

        IntOption verbosity("Open-WBO", "verbosity",
                            "Verbosity level (0=minimal, 1=more).\n", 0,
                            IntRange(0, 1));

        IntOption algorithm("Open-WBO", "algorithm",
                            "Search algorithm "
                                    "(_ALGORITHM_WBO_ = 0,_ALGORITHM_LINEAR_SU_,_ALGORITHM_MSU3_,"
                "_ALGORITHM_PART_MSU3_,_ALGORITHM_OLL_,_ALGORITHM_BEST_,_ALGORITHM_LSU_CLUSTER_,"
                "_ALGORITHM_LSU_MRSBEAVER_,_ALGORITHM_LSU_MCS_\n",
                            6, IntRange(0, 8));

        IntOption partition_strategy("PartMSU3", "partition-strategy",
                                     "Partition strategy (0=sequential, "
                                             "1=sequential-sorted, 2=binary)"
                                             "(only for unsat-based partition algorithms).",
                                     2, IntRange(0, 2));

        IntOption graph_type("PartMSU3", "graph-type",
                             "Graph type (0=vig, 1=cvig, 2=res) (only for unsat-"
                                     "based partition algorithms).",
                             2, IntRange(0, 2));

        BoolOption bmo("Open-WBO", "bmo", "BMO search.\n", true);

        IntOption cardinality("Encodings", "cardinality",
                              "Cardinality encoding (0=cardinality networks, "
                                      "1=totalizer, 2=modulo totalizer).\n",
                              1, IntRange(0, 2));

        IntOption amo("Encodings", "amo", "AMO encoding (0=Ladder).\n", 0,
                      IntRange(0, 0));

        IntOption pb("Encodings", "pb", "PB encoding (0=SWC,1=GTE,2=GTECluster).\n",
                     1, IntRange(0, 2));

        IntOption formula("Open-WBO", "formula",
                          "Type of formula (0=WCNF, 1=OPB).\n", 0, IntRange(0, 1));
        IntOption cpu_lim("Open-WBO", "cpu-lim",
                          "Limit on CPU time allowed in seconds.\n", 0,
                          IntRange(0, INT_MAX));

        IntOption weight(
                "WBO", "weight-strategy",
                "Weight strategy (0=none, 1=weight-based, 2=diversity-based).\n", 2,
                IntRange(0, 2));

        BoolOption symmetry("WBO", "symmetry", "Symmetry breaking.\n", true);

        IntOption symmetry_lim(
                "WBO", "symmetry-limit",
                "Limit on the number of symmetry breaking clauses.\n", 500000,
                IntRange(0, INT32_MAX));

        IntOption cluster_algorithm("Clustering", "ca",
                                    "Clustering algorithm "
                                            "(0=none, 1=DivisiveMaxSeparate)",
                                    1, IntRange(0, 1));
        IntOption num_clusters("Clustering", "c", "Number of agglomerated clusters",
                               100000, IntRange(1, INT_MAX));

        IntOption rounding_strategy(
                "Clustering", "rs",
                "Statistic used to select"
                        " common weights in a cluster (0=Mean, 1=Median, 2=Min)",
                0, IntRange(0, 2));


        IntOption num_conflicts(
                "Incomplete", "conflicts", "Limit on the number of conflicts.\n", 10000,
                IntRange(0, INT32_MAX));

        IntOption num_iterations(
                "Incomplete", "iterations", "Limit on the number of iterations.\n", 100000,
                IntRange(0, INT32_MAX));

        BoolOption local("Incomplete", "local", "Local limit on the number of conflicts.\n", false);

        BoolOption polConservative("TorcOpenWbo", "conservative", "Apply conservative polarity heuristic?\n", true);
        BoolOption conservativeUseAllVars("TorcOpenWbo", "conservative_use_all_vars",
                                          "Re-use the polarity of all the variables within the conservative approach (or, otherwise, only the initial once)?\n",
                                          true);
        BoolOption polOptimistic("TorcOpenWbo", "optimistic", "Set target variables' polarity to the optimum?\n", true);
        IntOption targetVarsBumpVal("TorcOpenWbo", "target_vars_bump_val",
                                    "Bump factor of the activity of the targets at the beginning\n", 113);
        BoolOption targetVarsBumpRelWeights("TorcOpenWbo", "target_vars_bump_rel_weights",
                                            "Bump the variable scores, where the bump value is relative to the weights?\n",
                                            true);

        IntOption targetVarsBumpMaxRandVal("TorcOpenWbo", "target_vars_bump_max_rand_val",
                                           "Maximal random bump factor\n", 552);
        BoolOption optC1("Timetabler", "opt-allocation",
                         "Optimality for Allocation?\n",
                         true);
        BoolOption optC2("Timetabler", "opt-stu",
                         "Optimality for Student?\n",
                         true);
        BoolOption optC3("Timetabler", "opt-cons",
                         "Optimality for Constraint?\n",
                         true);

        BoolOption dRoom("Timetabler", "invalid-room",
                         "Disruption type invalid room?\n",
                         false);
        BoolOption dTime("Timetabler", "invalid-time",
                         "Disruption type invalid time?\n",
                         false);
        IntOption dRoomp("Timetabler", "invalid-room-p",
                         "Probability for disruption type invalid room?\n",
                         25);
        IntOption dTimep("Timetabler", "invalid-time-p",
                         "Probability for disruption type invalid time?\n",
                         21);

        parseOptions(argc, argv, true);

        if ((int) num_tests) {
            if ((int) test_join) {
                for (int i = 0; i < (int) num_tests; i++) {
                    test_encoding_join();
                }
            } else {
                for (int i = 0; i < (int) num_tests; i++) {
                    test_encoding();
                }
            }

            return 0;
        }

        Torc::Instance()->SetPolConservative(polConservative);
        Torc::Instance()->SetConservativeAllVars(conservativeUseAllVars);
        Torc::Instance()->SetPolOptimistic(polOptimistic);
        Torc::Instance()->SetTargetVarsBumpVal(targetVarsBumpVal);
        Torc::Instance()->SetBumpRelWeights(targetVarsBumpRelWeights);
        Torc::Instance()->SetTargetBumpMaxRandVal(targetVarsBumpMaxRandVal);

        MaxSAT *S = NULL;

        Statistics rounding_statistic =
                static_cast<Statistics>((int) rounding_strategy);

        switch ((int) algorithm) {
            case _ALGORITHM_WBO_:
                S = new WBO(verbosity, weight, symmetry, symmetry_lim);
                break;

            case _ALGORITHM_LINEAR_SU_:
                if ((int) (cluster_algorithm) == 1) {
                    S = new LinearSUMod(verbosity, bmo, cardinality, pb,
                                        ClusterAlg::_DIVISIVE_, rounding_statistic,
                                        (int) (num_clusters));
                } else {
                    S = new LinearSU(verbosity, bmo, cardinality, pb);
                }
                break;

            case _ALGORITHM_PART_MSU3_:
                S = new PartMSU3(verbosity, partition_strategy, graph_type, cardinality);
                break;

            case _ALGORITHM_MSU3_:
                S = new MSU3(verbosity);
                break;

            case _ALGORITHM_LSU_CLUSTER_:
                S = new LinearSUClustering(verbosity, bmo, cardinality, pb,
                                           ClusterAlg::_DIVISIVE_, rounding_statistic,
                                           (int) (num_clusters));
                break;

            case _ALGORITHM_LSU_MRSBEAVER_:
                S = new OBV(verbosity, cardinality, num_conflicts, num_iterations, local);
                break;

            case _ALGORITHM_LSU_MCS_:
                S = new BLS(verbosity, cardinality, num_conflicts, num_iterations, local);
                break;

            case _ALGORITHM_OLL_:
                if ((int) (cluster_algorithm) == 1) {
                    S = new OLLMod(verbosity, cardinality, ClusterAlg::_DIVISIVE_,
                                   rounding_statistic, (int) (num_clusters));
                } else {
                    S = new OLL(verbosity, cardinality);
                }
                break;

            case _ALGORITHM_BEST_:
                break;

            default:
                printf("c Error: Invalid MaxSAT algorithm.\n");
                printf("s UNKNOWN\n");
                exit(_ERROR_);
        }
        signal(SIGXCPU, SIGINT_exit);
        signal(SIGTERM, SIGINT_exit);

        MaxSATFormula *maxsat_formula = new MaxSATFormula();
        maxsat_formula->setFormat(_FORMAT_PB_);
        ParserXMLTwo *parserXML = new ParserXMLTwo(maxsat_formula, optC1,
                                                   optC2, optC3);

        parserXML->parse(argv[1]);


        parserXML->aux();
        parserXML->room();


        if (cpu_lim != 0) parserXML->getInstance()->setTime(cpu_lim);


        parserXML->getInstance()->setAlgo((int) algorithm, optC1? "true" : "false",
                                          optC2? "true" : "false", optC3?"true" : "false");

        parserXML->genConstraint();
        if (dRoom) {
            Perturbation *p = new Perturbation();
            p->randomCloseRoom(parserXML->getInstance(), dRoomp);
            parserXML->closeroom();
        }
        if (dTime) {
            Perturbation *p = new Perturbation();
            p->randomSlotClose(parserXML->getInstance(), dTime);
            parserXML->una();

        }
        if (dRoom || dTime) {
            readOutputXML("data/output/ITC-2019/solution-" + parserXML->getInstance()->getName() + ".xml",
                          parserXML->getInstance());
            parserXML->distanceToSolutionLectures();
        }



        S->setInstance(parserXML->getInstance());


        printf("c |                                                                "
                       "                                       |\n");
        printf("c ========================================[ Problem Statistics "
                       "]===========================================\n");
        printf("c |                                                                "
                       "                                       |\n");

        if (maxsat_formula->getFormat() == _FORMAT_MAXSAT_)
            printf(
                    "c |  Problem Format:  %17s                                         "
                            "                          |\n",
                    "MaxSAT");
        else
            printf(
                    "c |  Problem Format:  %17s                                         "
                            "                          |\n",
                    "PB");

        if (maxsat_formula->getProblemType() == _UNWEIGHTED_)
            printf("c |  Problem Type:  %19s                                         "
                           "                          |\n",
                   "Unweighted");
        else
            printf("c |  Problem Type:  %19s                                         "
                           "                          |\n",
                   "Weighted");

        printf("c |  Number of variables:  %12d                                    "
                       "                               |\n",
               maxsat_formula->nVars());
        printf("c |  Number of hard clauses:    %7d                                "
                       "                                   |\n",
               maxsat_formula->nHard());
        printf("c |  Number of soft clauses:    %7d                                "
                       "                                   |\n",
               maxsat_formula->nSoft());
        printf("c |  Number of cardinality:     %7d                                "
                       "                                   |\n",
               maxsat_formula->nCard());
        printf("c |  Number of PB :             %7d                                "
                       "                                   |\n",
               maxsat_formula->nPB());
        printf("c |  Avg CLause size :             %7f                                "
                       "                                   |\n",
               maxsat_formula->avgClauseSize());
        double parsed_time = cpuTime();

        printf("c |  Parse time:           %12.2f s                                "
                       "                                 |\n",
               parsed_time - initial_time);
        printf("c |                                                                "
                       "                                       |\n");

        S->loadFormula(maxsat_formula);
        if ((int) (cluster_algorithm) == 1) {
            switch ((int) algorithm) {
                case _ALGORITHM_LINEAR_SU_:
                    static_cast<LinearSUMod *>(S)->initializeCluster();
                    break;
                case _ALGORITHM_OLL_:
                    static_cast<OLLMod *>(S)->initializeCluster();
                    break;
                case _ALGORITHM_LSU_CLUSTER_:
                    static_cast<LinearSUClustering *>(S)->initializeCluster();
                    break;
            }
        }


        std::cout << S->search() << std::endl;

        if(parserXML->getInstance()->getStudent().size()) {


            maxsat_formula = new MaxSATFormula();
            parserXML->genStudents(maxsat_formula);
            parserXML->sameTime();


            S = new LinearSU(verbosity, bmo, cardinality, pb);
            S->setInstance(parserXML->getInstance());
            maxsat_formula->setFormat(_FORMAT_PB_);


            S->loadFormula(maxsat_formula);

            printf("c |                                                                "
                           "                                       |\n");
            printf("c ========================================[ Problem Statistics "
                           "]===========================================\n");
            printf("c |                                                                "
                           "                                       |\n");

            if (maxsat_formula->getFormat() == _FORMAT_MAXSAT_)
                printf(
                        "c |  Problem Format:  %17s                                         "
                                "                          |\n",
                        "MaxSAT");
            else
                printf(
                        "c |  Problem Format:  %17s                                         "
                                "                          |\n",
                        "PB");

            if (maxsat_formula->getProblemType() == _UNWEIGHTED_)
                printf("c |  Problem Type:  %19s                                         "
                               "                          |\n",
                       "Unweighted");
            else
                printf("c |  Problem Type:  %19s                                         "
                               "                          |\n",
                       "Weighted");

            printf("c |  Number of variables:  %12d                                    "
                           "                               |\n",
                   maxsat_formula->nVars());
            printf("c |  Number of hard clauses:    %7d                                "
                           "                                   |\n",
                   maxsat_formula->nHard());
            printf("c |  Number of soft clauses:    %7d                                "
                           "                                   |\n",
                   maxsat_formula->nSoft());
            printf("c |  Number of cardinality:     %7d                                "
                           "                                   |\n",
                   maxsat_formula->nCard());
            printf("c |  Number of PB :             %7d                                "
                           "                                   |\n",
                   maxsat_formula->nPB());
            parsed_time = cpuTime();

            printf("c |  Parse time:           %12.2f s                                "
                           "                                 |\n",
                   parsed_time - initial_time);
            printf("c |                                                                "
                           "                                       |\n");
            std::cout << S->search() << std::endl;

            LocalSearch *l = new LocalSearch(parserXML->getInstance());
            l->LNS();
        }









    } catch (OutOfMemoryException &) {
        sleep(1);
        printf("c Error: Out of memory.\n");
        printf("s UNKNOWN\n");
        exit(_ERROR_);
    }
}


void createSmallerInstances(Instance *instance) {
    int i = 0;
    for (Curriculum *c: instance->getProblem()) {
        writeXMLInput(instance, i, c);
        i++;
    }

}

void printCurricular(Instance *instance) {
    for (Curriculum *c: instance->getProblem()) {
        std::cout << "New" << std::endl;
        for (ClusterbyRoom *clus: c->getPClass()) {
            for (Class *c: clus->getClasses()) {
                std::cout << "C" << c->getId() << std::endl;
            }

            for (Room *c: clus->getRooms()) {
                std::cout << "R" << c->getId() << std::endl;
            }

        }
    }
}

void printClusterofStudents(Instance *instance) {
    for (int l = 0; l < instance->getClusterStudent().size(); ++l) {
        std::cout << "NEW CLUSTER " << instance->getClusterStudent()[l]->getId() << " "
                  << instance->getClusterStudent()[l]->getStudent().size() << std::endl;
        for (Student s: instance->getClusterStudent()[l]->getStudent()) {
            std::cout << "S " << s.getId() << " C ";
            for (int j = 0; j < instance->getClusterStudent()[l]->getCourses().size(); ++j) {
                std::cout << instance->getClusterStudent()[l]->getCourses()[j]->getName() << " ";
            }
            std::cout << std::endl;

        }

    }
}

void printCNF(MaxSATFormula *f, std::string s) {
    int w = 0;
    for (int i = 0; i < f->nSoft(); i++) {
        w += f->getSoftClause(i).weight;
    }
    w++;
    std::ofstream file_stored(s + ".wcnf");

    file_stored << "p wcnf " << f->nVars() << " " << (f->nSoft() + f->nHard()) << " " << w << std::endl;
    for (int i = 0; i < f->nHard(); i++) {
        file_stored << w << " ";
        for (int l = 0; l < f->getHardClause(i).clause.size(); l++) {
            file_stored << (sign(f->getHardClause(i).clause[l]) ? "" : "-") << toInt(f->getHardClause(i).clause[l])
                        << " ";
        }
        file_stored << "0" << std::endl;
    }

    for (int i = 0; i < f->nSoft(); i++) {
        file_stored << f->getSoftClause(i).weight << " ";
        for (int l = 0; l < f->getSoftClause(i).clause.size(); l++) {
            file_stored << (sign(f->getHardClause(i).clause[l]) ? "" : "-") << toInt(f->getSoftClause(i).clause[l])
                        << " ";
        }
        file_stored << "0" << std::endl;
    }

    file_stored.close();

}



void readOutputXML(std::string filename, Instance *instance) {
    xml_document<> doc;
    std::ifstream file(filename);
    if (file.fail()) {
        std::cerr << "File not found: " + filename << std::endl;
        std::cerr << "Method: readOutputXML" << std::endl;
        std::exit(11);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();
    std::__1::string content(buffer.str());
    doc.parse<0>(&content[0]);
    xml_node<> *pRoot = doc.first_node();
    for (const xml_attribute<> *a = pRoot->first_attribute(); a; a = a->next_attribute()) {
        if (strcmp(a->name(), "name") == 0 && strcmp(a->value(), instance->getName().c_str()) != 0) {
            std::cerr << "Instance and Solution do not match" << std::endl;
        }
    }
    int total = 0;
    for (const xml_node<> *n = pRoot->first_node(); n; n = n->next_sibling()) {
        std::string weeks = " ", days = " ", room = " ";
        int id = -1, start = -1;
        for (const xml_attribute<> *a = n->first_attribute(); a; a = a->next_attribute()) {
            if (strcmp(a->name(), "id") == 0) {
                id = atoi(a->value());
            } else if (strcmp(a->name(), "start") == 0) {
                start = atoi(a->value());
            } else if (strcmp(a->name(), "room") == 0) {
                room = a->value();
            } else if (strcmp(a->name(), "weeks") == 0) {
                weeks = a->value();
            } else if (strcmp(a->name(), "days") == 0) {
                days = a->value();
            }
        }
        Class *s = instance->getClassbyId(id);
        if (std::strcmp(room.c_str(), " ") != 0) {
            s->setSolution(start, atoi(room.c_str()), room, weeks, days);
        } else
            s->setSolution(start, weeks, days);

        std::vector<int> student;
        for (const xml_node<> *stu = n->first_node(); stu; stu = stu->next_sibling()) {
            for (const xml_attribute<> *a = stu->first_attribute(); a; a = a->next_attribute()) {
                student.push_back(atoi(a->value()));
                instance->getStudent(atoi(a->value())).addClass(s);
            }
        }

        s->addStudents(student);
        total += s->getSteatedStudents();
    }
    instance->setTotalNumberSteatedStudent(total);
    instance->findOverlapConstraints();


}

















