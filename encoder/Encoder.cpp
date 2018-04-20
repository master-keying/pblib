#include "Encoder.h"

int64_t Encoder::valueFunction(int64_t number_of_clauses, int64_t numer_of_auxvars)
{
  return number_of_clauses;
}


void Encoder::encode(const std::shared_ptr<IncSimplePBConstraint> & pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars)
{
    assert(false && "Incremental encoding is not implemented in this encoder");
}

int64_t Encoder::encodingValue(const std::shared_ptr<IncSimplePBConstraint> & pbconstraint)
{
    return -1;
}

Encoder::Encoder(PBConfig & config) : config(config)
{
}
