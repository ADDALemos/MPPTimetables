/*!
 * \author Prateek Kumar - cs15btech11031@iith.ac.in
 *
 * @section LICENSE
 *
 * Open-WBO Copyright (c) 2018  Saurabh Joshi, Prateek Kumar, Sukrut Rao
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

#include <algorithm>
#include <iostream>
#include <vector>

#include "MaxSATFormulaExtended.h"

using namespace openwbo;

// Compares two soft clauses according to their weights
bool compare(const Soft &a, const Soft &b) { return a.weight < b.weight; }

// Sorts soft clauses accordiing to weights of the soft clauses
void MaxSATFormulaExtended::sortSoftClauses() {
  uint64_t size = soft_clauses.size();
  std::sort(soft_clauses + 0, soft_clauses + size,
            [](const Soft &a, const Soft &b) { return a.weight < b.weight; });
}

// Returns soft clauses
vec<Soft> &MaxSATFormulaExtended::getSoftClauses() { return soft_clauses; }
