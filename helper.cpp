#include "helper.h"

#include <iostream>

using namespace std;

void helper::printClause(std::vector< int32_t > & clause)
{
  cout << "[";
  if (clause.size() != 0)
    cout << clause[0];
  for (int i = 1; i < clause.size(); ++i)
  {
    cout << " ," << clause[i];
  }    
  cout << "]" << endl;
}


uint64_t RandomCounter::callCount = 0;


int RandomCounter::rand()
{
  callCount++;
  return std::rand();
}
