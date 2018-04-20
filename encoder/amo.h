#ifndef AMO_H
#define AMO_H
#include <vector>
#include <sstream>

#include "../SimplePBConstraint.h"
#include "../IncSimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "Encoder.h"

// nested amo encoding ... 
class AMO_Encoder : public Encoder
{
private:
    std::vector<Lit> _literals;
    
    void encodeEq ( const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars );
    int64_t clauseCount(int32_t n);
    int64_t litCount = 0;
    
    
public:
    void encode_intern( std::vector<Lit>& literals, ClauseDatabase & formula, AuxVarManager & auxvars);
    void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
    
    virtual int64_t encodingValue(const SimplePBConstraint& pbconstraint);


    AMO_Encoder(PBConfig & config);
    virtual ~AMO_Encoder();
};

#endif // AMO_H
