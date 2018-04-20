#ifndef INCREMENTALDATA_H
#define INCREMENTALDATA_H

#include <cstdint>
#include "clausedatabase.h"

class IncrementalData
{
private:
    IncrementalData(const IncrementalData& other) = delete;
    virtual bool operator==(const IncrementalData& other) const;
public:
    IncrementalData();
    virtual ~IncrementalData();
    
    virtual void encodeNewGeq(int64_t newGeq, ClauseDatabase & formula, AuxVarManager & auxVars, std::vector< int32_t > conditionals) = 0;
    virtual void encodeNewLeq(int64_t newLeq, ClauseDatabase & formula, AuxVarManager & auxVars, std::vector< int32_t > conditionals) = 0;

};

class IncrementalDontCare : public IncrementalData
{
public:
    virtual void encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
    virtual void encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
    IncrementalDontCare();
    virtual ~IncrementalDontCare();
};

class AMOIncrementalData : public IncrementalData
{
private:
  std::vector<int32_t> geqOneClause;
public:
    virtual void encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
    virtual void encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals);
    AMOIncrementalData (std::vector<int32_t> & geqOneClause);
    virtual ~AMOIncrementalData ();
};

#endif // INCREMENTALDATA_H
