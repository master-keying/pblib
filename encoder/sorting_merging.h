#ifndef SORTING_MERGING_H
#define SORTING_MERGING_H

#include "../SimplePBConstraint.h"
#include "../IncSimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "../IncrementalData.h"
#include "../helper.h"

#include <map>
#include <tuple>

// based on the publication of:
// A Parametric Approach for Smaller and Better Encodings of Cardinality Constraints
// http://dx.doi.org/10.1007/978-3-642-40627-0_9
//A Abío, Ignasi
//A Nieuwenhuis, Robert
//A Oliveras, Albert
//A Rodríguez-Carbonell, Enric
//P 80-96
//G English	
//B Principles and Practice of Constraint Programming
//V 8124
//S Lecture Notes in Computer Science
//E Schulte, Christian
// Springer Berlin Heidelberg
// 2013-01-01
//R 10.1007/978-3-642-40627-0_9

namespace PBLib
{
  class Sorting 
  {
    public:
      enum ImplicationDirection {INPUT_TO_OUTPUT, OUTPUT_TO_INPUT, BOTH};
    private:      
      static std::vector<std::vector<int32_t> > s_auxs;
      
      static std::map< std::pair<int32_t,int32_t>, int64_t> recursive_sorter_values;
      static std::map< std::tuple<int32_t,int32_t,int32_t>, int64_t> recursive_sorter_l_values;
      static std::map< std::tuple<int32_t,int32_t,int32_t>, int64_t> recursive_merger_values;
    
      static void counter_sorter(int m, std::vector<int32_t> const & input, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction);
      static void direct_sorter(int m, std::vector<int32_t> const & input, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction);
      static void recursive_sorter(int m, std::vector<int32_t> const & input, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction);
      static void recursive_sorter(int m, int l, std::vector<int32_t> const & input, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction);
      
      static void direct_merger(int m, std::vector<int32_t> const & input_a, std::vector<int32_t> const & input_b, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction);
      static void recursive_merger(int m, std::vector<int32_t> const & input_a, int a, std::vector<int32_t> const & input_b, int b, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction);
      
      static int64_t counter_sorter_value(int m, int n, ImplicationDirection direction);
      static int64_t direct_sorter_value(int m, int n, ImplicationDirection direction);
      static int64_t recursive_sorter_value(int m, int n, ImplicationDirection direction);
      static int64_t recursive_sorter_value(int m, int n, int l, ImplicationDirection direction);
      
      static int64_t direct_merger_value(int m, int a, int b, ImplicationDirection direction);
      static int64_t recursive_merger_value(int m, int a, int b, ImplicationDirection direction);
      
      static int64_t value_function(int num_clauses, int num_variables);
      
      static inline void comparator(const int32_t x1, const int32_t x2, const int32_t y1, const int32_t y2, ClauseDatabase & formula, ImplicationDirection direction);
      static inline void comparator(const int32_t x1, const int32_t x2, const int32_t y1, ClauseDatabase & formula, ImplicationDirection direction);
      
    public:
      static void sort(int m, std::vector<int32_t> const & input, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction = INPUT_TO_OUTPUT);
      static void merge(int m, std::vector<int32_t> const & input_a, std::vector<int32_t> const & input_b, ClauseDatabase & formula, AuxVarManager & auxvars, std::vector<int32_t> & output, ImplicationDirection direction = INPUT_TO_OUTPUT);
  };
}



#endif // SORTING_MERGING_H
