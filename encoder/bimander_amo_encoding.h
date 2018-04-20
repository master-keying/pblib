#ifndef BIMANDER_AMO_ENCODING_H
#define BIMANDER_AMO_ENCODING_H

#include <vector>
#include <sstream>

#include "../SimplePBConstraint.h"
#include "../IncSimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "naive_amo_encoder.h"
#include "Encoder.h"

class Bimander_amo_encoding :public Encoder
{
private:
    std::vector<Lit> _literals;
    std::vector<std::vector<Lit> > groups;
    std::vector<Lit> bits;
    int nBits;
    int two_pow_nbits;
    int k;
    int m;
    Naive_amo_encoder naive_amo_encoder;


    void encode_intern( std::vector<Lit>& literals, ClauseDatabase & formula, AuxVarManager & auxvars);    
public:
    void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
    int64_t encodingValue(const SimplePBConstraint& pbconstraint);

    Bimander_amo_encoding(PBConfig & config);
    virtual ~Bimander_amo_encoding();
};

#endif // BINARY_AMO_H
