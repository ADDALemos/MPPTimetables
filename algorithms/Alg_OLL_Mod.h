/*!
 * \author Ruben Martins - ruben@sat.inesc-id.pt
 *
 * @section LICENSE
 *
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

#ifndef Alg_OLL_Mod_h
#define Alg_OLL_Mod_h

#ifdef SIMP
#include "simp/SimpSolver.h"
#else
#include "core/Solver.h"
#endif

#include "../Encoder.h"
#include "../MaxSAT.h"
#include "../MaxTypes.h"
#include "../clusterings/Cluster.h"
#include "../clusterings/Cluster_DivisiveMaxSeparate.h"
#include <map>
#include <set>
#include <vector>

namespace openwbo {

//=================================================================================================
class OLLMod : public MaxSAT {

public:
  OLLMod(int verb = _VERBOSITY_MINIMAL_, int enc = _CARD_TOTALIZER_,
         ClusterAlg ca = ClusterAlg::_DIVISIVE_,
         Statistics cs = Statistics::_MEAN_, uint64_t num_clusters = 1) {
    solver = NULL;
    verbosity = verb;
    incremental_strategy = _INCREMENTAL_ITERATIVE_;
    encoding = enc;
    encoder.setCardEncoding(enc);
    min_weight = 1;
    cluster_algo = ca;
    cluster_stat = cs;
    this->num_clusters = num_clusters;
    best_cost = UINT64_MAX;
  }
  ~OLLMod() {
    if (solver != NULL)
      delete solver;
  }

  void search();

  void initializeCluster();

  // Print solver configuration.
  void printConfiguration() {

    printf("c ==========================================[ Solver Settings "
           "]============================================\n");
    printf("c |                                                                "
           "                                       |\n");
    printf("c |  Algorithm: %23s                                             "
           "                      |\n",
           "OLL");
    print_Card_configuration(encoding);
    printf("c |                                                                "
           "                                       |\n");
  }

  Cluster *cluster;

protected:
  // Rebuild MaxSAT solver
  //
  Solver *rebuildSolver(); // Rebuild MaxSAT solver.

  MaxSATFormula *original_formula;

  // Other
  void initRelaxation(); // Relaxes soft clauses.

  void unweighted();
  void weighted();

  Solver *solver;  // SAT Solver used as a black box.
  Encoder encoder; // Interface for the encoder of constraints to CNF.

  uint64_t computeOriginalCost(vec<lbool> &currentModel,
                               uint64_t weight = UINT64_MAX);

  // Controls the incremental strategy used by MSU3 algorithms.
  int incremental_strategy;
  // Controls the cardinality encoding used by MSU3 algorithms.
  int encoding;
  ClusterAlg cluster_algo; // Clustering algorithm
  Statistics cluster_stat; // Statistic used for clustering

  // Literals to be used in the constraint that excludes models.
  vec<Lit> objFunction;
  vec<uint64_t> coeffs; // Coefficients of the literals that are used in the
                        // constraint that excludes models.

  std::map<Lit, int> coreMapping; // Mapping between the assumption literal and
                                  // the respective soft clause.

  std::map<Lit, int>
      coreCardinality; // Mapping between the assumption literal and
                       // the respective soft clause.

  // lit -> <ID, bound, weight>
  std::map<Lit, std::pair<std::pair<int, uint64_t>, uint64_t>> boundMapping;

  // Soft clauses that are currently in the MaxSAT formula.
  vec<bool> activeSoft;

  uint64_t findNextWeightDiversity(uint64_t weight,
                                   std::set<Lit> &cardinality_assumptions);
  uint64_t findNextWeight(uint64_t weight,
                          std::set<Lit> &cardinality_assumptions);

  uint64_t min_weight;
  uint64_t num_clusters; // Number of clusters
  vec<lbool> best_model; // Best model found as per original weights
  uint64_t best_cost; // Best cost as per original weights
};
} // namespace openwbo

#endif
