#ifndef INCPBCONSTRAINT_H
#define INCPBCONSTRAINT_H

#include <vector>
#include <assert.h>
#include "weightedlit.h"
#include "pbconstraint.h"
#include "clausedatabase.h"
#include "IncSimplePBConstraint.h"
#include "auxvarmanager.h"

class IncPBConstraint
{
private:
    int64_t leq;
    int64_t geq;
    int64_t init_leq;
    int64_t init_geq;
    
    std::vector<PBLib::WeightedLit> weighted_literals;
    PBLib::Comparator comparator;

    virtual bool operator==(const IncPBConstraint& other) const;
    std::shared_ptr<IncSimplePBConstraint> inc_simple_pb_constraint;
    
    bool isDualEncoded;
    std::shared_ptr<IncSimplePBConstraint> leq_inc_simple_pb_constraint;
    std::shared_ptr<IncSimplePBConstraint> geq_inc_simple_pb_constraint;
    
    std::vector<int32_t> conditionals;
    
public:
    IncPBConstraint(std::vector<PBLib::WeightedLit> const & literals, PBLib::Comparator comparator, int64_t less_eq, int64_t greater_eq);
    IncPBConstraint(std::vector<PBLib::WeightedLit> const & literals, PBLib::Comparator comparator, int64_t bound);
    IncPBConstraint();
    virtual ~IncPBConstraint();
    
    void addConditional(int32_t lit);
    void addConditionals(std::vector<int32_t> lits);
    void clearConditionals();
    std::vector<int32_t> const & getConditionals() const;
    
    std::vector<PBLib::WeightedLit> const & getWeightedLiterals() const;
    int64_t getLeq() const;
    int64_t getGeq() const;

    std::shared_ptr< IncSimplePBConstraint > getIncSimplePBConstraint();
    
    void setComparator(PBLib::Comparator comp);
    void setIncSimplePBConstraint(std::shared_ptr< IncSimplePBConstraint > incSimplePBConstraint);
    
    void setLeqIncSimplePBConstraint(std::shared_ptr< IncSimplePBConstraint > incSimplePBConstraint);
    void setGeqIncSimplePBConstraint(std::shared_ptr< IncSimplePBConstraint > incSimplePBConstraint);
    
    void encodeNewGeq(int64_t newGeq, ClauseDatabase & formula, AuxVarManager & auxVars);
    void encodeNewLeq(int64_t newLeq, ClauseDatabase & formula, AuxVarManager & auxVars);
    
    PBLib::PBConstraint getNonIncConstraint();
    
    IncPBConstraint getGeqConstraint() const;
    IncPBConstraint getLeqConstraint() const;
    
    PBLib::Comparator getComparator() const;
    
    int getN() const;
    
    void print() const;
    
};

#endif // INCPBCONSTRAINT_H
