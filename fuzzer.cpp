#include <iostream>
#include <assert.h>
#include <stdlib.h> 
#include <sys/time.h>  

#include "pb2cnf.h"
#include "PBFuzzer.h"
#include "helper.h"

#include "BasicPBSolver/BasicSATSolver.h"
#include "BasicPBSolver/SATSolverClauseDatabase.h"


using namespace std;
using namespace PBLib;

const int test_rounds = 100000;

bool printLess = true;

void testNormalConstraints(PBConfig config, PBFuzzer & fuzzer, int64_t rounds = -1);
void testConditionalConstraints(PBConfig config, PBFuzzer & fuzzer, int64_t rounds = -1);
void testIncrementalConstraints(PBConfig config, PBFuzzer & fuzzer, int64_t rounds = -1);
void testConditionalIncrementalConstraints(PBConfig config, PBFuzzer & fuzzer, int64_t rounds = -1);

int main(int argc, char **argv)
{
  unordered_map<string,string> options;
  
  if (argc > 1)
  {
    string tmp;
    for (int i = 1; i < argc; ++i)
    {
	tmp = string(argv[i]);
	while (tmp[0] == '-')
	    tmp = tmp.substr(1);	

	options[tmp.substr(0,tmp.find("="))] = tmp.find("=") == string::npos ? "" : tmp.substr(tmp.find("=") + 1);
    }
  }
  
  
  struct timeval time; 
  gettimeofday(&time,NULL);
  unsigned int seed = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    
  if (options.find("seed") != options.end())
    seed = (unsigned int) atol(options["seed"].c_str());
  
  cout << "c seed: " << seed << endl;
  srand(seed);
  
  if (options.find("randCalls") != options.end())
  {
    int64_t randCalls = atol(options["randCalls"].c_str());
    for (int i = 0; i < randCalls; ++i)
      rand();
    
    RandomCounter::callCount = randCalls;
  }
  
  vector<PBConfig> configs;
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::COMMANDER;
    config->config_name = "amo commander";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::BDD;
    config->config_name = "amo bdd";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::BEST;
    config->config_name = "amo best";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::BIMANDER;
    config->config_name = "amo bimander";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::BINARY;
    config->config_name = "amo binary";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::KPRODUCT;
    config->config_name = "amo kproduct";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::NESTED;
    config->config_name = "amo nested";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amo_encoder = AMO_ENCODER::PAIRWISE;
    config->config_name = "amo pairwaise";
    
    configs.push_back(config);
  }
  
  // AMK
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amk_encoder = AMK_ENCODER::BDD;
    config->config_name = "amk bdd";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amk_encoder = AMK_ENCODER::BEST;
    config->config_name = "amk best";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->amk_encoder = AMK_ENCODER::CARD;
    config->config_name = "amk card";
    
    configs.push_back(config);
  }
  
  // PB
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::ADDER;
    config->config_name = "pb adder";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::BDD;
    config->config_name = "pb bdd";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::BEST;
    config->config_name = "pb best";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::BINARY_MERGE;
    config->use_gac_binary_merge = true;
    config->config_name = "pb binary merge gac";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::BINARY_MERGE;
    config->use_gac_binary_merge = false;
    config->config_name = "pb binary merge non-gac";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::SORTINGNETWORKS;
    config->config_name = "pb sorting networks";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::BINARY_MERGE;
    config->use_watch_dog_encoding_in_binary_merger = true;
    config->config_name = "pb watchdog";
    
    configs.push_back(config);
  }
  
  {
    PBConfig config = make_shared<PBConfigClass>();
    config->pb_encoder = PB_ENCODER::SWC;
    config->config_name = "pb swc";
    
    configs.push_back(config);
  }
  
  
  
  PBFuzzer fuzzer;
  
  fuzzer.pAMO = 10;
  fuzzer.pAMK = 10;
  fuzzer.pBIG = 0;
  fuzzer.pBOTH = 20;
  fuzzer.numer_of_variables = 10;
  fuzzer.max_constraint_size = 10;
  
  int64_t  count = 0;
  int64_t rounds = -1;
   
  while (rounds < 0 ? true : (count < rounds) )
  {
    count++; 
    for (auto config : configs)
    {
//       config->print_used_encodings=true;
//       if (config->config_name != "pb watchdog")
// 	continue;
      
      cout << "current config: " << config->config_name << endl;

      cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;
      testNormalConstraints(config, fuzzer,test_rounds);
      cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;
      testConditionalConstraints(config, fuzzer, test_rounds);
      cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;
      testIncrementalConstraints(config, fuzzer, test_rounds);
      cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;
      testConditionalIncrementalConstraints(config, fuzzer, test_rounds);
    }
    
  }
  cout << endl;
}

