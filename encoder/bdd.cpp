#include "bdd.h"
#include "adderencoding.h"
#include "../preencoder.h"

using namespace PBLib;
using namespace std;

bool BDD_Encoder::wasToBig() const
{
  return canceled;
}


void BDD_Encoder::iterativeEncoding(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars, bool noLimit, int64_t maxClauses)
{
  
  stack.clear();
  stack.resize(pbconstraint.getN() + 1);
  
  stack[0].maxsum = pbconstraint.getMaxSum();
  stack[0].currentsum = 0;
  
  int32_t index = 0;
  bool down = false;
  
  //note that k is not leq but less then  // TODO change this
  int64_t geq = 0;
  bool isBoth = (pbconstraint.getComparator() == BOTH);
  
  if (isBoth)
    geq = pbconstraint.getGeq();
  
  while (index >= 0)
  {
    if (!noLimit && maxClauses < 0)
    {
      canceled = true;
      return;
    }
    
    build_data & node = stack[index];
        
    if (node.currentsum >= geq && node.currentsum + node.maxsum <  k)
    {
      node.result = true_lit;
      index--;
      down = true;
      continue;
    }
    
    if (node.currentsum + node.maxsum < geq || node.currentsum >= k)
    {
      node.result = -true_lit;
      index--;
      down = true;
      continue;
    }
    
    if (sumHistory.count(pair<int32_t, int64_t>(inputVars[index].lit, node.currentsum)) > 0)
    {
      node.result = sumHistory[pair<int32_t, int64_t>(inputVars[index].lit, node.currentsum)];
      index--;
      down = true;
      continue;
    }
    
    if (node.high == 0)
    {
      if (down)
      {
	node.high = stack[index + 1].result;
	down = false;
	assert(node.low == 0);
      }
      else
      {
	stack[index + 1].maxsum = node.maxsum - inputVars[index].weight;
	stack[index + 1].currentsum = node.currentsum + inputVars[index].weight;
	stack[index + 1].high = 0;
	stack[index + 1].low = 0;
	stack[index + 1].result = 0;
	index++;
	down = false;
	continue;
      }
    }
    
    if (node.low == 0)
    {
      if (down)
      {
	node.low = stack[index + 1].result;
	down = false;
      }
      else
      {
	stack[index + 1].maxsum = node.maxsum - inputVars[index].weight;
	stack[index + 1].currentsum = node.currentsum;
	stack[index + 1].high = 0;
	stack[index + 1].low = 0;
	stack[index + 1].result = 0;
	index++;
	down = false;
	continue;
      }
    }
    
    if (node.low == node.high)
    {
      node.result = node.low;
      index--;
      down = true;
      continue;
    }

    
    if (config->use_real_robdds && nodeHistory.count(make_tuple(inputVars[index].lit, node.high, node.low)) > 0)
    {
      node.result = nodeHistory[make_tuple(inputVars[index].lit, node.high, node.low)];
      index--;
      down = true;
      continue;
    }

    
    if (node.high == -true_lit && node.low == true_lit)
    {
      node.result = -inputVars[index].lit;
    }
    else
    {
      node.result = auxvars.getVariable();  
      
      if (isBoth)
      {
	maxClauses = maxClauses - 3;
	if (node.high == true_lit)
	  maxClauses++;
	else
	if (node.high == -true_lit)
	  formula.addClause(-node.result, -inputVars[index].lit);
	else
	  formula.addClause(-node.result, -inputVars[index].lit, node.high);
	
	if (node.low == true_lit)
	  maxClauses++;
	else
	if (node.low == -true_lit)
	  formula.addClause(-node.result, inputVars[index].lit);
	else
	  formula.addClause(-node.result, inputVars[index].lit, node.low);
	  
	
	if (node.high == true_lit || node.low == true_lit)
	  maxClauses++;
	else
	if (node.high == -true_lit)
	  formula.addClause(-node.result, node.low);
	else
	if (node.low == -true_lit)  
	  formula.addClause(-node.result, node.high);
	else
	  formula.addClause(-node.result, node.high, node.low);
      }
      else
      {
	if (node.low != true_lit)
	{
	  maxClauses--;
	  formula.addClause(node.low, -node.result);
	}
	
	maxClauses--;
	if (node.high == -true_lit)
	  formula.addClause(-inputVars[index].lit, -node.result);
	else
	  formula.addClause(node.high,-inputVars[index].lit, -node.result);
      }
    }
    
    sumHistory[pair<int32_t, int64_t>(inputVars[index].lit, node.currentsum)] = node.result;
    if (config->use_real_robdds)
      nodeHistory[make_tuple(inputVars[index].lit, node.high, node.low)] = node.result;
    
    index--;
    down = true;
  }
  
  formula.addConditionals(pbconstraint.getConditionals());
  formula.addClause(stack[0].result);
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();

}


