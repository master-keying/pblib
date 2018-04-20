#ifndef PBSATSOLVER_H
#define PBSATSOLVER_H

#include <cstdint>
#include <vector>
class PBSATSolver
{
public:
	virtual bool addClause(std::vector<int32_t> const & clause) = 0;
    virtual bool  addAtMost(std::vector<int32_t> const & literals, int k) = 0;
    virtual bool  addPBLeq(const std::vector< int32_t >& literals, const std::vector< int64_t >& weights, int64_t k, int32_t * index = nullptr) = 0;
};

#endif // PBSATSOLVER_H

