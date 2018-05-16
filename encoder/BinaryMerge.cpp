#include "BinaryMerge.h"
#include "../helper.h"
#include <math.h>
#include "../preencoder.h"

#include "adderencoding.h"
#include "sorting_merging.h"

using namespace std;
using namespace PBLib;


int64_t BinaryMerge::encodingValue(const SimplePBConstraint& constraint)
{
  if (constraint.getComparator() == BOTH)
    return -1; // TODO implemnt BOTH encoding for BinaryMerge

  // just a approximation // TODO FIXME add calculation here
  int64_t num_of_bin_merge_clauses = (constraint.getComparator() == BOTH ? 2 : 1) * constraint.getN() * ceil(log2(constraint.getLeq())) * ceil(log2(constraint.getLeq())) * ceil(log2(constraint.getMaxWeight()));
  return valueFunction(num_of_bin_merge_clauses, num_of_bin_merge_clauses);
}


void BinaryMerge::binary_merge(const SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxvars, int32_t gac_lit)
{
  int64_t less_then = constraint.getLeq()+1;
  int64_t p = floor(log2(constraint.getMaxWeight()));
  int64_t m = ceil((double) (less_then) / (double) (pow(2,p))); // m is less (and not equal)
  int64_t new_less_thean = m * pow(2,p);
  int64_t T = (m * pow(2,p)) - (less_then);
  true_lit = auxvars.getVariable();

  formula.addClause(true_lit);

  vector<vector<int32_t> > buckets;

  int64_t bit = 1;

  // fill the bit buckets
  for (int64_t i = 0; i <= p; ++i)
  {
    buckets.push_back(vector<int32_t>());
    if ((T & bit) != 0)
      buckets[i].push_back(true_lit);

    for (int64_t j = 0; j < constraint.getN(); ++j)
    {
      if ((constraint.getWeightedLiterals()[j].weight & bit) != 0)
      {
	if (gac_lit != 0 && constraint.getWeightedLiterals()[j].weight >= less_then)
	{
	  formula.addConditionals(constraint.getConditionals());
	  formula.addClause(gac_lit, -constraint.getWeightedLiterals()[j].lit);
	  for (int i = 0; i < constraint.getConditionals().size(); ++i)
	    formula.getConditionals().pop_back();
	}
	else
	  buckets[i].push_back(constraint.getWeightedLiterals()[j].lit);
      }
    }
    bit = bit << 1;
  }

  vector<vector<int32_t> > bucket_card(p+1); assert(bucket_card.size() == buckets.size());
  vector<vector<int32_t> > bucket_merge(p+1);

  vector<int32_t> carries;
  for (int i = 0; i < buckets.size(); ++i)
  {
    int64_t k = ceil((double)new_less_thean / (double)pow(2,i)); // we need only k output bits to detect inconsistency

    //TODO
    if (config->use_watch_dog_encoding_in_binary_merger)
      totalizer(buckets[i], bucket_card[i], formula, auxvars);
    else
      Sorting::sort(k, buckets[i], formula, auxvars, bucket_card[i]);

    if (k <= buckets[i].size())
    {
      assert(k == bucket_card[i].size() || config->use_watch_dog_encoding_in_binary_merger);
      if ( gac_lit != 0)
      {
	formula.addConditionals(constraint.getConditionals());
	formula.addClause(gac_lit, -bucket_card[i][k-1]);
	for (int i = 0; i < constraint.getConditionals().size(); ++i)
	  formula.getConditionals().pop_back();
      }
      else
      {
	formula.addConditionals(constraint.getConditionals());
	formula.addClause(-bucket_card[i][k-1]);
	for (int i = 0; i < constraint.getConditionals().size(); ++i)
	  formula.getConditionals().pop_back();

      }
    }

    if (i > 0)
    {
      if (carries.size() > 0)
      {
	if (bucket_card[i].size() == 0)
	{
	  bucket_merge[i] = carries;
	}
	else
	{
	  if (config->use_watch_dog_encoding_in_binary_merger)
	    unary_adder(bucket_card[i], carries, bucket_merge[i], formula, auxvars);
	  else
	    Sorting::merge(k, bucket_card[i], carries, formula, auxvars, bucket_merge[i]);

	  if (k == bucket_merge[i].size() || (config->use_watch_dog_encoding_in_binary_merger && k <= bucket_merge[i].size()) )
	  {
	    if ( gac_lit != 0)
	    {
	      formula.addConditionals(constraint.getConditionals());
	      formula.addClause(gac_lit,-bucket_merge[i][k-1]);
	      for (int i = 0; i < constraint.getConditionals().size(); ++i)
		formula.getConditionals().pop_back();
	    }
	    else
	    {
	      formula.addConditionals(constraint.getConditionals());
	      formula.addClause(-bucket_merge[i][k-1]);
	      for (int i = 0; i < constraint.getConditionals().size(); ++i)
		formula.getConditionals().pop_back();
	    }
	  }
	}
      }
      else
      {
	bucket_merge[i] = bucket_card[i];
      }
    }

    carries.clear();
    if (i == 0)
    {
      for(int j = 1; j < bucket_card[0].size(); j = j + 2)
	carries.push_back(bucket_card[0][j]);
    }
    else
    {
      for(int j = 1; j < bucket_merge[i].size(); j = j + 2)
	carries.push_back(bucket_merge[i][j]);
    }
  }
}




