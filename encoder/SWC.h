#ifndef SWC_ENCODER_H
#define SWC_ENCODER_H
#include <vector>
#include <sstream>

#include "../SimplePBConstraint.h"
#include "../IncSimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "Encoder.h"

// A Compact Encoding of Pseudo-Boolean Constraints into SAT. 
// Steffen Hölldobler, Norbert Manthey, and Peter Steinke.
// KI 2012

class SWC_Encoder : public Encoder
{
private:
    class SWCIncData : public IncrementalData
    {
    private:
      std::vector<int32_t> outlits;
    public:
      SWCIncData(std::vector<int32_t> & outlits);
      ~SWCIncData();
      virtual void encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
      virtual void encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
    };
  
    std::vector<int32_t> outlits;
    bool isInc = false;
    
    void encode_intern(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars, bool encodeComplete = false);
    
public:
    void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
    int64_t encodingValue(const SimplePBConstraint& pbconstraint);
    
    void encode(const std::shared_ptr< IncSimplePBConstraint >& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars);
    int64_t encodingValue(const std::shared_ptr< IncSimplePBConstraint >& pbconstraint);
    
    SWC_Encoder(PBConfig config);
    virtual ~SWC_Encoder();

};

#endif // SWC_ENCODER_H
