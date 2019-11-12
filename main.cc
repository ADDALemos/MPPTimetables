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

static void SIGINT_exit(int signum) {
    mxsolver->printAnswer(_UNKNOWN_);
    exit(_UNKNOWN_);
}

#include "Test.h"
#include "ParserXMLTwo.h"
#include "WriteXML.h"



// void test_encoding();

//=================================================================================================
// Main:

int main(int argc, char **argv) {
    /*rlimit rl;
    getrlimit(RLIMIT_RSS, &rl);
    std::cout<<rl.rlim_max<<std::endl;
    std::exit(1);*/
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
                                    "(0=wbo,1=linear-su,2=msu3,3=part-msu3,4=oll,5=best,6="
                                    "bmo,7=obv,8=mcs)\n",
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
        //MaxSAT *S = new OLL(_VERBOSITY_MINIMAL_,_CARD_TOTALIZER_,parserXML->getInstance());//new OLLMod(_VERBOSITY_MINIMAL_, _CARD_TOTALIZER_, ClusterAlg::_DIVISIVE_,rounding_statistic, (int) (100000));//new LinearSUMod(_VERBOSITY_MINIMAL_,false,  _CARD_TOTALIZER_, 1,ClusterAlg::_DIVISIVE_, rounding_statistic,(int) (100000));//new BLS(_VERBOSITY_MINIMAL_, _CARD_TOTALIZER_, 100000, 100000, true);
        //new MSU3(_VERBOSITY_MINIMAL_);//new LinearSU(_VERBOSITY_MINIMAL_, 1, _CARD_TOTALIZER_, 1);
        S = new OLL(verbosity, cardinality);
        MaxSATFormula *maxsat_formula = new MaxSATFormula();
        maxsat_formula->setFormat(_FORMAT_PB_);

        ParserXMLTwo *parserXML = new ParserXMLTwo(maxsat_formula, strcmp(argv[1],"true")==0, strcmp(argv[2],"true")==0,strcmp(argv[3],"true")==0);
        parserXML->parse(argv[4]);

        std::cout<<"Read End"<<std::endl;
        printRAM();
        parserXML->aux();
        parserXML->room();
        std::cout<<"room End"<<std::endl;
        printRAM();


        //parserXML->sameTime();

        parserXML->getInstance()->setAlgo((int) algorithm,argv[1],argv[2],argv[3]);
        /*printProblemStats(parserXML->getInstance());
        printStudentsStats(parserXML->getInstance());
        printConstraintsStat(parserXML->getInstance());
        std::exit(0);*/
        //worstCost(parserXML->getInstance());

        //createSmallerInstances(parserXML->getInstance());
        //printConstraintsStat(parserXML->getInstance());
        //printDomainSize(parserXML->getInstance());
        //printCurricular(parserXML->getInstance());
        //printClusterofStudents(parserXML->getInstance());
        parserXML->genConstraint();
        std::cout<<"Fin1"<<std::endl;
        printRAM();
        //printCNF(maxsat_formula,parserXML->getInstance()->getName());
        //std::exit(0);
        //parserXML->sameTime();
        //parserXML->genStudents();
        //parserXML->getInstance()->setDistributionPen(1);
        //parserXML->getInstance()->setTimePen(1);

        S->setInstance(parserXML->getInstance());


        //S->setUB(9770);
        //S->setLB(9770);

        //ew WBO(_VERBOSITY_MINIMAL_, 1, symmetry, symmetry_lim);//

        //new OLL(_VERBOSITY_MINIMAL_,_CARD_TOTALIZER_,parserXML->getInstance());
        maxsat_formula->convertPBtoMaxSAT();


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
        S->loadFormula(maxsat_formula);
        /*if ((int) (cluster_algorithm) == 1) {
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
        }*/
        S->search();
        /*int count=1;
        while(maxsat_formula->getObjFunction()==NULL) {
            S->blockModel(S->getSolver());
            S->search();
            count++;

        }
        std::cout<<count<<std::endl;*/

        /*for (Class * c: parserXML->getInstance()->getClasses()) {
            if(c->getPossiblePairSize()==1 && c->getPossibleRooms().size()==0)
                std::cout<<"here "<<c->getId()<<std::endl;
        }*/
        //createSmallerInstances(parserXML->getInstance());





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
        std::cout << "NEW CLUSTER " << instance->getClusterStudent()[l]->getId() << " "<<instance->getClusterStudent()[l]->getStudent().size()<<std::endl;
        for (Student s: instance->getClusterStudent()[l]->getStudent()) {
            std::cout <<"S "<< s.getId() << " C ";
            for (int j = 0; j < instance->getClusterStudent()[l]->getCourses().size(); ++j) {
                std::cout << instance->getClusterStudent()[l]->getCourses()[j]->getName() << " ";
            }
            std::cout << std::endl;

        }

    }
}

void printCNF(MaxSATFormula *f, std::string s) {
    int w=0;
    for (int i = 0; i < f->nSoft(); i++){
        w+=f->getSoftClause(i).weight;
    }
    w++;
    std::ofstream file_stored(s+".wcnf");

    file_stored << "p wcnf "<<f->nVars()<<" "<<(f->nSoft()+f->nHard())<<" "<<w<<std::endl;
    for (int i = 0; i < f->nHard(); i++){
        file_stored<<w<<" ";
        for (int l=0;l< f->getHardClause(i).clause.size();l++) {
            file_stored << (sign(f->getHardClause(i).clause[l])?"":"-")<<toInt(f->getHardClause(i).clause[l])<<" ";
        }
        file_stored<<"0"<<std::endl;
    }

    for (int i = 0; i < f->nSoft(); i++){
        file_stored<<f->getSoftClause(i).weight<<" ";
        for (int l=0;l< f->getSoftClause(i).clause.size();l++) {
            file_stored << (sign(f->getHardClause(i).clause[l])?"":"-")<<toInt(f->getSoftClause(i).clause[l])<<" ";
        }
        file_stored<<"0"<<std::endl;
    }

    file_stored.close();

}














