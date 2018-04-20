#include "IncSimplePBConstraint.h"
using namespace PBLib;
using namespace std;

IncSimplePBConstraint::IncSimplePBConstraint(int64_t max_sum, int64_t max_weight, int64_t normalizedOffset, PBTYPE type, vector< WeightedLit >& literals, Comparator comparator, int64_t less_eq, int64_t greater_eq): SimplePBConstraint(max_sum, max_weight, type, literals, comparator, less_eq, greater_eq)
{
  this->normalized_offset = normalizedOffset;
  init_leq = less_eq;
  init_geq = greater_eq;
}

IncSimplePBConstraint::IncSimplePBConstraint(int64_t max_sum, int64_t max_weight,int64_t normalizedOffset, PBTYPE type, vector< WeightedLit >& literals, Comparator comparator, int64_t bound): SimplePBConstraint(max_sum, max_weight,type, literals, comparator, bound)
{
  this->normalized_offset = normalizedOffset;
  
  assert(comparator == LEQ);
  init_leq = bound;
  init_geq = 0;
}

void IncSimplePBConstraint::encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  assert(comparator == BOTH);
  geq = newGeq + normalized_offset; // add the offset from the normalization to the new bound
  
  if (geq <= init_geq)
  {
    geq = init_geq;
    return;
  }
  
  
  if ((leq < geq))
  {
    formula.addConditionals(conditionals);
    formula.addUnsat();
    for (int i = 0; i < conditionals.size(); ++i)
	formula.getConditionals().pop_back();
    return;
  }
  
  if (geq <= 0)
    return;
  
  incremental_data->encodeNewGeq(geq, formula, auxVars, conditionals);
}

void IncSimplePBConstraint::encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  leq = newLeq + normalized_offset; // add the offset from the normalization to the new bound
  if (leq >= init_leq)
  {
    leq = init_leq;
    return;
  }
  
  // check if constraint became trivial
  if (leq < 0 || (leq < geq))
  {
    formula.addConditionals(conditionals);
    formula.addUnsat();
    for (int i = 0; i < conditionals.size(); ++i)
	formula.getConditionals().pop_back();
	
    return;
  }
  else if (leq == 0)
  {
    // set all literals to false to satisfied == 0
    for (int i = 0; i < (int)weighted_literals.size(); ++i)
    {
      formula.addConditionals(conditionals);
      formula.addClause(-weighted_literals[i].lit);
      for (int i = 0; i < conditionals.size(); ++i)
	  formula.getConditionals().pop_back();
      
    }
    return;
  }
  

  incremental_data->encodeNewLeq(leq, formula, auxVars, conditionals);
}


void IncSimplePBConstraint::setIncrementalData(shared_ptr< IncrementalData > incrementalData)
{
  incremental_data = incrementalData;
}


IncSimplePBConstraint::~IncSimplePBConstraint()
{

}


