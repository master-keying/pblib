#include "naive_amo_encoder.h"

using namespace std;

void Naive_amo_encoder::encode_intern(vector< Lit >& literals, ClauseDatabase& formula)
{
  for (int i = 0; i < literals.size(); ++i)
  {
    for (int j = i + 1 ; j < literals.size(); ++j)
    {
      formula.addClause(-literals[i], -literals[j]);
    }
  }
}

int64_t Naive_amo_encoder::encodingValue(const SimplePBConstraint& pbconstraint)
{
  int n = pbconstraint.getN();
  return valueFunction( (double)(n*n+n) / (double)2, 0);
}


void Naive_amo_encoder::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  formula.addConditionals(pbconstraint.getConditionals());

  if (config->print_used_encodings)
    cout << "c encode with naive amo encoder" << endl;
  
  assert(pbconstraint.getLeq() == 1);

  _literals.clear();

  for (int i = 0; i < (int) pbconstraint.getN(); ++i)
    _literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);


  if (pbconstraint.getComparator() == PBLib::BOTH && (pbconstraint.getGeq() == 1))
  {
    assert(pbconstraint.getGeq() == 1 && pbconstraint.getLeq() == 1);
    formula.addClause(_literals);
  }
  
  encode_intern(_literals, formula);
  
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}

Naive_amo_encoder::Naive_amo_encoder(PBConfig& config) : Encoder(config)
{

}

Naive_amo_encoder::~Naive_amo_encoder()
{

}
