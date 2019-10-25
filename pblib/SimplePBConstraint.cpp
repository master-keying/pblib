#include "SimplePBConstraint.h"

#include <iostream>

using namespace PBLib;
using namespace std;

PBTYPE SimplePBConstraint::getType() const
{
  return type;
}


SimplePBConstraint::SimplePBConstraint(int64_t max_sum, int64_t max_weight, PBTYPE type, vector< WeightedLit > const & literals, Comparator comparator, int64_t less_eq, int64_t greater_eq) : PBConstraint(literals, comparator, less_eq, greater_eq), type(type), max_sum(max_sum), max_weight(max_weight)
{
  assert(comparator == BOTH);
  assert(max_weight <= max_sum);
}

SimplePBConstraint::SimplePBConstraint(int64_t max_sum, int64_t max_weight, PBTYPE type, vector< WeightedLit > const & literals, Comparator comparator, int64_t bound) : PBConstraint(literals, comparator, bound), type(type), max_sum(max_sum), max_weight(max_weight)
{
  assert(comparator == LEQ);
  assert(max_weight <= max_sum);
}

void SimplePBConstraint::printNoNL(bool std_err) const
{
  using namespace std;
  if (getType() == DONTCARE)
  {
    if(std_err)
      cerr << "DONTCARE" << " ";
    else
      cout << "DONTCARE" << " ";
  }
  else
    PBConstraint::printNoNL(std_err);
}

void SimplePBConstraint::print(bool std_err) const
{
  using namespace std;
  if (getType() == DONTCARE)
  {
    if(std_err)
      cerr << "DONTCARE" << endl;
    else
      cout << "DONTCARE" << endl;
  }
  else
    PBConstraint::print(std_err);
}


bool SimplePBConstraint::operator==(const SimplePBConstraint&) const
{
  return false;
}

int64_t SimplePBConstraint::getMaxSum() const
{
  return max_sum;
}

int64_t SimplePBConstraint::getMaxWeight() const
{
  return max_weight;
}
