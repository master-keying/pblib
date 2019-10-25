#include "Encoder.h"

int64_t Encoder::valueFunction(int64_t number_of_clauses, int64_t)
{
  return number_of_clauses;
}


void Encoder::encode(const std::shared_ptr<IncSimplePBConstraint>&, ClauseDatabase&, AuxVarManager&)
{
    assert(false && "Incremental encoding is not implemented in this encoder");
}

int64_t Encoder::encodingValue(const std::shared_ptr<IncSimplePBConstraint>&)
{
    return -1;
}

Encoder::Encoder(PBConfig & config) : config(config)
{
}
