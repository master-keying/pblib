#ifndef ADDERENCODING_H
#define ADDERENCODING_H
#include "../IncSimplePBConstraint.h"
#include "../SimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "../IncrementalData.h"
#include <queue>
#include <tuple>
#include "Encoder.h"

namespace PBLib
{
int ld64(const uint64_t x);
}

class AdderEncoding : public Encoder
{
private:
  
  
  class AdderIncData : public IncrementalData
  {
    private:
      std::vector<int32_t> kBits;
      std::vector<int32_t> result;
      
    public:
      virtual void encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
      virtual void encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
      
      AdderIncData(std::vector<int32_t> result);
      virtual ~AdderIncData();
  };

  

  bool isInc;
  std::vector<int32_t> result;
  
  ClauseDatabase * formula;
  AuxVarManager * auxVars;
  
  std::vector<Lit> clause;
  
  
  
  Lit FA_sum(Lit a, Lit b, Lit c);
  Lit FA_carry(Lit a, Lit b, Lit c);
  Lit HA_sum(Lit a, Lit b);
  Lit HA_carry(Lit a, Lit b);
  void FA_extra( Lit xc, Lit xs, Lit a, Lit b, Lit c );
  
  void adderTree(std::vector<std::queue<Lit>> & buckets, std::vector<Lit> & result);
  
  static void lessThanOrEqual(std::vector< Lit >& xs, std::vector< int32_t >& ys, ClauseDatabase & formula);
//   static void greaterThanOrEqual(std::vector< Lit >& xs, std::vector< int32_t >& ys, ClauseDatabase & formula);
  
  static void numToBits(std::vector<int32_t> & bits, int64_t n, int64_t number);
  void resultIsEqual (std::vector< Lit >& result, std::vector< int32_t >& kBits);
  
public:
  int64_t encodingValue(const SimplePBConstraint& pbconstraint);
  int64_t encodingValue(const std::shared_ptr< IncSimplePBConstraint >& pbconstraint);
  
  void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
  void encode(const std::shared_ptr<IncSimplePBConstraint> & pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
    
  AdderEncoding(PBConfig & config) : Encoder(config), isInc(false) {};
};

#endif // ADDERENCODING_H
