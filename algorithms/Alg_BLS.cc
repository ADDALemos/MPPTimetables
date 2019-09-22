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

#include "Alg_BLS.h"

using namespace openwbo;


/************************************************************************************************
 //
 // Utils for model management
 //
 ************************************************************************************************/ 

/*_________________________________________________________________________________________________
  |
  |  saveModel : (currentModel : vec<lbool>&)  ->  [void]
  |  
  |  Description:
  |		 
  |    Saves the current model found by the SAT solver.
  |
  |  Pre-conditions:
  |    * Assumes that 'nbInitialVariables' has been initialized.
  |    * Assumes that 'currentModel' is not empty.
  |
  |  Post-conditions:
  |    * 'model' is updated to the current model.
  |    * 'nbSatisfiable' is increased by 1.
  |
  |________________________________________________________________________________________________@*/
void BLS::saveModel(vec<lbool> &currentModel){
  //assert (n_initial_vars != 0);
  assert (currentModel.size() != 0);
  
  model.clear();
  // Only store the value of the variables that belong to the original MaxSAT formula.
  for (int i = 0; i < maxsat_formula->nVars(); i++){
    model.push(currentModel[i]);
  }
  
  nbSatisfiable++;
}


void BLS::saveSmallestModel(vec<lbool> &currentModel){
  //assert (n_initial_vars != 0);
  assert (currentModel.size() != 0);
  
  _smallestModel.clear();
  // Only store the value of the variables that belong to the original MaxSAT formula.
  for (int i = 0; i < maxsat_formula->nVars(); i++){
    _smallestModel.push(currentModel[i]);
  }
}


/************************************************************************************************
 //
 // BLS search
 //
 ************************************************************************************************/ 

// Checks if a soft clause is satisfied by saved model
bool BLS::satisfiedSoft(int i) {
  for (int j = 0; j < maxsat_formula->getSoftClause(i).clause.size(); j++){
    assert (var(maxsat_formula->getSoftClause(i).clause[j]) < model.size());
    if ((sign(maxsat_formula->getSoftClause(i).clause[j]) && model[var(maxsat_formula->getSoftClause(i).clause[j])] == l_False) || 
	(!sign(maxsat_formula->getSoftClause(i).clause[j]) && model[var(maxsat_formula->getSoftClause(i).clause[j])] == l_True)) {
      return true;
    }
  }
  return false;
}

// Call to the SAT solver...
lbool BLS::solve() {
  nbSatCalls++;  
  
  if (local_limit)
    solver->setConfBudget(conflict_limit);

#ifdef SIMP
  return ((SimpSolver*)solver)->solveLimited(assumptions);
#else
  return solver->solveLimited(assumptions);
#endif
}


void BLS::addMCSClause(vec<int>& unsatClauses) {
  vec<Lit> lits;
  for (int i = 0; i < unsatClauses.size(); i++) {
    lits.push(~maxsat_formula->getSoftClause(unsatClauses[i]).assumption_var);
  }
  
  maxsat_formula->addHardClause(lits);
  
  solver->addClause(maxsat_formula->getHardClause(maxsat_formula->nHard()-1).clause);
  
}


void BLS::initUndefClauses(vec<int>& undefClauses) {
  for (int i = 0; i < maxsat_formula->nSoft(); i++) 
    undefClauses.push(i);
}

void BLS::LSU() {

  printf("c Warn: changing to LSU algorithm.\n");

  lbool res = l_True;
  uint64_t newCost = UINT64_MAX;
  if (model.size() != 0){
    newCost = _smallestMCS;
    if (!encoder.hasCardEncoding())
      encoder.encodeCardinality(solver, objFunction, newCost - 1);
    else
      encoder.updateCardinality(solver, newCost - 1);
  }

  while (res == l_True) {

    assumptions.clear();
    res = solve();

    if (res == l_True) {
      nbSatisfiable++;
      newCost = computeCostModel(solver->model);
      saveModel(solver->model);
      printf("o %" PRId64 "\n", newCost + off_set); 

      if (newCost == 0) {
        // If there is a model with value 0 then it is an optimal model
        ubCost = newCost;

        printAnswer(_OPTIMUM_);
        exit(_OPTIMUM_);

      } else {
          // Unweighted.
          if (!encoder.hasCardEncoding())
            encoder.encodeCardinality(solver, objFunction, newCost - 1);
          else
            encoder.updateCardinality(solver, newCost - 1);

        ubCost = newCost;
      }
    } else {
      nbCores++;
      if (model.size() == 0) {
        assert(nbSatisfiable == 0);
        // If no model was found then the MaxSAT formula is unsatisfiable
        printAnswer(_UNSATISFIABLE_);
        exit(_UNSATISFIABLE_);
      } else {
        printAnswer(_OPTIMUM_);
        exit(_OPTIMUM_);
      }
    }
  }
}