void testNormalConstraints(PBConfig config, PBFuzzer& fuzzer, int64_t rounds)
{
  if (!printLess) cout << "testing normal constraints                                      " << endl;
  
  
  double numSAT = 0;
  double numUNSAT = 0;

  PB2CNF pb2cnf(config);

    
   int count = 0;
  while (rounds < 0 ? true : (count < rounds) )
  {
//     cout << "new round " << endl;
    count++; //cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;
    AuxVarManager auxvars(fuzzer.numer_of_variables + 1);
    BasicSATSolver satsolver;
    SATSolverClauseDatabase formula(config, &satsolver);
    
    vector<bool> assignment(fuzzer.numer_of_variables+1);
    for (int i = 1; i <= fuzzer.numer_of_variables; ++i)
    {
      if (RandomCounter::rand() % 2 == 0)
      {
	assignment[i] = false;
	formula.addClause(-i);
// 	cout << -i << endl;
      }
      else
      {
	assignment[i] = true;
	formula.addClause(i);
// 	cout << i << endl;
      }
    }
    
    PBConstraint constraint = fuzzer.generatePBConstraint();
    
    pb2cnf.encode(constraint, formula, auxvars);
    
    
    
    int64_t sum = 0;
    for (auto wlit : constraint.getWeightedLiterals())
    {

      if (wlit.lit < 0 ? !assignment[-wlit.lit] : assignment[wlit.lit] )
	sum += wlit.weight;
    }
    
    bool haveToResult = false;
    
    if(constraint.getComparator() == LEQ)
      haveToResult = (sum <= constraint.getLeq());
    else
    if(constraint.getComparator() == GEQ)
      haveToResult = (sum >= constraint.getGeq());
    else 
      haveToResult = (sum <= constraint.getLeq()) && (sum >= constraint.getGeq());
    
    bool isResult = satsolver.solve();
    
    if (isResult)
      numSAT++;
    else
      numUNSAT++;

    if (isResult == haveToResult)
      ;
    else
    {
      if (isResult)
	cout << "was SAT but have to be UNSAT" << endl;
      else
	cout << "was UNSAT but have to be SAT" << endl;
      
      cout << "not ok" << endl;
      constraint.print();
      exit(-1);
    }
    
//     if ( ((int64_t)numSAT + (int64_t)numUNSAT) % 100 == 0)
//     {
//       cout.precision(5);
// 	
//       if ((numSAT + numUNSAT) != 0)
// 	cout << "SAT / UNSAT = " << numSAT / (numSAT + numUNSAT) << " SAT " << numSAT  << " UNSAT " << numUNSAT << " total: " << (int64_t)numSAT + (int64_t)numUNSAT << "     \r";
//       cout << flush;
//     }
  }
}

void testConditionalConstraints(PBConfig config, PBFuzzer& fuzzer, int64_t rounds)
{
  if (!printLess) cout << "testing conditional constraints                                      " << endl;
  
  
  double numSAT = 0;
  double numUNSAT = 0;

  PB2CNF pb2cnf(config);

    
   int count = 0;
  while (rounds < 0 ? true : (count < rounds) )
  {
    count++; // cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;
    
    int conditionalCount = RandomCounter::rand() % 3+1;
    AuxVarManager auxvars(fuzzer.numer_of_variables + conditionalCount + 1);
    BasicSATSolver satsolver;
    SATSolverClauseDatabase formula(config, &satsolver);
    
    
    
    vector<int32_t> conditionals;
    
    for (int i = 0; i < conditionalCount; ++i)
    {
      conditionals.push_back( (fuzzer.numer_of_variables + i + 1) * (RandomCounter::rand() % 2 == 0 ? -1 : 1) );
    }
    
    
    
    vector<bool> assignment(fuzzer.numer_of_variables + conditionalCount + 1);
    for (int i = 1; i <= fuzzer.numer_of_variables + conditionalCount; ++i)
    {
      if (RandomCounter::rand() % 2 == 0)
      {
	assignment[i] = false;
	formula.addClause(-i);
// 	cout << -i << endl;
      }
      else
      {
	assignment[i] = true;
	formula.addClause(i);
// 	cout << i << endl;
      }
    }
    
    PBConstraint constraint = fuzzer.generatePBConstraint();
    constraint.addConditionals(conditionals);

    pb2cnf.encode(constraint, formula, auxvars);
    
//     VectorClauseDatabase test(config);
//     pb2cnf.encode(constraint, test, auxvars);
//     test.printFormula();
    
    int64_t sum = 0;
    for (auto wlit : constraint.getWeightedLiterals())
    {

      if (wlit.lit < 0 ? !assignment[-wlit.lit] : assignment[wlit.lit] )
	sum += wlit.weight;
    }
    
    bool conditionalIsTrue = true;
    
    for (int32_t lit : conditionals)
    {
      conditionalIsTrue = conditionalIsTrue && (lit < 0 ? !assignment[-lit] : assignment[lit]);
    }
    
    bool haveToResult = false;
    
    if(constraint.getComparator() == LEQ)
      haveToResult = (sum <= constraint.getLeq());
    else
    if(constraint.getComparator() == GEQ)
      haveToResult = (sum >= constraint.getGeq());
    else 
      haveToResult = (sum <= constraint.getLeq()) && (sum >= constraint.getGeq());
    
    if (!conditionalIsTrue)
    {
      haveToResult = true;
    }
    
    bool isResult = satsolver.solve();
    
    if (isResult)
    {
      numSAT++;
//       cout << "isSAT" << endl;
    }
    else
    {
      numUNSAT++;
//       cout << "isUNSAT" << endl;
    }

    if (isResult == haveToResult)
      ;
    else
    {
      if (isResult)
	cout << "was SAT but have to be UNSAT" << endl;
      else
	cout << "was UNSAT but have to be SAT" << endl;
      
      cout << "not ok" << endl;
      constraint.print();
      exit(-1);
    }
    
//     if ( ((int64_t)numSAT + (int64_t)numUNSAT) % 100 == 0)
//     {
//       cout.precision(5);
// 	
//       if ((numSAT + numUNSAT) != 0)
// 	cout << "SAT / UNSAT = " << numSAT / (numSAT + numUNSAT) << " SAT " << numSAT  << " UNSAT " << numUNSAT << " total: " << (int64_t)numSAT + (int64_t)numUNSAT << "     \r";
//       cout << flush;
//     }
  }
}