int32_t BDD_Encoder::buildBDD(int index, int64_t currentsum, int64_t maxsum, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  test_counter = max (test_counter, index);
  
  if (currentsum + maxsum < k)
    return true_lit;
  if (currentsum >= k)
    return -true_lit;
  
  if (sumHistory.count(pair<int32_t, int64_t>(inputVars[index].lit, currentsum)) > 0)
    return sumHistory[pair<int32_t, int64_t>(inputVars[index].lit, currentsum)];
  
  int32_t high = buildBDD(index+1, currentsum + inputVars[index].weight, maxsum - inputVars[index].weight, formula, auxvars);
  int32_t  low = buildBDD(index+1, currentsum, maxsum - inputVars[index].weight, formula, auxvars);
  
  if (high == low)
    return high;
  
  if (config->use_real_robdds && nodeHistory.count(make_tuple(inputVars[index].lit, high, low)) > 0)
    return nodeHistory[make_tuple(inputVars[index].lit, high, low)];
  
  int32_t node;
  
  if (high == -true_lit && low == true_lit)
  {
    node = -inputVars[index].lit;
  }
  else
  {
    node = auxvars.getVariable();  
    
    if (low != true_lit)
      formula.addClause(low, -node);
    
    if (high == -true_lit)
      formula.addClause(-inputVars[index].lit, -node);
    else
      formula.addClause(high,-inputVars[index].lit, -node);
  }
  
  sumHistory[pair<int32_t, int64_t>(inputVars[index].lit, currentsum)] = node;
  if (config->use_real_robdds)
    nodeHistory[make_tuple(inputVars[index].lit, high, low)] = node;
  
  return node;
}

void BDD_Encoder::recusiveEncoding(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  int32_t bdd_lit = buildBDD(0, 0, pbconstraint.getMaxSum(), formula, auxvars);
  
  formula.addConditionals(pbconstraint.getConditionals());
  formula.addClause(bdd_lit);
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}


void BDD_Encoder::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  if (config->print_used_encodings)
      cout << "c encode with BDD encoding" << endl;
  
  bddEncode(pbconstraint, formula, auxvars);
}

int64_t BDD_Encoder::encodingValue(const SimplePBConstraint& pbconstraint)
{  
  CountingClauseDatabase formula(config);
  AuxVarManager auxvars(1000000);
  bddEncode(pbconstraint, formula, auxvars, false, config->MAX_CLAUSES_PER_CONSTRAINT);
  
  if (canceled)
    return -1;
  
  return valueFunction(formula.getNumberOfClauses(), auxvars.getBiggestReturnedAuxVar() - 1000000);
}



void BDD_Encoder::bddEncode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars, bool noLimit, int64_t maxClauses)
{
  canceled = false;
  
  sumHistory.clear();
  nodeHistory.clear();

  k = pbconstraint.getLeq() + 1;
  inputVars = pbconstraint.getWeightedLiterals();

  
  sort(inputVars.begin(), inputVars.end(),WeightedLit::compVariable_des);
  
  true_lit = auxvars.getVariable();
  formula.addClause(true_lit);
  
  if (config->use_recursive_bdd_test && noLimit && pbconstraint.getComparator() == LEQ) // TODO implement clause limit to recursive and BOTH
    recusiveEncoding(pbconstraint, formula, auxvars);
  else
    iterativeEncoding(pbconstraint, formula, auxvars, noLimit, maxClauses);
    
  sumHistory.clear();
  nodeHistory.clear();  
}
