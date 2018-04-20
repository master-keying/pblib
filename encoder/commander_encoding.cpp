#include "commander_encoding.h"

using namespace std;

int64_t commander_encoding::encodingValue(const SimplePBConstraint& pbconstraint)
{
  int k = config->commander_encoding_k;
  int n = pbconstraint.getN();
  
  int64_t auxvars = (double)n*((double)1/k)/(1-(double)1/k);
  int64_t clauses = ((k*(double)(k+1)/2)+1) * auxvars;
  
  return valueFunction(clauses, auxvars);
}


void commander_encoding::encode_non_recursive(vector< Lit >& literals, ClauseDatabase& formula, AuxVarManager& auxvars)
{
//    basic_amo_encoder.encode_intern(literals, formula, auxvars);
  naive_amo_encoder.encode_intern(literals, formula);
}


void commander_encoding::encode_intern(vector< Lit >& literals, ClauseDatabase& formula, AuxVarManager& auxvars, bool isExactlyOne)
{
  int i;

  while (current_literals.size() > k)
  {
    _literals.clear();
    next_literals.clear();
    for (i = 0; i < current_literals.size(); ++i)
    {
      _literals.push_back(current_literals[i]);
      
      if (i % k == (k-1) || (i == current_literals.size() - 1) ) // is group full?
      {
	// 1. at most one variable in a gtroup can be true
	encode_non_recursive(_literals, formula, auxvars); 
	
	_literals.push_back(auxvars.getVariable());
	next_literals.push_back(-1 * _literals.back());
	
	if (isExactlyOne)
	{
	  //2. if the commander variable of a group is true, then at least one of the variables in the group must be true (only for the exactly one case)
	  formula.addClause(_literals); // at least one
	}
	
	// 3. if the commander variable of a group is false, then none of the variables in the group can be true
	for (int j = 0; j < _literals.size() - 1; ++j)
	{
	  formula.addClause(-1 * _literals.back(), -1 * _literals[j]);
	}
	_literals.clear();
      }
    }
    
    // exactly one of the commander variables is true
    current_literals = next_literals;
    isExactlyOne = true;
  }
  
  // at most one 
  encode_non_recursive(current_literals, formula, auxvars); 

  
  if (isExactlyOne)
  {
    // at least one
    formula.addClause(current_literals); 
  }
  
}


void commander_encoding::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{  
    formula.addConditionals(pbconstraint.getConditionals());
  
    if (config->print_used_encodings)
      cout << "c encode with command encoder amo" << endl;
    
    assert(pbconstraint.getLeq() == 1);

    current_literals.clear();
    for (int i = 0; i < (int) pbconstraint.getN(); ++i)
      current_literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);
    
    encode_intern(current_literals, formula, auxvars, (pbconstraint.getComparator() == PBLib::BOTH && (pbconstraint.getGeq() == 1)));
    
    
    for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
      formula.getConditionals().pop_back();
}


commander_encoding::commander_encoding(PBConfig& config) : Encoder(config), basic_amo_encoder(config), naive_amo_encoder(config)
{
  k = config->commander_encoding_k;
}

commander_encoding::~commander_encoding()
{

}