// Always consider all clauses by adding assumption var to each soft clause.

void BLS::basicSearch(int maxMCS = 30) {
  printConfiguration();

  // Init Structures
  init();
  
  //Build solver
  solver = buildSolver();
  solver->setConfBudget(conflict_limit);
  
  while (maxMCS) {
    if (!findNextMCS()) break;
    
    maxMCS--;
  }
  if (maxMCS == 0) 
    printf("c All requested MCSs found\n");
}

// Find next MCS.
// Returns false if the SAT solver was not able to finish. Otherwise, returns true.

bool BLS::findNextMCS() {
  vec<int> undefClauses;
  vec<int> satClauses;
  vec<int> unsatClauses;
  uint64_t costModel = _maxWeight;
  lbool res = l_True;
  //int conflict_limit = 100000;
  
  initUndefClauses(undefClauses);
  assumptions.clear();
  
  // make first call.
  res = solve();
  
  // Check outcome of first call
  if (res == l_False) {
    // Hard clause set in unsat!
    if (nbMCS == 0) {
      printAnswer(_UNSATISFIABLE_);
      exit(_UNSATISFIABLE_);
    }
    else {
      // It is not the first MCS. Hence, all MCS were found.
      printAnswer(_OPTIMUM_);
      exit(_OPTIMUM_);
    }
  }
  else if (res == l_True) {
  	uint64_t newCost = computeCostModel(solver->model);
  	if (newCost < _smallestMCS){
  		saveModel(solver->model);	
  		printf("o %" PRId64 "\n", newCost);
  		_smallestMCS = newCost;
  	}
  }
  else {
    printf("c Warn: SAT Solver exit due to conflict budget.\n");
    if (costModel < _smallestMCS) {
      _smallestMCS = costModel;
      //Last saved model is smallest MCS...
    }
    return false;
  }
  
  // Iterate to find next MCS
  while (undefClauses.size()) {
    int i = 0;
    if (res == l_True) {
      //Remove satisfied soft clauses from undefClauses
      
      while (i < undefClauses.size()) {
      	if (satisfiedSoft(undefClauses[i])) {
      	  //Add soft clause as Hard!
      	  satClauses.push(undefClauses[i]);
	        assumptions.push(~(maxsat_formula->getSoftClause(undefClauses[i]).assumption_var));
      	  costModel -= maxsat_formula->getSoftClause(undefClauses[i]).weight;
      	  undefClauses[i] = undefClauses.last();
      	  undefClauses.pop();
      	}
      	else i++;
      }
      
      if (costModel < _smallestMCS) {
	       //saveSmallestModel(solver->model);
      	   saveModel(solver->model);
	       printf("o %" PRId64 "\n", costModel);
	       _smallestMCS = costModel;
      }
    }
    
    if (undefClauses.size() == 0) {
      break;
    }
    
    i = undefClauses.last();
    undefClauses.pop();
    satClauses.push(i);
    assumptions.push(~(maxsat_formula->getSoftClause(i).assumption_var));
    
    res = solve();
    
    if (res == l_False) {
      unsatClauses.push(satClauses.last());
      satClauses.pop();
      assumptions.pop();
    }
    else if (res == l_True) {
      costModel -= maxsat_formula->getSoftClause(satClauses.last()).weight;
      if (undefClauses.size() == 0 && costModel < _smallestMCS){
        printf("o %" PRId64 "\n", costModel);
        saveModel(solver->model);
        _smallestMCS = costModel;
      }
    }
    else {
      printf("c Warn: SAT Solver exit due to conflict budget.\n");
      if (costModel < _smallestMCS) {
        _smallestMCS = costModel;
        //Last saved model is smallest MCS...
      }
      return false;
    }
  }
  
  if (costModel < _smallestMCS) {
    _smallestMCS = costModel;
    //Last saved model is smallest MCS...
  }
  nbMCS++;
  printf("c MCS #%d Weight: %" PRId64 "\n", nbMCS, costModel);
  
  addMCSClause(unsatClauses);
  return true;
}


