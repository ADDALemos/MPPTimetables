/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * MiniSat,  Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
 *           Copyright (c) 2007-2010, Niklas Sorensson
 * Open-WBO, Copyright (c) 2013-2017, Ruben Martins, Vasco Manquinho, Ines Lynce
 *
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

#include "utils/HardwareStats.h"
#include "WriteXML.h"
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

void printCurricular(Instance *instance);

void createSmallerInstances(Instance *);

static void SIGINT_exit(int signum) {
    mxsolver->printAnswer(_UNKNOWN_);
    exit(_UNKNOWN_);
}

#include "Test.h"



// void test_encoding();

//=================================================================================================
// Main:

int main(int argc, char **argv) {
    if (!quiet)
        std::cout << "Starting Reading File: "
                  << "/Volumes/MAC/ClionProjects/timetabler/data/input/ITC-2019/tg-fal17.xml" << std::endl;

    if (!quiet)
        printRAM();
    MaxSATFormula *maxsat_formula = new MaxSATFormula();

    ParserXML *parserXML = new ParserXML(maxsat_formula);
    parserXML->parse("/Volumes/MAC/ClionProjects/timetabler/data/input/ITC-2019/pu-cs-fal07.xml");
    parserXML->genConstraint();
    maxsat_formula->setFormat(_FORMAT_PB_);
    MaxSAT *S = new OLL(_VERBOSITY_MINIMAL_,_CARD_TOTALIZER_,parserXML->getInstance());
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
    S->loadFormula(maxsat_formula);
    S->search();
    std::exit(0);
    /*for (Class * c: parserXML->getInstance()->getClasses()) {
        if(c->getPossiblePairSize()==1 && c->getPossibleRooms().size()==0)
            std::cout<<"here "<<c->getId()<<std::endl;
    }*/
    //createSmallerInstances(parserXML->getInstance());



    try {

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
                                    "(0=wbo,1=linear-su,2=msu3,3=part-msu3,4=oll,5=best,6="
                                    "bmo,7=obv,8=mcs)\n",
                            4, IntRange(0, 8));

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

        BoolOption local("Incomplete", "local", "Local limit on the number of conflicts.\n", false);//???

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

        double initial_time = cpuTime();
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
//                    S = new OLL(verbosity, cardinality);
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

        if ((int) formula == _FORMAT_MAXSAT_) {
            maxsat_formula->setFormat(_FORMAT_MAXSAT_);
        } else {
            ParserPB *parser_pb = new ParserPB();
            parser_pb->parsePBFormula(argv[1], maxsat_formula);
            maxsat_formula->setFormat(_FORMAT_PB_);
        }

        if ((int) test_rhs) {
            if ((int) test_rhs2) {
                test_encoding(maxsat_formula, (uint64_t) test_rhs, (uint64_t) test_rhs2,
                              (uint64_t) test_nsoft);
            } else {
                test_encoding(maxsat_formula, (uint64_t) test_rhs);
            }
            return 0;
        }

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
        double parsed_time = cpuTime();

        printf("c |  Parse time:           %12.2f s                                "
                       "                                 |\n",
               parsed_time - initial_time);
        printf("c |                                                                "
                       "                                       |\n");

        if (algorithm == _ALGORITHM_BEST_) {
            assert(S == NULL);

            if (maxsat_formula->getProblemType() == _UNWEIGHTED_) {
                // Unweighted
                S = new PartMSU3(_VERBOSITY_MINIMAL_, _PART_BINARY_, RES_GRAPH,
                                 cardinality);
                S->loadFormula(maxsat_formula);

                if (((PartMSU3 *) S)->chooseAlgorithm() == _ALGORITHM_MSU3_) {
                    // FIXME: possible memory leak
                    S = new MSU3(_VERBOSITY_MINIMAL_);
                }

            } else {
                // Weighted
//        S = new OLL(_VERBOSITY_MINIMAL_, cardinality, nullptr);
            }
        }

        if (S->getMaxSATFormula() == NULL) {
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
        }
        S->setPrintModel(printmodel);
        S->setInitialTime(initial_time);
        mxsolver = S;
        mxsolver->search();

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













