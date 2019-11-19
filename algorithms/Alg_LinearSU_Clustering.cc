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

#include "Alg_LinearSU_Clustering.h"
#include "../MaxTypes.h"
#include "../Torc.h"

#include <algorithm>
#include <fstream>
#include <iostream>

#define MAX_CLAUSES 3000000

using namespace openwbo;


/*_________________________________________________________________________________________________
  |
  |  initializeCluster : [void] -> [void]
  |
  |  Description:
  |
  |    Initializes cluster according to the algorithm specified.
  |
  |  Pre-conditions:
  |    * cluster_algo contains the clustering method
  |
  |  Post-conditions:
  |    * cluster is initialized
  |
  |________________________________________________________________________________________________@*/
void LinearSUClustering::initializeCluster() {	
  switch (cluster_algo) {
  case ClusterAlg::_DIVISIVE_:
    cluster = new Cluster_DivisiveMaxSeparate(
        static_cast<MaxSATFormulaExtended *>(maxsat_formula), cluster_stat);
    break;
  }
}

/*_________________________________________________________________________________________________
  |
  |  computeCostModel : (currentModel : vec<lbool>&) (weight : int) ->
  |                     [uint64_t]
  |
  |  Description:
  |
  |    Computes the cost of 'currentModel'. The cost of a model is the sum of
  |    the weights of the unsatisfied soft clauses.
  |    If a weight is specified, then it only considers the sum of the weights
  |    of the unsatisfied soft clauses with the specified weight.
  |
  |  Pre-conditions:
  |    * Assumes that 'currentModel' is not empty.
  |
  |________________________________________________________________________________________________@*/
uint64_t LinearSUClustering::computeCostModel(vec<lbool> &currentModel,
                                              uint64_t weight) {

  assert(currentModel.size() != 0);
  uint64_t currentCost = 0;

  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    bool unsatisfied = true;
    for (int j = 0; j < maxsat_formula->getSoftClause(i).clause.size(); j++) {

      if (weight != UINT64_MAX &&
          maxsat_formula->getSoftClause(i).weight != weight) {
        unsatisfied = false;
        continue;
      }

      assert(var(maxsat_formula->getSoftClause(i).clause[j]) <
             currentModel.size());
      if ((sign(maxsat_formula->getSoftClause(i).clause[j]) &&
           currentModel[var(maxsat_formula->getSoftClause(i).clause[j])] ==
               l_False) ||
          (!sign(maxsat_formula->getSoftClause(i).clause[j]) &&
           currentModel[var(maxsat_formula->getSoftClause(i).clause[j])] ==
               l_True)) {
        unsatisfied = false;
        break;
      }
    }

    if (unsatisfied) {
      currentCost += maxsat_formula->getSoftClause(i).weight;
    }
  }

  return currentCost;
}

/*_________________________________________________________________________________________________
  |
  |  computeOriginalCost : (currentModel : vec<lbool>&) (weight : uint64_t) ->
  |                     [uint64_t]
  |
  |  Description:
  |
  |    Computes the cost of 'currentModel' as per the original weights.
  |    The cost of a model is the sum of the weights of the unsatisfied soft clauses.
  |    If a weight is specified, then it only considers the sum of the weights
  |    of the unsatisfied soft clauses with the specified weight.
  |
  |  Pre-conditions:
  |    * Assumes that 'currentModel' is not empty.
  |
  |________________________________________________________________________________________________@*/
uint64_t LinearSUClustering::computeOriginalCost(vec<lbool> &currentModel,
                                                 uint64_t weight) {

  assert(currentModel.size() != 0);
  uint64_t currentCost = 0;

  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    bool unsatisfied = true;
    for (int j = 0; j < maxsat_formula->getSoftClause(i).clause.size(); j++) {

      if (weight != UINT64_MAX &&
          maxsat_formula->getSoftClause(i).weight != weight) {
        unsatisfied = false;
        continue;
      }

      assert(var(maxsat_formula->getSoftClause(i).clause[j]) <
             currentModel.size());
      if ((sign(maxsat_formula->getSoftClause(i).clause[j]) &&
           currentModel[var(maxsat_formula->getSoftClause(i).clause[j])] ==
               l_False) ||
          (!sign(maxsat_formula->getSoftClause(i).clause[j]) &&
           currentModel[var(maxsat_formula->getSoftClause(i).clause[j])] ==
               l_True)) {
        unsatisfied = false;
        break;
      }
    }

    if (unsatisfied) {
      currentCost += cluster->getOriginalWeight(i);
    }
  }

  return currentCost;
}

