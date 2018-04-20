#include "preencoder.h"

#include <vector>

using namespace PBLib;
using namespace std;
template <class PBCon>
void PreEncoder::init_and_normalize(PBCon const & pbconstraint, ClauseDatabase& formula)
{
  literals = pbconstraint.getWeightedLiterals();
  comparator = pbconstraint.getComparator();
  max_sum = 0;
  max_weight = 0;
  bound_offset = 0;
  type = PB;
  n = literals.size();
  isAMK = false;
  isAMKEqual = false;
  leq = pbconstraint.getLeq();
  geq = pbconstraint.getGeq();
  
  if (comparator == GEQ)
  { // normalize to LEQ by negation
    comparator = LEQ;
    leq = -geq;
    for (int i = 0; i < (int)literals.size(); ++i)
      literals[i].weight = -literals[i].weight;
  }
  
  join_duplicat_literals();
  normalize_variables();
 
  leq += bound_offset;
  geq += bound_offset;
  
  remove_lits_with_w_greater_leq_and_check_isamk(formula);
}


void PreEncoder::remove_lits_with_w_greater_leq_and_check_isamk(ClauseDatabase& formula)
{
  isAMK = true;
  isAMKEqual = true;
  check_amk_equal = 0;
  if (literals.size() > 0)
  {
     check_amk_equal = literals[0].weight;
  }
  else
  {
    isAMKEqual = false;
    isAMK = false;
  }
  
  for (int i = 0; i < (int)literals.size(); ++i)
  {  
    tmpWeight = literals[i].weight;
    
    if(tmpWeight > leq)
    {
      formula.addClause(-literals[i].lit);
      literals[i] = literals[literals.size() - 1];
      literals.pop_back();
      i--;
      n--;
      continue;
    }
    
    max_sum += tmpWeight;
    
    if (isAMK && tmpWeight != 1)
    {
      isAMK = false;
    }
    
    if (isAMKEqual && tmpWeight != check_amk_equal)
    {
      isAMKEqual = false;
    }
    
    if (tmpWeight > max_weight)
      max_weight = tmpWeight;
  }
  
  if (check_amk_equal == 0)
    isAMKEqual = false;
}


void PreEncoder::normalize_variables()
{
  for (int i = 0; i < (int)literals.size(); ++i)
  {
    tmpWeight = literals[i].weight;
    if ( tmpWeight == 0) // ignore literals with weight 0
    {
      literals[i] = literals[literals.size() - 1];
      literals.pop_back();
      i--;
      n--;
      continue;
    }
    
    if (tmpWeight < 0) // if the weight is negative, normalize literal
    {
      tmpWeight = -tmpWeight; // results in a positive weight
      bound_offset += tmpWeight; // adjust bound offset (for leq / geq lateron)
      literals[i].weight = tmpWeight; // save new (positive) weight
      literals[i].lit = -literals[i].lit; // save the complement of the literal
    }
  }
}


void PreEncoder::join_duplicat_literals()
{
  sort(literals.begin(), literals.end(),WeightedLit::compVariable_des_var);
  vector<int32_t> del_element;
  for (int i = 1; i < literals.size(); ++i)
  {
    if (literals[i].lit == literals[i-1].lit)
    {
      literals[i].weight += literals[i-1].weight;
      del_element.push_back(i-1);
    }
    else if (literals[i].lit == -literals[i-1].lit)
    {
      bound_offset -= literals[i-1].weight;
      literals[i].weight += -literals[i-1].weight;
      del_element.push_back(i-1);
    }
  }
  
  for (int i = del_element.size() - 1; i >= 0; --i)
  {
    // no delete duplicates
    literals[del_element[i]] = literals.back();
    literals.pop_back();
    n--;
  }
}