void BinaryMerge::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  if (config->print_used_encodings)
      cout << "c encode with binary merge encoding" << endl;

  assert(pbconstraint.getComparator() == LEQ);

  if (!config->use_gac_binary_merge)
    binary_merge(pbconstraint, formula, auxvars); // just check concistency with unit propagation
  else
  {
    // gac
    int32_t x;
    bool encode_complete_constrainte = false;
    vector<WeightedLit> lits(pbconstraint.getWeightedLiterals());
    for (int i = 0; i < lits.size(); ++i)
    {
      if (config->binary_merge_no_support_for_single_bits && floor(log2(lits[i].weight)) == log2(lits[i].weight))
      {
	encode_complete_constrainte = true;
	continue;
      }

      WeightedLit tmp(lits[i].lit, lits[i].weight); // copy
      lits[i] = lits.back(); // and delete current lit
      lits.pop_back(); //

      x = tmp.lit;

      if (pbconstraint.getMaxWeight() == tmp.weight)
      {
	int64_t maxWeight = 0;
	for (int j = 0; j < lits.size(); ++j)
	{
	    maxWeight = max(maxWeight, lits[j].weight);
	}
	SimplePBConstraint constraint(pbconstraint.getMaxSum() - tmp.weight, maxWeight, PB, lits, LEQ, pbconstraint.getLeq() - tmp.weight);
	constraint.addConditionals(pbconstraint.getConditionals());
	if (constraint.getLeq() <= 0)
	{
	  for (int j = 0; j < lits.size(); ++j)
	  {
	    formula.addConditionals(constraint.getConditionals());
	    formula.addClause(-x, -lits[j].lit);
	    for (int i = 0; i < constraint.getConditionals().size(); ++i)
	      formula.getConditionals().pop_back();

	  }
	}
	else
	  binary_merge(constraint, formula, auxvars, -x);
      }
      else
      {
	SimplePBConstraint constraint(pbconstraint.getMaxSum() - tmp.weight, pbconstraint.getMaxWeight(), pbconstraint.getType(), lits, LEQ, pbconstraint.getLeq() - tmp.weight);
	constraint.addConditionals(pbconstraint.getConditionals());
	if (constraint.getLeq() <= 0)
	{
	  for (int j = 0; j < lits.size(); ++j)
	  {
	    formula.addConditionals(constraint.getConditionals());
	    formula.addClause(-x, -lits[j].lit);
	    for (int i = 0; i < constraint.getConditionals().size(); ++i)
	      formula.getConditionals().pop_back();
	  }
	}
	  binary_merge(constraint, formula, auxvars, -x);
      }

      if (i < lits.size())
      {
	lits.push_back(lits[i]);
	lits[i] = tmp;
      }
    }

    if (config->binary_merge_no_support_for_single_bits && encode_complete_constrainte)
      binary_merge(pbconstraint, formula, auxvars);
  }
}

BinaryMerge::BinaryMerge(PBConfig& config) : Encoder(config), old_card_encoder(config)
{

}


void BinaryMerge::totalizer(vector< int32_t > const & x, vector< int32_t > & u_x, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  u_x.clear();
  if (x.size() == 0)
    return;

  if (x.size() == 1)
  {
    u_x.push_back(x[0]);
  }
  else
  {
    for (int i = 0; i < x.size(); ++i)
      u_x.push_back(auxvars.getVariable());

    vector<int32_t> x_1;
    vector<int32_t> x_2;
    int i = 0;
    for (;  i < x.size() / 2; ++i)
      x_1.push_back(x[i]);
    for (;  i < x.size(); ++i)
      x_2.push_back(x[i]);


    vector<int32_t> u_x_1;
    vector<int32_t> u_x_2;
    totalizer(x_1, u_x_1, formula, auxvars);
    totalizer(x_2, u_x_2, formula, auxvars);

    unary_adder(u_x_1, u_x_2, u_x, formula, auxvars);
  }
}

void BinaryMerge::unary_adder(vector< int32_t > const & u, vector< int32_t > const & v, vector< int32_t > & w, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  w.clear();
  if (u.size() == 0)
  {
    for (int i = 0; i < v.size(); ++i)
      w.push_back(v[i]);
  }
  else if (v.size() == 0)
  {
    for (int i = 0; i < u.size(); ++i)
      w.push_back(u[i]);
  }
  else
  {
    for (int i = 0; i < u.size() + v.size(); ++i)
      w.push_back(auxvars.getVariable());

    for (int a=0; a < u.size(); ++a)
    {
      for (int b=0; b < v.size(); ++b)
      {
	formula.addClause(-u[a], -v[b], w[a+b+1]);
      }
    }

    for (int i=0; i < v.size(); ++i)
    {
      formula.addClause(-v[i], w[i]);
    }

    for (int i=0; i < u.size(); ++i)
    {
      formula.addClause(-u[i], w[i]);
    }
  }
}





















