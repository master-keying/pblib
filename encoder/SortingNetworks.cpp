#include "SortingNetworks.h"

using namespace std;

void SortingNetworks::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  if (config->print_used_encodings)
      cout << "c encode with sorting networks" << endl;
 
  
  formula.addConditionals(pbconstraint.getConditionals());
  
  formula.addFormula(buildConstraint(pbconstraint), auxvars);
  
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}



SortingNetworks::SortingNetworks(PBConfig& config) : Encoder(config)
{

}

int64_t SortingNetworks::encodingValue(const SimplePBConstraint& pbconstraint)
{
  return config->MAX_CLAUSES_PER_CONSTRAINT - 1; // since sorting networks from minisat+ are not GAC we use this as fallback only
}
