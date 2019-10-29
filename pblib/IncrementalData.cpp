#include "IncrementalData.h"

using namespace std;

AMOIncrementalData::AMOIncrementalData(vector< int32_t >& geqOneClause) : geqOneClause(geqOneClause)
{

}

void AMOIncrementalData::encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager&, vector< int32_t > conditionals)
{
  formula.addConditionals(conditionals);

  assert(newGeq == 1);
  formula.addClause(geqOneClause);

  for (int i = 0; i < conditionals.size(); ++i)
      formula.getConditionals().pop_back();

}

void AMOIncrementalData::encodeNewLeq(int64_t, ClauseDatabase&, AuxVarManager&, vector<int32_t>)
{
  assert(false && "this should never happen");
}


void IncrementalDontCare::encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager&, vector< int32_t > conditionals)
{
  formula.addConditionals(conditionals);

  if (newGeq > 0)
    formula.addUnsat();

  for (int i = 0; i < conditionals.size(); ++i)
      formula.getConditionals().pop_back();

}

void IncrementalDontCare::encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager&, vector< int32_t > conditionals)
{
  formula.addConditionals(conditionals);

  if (newLeq < 0)
    formula.addUnsat();

  for (int i = 0; i < conditionals.size(); ++i)
      formula.getConditionals().pop_back();
}
