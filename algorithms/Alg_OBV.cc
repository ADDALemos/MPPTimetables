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

#include "Alg_OBV.h"
#include "../Torc.h"
#include <cstdlib>
#include <ctime>     
#include <vector>
#include <algorithm> 

using namespace openwbo;

uint64_t OBV::MrsBeaver(Solver * solver, int iterations, int conflicts, uint64_t ub){

  std::srand ( 1971603567 );
  std::vector<Lit> outputs;
  
  for (int i = 0; i < objFunction.size(); i++)
    outputs.push_back(objFunction[i]);

  vec<Lit> dummy;
  lbool res = searchSATSolver(solver, dummy);
  if (res != l_True)
    return 0;

  uint64_t current_ub = computeCostModel(solver->model);
  saveModel(solver->model);
  printf("o %" PRId64 "\n", current_ub);
  
  vec<lbool> original_model;
  solver->model.copyTo(original_model);
  solver->setConfBudget(conflicts);

  for (int t = 0; t < iterations; t++){
    if (!_budget) {
      printf("c Warn: SAT Solver exit due to conflict budget.\n");
      break;
    }
    printf ("c Iteration #%d\n",t+1);
    model_all.clear();
    original_model.copyTo(model_all);

    if (t % 4 == 0 || t % 4 == 1){
      current_ub = ums_obv_bs(solver, outputs, current_ub, conflicts);
      std::reverse(outputs.begin(),outputs.end());
    } else {
      current_ub = obv_bs(solver, outputs, current_ub, conflicts);
      std::reverse(outputs.begin(),outputs.end());
    }

    if (t % 4 == 0 && t > 0){
      std::random_shuffle ( outputs.begin(), outputs.end() );
    }

  }
  solver->budgetOff();

  return current_ub;

}

uint64_t OBV::obv_bs(Solver * solver, std::vector<Lit>& outputs, uint64_t ub, int conflicts){

  vec<Lit> assumptions;
  vec<lbool> current_model;
  assert (model_all.size() != 0);
  model_all.copyTo(current_model);
  uint64_t last_ub = ub;

  for (int i =0; i < outputs.size(); i++){

    if (current_model[var(outputs[i])] == l_False){
      assumptions.push(~outputs[i]);
    } else {
      vec<Lit> current_assumptions;
      assumptions.copyTo(current_assumptions);
      current_assumptions.push(~outputs[i]);

      for (int i = 0; i < outputs.size(); i++){
        solver->setPolarity(var(outputs[i]),true);
      }
      
      if (_local)
        solver->setConfBudget(conflicts);

      if (_local && solver->conflicts > 100*conflicts){
	_budget = false;
        return last_ub;
      }
      
      lbool res = searchSATSolver(solver, current_assumptions);
      if (!_local && res != l_True && res != l_False){
        _budget = false;
        return last_ub;
      }

      if (res == l_True){
        assumptions.push(~outputs[i]);
        uint64_t newCost = computeCostModel(solver->model);
        if (newCost < last_ub){
          saveModel(solver->model);
          printf("o %" PRId64 "\n", newCost);
          last_ub = newCost;
        }
        current_model.clear();
        solver->model.copyTo(current_model);
      } else {
        assumptions.push(outputs[i]);
      }
    }
  }
  //solver->budgetOff();
  return last_ub;
}

