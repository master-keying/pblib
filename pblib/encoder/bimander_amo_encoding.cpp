#include "bimander_amo_encoding.h"

#include <cmath>

using namespace PBLib;
using namespace std;

template <typename T>
ostream & operator<< (ostream & out, vector<T> & vec)
{
  if (vec.size() == 0)
  {
    cout << "{ }";
    return out;
  }

  out << "{ " << vec[0];

  for (int i = 1; i < vec.size(); ++i)
    out << ", " << vec[i];

  out << " }";
  return out;
}


int64_t Bimander_amo_encoding::encodingValue(const SimplePBConstraint& pbconstraint)
{
  int m = config->bimander_m;
  int n = pbconstraint.getN();

  if (config->bimander_m_is == BIMANDER_M_IS::FIXED)
    m = config->bimander_m;
  else if (config->bimander_m_is == BIMANDER_M_IS::N_HALF)
    m = ceil ((double)n / 2);
  else if (config->bimander_m_is == BIMANDER_M_IS::N_SQRT)
    m = ceil (sqrt((double)n));
  else
    m = config->bimander_m;


  int64_t clauses = ceil((double) (n*n) / (double) (2*m)) + n * ceil(log2(m)) - ceil ((double)n/2); // from the paper .. seems to overestimating .. FIXME
  int64_t auxvars = ceil(log2(m));

  return valueFunction(clauses, auxvars);
}


void Bimander_amo_encoding::encode_intern(vector< Lit >& literals, ClauseDatabase& formula, AuxVarManager& auxvars)
{

  int n = literals.size();

  if (config->bimander_m_is == BIMANDER_M_IS::FIXED)
    m = config->bimander_m;
  else if (config->bimander_m_is == BIMANDER_M_IS::N_HALF)
    m = ceil ((double)n / 2);
  else if (config->bimander_m_is == BIMANDER_M_IS::N_SQRT)
    m = ceil (sqrt((double)n));
  else
    m = config->bimander_m;


  assert(m > 0);
  // create the m groups
  groups.clear();
  groups.resize(m);

  int g = ceil( (double) n / m);
  int ig = 0;
  int i;
  for (i = 0; i < literals.size(); )
  {
      while (i < g)
      {
	groups[ig].push_back(literals[i]);
	i++;
      }
      ig++;
      g = g + ceil( (double) (n - i) / (m-ig));
  }
  assert(ig == m);
  assert(m == groups.size());
  assert(i == literals.size());


  for (int i = 0; i < groups.size(); ++i)
  {
    naive_amo_encoder.encode_intern(groups[i], formula);
  }


  bits.clear();
  nBits = ceil(log2(m));
  two_pow_nbits = pow(2,nBits);
  k = (two_pow_nbits - m) * 2; // k is the number of literals that share a bit because of redundancy

  for (int i = 0; i < nBits; ++i)
    bits.push_back(auxvars.getVariable());

  int gray_code;
  int next_gray;
  i = 0;
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
	{
	  for (int g = 0; g < groups[index].size(); ++g)
	  {
	    formula.addClause(-groups[index][g], bits[j]);
	  }
	}
	else
	{
	  for (int g = 0; g < groups[index].size(); ++g)
	  {
	    formula.addClause(-groups[index][g], -bits[j]);
	  }
	}
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
	{
	  for (int g = 0; g < groups[index].size(); ++g)
	  {
	    formula.addClause(-groups[index][g], bits[j]);
	  }
	}
	else
	{
	  for (int g = 0; g < groups[index].size(); ++g)
	  {
	    formula.addClause(-groups[index][g], -bits[j]);
	  }
	}
    }
  }
  assert(index + 1 == groups.size());
}


void Bimander_amo_encoding::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  formula.addConditionals(pbconstraint.getConditionals());

  if (config->print_used_encodings)
    cout << "c encode with bimander amo" << endl;

  assert(pbconstraint.getLeq() == 1);

  _literals.clear();

  for (int i = 0; i < (int) pbconstraint.getN(); ++i)
    _literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);


  if (pbconstraint.getComparator() == BOTH && (pbconstraint.getGeq() == 1))
  {
    assert(pbconstraint.getGeq() == 1 && pbconstraint.getLeq() == 1);
    formula.addClause(_literals);
  }

  encode_intern(_literals, formula, auxvars);
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}



Bimander_amo_encoding::Bimander_amo_encoding(PBConfig& config) : Encoder(config), naive_amo_encoder(config)
{

}