void PreEncoder::check_for_trivial_constraints(ClauseDatabase & formula)
{
  assert(literals.size() == n);
  
  if (leq < 0 || (comparator == BOTH && leq < geq) || (comparator == BOTH && max_sum < geq))
  {
    type = DONTCARE;
    formula.addUnsat();
    stats->num_trivial++;
  }
  else if (n == 0)
  {
    if (comparator == BOTH && geq > 0) 
      formula.addUnsat();    
    
    type = DONTCARE;
    stats->num_trivial++;
    max_weight = 0;
    max_sum = 0;
  }
  else if (leq == 0)
  {
    // in the incremental case we will detected leq < 0 in the encodeNewLeq methode of IncSimplePBConstraint
    // set all literals to false to satisfied == 0
    type = DONTCARE;
    stats->num_trivial++;
    for (int i = 0; i < (int)literals.size(); ++i)
    {
      formula.addClause(-literals[i].lit);
    }
    literals.clear();
    n = 0;
    max_weight = 0;
    max_sum = 0;
  }
}

void PreEncoder::sort_literals()
{
  sort(literals.begin(), literals.end(),WeightedLit::compVariable_des);
//   sort(literals.begin(), literals.end(),WeightedLit::compVariable_asc);
}

// WARNING DONT TOUCH THIS, THIS HAVE TO BE REWRITTEN (TO MANY DEPENDENCIES) TODO
shared_ptr< IncSimplePBConstraint > PreEncoder::preEncodeIncPBConstraint(IncPBConstraint & pbconstraint, ClauseDatabase& formula)
{
  formula.addConditionals(pbconstraint.getConditionals());

  init_and_normalize(pbconstraint, formula);
  check_for_trivial_constraints(formula);
  
  if (type != DONTCARE)
  {   
    if (isAMK)
    {
      if (leq == 1)
      {
	type = AMO;
	stats->num_amo++;
      }
      else
      {
	type = AMK;
	stats->num_amk++;
      }
    }
    else
    {
      type = PB;
      stats->num_pb++;
    }
    sort_literals();
  }
  
    
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
    
  if (leq > max_sum)
    leq = max_sum;
    
  if (comparator == LEQ)
  { 
    shared_ptr<IncSimplePBConstraint> incSimplePbConstraint = make_shared<IncSimplePBConstraint>(IncSimplePBConstraint(max_sum, max_weight, bound_offset, type, literals, LEQ, leq));
    pbconstraint.setIncSimplePBConstraint(incSimplePbConstraint);
    incSimplePbConstraint->addConditionals(pbconstraint.getConditionals());
    return incSimplePbConstraint;
  }
  else
  {
    if (geq < 0)
      geq = 0;
    
    assert (comparator == BOTH);
    shared_ptr<IncSimplePBConstraint> incSimplePbConstraint = make_shared<IncSimplePBConstraint>(IncSimplePBConstraint(max_sum, max_weight, bound_offset, type, literals, comparator, leq, geq));
    pbconstraint.setIncSimplePBConstraint(incSimplePbConstraint);
    incSimplePbConstraint->addConditionals(pbconstraint.getConditionals());
    return incSimplePbConstraint;
  }
}


