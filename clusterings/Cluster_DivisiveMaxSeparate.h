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

#ifndef Cluster_DivisiveMaxSeparate_h
#define Cluster_DivisiveMaxSeparate_h

#ifdef SIMP
#include "simp/SimpSolver.h"
#else
#include "core/Solver.h"
#endif

#include "Cluster.h"
#include <vector>

using NSPACE::vec;
using std::vector;

namespace openwbo {

/*
 * Implements a divisive clustering algorithm. Clustering is performed in a
 * top-down manner. Given a list of sorted weights in ascending order, this
 * algorithm starts with all weights in one cluster and iteratively creates new
 * clusters by splitting at points where the weight difference is highest inside
 * a cluster, across all clusters.
 */
class Cluster_DivisiveMaxSeparate : public Cluster {

public:
  // indices to mark the start of each cluster
  vec<cluster_index> cluster_indices;
  uint64_t max_c;          // number of clusters created so far
  vec<uint64_t> distances; // differences between consecutive weights, when
                           // arranged in ascending order

  Cluster_DivisiveMaxSeparate(MaxSATFormulaExtended *formula,
                              Statistics cluster_stat);

  // performs clustering
  void clusterWeights(MaxSATFormulaExtended *formula, uint64_t c);
};

} // namespace openwbo

#endif