/************************************************************************************************
 //
 // Incremental Linear Search Algorithm with Boolean Multilevel Optimization (BMO)
 //
 ************************************************************************************************/

/*_________________________________________________________________________________________________
  |
  |  bmoSearch : [void] ->  [void]
  |
  |  Description:
  |
  |    Incremental Linear search algorithm with lexicographical optimization modified for 
  |    incomplete weighted MaxSAT.
  |
  |  For further details see:
  |    * Joao Marques-Silva, Josep Argelich, Ana Graça, Ines Lynce: Boolean
  |      lexicographic optimization: algorithms & applications. Ann. Math.
  |      Artif. Intell. 62(3-4): 317-343 (2011)
  |
  |  Post-conditions:
  |    * 'lbCost' is updated.
  |    * 'ubCost' is updated.
  |    * 'nbSatisfiable' is updated.
  |    * 'nbCores' is updated.
  |
  |________________________________________________________________________________________________@*/
bool LinearSUClustering::bmoSearch() throw() {
  assert(orderWeights.size() > 0);
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
	
  uint64_t currentWeight = orderWeights[0];
  uint64_t minWeight = orderWeights[orderWeights.size() - 1];
  int posWeight = 0;

  vec<vec<Lit>> functions;
  vec<uint64_t> rhs;
  vec<uint64_t> ub_rhs;
  vec<uint64_t> best_rhs;
  uint64_t repair_cost = UINT64_MAX;
  std::vector<Encoder*> encoders;
  vec<Lit> encodingAssumptions;
  vec<Lit> current_assumptions;
  vec<vec<Lit>> functions_to_assumptions;
  vec<bool> encoder_created;
  encoder_created.growTo(orderWeights.size(), false);
  Encoder *pb = new Encoder();
  pb->setPBEncoding(_PB_GTE_);

  for (int j = 0; j < orderWeights.size(); j++){
    functions.push();
    new (&functions[j]) vec<Lit>();
    functions_to_assumptions.push();
    new (&functions_to_assumptions[j]) vec<Lit>();

    for (int i = 0; i < maxsat_formula->nSoft(); i++) {
      if (maxsat_formula->getSoftClause(i).weight == orderWeights[j]) {
        functions[j].push(maxsat_formula->getSoftClause(i).relaxation_vars[0]);
      }
    }
    rhs.push(UINT64_MAX);
    ub_rhs.push(UINT64_MAX);
    best_rhs.push(UINT64_MAX);
    Encoder* enc = new Encoder();
    enc->setIncremental(_INCREMENTAL_ITERATIVE_);
    enc->setCardEncoding(_CARD_TOTALIZER_);
    encoders.push_back(enc);
  }

  int current_function_id = 0;
  vec<Lit> assumptions;
  //printf("c objective function %d out of %d\n",current_function_id,orderWeights.size());

  bool repair = false;
  int repair_lvl = 0;

  vec<Lit> pb_function;
  vec<uint64_t> pb_coeffs;  
  
  for(;;){
    sat:

    res = searchSATSolver(solver, assumptions);
    if (res == l_True) {
      if (!repair){
        nbSatisfiable++;

  //printf("c weight %llu with size %d\n",orderWeights[current_function_id],functions[current_function_id].size());
        uint64_t newCost = computeCostModel(solver->model, orderWeights[current_function_id])/orderWeights[current_function_id];
        uint64_t originalCost = computeOriginalCost(solver->model);
  //printf("c objective function %d = o %" PRId64 " \n",current_function_id,newCost);
        if(best_cost > originalCost) {
            if (!saveModel(solver->model))
                return false;
          solver->model.copyTo(best_model);
          best_cost = originalCost;
          printf("o %" PRId64 " \n", originalCost);                    
        }

        if (newCost < rhs[current_function_id])
          rhs[current_function_id] = newCost;

        if (newCost == 0){
    // no need for cardinality constraint
          goto unsat;

        } else {
    // no cardinality constraint created
          if (!encoder_created[current_function_id]){
            if (newCost - 1 == 0){
              encodingAssumptions.clear();
              functions_to_assumptions[current_function_id].clear();
              for (int i = 0; i < functions[current_function_id].size(); i++){
                functions_to_assumptions[current_function_id].push(~functions[current_function_id][i]);
                encodingAssumptions.push(~functions[current_function_id][i]);
              }
            } else {
        //printf("c creating encoder with id = %d and value = %d\n",current_function_id,rhs[current_function_id]);
              encoders[current_function_id]->buildCardinality(solver, functions[current_function_id], newCost-1);
              if (encoders[current_function_id]->hasCardEncoding()){
                encoder_created[current_function_id] = true;
                encoders[current_function_id]->incUpdateCardinality(solver, functions[current_function_id], newCost-1, encodingAssumptions);
                assert(encodingAssumptions.size() == 1);

                functions_to_assumptions[current_function_id].clear();
                functions_to_assumptions[current_function_id].push(encodingAssumptions[0]);
              }
            }
          } else {
      //  printf("c updating the cost to %llu\n",newCost-1);
            encodingAssumptions.clear();
            encoders[current_function_id]->incUpdateCardinality(solver, functions[current_function_id], newCost-1, encodingAssumptions);
            assert(encodingAssumptions.size() == 1);

            functions_to_assumptions[current_function_id].clear();
            functions_to_assumptions[current_function_id].push(encodingAssumptions[0]);
          }
        }

        
        assumptions.clear();
        for (int i = 0; i <= current_function_id; i++){
          for (int j = 0; j < functions_to_assumptions[i].size(); j++){
            assumptions.push(functions_to_assumptions[i][j]);
      //printf("z = %d\n",var(functions_to_assumptions[i][j]));
          }
        }

      } else {
  // perform a linear search by decreasing the repair_cost
        uint64_t newCost = computeCostModel(solver->model, orderWeights[current_function_id])/orderWeights[current_function_id];
        uint64_t originalCost = computeOriginalCost(solver->model);
  //printf("c o %" PRId64 " \n", originalCost);
  //printf("repair_cost = %llu\n",repair_cost);
        if(best_cost > originalCost) {
            if (!saveModel(solver->model))
                return false;
          solver->model.copyTo(best_model);
          best_cost = originalCost;
          repair_cost = best_cost - 1;
          printf("o %" PRId64 " \n", originalCost);
        } else {
          repair_cost -= 1;
        }

        rescale:
  // rescale
        for (int i = 0; i < rhs.size(); i++){
          uint64_t value = repair_cost/orderWeights[i];
          if (value > functions[i].size())
            value = functions[i].size();
          if (value != ub_rhs[i]){
            ub_rhs[i] = value;
          }
        }

        if (!pb->hasPBEncoding()){
          pb->encodePB(solver, pb_function, pb_coeffs, repair_cost);
        }
        else
          pb->updatePB(solver, repair_cost);

        if (all_weights){
          for (int i = 0; i < functions_to_assumptions.size(); i++){
            functions_to_assumptions[i].clear();
            if (encoder_created[i]){
              if (ub_rhs[i] == 0){
                for (int i = 0; i < functions[i].size(); i++){
                  functions_to_assumptions[i].push(~functions[i][i]);
                }
              } else if (functions[i].size() != ub_rhs[i]){
    //printf("encoding %lu\n",ub_rhs[i]);
                encoders[i]->incUpdateCardinality(solver, functions[i], ub_rhs[i], encodingAssumptions);
    //printf("encodingAssumptions.size() = %d\n",encodingAssumptions.size());
                assert(encodingAssumptions.size() == 1);
                functions_to_assumptions[i].push(encodingAssumptions[0]);
              } else {
    //printf("size if the same!\n");
              }
            } else {
        //printf("ERROR\n");
              encoders[i]->buildCardinality(solver, functions[i], ub_rhs[i]);
              if (encoders[i]->hasCardEncoding()){
                encoder_created[i] = true;
                encoders[i]->incUpdateCardinality(solver, functions[i], ub_rhs[i], encodingAssumptions);
                assert(encodingAssumptions.size() == 1);
                functions_to_assumptions[i].push(encodingAssumptions[0]);
              }
        //          assert(false);
            }
          }

          assumptions.clear();
          for (int i = 0; i < functions_to_assumptions.size(); i++){
            for (int j = 0; j < functions_to_assumptions[i].size(); j++){
              assumptions.push(functions_to_assumptions[i][j]);
        //printf("function i =%d assumption= %d\n",i,var(functions_to_assumptions[i][j]));
            }
          }
        } else
        assumptions.clear();

      }

    } else {
      unsat:
      //printf("c UNSATISFIABLE\n");
      if (current_function_id == orderWeights.size()-1){
  // last function

        if (!complete){
          printAnswer(_OPTIMUM_);
          exit(_OPTIMUM_);
        }
        // ignore the complete part for now!
        if(repair){
          printAnswer(_OPTIMUM_);
          exit(_OPTIMUM_);
        }

        repair = true;

        printf("c Warn: changing to LSU algorithm.\n"); 

        if (best_cost < repair_cost){
          for (int i = 0; i < rhs.size(); i++){
            ub_rhs[i] = best_cost/orderWeights[i];
            //best_rhs[i] = rhs[i];
          }
          repair_cost = best_cost;
        }

        if (repair_cost == 0){
          printAnswer(_OPTIMUM_);
          exit(_OPTIMUM_);
        }

        if (all_weights){

          for (int i = 0; i < functions.size(); i++){
            for (int j =0; j < functions[i].size(); j++){
              pb_function.push(functions[i][j]);
              pb_coeffs.push(orderWeights[i]);
            }
          }

    // rescale
          for (int i = 0; i < rhs.size(); i++){
            ub_rhs[i] = repair_cost/orderWeights[i];
            if (ub_rhs[i] > functions[i].size())
              ub_rhs[i] = functions[i].size();
      //printf("i = %d rhs= %lu size= %d weigth=%llu\n",i,ub_rhs[i],functions[i].size(),orderWeights[i]);
          }

          for (int i = 0; i < functions_to_assumptions.size(); i++){
            functions_to_assumptions[i].clear();
      //printf("rhs = %lu\n",ub_rhs[i]);
            if (encoder_created[i]){
              if (ub_rhs[i] == 0){
                for (int i = 0; i < functions[i].size(); i++){
                  functions_to_assumptions[i].push(~functions[i][i]);
                }
              } else if (functions[i].size() != ub_rhs[i]){
                encoders[i]->incUpdateCardinality(solver, functions[i], ub_rhs[i], encodingAssumptions);
                assert(encodingAssumptions.size() == 1);
                functions_to_assumptions[i].push(encodingAssumptions[0]);
              }
            } else {
        //printf("ERROR\n");
              encoders[i]->buildCardinality(solver, functions[i], ub_rhs[i]);
              if (encoders[i]->hasCardEncoding()){
                encoder_created[i] = true;
                encoders[i]->incUpdateCardinality(solver, functions[i], ub_rhs[i], encodingAssumptions);
                assert(encodingAssumptions.size() == 1);
                functions_to_assumptions[i].push(encodingAssumptions[0]);
              }
            }
          }

          assumptions.clear();
          for (int i = 0; i < functions_to_assumptions.size(); i++){
            for (int j = 0; j < functions_to_assumptions[i].size(); j++){
              assumptions.push(functions_to_assumptions[i][j]);
            }
          }

          pb->setPBEncoding(_PB_GTE_);
          int expected_clauses = pb->predictPB(solver, pb_function, pb_coeffs, repair_cost-1);
          printf("c GTE auxiliary #clauses = %d\n",expected_clauses);
          if (expected_clauses >= MAX_CLAUSES) {
            printf("c Warn: changing to Adder encoding.\n");
            pb->setPBEncoding(_PB_ADDER_);
          }
          pb->encodePB(solver, pb_function, pb_coeffs, repair_cost-1);

        } else {

    // reverting to complete mode with original weights
          //printf("c reverting to original weights\n");
          assumptions.clear();
          pb_function.clear();
          pb_coeffs.clear();
          
          for (int i = 0; i < maxsat_formula->nSoft(); i++) {
            pb_function.push(maxsat_formula->getSoftClause(i).relaxation_vars[0]);
            pb_coeffs.push(cluster->getOriginalWeight(i));
          }

          pb->setPBEncoding(_PB_GTE_);
          int expected_clauses = pb->predictPB(solver, pb_function, pb_coeffs, repair_cost-1);
          printf("c GTE auxiliary #clauses = %d\n",expected_clauses);
          if (expected_clauses >= MAX_CLAUSES) {
            printf("c Warn: changing to Adder encoding\n");
            pb->setPBEncoding(_PB_ADDER_);
          }
          pb->encodePB(solver, pb_function, pb_coeffs, repair_cost-1);

        }

        goto sat;

      } else {
  // go to the next function
  //rhs[current_function_id];
        encodingAssumptions.clear();
        functions_to_assumptions[current_function_id].clear();
  //printf("c encoding created %d\n",encoder_created[current_function_id]);
  //printf("c objective function %d = best o %llu\n",current_function_id,rhs[current_function_id]);
        if (rhs[current_function_id] == 0){
          for (int i = 0; i < functions[current_function_id].size(); i++){
            functions_to_assumptions[current_function_id].push(~functions[current_function_id][i]);
            encodingAssumptions.push(~functions[current_function_id][i]);
          }
        } else if (encoder_created[current_function_id]){
    //printf("current function =%d\n",current_function_id);
    //    printf("c updating the cardinality to %llu\n",rhs[current_function_id]);
    //  printf("c size of function %d\n",functions[current_function_id].size());
          if (functions[current_function_id].size() != rhs[current_function_id]){
            encoders[current_function_id]->incUpdateCardinality(solver, functions[current_function_id], rhs[current_function_id], encodingAssumptions);
            assert(encodingAssumptions.size() == 1);
            functions_to_assumptions[current_function_id].push(encodingAssumptions[0]);
          }
        } else {
    //  printf("c creating encoder with id = %d and value = %d\n",current_function_id,rhs[current_function_id]);
          if (functions[current_function_id].size() != rhs[current_function_id]){
            encoders[current_function_id]->buildCardinality(solver, functions[current_function_id], rhs[current_function_id]);
            if (encoders[current_function_id]->hasCardEncoding()){
              encoders[current_function_id]->incUpdateCardinality(solver, functions[current_function_id], rhs[current_function_id], encodingAssumptions);
              assert(encodingAssumptions.size() == 1);
              functions_to_assumptions[current_function_id].push(encodingAssumptions[0]);
              encoder_created[current_function_id] = true;
            }
          }
        }

        assumptions.clear();
        for (int i = 0; i <= current_function_id; i++){
          for (int j = 0; j < functions_to_assumptions[i].size(); j++){
            assumptions.push(functions_to_assumptions[i][j]);
      //printf("z = %d\n",var(functions_to_assumptions[i][j]));
          }
        }
        current_function_id++;
      }
    }
  }
    return true;

}