// WARNING DONT TOUCH THIS, THIS HAVE TO BE REWRITTEN (TO MANY DEPENDENCIES) TODO
SimplePBConstraint PreEncoder::preEncodePBConstraint(const PBConstraint& pbconstraint, ClauseDatabase& formula)
{
  formula.addConditionals(pbconstraint.getConditionals());
  init_and_normalize(pbconstraint, formula);
  check_for_trivial_constraints(formula);
  
  if (type == DONTCARE)
    ; // we already encode this constraint and continue below
  else if (comparator == BOTH && geq == max_sum) // note that the following trivial constraints cannot used for incremental constraints
  {
    // set all literals to true to satisfied == 0
    type = DONTCARE;
    stats->num_trivial++;
    for (int i = 0; i < (int)literals.size(); ++i)
    {
      formula.addClause(literals[i].lit);
    }
  }
  else if (max_sum <= leq)
  {
    if (comparator == LEQ)
    {
      stats->num_trivial++;
      type = DONTCARE;
    }
    else
    {
      assert(comparator == BOTH);
      PBConstraint c(literals, GEQ, geq);
      c.addConditionals(pbconstraint.getConditionals());
      
      for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
	formula.getConditionals().pop_back();
      
      return preEncodePBConstraint(c, formula);
    }
  }
  else if (n == 2 && comparator == LEQ && max_sum > leq)
  {
    // since maxsum > leq and n = 2 we know that this is an AMO and with n= 2 that this is a clause
    type = DONTCARE;
    clause.clear();
    
    for (int i = 0; i < (int) literals.size(); ++i)
    {
      clause.push_back(-literals[i].lit);
    }
    formula.addClause(clause);
    stats->num_clause++;
  }
  else
  {
    // no trivial constraint found ... trying to simplify
      if (comparator == BOTH && ((max_sum - leq) < geq)  && ((max_sum - geq) < leq) )
      {
	// both bounds getting smaller if we negate the formula
	for (int i = 0; i < (int)literals.size(); ++i)
	{
	  literals[i].lit = -literals[i].lit;
	}
	
	// we can simply use max_sum as bound offset, since we know that all weights are > 0
	// but we have to change leq in geq and vice versa, because of the negation
	int64_t oldGeq = geq;
	geq = max_sum - leq;
	leq = max_sum - oldGeq;
	
	// we have changed k so we have to readjust the variables
	PBConstraint c(literals, BOTH, leq, geq);
	c.addConditionals(pbconstraint.getConditionals());
	
	for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
	  formula.getConditionals().pop_back();
	
	return preEncodePBConstraint(c, formula);
      }

      if (comparator == BOTH && geq <= 0)
	  comparator = LEQ; // since we know that all weights > 0
	  
      if (comparator == BOTH && geq == 1)
      {
	// >= 1 is a clause since all weights are > 0 ... we simply add this clause and remove GEQ constraint
	comparator = LEQ;
	clause.clear();
	
	for (int i = 0; i < (int) literals.size(); ++i)
	{
	  clause.push_back(literals[i].lit);
	}
	formula.addClause(clause);
	
	stats->num_clause++;
	comparator = LEQ;
	
	// we have change the comparator so we can try to find new simplifications
	PBConstraint c(literals, LEQ, leq);
	c.addConditionals(pbconstraint.getConditionals());
	
	for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
	  formula.getConditionals().pop_back();
	
	return preEncodePBConstraint(c, formula);
      }
    
    if (isAMK)
    {
      // it should not happen that comparator is BOTH and n == leq + 1, since is propably handled already be the two previouse simplification
      if (comparator == LEQ && n == leq + 1)
      {
	// this is a clause
	type = DONTCARE;
	clause.clear();
	
	for (int i = 0; i < (int) literals.size(); ++i)
	{
	  clause.push_back(-literals[i].lit);
	}
	formula.addClause(clause);
	stats->num_clause++;
      }
      else
      {
	if (leq == 1)
	{
	  stats->num_amo++;
	  type = AMO;
	}
	else
	{
	  stats->num_amk++;
	  type = AMK;
	}
      }
    }
    else
    {
      type = PB;
      stats->num_pb++;
    }
    
    // NOTE that the XOR (a + b = 1) case is already handled by the previouse leq and geq clause detactions
    
    sort_literals();
  }
  
  
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
  
  if (comparator == LEQ)
  {
    SimplePBConstraint sc(max_sum, max_weight, type, literals, comparator, leq);
    sc.addConditionals(pbconstraint.getConditionals());
    return sc;
  }
  else
  {
    SimplePBConstraint sc(max_sum, max_weight,type, literals, comparator, leq, geq);
    sc.addConditionals(pbconstraint.getConditionals());
    return sc;
  }
}


PreEncoder::PreEncoder(PBConfig config, statistic* _stats) : config(config), stats(_stats)
{
  if (stats == 0)
  {
    stats = new statistic;
    private_stats = true;
  }
  else
  {
    private_stats = false;
  }
}

PreEncoder::~PreEncoder()
{
  if (private_stats)
    delete stats;
}

