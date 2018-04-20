#ifndef PBFUZZER_H
#define PBFUZZER_H

#include "pbconstraint.h"
#include "weightedlit.h"
#include "PBConfig.h"
#include <vector>
#include <cstdint>

class PBFuzzer
{

std::vector<PBLib::WeightedLit> literals;

void scramble(std::vector<PBLib::PBConstraint> & constraints);

bool doScramble = true;


public:
  PBLib::PBConstraint generatePBConstraint();
  PBLib::PBConstraint generatePBProblem(std::vector< PBLib::PBConstraint> & constraints);
  int pAMO = 10;
  int pAMK = 20;
  int pBOTH = 100;
  int pBIG = 5;
  bool constant_constraint_length = false;
    
  int number_of_constraints = 10;
  int numer_of_variables = 80;

  int max_constraint_size = 10;
  int max_big_constraint_size = 20;
  int max_weight_size = 100;
  int64_t max_big_weight_size = 35184372088832;  // max_weight_size * max_opt_constraint_size <= 100000000000000000 // 17 digites
  int max_opt_constraint_size = 10;
  int polarity_ratio = 50;
  int max_trivial_constraints = 1;
    PBFuzzer();
    
    virtual ~PBFuzzer();
    
    // copy constraints! DO NOT USE A REFERENCE HERE
    void writeToPBFile(PBLib::PBConstraint opt_constraint, std::vector< PBLib::PBConstraint > constraints, int numVars, std::string file);
};

#endif // PBFUZZER_H
