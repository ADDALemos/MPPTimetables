/*!
 * \author Sukrut Rao - cs15btech11036@iith.ac.in
 *
 * @section LICENSE
 *
 * Open-WBO, Copyright (c) 2018  Saurabh Joshi, Prateek Kumar, Sukrut Rao
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

#ifndef Cluster_h
#define Cluster_h

#ifdef SIMP
#include "simp/SimpSolver.h"
#else
#include "core/Solver.h"
#endif

#include "../MaxSATFormulaExtended.h"
#include "SequenceStatistic.h"
#include <vector>

using NSPACE::vec;
using std::vector;

namespace openwbo {

/*
 * Store indices to demarcate clusters in a formula.
 * An index 'x' indicates the xth clause in order of increasing weights marks
 * the start of a new cluster.
 * '0' is always a clustering index, marking the starting point of the first
 * cluster.
 */
struct cluster_index {
  uint64_t index;
  bool all_equal_in_cluster;
};

/*
 * Base class for all clustering algorithms.
 */
class Cluster {

public:
  vec<uint64_t> original_weights; // The original weights of the formula

  Statistics cluster_statistic; // The statistic used to store the clustered
                                // weights. Default is Mean

  // Handle finding the appropriate statistic
  SequenceStatistic statistic_finder;

  Cluster(MaxSATFormulaExtended *formula, Statistics cluster_statistic);

  // Cluster the weights in the formula, given the number of clusters needed
  // Implemented in each clustering algorithm
  virtual void clusterWeights(MaxSATFormulaExtended *formula, uint64_t c) = 0;

  // Save original weights in the formula
  void saveWeights(MaxSATFormulaExtended *formula);

  // Restore the original weights in the formula
  void restoreWeights(MaxSATFormulaExtended *formula);

  // Find weights for each cluster in the formula and replace weights, given the
  // cluster indices
  void replaceWeights(MaxSATFormulaExtended *formula, vec<uint64_t> &clusters);
  void replaceWeights(MaxSATFormulaExtended *formula,
                      vec<cluster_index> &clusters);

  // Get the original weight for a particular clause
  uint64_t getOriginalWeight(int index);

}; // class Cluster

} // namespace openwbo

#endif