// Public search method
bool LinearSUClustering::search() {


  MaxSATFormulaExtended *maxsat_formula_extended =
      static_cast<MaxSATFormulaExtended *>(maxsat_formula);
  cluster->clusterWeights(maxsat_formula_extended, num_clusters);

  for (int i = 0; i < maxsat_formula_extended->getSoftClauses().size(); i++) {
    unique_weights.insert(maxsat_formula_extended->getSoftClauses()[i].weight);
  }

  std::set<uint64_t> originalWeights;
  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    originalWeights.insert(cluster->getOriginalWeight(i));
  }

  // considers the problem as a lexicographical problem using the clustering as
  // objective functions
  orderWeights.clear();
  for (std::set<uint64_t>::iterator it = unique_weights.begin();
       it != unique_weights.end(); ++it) {
    orderWeights.push_back(*it);
  }
  if (unique_weights.size() == originalWeights.size())
    all_weights = true;

  std::sort(orderWeights.begin(), orderWeights.end(), greaterThan);

  printf("c #Diff Weights= %lu | #Modified Weights= %lu\n",
         originalWeights.size(), orderWeights.size());

  //printConfiguration(is_bmo, maxsat_formula->getProblemType());
    if (!bmoSearch())
        return false;
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
Solver *LinearSUClustering::rebuildSolver(uint64_t min_weight) {

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
void LinearSUClustering::initRelaxation() {
  for (int i = 0; i < maxsat_formula->nSoft(); i++) {
    Lit l = maxsat_formula->newLiteral();
    maxsat_formula->getSoftClause(i).relaxation_vars.push(l);
    objFunction.push(l);
    coeffs.push(maxsat_formula->getSoftClause(i).weight);
  }
}

// Print LinearSU configuration.
void LinearSUClustering::print_LinearSU_configuration() {
  printf("c |  Algorithm: %23s                                             "
         "                      |\n",
         "LinearSU");

  if (maxsat_formula->getProblemType() == _WEIGHTED_) {
    if (bmoMode)
      printf("c |  BMO strategy: %20s                      "
             "                                             |\n",
             "On");
    else
      printf("c |  BMO strategy: %20s                      "
             "                                             |\n",
             "Off");

    if (bmoMode) {
      if (is_bmo)
        printf("c |  BMO search: %22s                      "
               "                                             |\n",
               "Yes");
      else
        printf("c |  BMO search: %22s                      "
               "                                             |\n",
               "No");
    }
  }
}