void testIncrementalConstraints(PBConfig config, PBFuzzer& fuzzer, int64_t rounds)
{
  if (!printLess) cout << "testing incremental constraints                                      " << endl;
  
  
  double numSAT = 0;
  double numUNSAT = 0;

  PB2CNF pb2cnf(config);

    
  int count = 0;
  while (rounds < 0 ? true : (count < rounds) )
  {
//     cout << endl << "new round" << endl;
    count++; // cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;  
    AuxVarManager auxvars(fuzzer.numer_of_variables + 1);
    BasicSATSolver satsolver;
    SATSolverClauseDatabase formula(config, &satsolver);
    
    vector<bool> assignment(fuzzer.numer_of_variables+1);
    for (int i = 1; i <= fuzzer.numer_of_variables; ++i)
    {
      if (RandomCounter::rand() % 2 == 0)
      {
	assignment[i] = false;
	formula.addClause(-i);
// 	cout << -i << endl;
      }
      else
      {
	assignment[i] = true;
	formula.addClause(i);
// 	cout << i << endl;
      }
    }
    
    PBConstraint c = fuzzer.generatePBConstraint();
    IncPBConstraint constraint(c.getWeightedLiterals(), PBLib::BOTH, c.getLeq(), c.getGeq());
    
    if (c.getComparator() != BOTH)
      constraint.setComparator(c.getComparator());
    
    
    pb2cnf.encodeIncInital(constraint, formula, auxvars);
    
    
    if (RandomCounter::rand() % 100 + 1 <= 90)
    {
      if ((constraint.getComparator() == BOTH && (RandomCounter::rand() % 100 + 1 <= 50) ) || (constraint.getComparator() == PBLib::GEQ) )
      {
	constraint.encodeNewGeq(constraint.getGeq() + (RandomCounter::rand() % 100), formula, auxvars);
      }
      else
      {
	constraint.encodeNewLeq(constraint.getLeq() - (RandomCounter::rand() % 100), formula, auxvars);
      }
    }
    

    int64_t sum = 0;
    for (auto wlit : constraint.getWeightedLiterals())
    {

      if (wlit.lit < 0 ? !assignment[-wlit.lit] : assignment[wlit.lit] )
	sum += wlit.weight;
    }
    
    bool haveToResult = false;
    
    if(constraint.getComparator() == LEQ)
      haveToResult = (sum <= constraint.getLeq());
    else
    if(constraint.getComparator() == GEQ)
      haveToResult = (sum >= constraint.getGeq());
    else 
      haveToResult = (sum <= constraint.getLeq()) && (sum >= constraint.getGeq());
    
    bool isResult = satsolver.solve();
    
    if (isResult)
      numSAT++;
    else
      numUNSAT++;

    if (isResult == haveToResult)
      ;
    else
    {
      if (isResult)
	cout << "was SAT but have to be UNSAT" << endl;
      else
	cout << "was UNSAT but have to be SAT" << endl;
      
      cout << "not ok" << endl;
      constraint.print();
      exit(-1);
    }
    
//     if ( ((int64_t)numSAT + (int64_t)numUNSAT) % 100 == 0)
//     {
//       cout.precision(5);
// 	
//       if ((numSAT + numUNSAT) != 0)
// 	cout << "SAT / UNSAT = " << numSAT / (numSAT + numUNSAT) << " SAT " << numSAT  << " UNSAT " << numUNSAT << " total: " << (int64_t)numSAT + (int64_t)numUNSAT << "     \r";
//       cout << flush;
//     }
  }
}


