/*!
 * \author Vasco Manquinho - vmm@sat.inesc-id.pt
 *
 * @section LICENSE
 *
 * Open-WBO, Copyright (c) 2013-2018, Ruben Martins, Vasco Manquinho, Ines Lynce
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

#ifndef BLS_h
#define BLS_h

#ifdef SIMP
#include "simp/SimpSolver.h"
#else
#include "core/Solver.h"
#endif

#include "../MaxSAT.h"
#include "../Encoder.h"
#include "utils/System.h"
#include <utility>
#include <map>
#include <set>

namespace openwbo {
  
  class BLS : public MaxSAT {
    
  public:
    BLS(int verb = _VERBOSITY_MINIMAL_, int card = _CARD_MTOTALIZER_, int limit = 100000, int mcs = 50, bool local = false){
      solver = NULL;
      verbosity = verb;
      local_limit = local;
      
      nbCores = 0;
      nbSatisfiable = 0;
      nbSatCalls = 0;
      sumSizeCores = 0;
      conflict_limit = limit;
      
      nbMCS = 0;
      _smallestMCS = UINT64_MAX;
      _maxMCS = mcs;
      encoding = card;
      encoder.setCardEncoding(card);
    }
    
    ~BLS(){
      if (solver != NULL)
        delete solver;
    }
    
    void search();                                      // BLS search.
    void printAnswer(int type);                         // Print the answer.
    
        
    uint64_t getCost(){ // Unweighted. TODO: Save weighted solution!
      return _smallestMCS;
    }

    Solver* getSolver(){
      return solver;
    }
    
  protected:
    
    // Rebuild MaxSAT solver
    //
    Solver * buildSolver();

    void LSU();
    
    lbool solve();                                      // SAT solver call
    
    // Utils for model management
    //
    void saveModel(vec<lbool> &currentModel);             // Saves a Model.
    void saveSmallestModel(vec<lbool> &currentModel);     // Saves the smallest model found until now.
    
    // init methods
    void init();
    void initUndefClauses(vec<int>& undefClauses);
    
    // BLS search
    //
    void basicSearch(int maxMCS);
    
    bool findNextMCS();
    void addMCSClause(vec<int>& unsatClauses);
    
    // Utils for printing
    //
    void printModel();                                  // Print the last model.
    
    // Other utils
    bool satisfiedSoft(int i);

    void printConfiguration();
    
  protected:
    //Data Structures
    
    // SAT solver and MaxSAT database
    //
    Solver* solver;                                     // SAT solver used as a black box.
    int verbosity;                                      // Controls the verbosity of the solver.
    
    // Options
    int _maxMCS;
    int conflict_limit;
    bool local_limit;
    int encoding;
    
    // Core extraction 
    //
    std::map<Lit,int> coreMapping;                      // Maps the assumption literal to the number of the soft clause.
    vec<Lit> assumptions;                               // Stores the assumptions to be used in the extraction of the core.
    vec<int> _prevAssumptions;
    
    vec<Lit> objFunction; // Literals to be used in the constraint that excludes
                        // models.

    Encoder encoder; // Interface for the encoder of constraints to CNF.
    
    // Symmetry breaking
    //
    vec<int> indexSoftCore;                             // Indexes of soft clauses that appear in the current core.
    vec< vec<int> > softMapping;                        // Maps the soft clause with the cores where they appears.
    
    // Statistics
    //
    int nbCores;                                        // Number of cores.
    uint64_t sumSizeCores;                              // Sum of the sizes of cores.
    int nbSatisfiable;                                  // Number of satisfiable calls.
    int nbSatCalls;                                     // Number of SAT solver calls.
    int nbMCS;
    
    //MCS Management
    uint64_t _maxWeight;
    uint64_t _smallestMCS;
    uint64_t _lbWeight;
    vec<lbool> _smallestModel;
    int _nMCS;
    
  };
}

#endif
 
