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

#ifndef Alg_LinearSU_Mod_h
#define Alg_LinearSU_Mod_h

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

using NSPACE::Lit;
using NSPACE::vec;

namespace openwbo {

//=================================================================================================
class LinearSUMod : public MaxSAT {

public:
  LinearSUMod(int verb = _VERBOSITY_MINIMAL_, bool bmo = true,
              int enc = _CARD_MTOTALIZER_, int pb = _PB_SWC_,
              ClusterAlg ca = ClusterAlg::_DIVISIVE_,
              Statistics cs = Statistics::_MEAN_, uint64_t num_clusters = 1)
      : solver(NULL), is_bmo(false) {
    pb_encoding = pb;
    verbosity = verb;
    bmoMode = bmo;
    encoding = enc;
    encoder = new Encoder(_INCREMENTAL_NONE_, _CARD_MTOTALIZER_,_AMO_LADDER_, _PB_GTE_);
    encoder->setCardEncoding(encoding);
    encoder->setPBEncoding(pb);
    cluster_algo = ca;
    cluster_stat = cs;
    this->num_clusters = num_clusters;
    best_cost = UINT64_MAX;
    complete = false;
  }

  ~LinearSUMod() {
    if (solver != NULL)
      delete solver;

    objFunction.clear();
    coeffs.clear();
  }

  void initializeCluster();

  void search(); // Linear search.

  // Print solver configuration.
  void printConfiguration(bool bmo, int ptype) {

    printf("c ==========================================[ Solver Settings "
           "]============================================\n");
    printf("c |                                                                "
           "                                       |\n");
    print_LinearSU_configuration();
    if (bmo || ptype == _UNWEIGHTED_)
      print_Card_configuration(encoder->getCardEncoding());
    else
      print_PB_configuration(encoder->getPBEncoding());
    printf("c |                                                                "
           "                                       |\n");
  }

  Cluster *cluster;

protected:
  // Rebuild MaxSAT solver
  //
  // Rebuild MaxSAT solver with BMO algorithm.
  Solver *rebuildBMO(vec<vec<Lit>> &functions, vec<int> &weights,
                     uint64_t currentWeight);
  Solver *rebuildSolver(uint64_t min_weight = 1); // Rebuild MaxSAT solver.

  // Linear search algorithms.
  //
  void normalSearch(); // Classic linear search algorithm.
  void bmoSearch();    // Linear search algorithm with lexicographical order.

  // Greater than comparator.
  bool static greaterThan(int i, int j) { return (i > j); }

  // Other
  void initRelaxation(); // Relaxes soft clauses.

  // Print LinearSU configuration.
  void print_LinearSU_configuration();

  // Compute the cost of a model.
  uint64_t computeCostModel(vec<lbool> &currentModel,
                            uint64_t weight = UINT64_MAX);

  // Compute the cost of the model according to original weights
  uint64_t computeOriginalCost(vec<lbool> &currentModel,
                               uint64_t weight = UINT64_MAX);

  Solver *solver;  // SAT Solver used as a black box.
  Encoder *encoder; // Interface for the encoder of constraints to CNF.
  int encoding;    // Encoding for cardinality constraints.
  int pb_encoding;
  ClusterAlg cluster_algo; // Clustering algorithm
  Statistics cluster_stat; // Statistic used for clustering

  bool complete;

  bool bmoMode;  // Enables BMO mode.
  bool allFalse; // Forces relaxation variables to be false.

  vec<Lit> objFunction; // Literals to be used in the constraint that excludes
                        // models.
  vec<uint64_t> coeffs; // Coefficients of the literals that are used in the
                        // constraint that excludes models.

  bool is_bmo; // Stores if the formula is BMO or not.

  uint64_t num_clusters; // Number of clusters
  vec<lbool> best_model; // Best model as per original weights
  uint64_t best_cost; // Best cost as per original weights
};
} // namespace openwbo

#endif
