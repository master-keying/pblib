#ifndef INCSIMPLEPBCONSTRAINT_H
#define INCSIMPLEPBCONSTRAINT_H

#include "SimplePBConstraint.h"
#include "IncrementalData.h"


class IncSimplePBConstraint : public SimplePBConstraint
{
private:
    virtual bool operator==(const IncSimplePBConstraint& other) const = delete;
    int64_t normalized_offset;
    int64_t init_leq;
    int64_t init_geq;
    std::shared_ptr<IncrementalData> incremental_data;
public:
    virtual ~IncSimplePBConstraint();
    IncSimplePBConstraint(int64_t max_sum, int64_t max_weight, int64_t normalizedOffset, PBTYPE type, std::vector<PBLib::WeightedLit>& literals, PBLib::Comparator comparator, int64_t less_eq, int64_t greater_eq);
    IncSimplePBConstraint(int64_t max_sum, int64_t max_weight, int64_t normalizedOffset, PBTYPE type, std::vector<PBLib::WeightedLit>& literals, PBLib::Comparator comparator, int64_t bound);

    void setIncrementalData(std::shared_ptr<IncrementalData>  incremental_data);
    
    void encodeNewGeq(int64_t newGeq, ClauseDatabase & formula, AuxVarManager & auxVars);
    void encodeNewLeq(int64_t newLeq, ClauseDatabase & formula, AuxVarManager & auxVars);
};

#endif // INCSIMPLEPBCONSTRAINT_H
