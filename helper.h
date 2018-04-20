#ifndef HELPER_H
#define HELPER_H

#include <vector>
#include <cstdint>
#include <ostream>

#include "weightedlit.h"

namespace std
{
  template<typename a, typename b>
  struct hash< std::pair<a, b> >
  {
  private:
    const hash<a> ah;
    const hash<b> bh;
  public:
    hash() : ah(), bh() {}
    size_t operator()(const std::pair<a, b> &p) const {
	return ah(p.first) ^ bh(p.second);
  }
  };
};

/// print WeightedLit
template <typename T>
inline std::ostream& operator<<(std::ostream& other, const PBLib::WeightedLit & lit )
{
  if (lit.weight < 0)
    other << lit.weight << " ~" << lit.lit;
  else
    other << lit.weight << " " << lit.lit;
  return other;
}

/// print elements of a vector
template <typename T>
inline std::ostream& operator<<(std::ostream& other, const std::vector<T>& data )
{
  for( int i = 0 ; i < data.size(); ++ i )
    other << " " << data[i];
  return other;
}


class RandomCounter
{  
public:
  static uint64_t callCount;
  static int rand();
};

namespace helper
{
  void printClause(std::vector<int32_t> & clause);
  
  
}


#endif 