void testConditionalIncrementalConstraints(PBConfig config, PBFuzzer& fuzzer, int64_t rounds)
{
  if (!printLess) cout << "testing incremental conditional constraints                                      " << endl;
  
  
  double numSAT = 0;
  double numUNSAT = 0;

  PB2CNF pb2cnf(config);

    
   int count = 0;
  while (rounds < 0 ? true : (count < rounds) )
  {
    count++; // cout << "c rand() counts: " << RandomCounter::callCount << "                                              " <<endl;
    
    int conditionalCount = RandomCounter::rand() % 3+1;
    AuxVarManager auxvars(fuzzer.numer_of_variables + conditionalCount + 1);
    BasicSATSolver satsolver;
    SATSolverClauseDatabase formula(config, &satsolver);
    
    
    
    vector<int32_t> conditionals;
    
    for (int i = 0; i < conditionalCount; ++i)
    {
      conditionals.push_back( (fuzzer.numer_of_variables + i + 1) * (RandomCounter::rand() % 2 == 0 ? -1 : 1) );
    }
    
    
    
    vector<bool> assignment(fuzzer.numer_of_variables + conditionalCount + 1);
    for (int i = 1; i <= fuzzer.numer_of_variables + conditionalCount; ++i)
    {
      if (RandomCounter::rand() % 2 == 0)
      {
	assignment[i] = false;
	formula.addClause(-i);
// 	cout << -i << endl;
      }
      else
      {
	assignment[i] = true;
	formula.addClause(i);
// 	cout << i << endl;
      }
    }
    
    PBConstraint c = fuzzer.generatePBConstraint();    

    IncPBConstraint constraint(c.getWeightedLiterals(), PBLib::BOTH, c.getLeq(), c.getGeq());
    constraint.addConditionals(conditionals);
    
    if (c.getComparator() != BOTH)
      constraint.setComparator(c.getComparator());
    
    
    pb2cnf.encodeIncInital(constraint, formula, auxvars);
    
    
    if (RandomCounter::rand() % 100 + 1 <= 90)
    {
      if ((constraint.getComparator() == BOTH && (RandomCounter::rand() % 100 + 1 <= 50) ) || (constraint.getComparator() == PBLib::GEQ) )
      {
	constraint.encodeNewGeq(constraint.getGeq() + (RandomCounter::rand() % 100), formula, auxvars);
      }
      else
      {
	constraint.encodeNewLeq(constraint.getLeq() - (RandomCounter::rand() % 100), formula, auxvars);
      }
    }
    
    
//     VectorClauseDatabase test(config);
//     pb2cnf.encode(constraint, test, auxvars);
//     test.printFormula();
    
    int64_t sum = 0;
    for (auto wlit : constraint.getWeightedLiterals())
    {

      if (wlit.lit < 0 ? !assignment[-wlit.lit] : assignment[wlit.lit] )
	sum += wlit.weight;
    }
    
    bool conditionalIsTrue = true;
    
    for (int32_t lit : conditionals)
    {
      conditionalIsTrue = conditionalIsTrue && (lit < 0 ? !assignment[-lit] : assignment[lit]);
    }
    
    bool haveToResult = false;
    
    if(constraint.getComparator() == LEQ)
      haveToResult = (sum <= constraint.getLeq());
    else
    if(constraint.getComparator() == GEQ)
      haveToResult = (sum >= constraint.getGeq());
    else 
      haveToResult = (sum <= constraint.getLeq()) && (sum >= constraint.getGeq());
    
    if (!conditionalIsTrue)
    {
      haveToResult = true;
    }
    
    bool isResult = satsolver.solve();
    
    if (isResult)
    {
      numSAT++;
//       cout << "isSAT" << endl;
    }
    else
    {
      numUNSAT++;
//       cout << "isUNSAT" << endl;
    }

    if (isResult == haveToResult)
      ;
    else
    {
      if (isResult)
	cout << "was SAT but have to be UNSAT" << endl;
      else
	cout << "was UNSAT but have to be SAT" << endl;
      
      cout << "not ok" << endl;
      constraint.print();
      exit(-1);
    }
    
//     if ( ((int64_t)numSAT + (int64_t)numUNSAT) % 100 == 0)
//     {
//       cout.precision(5);
// 	
//       if ((numSAT + numUNSAT) != 0)
// 	cout << "SAT / UNSAT = " << numSAT / (numSAT + numUNSAT) << " SAT " << numSAT  << " UNSAT " << numUNSAT << " total: " << (int64_t)numSAT + (int64_t)numUNSAT << "     \r";
//       cout << flush;
//     }
  }
}