uint64_t OBV::ums_obv_bs(Solver * solver, std::vector<Lit>& outputs, uint64_t ub, int conflicts){
  vec<Lit> assumptions;
  vec<lbool> current_model;
  assert (model.size() != 0);
  model.copyTo(current_model);
  uint64_t last_ub = ub;
  
  std::vector<Lit> outputs_mod;
  for (int i = 0; i < outputs.size(); i++){
    outputs_mod.push_back(outputs[i]);
  }

  for (int i =0; i < outputs_mod.size(); i++){
    if (current_model[var(outputs_mod[i])] == l_False){
      assumptions.push(~outputs_mod[i]);
    } else {
      vec<Lit> current_assumptions;
      assumptions.copyTo(current_assumptions);
      current_assumptions.push(~outputs_mod[i]);

      for (int i = 0; i < outputs.size(); i++){
        solver->setPolarity(var(outputs[i]),true);
      }

      if (_local)
        solver->setConfBudget(conflicts);

      if (_local && solver->conflicts > 100*conflicts){
	_budget = false;
        return last_ub;
      }
      
      lbool res = searchSATSolver(solver, current_assumptions);
	
      if (!_local && res != l_True && res != l_False){
        _budget = false;
        return last_ub;
      }

      // move bits
      if (res == l_True){
        int k = i+1;
        for (int j = i+1; j < outputs_mod.size(); j++){
          if (solver->model[var(outputs_mod[j])] == l_False){
            if (k!=j){
              Lit a = outputs_mod[k];
              outputs_mod[k] = outputs_mod[j];
              outputs_mod[j] = a;
            }
            k++;
          }
        }
      }
      if (res == l_True){
        assumptions.push(~outputs_mod[i]);
        uint64_t newCost = computeCostModel(solver->model);
        if (newCost < last_ub){
          saveModel(solver->model);
          printf("o %" PRId64 "\n", newCost);
          last_ub = newCost;
        }
        current_model.clear();
        solver->model.copyTo(current_model);
      } else {
        assumptions.push(outputs_mod[i]);
      }
    }
  }
  //solver->budgetOff();
  return last_ub;
}

/*_________________________________________________________________________________________________
  |
  |  normalSearch : [void] ->  [void]
  |
  |  Description:
  |
  |    Linear search algorithm.
  |
  |  For further details see:
  |    *  Daniel Le Berre, Anne Parrain: The Sat4j library, release 2.2. JSAT
  |       7(2-3): 59-6 (2010)
  |    *  Miyuki Koshimura, Tong Zhang, Hiroshi Fujita, Ryuzo Hasegawa: QMaxSAT:
  |       A Partial Max-SAT Solver. JSAT 8(1/2): 95-100 (2012)
  |
  |  Post-conditions:
  |    * 'ubCost' is updated.
  |    * 'nbSatisfiable' is updated.
  |    * 'nbCores' is updated.
  |
  |________________________________________________________________________________________________@*/
