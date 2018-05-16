#include "k-product.h"

using namespace std;

// TODO implement an iterative (non-recursive) encoding  ... is this necessary? TODO performance check
void k_Product::encode_intern(vector< Lit >& literals, ClauseDatabase& formula, AuxVarManager& auxvars)
{
    if (literals.size() == 1)
      return;

    if (literals.size() == 2)
    {
      if (literals[0] == -literals[1])
	return;
    }

    if (literals.size() < minimum_lit_count)
    {
      encode_non_recursive(literals, formula, auxvars);
      return;
    }

    // seting up the dimensions vector with the auxilary varibales
    vector<vector<Lit> > dimensions;
    dimensions.resize(k);
    int n = literals.size();
    int tmp_count;
    for (int i = 0; i < k - 1; ++i)
    {
      tmp_count = ceil( pow(n, (double)1 / k) );

      if (tmp_count == 2)
      {
	dimensions[i].push_back(auxvars.getVariable());
	dimensions[i].push_back(-dimensions[i][0]);
      }
      else
      {
	for (int j = 0; j < tmp_count; ++j)
	  dimensions[i].push_back(auxvars.getVariable());
      }

      n = ceil((double)n / tmp_count);
    }

    tmp_count = n;
    for (int j = 0; j < tmp_count; ++j)
      dimensions[k-1].push_back(auxvars.getVariable());

    // encode mapping from grid to input literal
    int index;
    for (int i = 0; i < literals.size(); ++i)
    {
      Lit x = literals[i];
      int tmp_div = 1;
      for (int d = 0; d < dimensions.size(); ++d)
      {
	index = (i / tmp_div) % (dimensions[d].size());
	tmp_div = tmp_div * dimensions[d].size();
	Lit v = dimensions[d][index];
	formula.addClause(-x,v);
      }
    }

    // encode at most one constraints for each dimension
    for (int i = 0; i < dimensions.size(); ++i)
      encode_intern(dimensions[i], formula, auxvars);

}

void k_Product::encode_non_recursive(vector< Lit >& literals, ClauseDatabase& formula, AuxVarManager& auxvars)
{
//   basic_amo_encoder.encode_intern(literals, formula, auxvars);
  naive_amo_encoder.encode_intern(literals, formula);
}

int64_t k_Product::encodingValue(const SimplePBConstraint& pbconstraint)
{
  int n = pbconstraint.getN();

  if (config->k_product_k == 2)
  {
    int64_t clauses = ceil(2*n + (double)4 * sqrt(n)+pow(n,0.25f));
    int64_t auxvars = ceil((double)2 * sqrt(n)+pow(n,0.25f));

    return valueFunction(clauses, auxvars);
  }
  else
  {
    CountingClauseDatabase formula(config);
    AuxVarManager auxvars(1000000);

    _literals.clear();
    for (int i = 0; i < (int) pbconstraint.getN(); ++i)
      _literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);

    if (pbconstraint.getComparator() == PBLib::BOTH)
    {
      assert(pbconstraint.getGeq() == 1 && pbconstraint.getLeq() == 1);
      formula.addClause(_literals);
    }

    encode_intern(_literals, formula, auxvars);

    return valueFunction(formula.getNumberOfClauses(), auxvars.getBiggestReturnedAuxVar() - 1000000);
  }
}


void k_Product::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
    formula.addConditionals(pbconstraint.getConditionals());

    if (config->print_used_encodings)
      cout << "c encode with k-product amo" << endl;

    assert(pbconstraint.getLeq() == 1);

    _literals.clear();
    for (int i = 0; i < (int) pbconstraint.getN(); ++i)
      _literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);

    if (pbconstraint.getComparator() == PBLib::BOTH && (pbconstraint.getGeq() == 1))
    {
      assert(pbconstraint.getGeq() == 1 && pbconstraint.getLeq() == 1);
      formula.addClause(_literals);
    }

    encode_intern(_literals, formula, auxvars);

    for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
      formula.getConditionals().pop_back();
}


k_Product::k_Product(PBConfig& config) : Encoder(config), basic_amo_encoder(config), naive_amo_encoder(config)
{
  minimum_lit_count = config->k_product_minimum_lit_count_for_splitting;
  k = config->k_product_k;
  assert(k >= 2);
}

k_Product::~k_Product()
{

}

