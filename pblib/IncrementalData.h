#ifndef INCREMENTALDATA_H
#define INCREMENTALDATA_H

#include <cstdint>
#include "clausedatabase.h"

class IncrementalData
{
private:
    IncrementalData(const IncrementalData& other) = delete;
public:
    IncrementalData() = default;
    virtual ~IncrementalData() = default;

    virtual void encodeNewGeq(int64_t newGeq, ClauseDatabase & formula, AuxVarManager & auxVars, std::vector< int32_t > conditionals) = 0;
    virtual void encodeNewLeq(int64_t newLeq, ClauseDatabase & formula, AuxVarManager & auxVars, std::vector< int32_t > conditionals) = 0;

};

class IncrementalDontCare : public IncrementalData
{
public:
    void encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals) override;
    void encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals) override;
    ~IncrementalDontCare() override = default;
};

class AMOIncrementalData : public IncrementalData
{
private:
  std::vector<int32_t> geqOneClause;
public:
    void encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals) override;
    void encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, std::vector< int32_t > conditionals) override;
    AMOIncrementalData (std::vector<int32_t> & geqOneClause);
    ~AMOIncrementalData() override = default;
};

#endif // INCREMENTALDATA_H