// Public search method
void BLS::search() {

  basicSearch(_maxMCS);

  // Make sure the conflict budget is turned off.
  solver->budgetOff();  
  local_limit = false;

  LSU();
}


/************************************************************************************************
 //
 // Utils for printing
 //
 ************************************************************************************************/ 

// Prints the best satisfying model. Assumes that 'model' is not empty.
void BLS::printModel(){
  
  assert (model.size() != 0);
  
  printf("v ");
  for (int i = 0; i < model.size(); i++){
    if (model[i] == l_True) printf("%d ",i+1);
    else printf("%d ",-(i+1));
  }
  printf("\n");
}

// Prints the corresponding answer.
void BLS::printAnswer(int type){

  if (type == _UNKNOWN_ && model.size() > 0)
    type = _SATISFIABLE_;
  
  switch(type){
  case _SATISFIABLE_:
    printf("s SATISFIABLE\n");
    printModel();
    break;
  case _OPTIMUM_:
    printf("s OPTIMUM FOUND\n");
    printModel();
    break;
  case _UNSATISFIABLE_:
    printf("s UNSATISFIABLE\n");
    break;  
  case _UNKNOWN_:
    printf("s UNKNOWN\n");
    break;
  default:
    printf("c Error: Unknown answer type.\n");
  }
}

/************************************************************************************************
 //
 // Other protected methods
 //
 ************************************************************************************************/ 

Solver *BLS::buildSolver() {

  vec<bool> seen;
  seen.growTo(maxsat_formula->nVars(), false);

  Solver *S = newSATSolver();
  
  for (int i = 0; i < maxsat_formula->nVars(); i++)
    newSATVariable(S);
  
  for (int i = 0; i < maxsat_formula->nHard(); i++)
    S->addClause(maxsat_formula->getHardClause(i).clause);

  vec<Lit> clause;
  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    
    clause.clear();
    maxsat_formula->getSoftClause(i).clause.copyTo(clause);
    
    for (int j = 0; j < maxsat_formula->getSoftClause(i).relaxation_vars.size();
	 j++) {
      clause.push(maxsat_formula->getSoftClause(i).relaxation_vars[j]);
    }
    
    S->addClause(clause);
  }
  
  return S;
}

// Prints search statistics.
void BLS::printConfiguration(){

    printf("c ==========================================[ Solver Settings "
           "]============================================\n");
    printf("c |                                                                "
           "                                       |\n");
    printf("c |  Algorithm: %23s                                             "
           "                      |\n",
           "MCS");
    print_Card_configuration(encoding);
    printf("c |  Limit number conflicts: %10d                                 "
           "                                  |\n", conflict_limit);
    printf("c |  Limit number iterations: %9d                                 "
      "                                  |\n", _maxMCS);
    if (local_limit)
      printf("c |  Global limit number conflicts:   F                          "
        "                                         |\n");
    else
      printf("c |  Global limit number conflicts:   T                          "
        "                                         |\n");
    printf("c |                                                                "
           "                                       |\n");
}

void BLS::init() {
  _maxWeight = 0;
  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    Lit l = maxsat_formula->newLiteral();
    maxsat_formula->getSoftClause(i).relaxation_vars.push(l);
    maxsat_formula->getSoftClause(i).assumption_var = maxsat_formula->getSoftClause(i).relaxation_vars[0]; // Assumption Var is relaxation var
    objFunction.push(l);
    _maxWeight += maxsat_formula->getSoftClause(i).weight;
  }
  printf("c Max. Weight: %" PRId64 "\n", _maxWeight);
}
