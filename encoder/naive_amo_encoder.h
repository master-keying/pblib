/*
 *
 */

#ifndef NAIVE_AMO_ENCODER_H
#define NAIVE_AMO_ENCODER_H

#include <vector>
#include <sstream>

#include "../SimplePBConstraint.h"
#include "../IncSimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "Encoder.h"

class Naive_amo_encoder : public Encoder
{
private:
    std::vector<Lit> _literals;

public:
    void encode_intern( std::vector<Lit>& literals, ClauseDatabase & formula);
    void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars);
    int64_t encodingValue(const SimplePBConstraint& pbconstraint);

    Naive_amo_encoder(PBConfig & config);
    virtual ~Naive_amo_encoder();
};

#endif // COMMANDER_ENCODING_H