void OBV::normalSearch() {

  lbool res = l_True;

  initRelaxation();
  solver = rebuildSolver();
  
  if (Torc::Instance()->GetPolOptimistic())
	{
		 if (Torc::Instance()->TargetIsVarTarget().size() == 0) {
			  Torc::Instance()->TargetIsVarTarget().growTo(solver->nVars(), false);
			  
			  for (int i = 0; i < objFunction.size(); i++) {
				  auto v = var(objFunction[i]);
				  assert(sign(objFunction[i]) == 0);
				  Torc::Instance()->TargetIsVarTarget()[v] = true;				  				  
			  }			  
		  }		
	}

	if (Torc::Instance()->GetTargetVarsBumpVal() != 0)
	{
		BumpTargets(objFunction, coeffs, solver);
	}
  
  bool mrsb = true;
  uint64_t newCost = UINT64_MAX;

  while (res == l_True) {

    vec<Lit> dummy;
    // invoke Mrs. Beaver
    if (mrsb) {
          printf("c Mrs. Beaver incomplete stage\n");
          newCost = MrsBeaver(solver, _iterations, _limit, 0);
          printf("c Mrs. Beaver ub %" PRId64 "\n", newCost);
          printf("c Warn: changing to LSU algorithm.\n"); 
    } else {
      res = searchSATSolver(solver, dummy);
    }

    if (model.size() == 0){
      printAnswer(_UNSATISFIABLE_);
      exit(_UNSATISFIABLE_);
    }

    if (res == l_True) {
      nbSatisfiable++;
      if  (!mrsb){
        newCost = computeCostModel(solver->model);
        saveModel(solver->model);
      } else {
        mrsb = false;
      }
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

// Public search method
bool OBV::search() {

  assert (maxsat_formula->getProblemType() == _UNWEIGHTED_);

  printConfiguration();
  normalSearch();
    return true;

}

/************************************************************************************************
 //
 // Rebuild MaxSAT solver
 //
 ************************************************************************************************/

/*_________________________________________________________________________________________________
  |
  |  rebuildSolver : (minWeight : int)  ->  [Solver *]
  |
  |  Description:
  |
  |    Rebuilds a SAT solver with the current MaxSAT formula.
  |    If a weight is specified, then it only considers soft clauses with weight
  |    smaller than the specified weight.
  |    NOTE: a weight is specified in the 'bmo' approach.
  |
  |________________________________________________________________________________________________@*/
Solver *OBV::rebuildSolver(uint64_t min_weight) {

  vec<bool> seen;
  seen.growTo(maxsat_formula->nVars(), false);

  Solver *S = newSATSolver();

  for (int i = 0; i < maxsat_formula->nVars(); i++)
    newSATVariable(S);

  for (int i = 0; i < maxsat_formula->nHard(); i++)
    S->addClause(maxsat_formula->getHardClause(i).clause);

  for (int i = 0; i < maxsat_formula->nPB(); i++) {
    Encoder *enc = new Encoder(_INCREMENTAL_NONE_, _CARD_MTOTALIZER_,
                               _AMO_LADDER_, _PB_GTE_);

    // Make sure the PB is on the form <=
    // if (maxsat_formula->getPBConstraint(i)->_sign)
    //  maxsat_formula->getPBConstraint(i)->changeSign();
    assert(maxsat_formula->getPBConstraint(i)->_sign);

    enc->encodePB(S, maxsat_formula->getPBConstraint(i)->_lits,
                  maxsat_formula->getPBConstraint(i)->_coeffs,
                  maxsat_formula->getPBConstraint(i)->_rhs);

    delete enc;
  }

  for (int i = 0; i < maxsat_formula->nCard(); i++) {
    Encoder *enc = new Encoder(_INCREMENTAL_NONE_, _CARD_MTOTALIZER_,
                               _AMO_LADDER_, _PB_GTE_);

    if (maxsat_formula->getCardinalityConstraint(i)->_rhs == 1) {
      enc->encodeAMO(S, maxsat_formula->getCardinalityConstraint(i)->_lits);
    } else {

      enc->encodeCardinality(S,
                             maxsat_formula->getCardinalityConstraint(i)->_lits,
                             maxsat_formula->getCardinalityConstraint(i)->_rhs);
    }

    delete enc;
  }

  vec<Lit> clause;
  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    if (maxsat_formula->getSoftClause(i).weight < min_weight)
      continue;

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

/*_________________________________________________________________________________________________
  |
  |  rebuildBMO : (functions : int)  ->  [Solver *]
  |
  |  Description:
  |
  |    Rebuilds a SAT solver with the current MaxSAT formula.
  |    Only considers soft clauses with the weight of the current
  |    lexicographical optimization weight ('currentWeight')
  |    For each function already computed in the BMO algorithm it encodes the
  |    respective cardinality constraint.
  |
  |________________________________________________________________________________________________@*/
Solver *OBV::rebuildBMO(vec<vec<Lit>> &functions, vec<int> &rhs,
                             uint64_t currentWeight) {

  assert(functions.size() == rhs.size());

  Solver *S = rebuildSolver(currentWeight);

  objFunction.clear();
  coeffs.clear();
  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    if (maxsat_formula->getSoftClause(i).weight == currentWeight) {
      objFunction.push(maxsat_formula->getSoftClause(i).relaxation_vars[0]);
      coeffs.push(maxsat_formula->getSoftClause(i).weight);
    }
  }

  for (int i = 0; i < functions.size(); i++)
    encoder.encodeCardinality(S, functions[i], rhs[i]);

  return S;
}

/************************************************************************************************
 //
 // Other protected methods
 //
 ************************************************************************************************/

/*_________________________________________________________________________________________________
  |
  |  initRelaxation : (objective : vec<Lit>&) (weights : vec<int>&)  ->  [void]
  |
  |  Description:
  |
  |    Initializes the relaxation variables by adding a fresh variable to the
  |    'relaxationVars' of each soft clause.
  |
  |  Post-conditions:
  |    * 'objFunction' contains all relaxation variables that were added to soft
  |       clauses.
  |    * 'coeffs' contains the weights of all soft clauses.
  |
  |________________________________________________________________________________________________@*/
void OBV::initRelaxation() {
  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    Lit l = maxsat_formula->newLiteral();
    maxsat_formula->getSoftClause(i).relaxation_vars.push(l);
    objFunction.push(l);
    coeffs.push(maxsat_formula->getSoftClause(i).weight);
  }
}
