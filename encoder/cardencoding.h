#ifndef CARDENCODING_H
#define CARDENCODING_H

#include "../SimplePBConstraint.h"
#include "../IncSimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "../IncrementalData.h"
#include "Encoder.h"

// Cardinality Networks and their Applications (SAT 2009)
// Roberto As ́ın, Robert Nieuwenhuis, Albert Oliveras, Enric Rodr ́ıguez-Carbonell
class CardEncoding : public Encoder
{  
private:
    class CardIncData : public IncrementalData
    {
      private:
	  std::vector<Lit> outlits;
      public:
	  CardIncData(std::vector<Lit> & outlits);
	  virtual ~CardIncData();
	  virtual void encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
	  virtual void encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
    };
  

  
public:
    void encode(const std::shared_ptr<IncSimplePBConstraint> & pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
    void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
    
    int64_t encodingValue(const SimplePBConstraint& pbconstraint);
    int64_t encodingValue(const std::shared_ptr< IncSimplePBConstraint >& pbconstraint);
        
    CardEncoding (PBConfig & config);
    virtual ~CardEncoding();
};

#endif // CARDENCODING_H
