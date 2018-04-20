#include "cardencoding.h"
#include "sorting_merging.h"

using namespace PBLib;
using namespace std;
CardEncoding::CardIncData::~CardIncData()
{
}


CardEncoding::CardIncData::CardIncData(vector< Lit >& outlits) : outlits(outlits) {}

void CardEncoding::CardIncData::encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, vector< int32_t > conditionals)
{
  formula.addConditionals(conditionals);
  
  if (outlits.size() < newGeq)
    formula.addUnsat();
  else
  if (newGeq > 0)
    formula.addClause(outlits[newGeq - 1]); // setting newGeq to true forcing the sum to be at least equal newGeq
    
  for (int i = 0; i < conditionals.size(); ++i)
    formula.getConditionals().pop_back();
}


void CardEncoding::CardIncData::encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, vector< int32_t > conditionals)
{
  formula.addConditionals(conditionals);
  
  if (outlits.size() > newLeq)
    formula.addClause(-outlits[newLeq]); // setting newLeq + 1 to false forcing the sum to be less equal newLeq
    
  for (int i = 0; i < conditionals.size(); ++i)
    formula.getConditionals().pop_back();
  
}

int64_t CardEncoding::encodingValue(const shared_ptr< IncSimplePBConstraint >& pbconstraint)
{
  int n = pbconstraint->getN();
  int64_t num_clauses_approx = (n * pow(ceil(log2(n)),2));
  
  return valueFunction(num_clauses_approx, num_clauses_approx);
}

int64_t CardEncoding::encodingValue(const SimplePBConstraint& pbconstraint)
{
  int n = pbconstraint.getN();
  int64_t num_clauses_approx = (n * pow(ceil(log2(n)),2));
  
  if (num_clauses_approx > config->MAX_CLAUSES_PER_CONSTRAINT)
    return valueFunction(num_clauses_approx, num_clauses_approx);
  
  CountingClauseDatabase formula(config);
  AuxVarManager auxvars(1000000);
  encode(pbconstraint, formula, auxvars);
  
  return valueFunction(formula.getNumberOfClauses(), auxvars.getBiggestReturnedAuxVar() - 1000000);
}



void CardEncoding::encode(const shared_ptr<IncSimplePBConstraint> & pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  if (config->print_used_encodings)
    cout << "c encode incremental with card" << endl;
  
  vector<int32_t> input, output;
  int64_t leq = pbconstraint->getLeq();
  
  for (auto l : pbconstraint->getWeightedLiterals())
    input.push_back(l.lit);  
  
  if (pbconstraint->getComparator() == BOTH)
    Sorting::sort(leq+1, input, formula, auxvars, output, Sorting::BOTH);
  else
    Sorting::sort(leq+1, input, formula, auxvars, output, Sorting::INPUT_TO_OUTPUT);
  
  
  
  formula.addConditionals(pbconstraint->getConditionals());
  
  if (output.size() > leq) // if not, the constraint is always satisfiable (initially)
    formula.addClause(-output[leq]); // setting k + 1 to false forcing the sum to be less equal k
  
  
  if (pbconstraint->getComparator() == BOTH)
  {
    if (output.size() < pbconstraint->getGeq())
      formula.addUnsat();
    else
    for (int i = 0; i < pbconstraint->getGeq(); ++i)
      formula.addClause(output[i]);
  }
  
  for (int i = 0; i < pbconstraint->getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
  
  pbconstraint->setIncrementalData(make_shared<CardIncData>(output));
}


void CardEncoding::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars)
{
  
  if (config->print_used_encodings)
    cout << "c encode with card" << endl;
  
  vector<int32_t> input, output;
  int64_t leq = pbconstraint.getLeq();
  
  if (pbconstraint.getComparator() == LEQ && leq > (pbconstraint.getN() / 2) )
  {
    int64_t geq = pbconstraint.getN() - leq; // we treat this as geq constraint
    
    for (auto l : pbconstraint.getWeightedLiterals())
      input.push_back(-l.lit);  // negate everything
    
      
    Sorting::sort(geq, input, formula, auxvars, output, Sorting::OUTPUT_TO_INPUT);
    
    formula.addConditionals(pbconstraint.getConditionals());

    for (int i = 0; i < geq; ++i)
      formula.addClause(output[i]);
      
    for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
      formula.getConditionals().pop_back();
    
    return;
  }
  
  for (auto l : pbconstraint.getWeightedLiterals())
    input.push_back(l.lit);  
  
  if (pbconstraint.getComparator() == BOTH)
    Sorting::sort(leq+1, input, formula, auxvars, output, Sorting::BOTH);
  else
    Sorting::sort(leq+1, input, formula, auxvars, output, Sorting::INPUT_TO_OUTPUT);
  
  
  assert(output.size() > leq);
  
  formula.addConditionals(pbconstraint.getConditionals());
  
  formula.addClause(-output[leq]); // setting k + 1 to false forcing the sum to be less equal k
  
  if (pbconstraint.getComparator() == BOTH)
  {
    for (int i = 0; i < pbconstraint.getGeq(); ++i)
      formula.addClause(output[i]);
  }
  
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}


CardEncoding::CardEncoding(PBConfig& config) : Encoder(config)
{

}

CardEncoding::~CardEncoding()
{

}


