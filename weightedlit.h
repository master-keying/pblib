#ifndef WEIGHTED_VAR_H_
#define WEIGHTED_VAR_H_

#include <cstdint>
#include <cstdlib>


namespace PBLib
{
class WeightedLit {
	public:
	int32_t lit; int64_t weight;
	WeightedLit(int32_t lit, int64_t weight): lit(lit), weight(weight) {}
	
	static bool compVariable_asc(const WeightedLit & lhs, const WeightedLit & rhs)
	{
	  return lhs.weight < rhs.weight;
	}
	
	static bool compVariable_des(const WeightedLit & lhs, const WeightedLit & rhs)
	{
	  return lhs.weight > rhs.weight;
	}
	
	static bool compVariable_des_var(const WeightedLit & lhs, const WeightedLit & rhs)
	{
	  return abs(lhs.lit) > abs(rhs.lit);
	}
	
	bool operator< (WeightedLit const & rhs)
	{
	  return lit < rhs.lit;
	}
	
	virtual ~WeightedLit() {};
};
}
#endif
