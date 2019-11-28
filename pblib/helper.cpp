#include "helper.h"

#include <iostream>

using namespace std;

uint64_t RandomCounter::callCount = 0;

int RandomCounter::rand()
{
  callCount++;
  return std::rand();
}
