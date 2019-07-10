#include "binary_amo.h"

#include <cmath>

using namespace std;

void Binary_AMO_Encoder::encode_intern(vector< int32_t >& literals, ClauseDatabase & formula, AuxVarManager & auxvars)
{
  int gray_code;
  int next_gray;
  int i = 0;
  int index = -1;
  for (; i < k; ++i)
  {
    index++;
    gray_code = i ^ (i >> 1);
    i++; // we skip the next binary string to use the redundancy
    next_gray = i ^ (i >> 1);
    for (int j = 0; j < nBits; ++j)
    {
      if ( (gray_code & (1 << j)) == (next_gray & (1 << j)) )
      {
	if ( (gray_code & (1 << j)) != 0)
	  formula.addClause(-literals[index], bits[j]);
	else
	  formula.addClause(-literals[index], -bits[j]);
      }
      // else skip that bit since it is redundant
    }
  }


  for (; i < two_pow_nbits; ++i)
  {
    index++;
    gray_code = i ^ (i >> 1);
    for (int j = 0; j < nBits; ++j)
    {
      if ( (gray_code & (1 << j)) != 0)
	formula.addClause(-literals[index], bits[j]);
      else
	formula.addClause(-literals[index], -bits[j]);
    }
  }
  assert(index + 1 == literals.size());
}


int64_t Binary_AMO_Encoder::encodingValue(const SimplePBConstraint& pbconstraint)
{
  int n = pbconstraint.getN();
  return valueFunction(n*ceil(log2(n)), ceil(log2(n))); // from the paper .. seems to overestimating .. FIXME
}



void Binary_AMO_Encoder::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars)
{
  formula.addConditionals(pbconstraint.getConditionals());

  if (config->print_used_encodings)
    cout << "c encode with binary amo" << endl;

  assert(pbconstraint.getLeq() == 1);

  _literals.clear();
  bits.clear();

  for (int i = 0; i < (int) pbconstraint.getN(); ++i)
    _literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);


  if (pbconstraint.getComparator() == PBLib::BOTH && (pbconstraint.getGeq() == 1))
  {
    assert(pbconstraint.getGeq() == 1 && pbconstraint.getLeq() == 1);
    formula.addClause(_literals);
  }

  nBits = ceil(log2(_literals.size()));
  two_pow_nbits = pow(2,nBits);
  k = (two_pow_nbits - _literals.size()) * 2; // k is the number of literals that share a bit because of redundancy

  for (int i = 0; i < nBits; ++i)
    bits.push_back(auxvars.getVariable());

  encode_intern(_literals, formula, auxvars);

  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}


Binary_AMO_Encoder::Binary_AMO_Encoder(PBConfig& config) : Encoder(config)
{

}
