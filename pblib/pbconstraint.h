#ifndef PBCONSTRAINT_H
#define PBCONSTRAINT_H

#include <vector>
#include <assert.h>
#include "weightedlit.h"


namespace PBLib
{
enum Comparator { LEQ, GEQ, BOTH };

class PBConstraint
{
protected:
    int64_t leq;
    int64_t geq;
    std::vector<PBLib::WeightedLit> weighted_literals;
    Comparator comparator;
    std::vector<int32_t> conditionals;

public:
   PBConstraint(std::vector<PBLib::WeightedLit> const & literals, Comparator comparator, int64_t less_eq, int64_t greater_eq);
   PBConstraint(std::vector<PBLib::WeightedLit> const & literals, Comparator comparator, int64_t bound);
   PBConstraint();
   void addConditional(int32_t lit);
   void addConditionals(std::vector<int32_t> lits);
   void clearConditionals();
   std::vector<int32_t> const & getConditionals() const;

    std::vector<PBLib::WeightedLit> & getWeightedLiterals();
    std::vector<PBLib::WeightedLit> const & getWeightedLiterals() const;
    int64_t getLeq() const;
    int64_t getGeq() const;
    virtual ~PBConstraint() = default;
    Comparator getComparator() const;

   PBConstraint getGeqConstraint() const;
   PBConstraint getLeqConstraint() const;

    int64_t getMinSum() const;
    int64_t getMaxSum() const;

    void setLeq(int64_t leq);
    void setGeq(int64_t geq);

    int getN() const;

    void setComparator(Comparator comparator);

    virtual void printGeq(bool std_err = false) const;
    virtual void print(bool std_err = false) const;
    virtual void printNoNL(bool std_err = false) const;
};
}
#endif // PBCONSTRAINT_H
