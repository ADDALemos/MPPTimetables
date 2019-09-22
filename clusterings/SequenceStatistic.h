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

#ifndef SequenceStatistic_h
#define SequenceStatistic_h

#ifdef SIMP
#include "simp/SimpSolver.h"
#else
#include "core/Solver.h"
#endif

#include "../MaxTypes.h"

using NSPACE::vec;

namespace openwbo {

// Assume that sequence is sorted

/*
 * Manager for finding a particular statistic for a sequence of weights.
 * Currently supported statistics are Mean, Median, Max, and Min.
 */
class SequenceStatistic {

public:
  SequenceStatistic(Statistics statistic = Statistics::_MEAN_);

  // set the statistic to be used
  void setStatistic(Statistics statistic);

  // get the statistic value corresponding to the set statistic for a sequence
  // on a specific range of indices
  uint64_t getSequenceStatistic(vec<uint64_t> &sequence, uint64_t start_index,
                                uint64_t end_index);

  // get the mean statistic
  uint64_t meanStatistic(vec<uint64_t> &sequence, uint64_t start_index,
                         uint64_t end_index);

  // get the median statistic
  uint64_t medianStatistic(vec<uint64_t> &sequence, uint64_t start_index,
                           uint64_t end_index);

  // get the max statistic
  uint64_t maxStatistic(vec<uint64_t> &sequence, uint64_t start_index,
                        uint64_t end_index);

  // get the min statistic
  uint64_t minStatistic(vec<uint64_t> &sequence, uint64_t start_index,
                        uint64_t end_index);

protected:
  // statistic to be used, set by constructor or 'setStatistic()'
  Statistics statistic;
};

} // namespace openwbo

#endif
