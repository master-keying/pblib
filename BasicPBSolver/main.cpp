#include <vector>
#include <iostream>
#include <stdlib.h>
#include <time.h>  
#include <sys/resource.h> 

// PBLib
// #include "incpbconstraint.h"
// #include "PBConfig.h"
#include "PBParser.h"
#include "pb2cnf.h"
#include "VectorClauseDatabase.h"
#include "SATSolverClauseDatabase.h"
#include <sys/time.h>  
// MiniSAT
#include "BasicSATSolver.h"


using namespace PBLib;
using namespace std;

const int UNSAT = 20;
const int SAT = 10;
const int OPTIMUM = 5;
const int UNKNOWN = 0;

int readValueVector( vector<int32_t>& valueVector, int64_t & ovalue, bool & hasOValue)
{
  valueVector.clear();
  
  istream* fileptr= &cin;
  istream& file = *fileptr;

  int solution = -1;
  string line;
  hasOValue = false;
  
  while( getline( file, line ) ) {
    
  // parse satisfiability
    if( line[0] == 's' ) { 
      if( line.find( "UNSATISFIABLE" ) != string::npos ) {
			  solution = UNSAT;
			  break;
      } else if( line.find( "UNKNOWN" ) != string::npos ) { 
			  solution = UNKNOWN;
			  break;
      } else if( line.find( "SATISFIABLE" ) != string::npos ) { 
	  solution = SAT;
	  break;
      } else if( line.find( "OPTIMUM FOUND" ) != string::npos ) { 
			  solution = OPTIMUM;
			  break;
      }
    }
    
    
    
    // parse o value
    if( line[0] == 'o' ) {
      hasOValue = true;
      int ind = 2;
	while( ind < line.size() && line[ind] == ' ' ) ++ind;
	if(line.size() > ind)
	{
		int number = 0;
		bool negative = false;
		if ( ind < line.size() && line[ind] == '-')
		{
			negative = true;
			ind++;
		}
		while( ind < line.size() && line[ind] >= '0' && line[ind] <= '9' )
		{
			number *=10;
			number += line[ind++] - '0';
		}
		
		
		ovalue = (negative) ? 0 - number : number;
	}
    }
      
    // parse model
    if( line[0] == 'v' ) {
	int ind = 2;
	while( ind < line.size() && line[ind] == ' ' ) ++ind;
	while(line.size() > ind)
	{
		int number = 0;
		bool negative = false;
		if ( ind < line.size() && line[ind] == '-')
		{
			negative = true;
			ind++;
		}
		if ( ind < line.size() && line[ind] == 'x')
		{
			ind++;
		}
		while( ind < line.size() && line[ind] >= '0' && line[ind] <= '9' )
		{
			number *=10;
			number += line[ind++] - '0';
		}
		if( number == 0 ) break;
		
		if (number >= valueVector.size())
		  valueVector.resize(number+1,0);
		
		valueVector[number] = (negative) ? 0 - number : number;
		while(line[ind] == ' ') ind++;
	}
    }
  }
  
  return solution;
}

bool check_constraints(vector<int32_t> const & model, vector<PBConstraint> const & constraints)
{
  
  for (PBConstraint const & constraint : constraints)
  {
    int64_t sum = 0;
    
    for (WeightedLit lit : constraint.getWeightedLiterals())
    {
      if (abs(lit.lit) >= model.size())
      {
	cout << "c error: variable greater then model size" << endl;
	return false;
      }
      
      if (model[abs(lit.lit)] == lit.lit)
	sum += lit.weight;
    }
    
    if (constraint.getComparator() == LEQ || constraint.getComparator() == BOTH)
    {
      if (sum > constraint.getLeq())
      {
	cout << "c not a model for: ";
	constraint.print();
	return false;
      }
    }
    
    if (constraint.getComparator() == GEQ || constraint.getComparator() == BOTH)
    {
      if (sum < constraint.getGeq())
      {
	cout << "c not a model for: ";
	constraint.print();
	return false;
      }
    }
  }
  
  return true;
}

template <class P>
int64_t getSumFromModel(P & constraint, vector<int32_t> const & model)
{  
    
    int64_t sum = 0;
    
    for (int i = 0; i < constraint.getN(); ++i)
    {
      if (abs(constraint.getWeightedLiterals()[i].lit) >= model.size())
      {
	cout << "c error: variable greater then model size" << endl;
	continue;
      }
      
      if (model[abs(constraint.getWeightedLiterals()[i].lit)] * (constraint.getWeightedLiterals()[i].lit > 0 ? 1 : -1) > 0)
	sum += constraint.getWeightedLiterals()[i].weight;
    }
    
    return sum;
}

template <class P>
int64_t getSumFromModel(P & constraint, BasicSATSolver * satsolver)
{  
    vector<int32_t> model;
    satsolver->getModel(model);
    
    int64_t sum = 0;
    
    for (int i = 0; i < constraint.getN(); ++i)
    {
      if (model[abs(constraint.getWeightedLiterals()[i].lit)] * (constraint.getWeightedLiterals()[i].lit > 0 ? 1 : -1) > 0)
	sum += constraint.getWeightedLiterals()[i].weight;
    }
    
    return sum;
}





void printModel(vector<int32_t> & model, int number_of_vars)
{
    int count = 0;


    if (model.size() <= number_of_vars)
    {
	cout << "c error: cant print model since model size is smaller then the number of variables" << endl;
	return;
    }
    
    
    stringstream output;
    output << "v";
    for (int i = 1; i <= number_of_vars; ++i)
    {
// 	if (count == 50)
// 	{
// 	    output << endl << "v";
// 	    count = 0;
// 	}

	output << " ";
	if (model[i] != i)
	    output << "-";
    
	output << "x" << i;
// 	count++;
    }

    output << endl;

    cout << output.str() << endl;

}


void basicSearch(bool cnf_output_only, double tstart, vector<PBConstraint> & constraints, PBParser & parser, PBConfig config)
{
  statistic stats;
  PB2CNF pb2cnf(config, &stats);
  
  double tend;
  VectorClauseDatabase formula(config);
  
  BasicSATSolver * satsolver = new BasicSATSolver();
  satsolver->increseVariables(parser.getMaxVarID());
  
  
  cout << "c start encoding ... "; cout.flush();
  if (config->print_used_encodings)
    cout << endl;
  
  AuxVarManager initAuxVars(parser.getMaxVarID() + 1);
  
//   config->print_used_encodings = true;
//   config->pb_encoder = PB_ENCODER::BINARY_MERGE;
//   config->pb_encoder = PB_ENCODER::BDD;
//   cout << endl;
  for(int i = 0; i < (int) constraints.size(); ++i)
  {
//       cout << "\r" << i << " / " << constraints.size() << "       "; cout.flush();
      pb2cnf.encode(constraints[i], formula, initAuxVars);
  }
//   cout << endl;
  
  tend = clock();
  cout << "done (parsing and encoding: " << (tend - tstart) / CLOCKS_PER_SEC << " sec )" << endl;
  
  
  tstart = tend;

  
  for (auto clause : formula.getClauses())
    satsolver->addClause(clause);
  
  bool result = satsolver->solve();
  
  if(!result)
  {
    cout << "s UNSATISFIABLE" << endl;
    delete satsolver;
    return;
  }
  else if (!parser.hasObjectiveFunction())
  {
    
    cout << "s SATISFIABLE" << endl;
    
     if (config->cmd_line_options.find("model") != config->cmd_line_options.end() || config->cmd_line_options.find("allmodels") != config->cmd_line_options.end())
     {
	 vector<int32_t> model;
	 satsolver->getModel(model);
	 
	 printModel(model,parser.getMaxVarID());
     }
     delete satsolver;
     return;
  }
  

 
  PBConstraint opt_constraint = parser.getObjConstraint();

  if (opt_constraint.getComparator() == BOTH)
  {
    // TODO this should be very simple
    cout << "c opt constraint with LEQ and GEQ is not supported yet" << endl;
  }
  
  assert(opt_constraint.getComparator() == LEQ);
  
  int64_t current_bound = getSumFromModel(opt_constraint, satsolver);;
    
  
  int32_t first_free_var = initAuxVars.getBiggestReturnedAuxVar() + 1;

  vector<int32_t> model;
  satsolver->getModel(model);

  while (true)
  {   
    cout << "o " << current_bound << endl;

    if (config->cmd_line_options.find("allmodels") != config->cmd_line_options.end())
    {	 
	 printModel(model,parser.getMaxVarID());
    }


    opt_constraint.setLeq(current_bound - 1);

    AuxVarManager opt_aux(first_free_var);
    
    delete satsolver;
    satsolver = new BasicSATSolver();
    satsolver->increseVariables(parser.getMaxVarID());
    
    VectorClauseDatabase opt_formula(config);
    
    for (auto clause : formula.getClauses())
	satsolver->addClause(clause);  
    
    pb2cnf.encode(opt_constraint, opt_formula, opt_aux);
    satsolver->addClauses(opt_formula.getClauses());
    
    
    if (!satsolver->solve())
      break;

     model.clear();
     satsolver->getModel(model);
	 

    
    current_bound = getSumFromModel(opt_constraint, satsolver);
  }

  cout << "s OPTIMUM FOUND" << endl;
  
  if (config->cmd_line_options.find("model") != config->cmd_line_options.end())
  {
      printModel(model, parser.getMaxVarID());
  }
  
  delete satsolver;
}


void analyse(char** argv)
{
  PBConfig config = make_shared<PBConfigClass>();
  statistic stats;
  PB2CNF pb2cnf(config, &stats);

  PreEncoder  pre_encoder(config);
        
  PBParser parser;

//   cout << "c start parsing ... "; cout.flush();
  
  vector<PBConstraint> constraints = parser.parseFile(argv[1]);
  
  if (!parser.isOk())
  {
    cout << "c error: could not parse input file" << endl;
    cout << "0" << endl;
    exit(-1);
  }
  
//   cout << "done" << endl;
  
  
  AuxVarManager auxVars(parser.getMaxVarID() + 1);
  
  CountingClauseDatabase formula(config);
  
  int64_t count = 0;
  
  for(auto c : constraints)
  {
    SimplePBConstraint s = pre_encoder.preEncodePBConstraint(c, formula);
    if (s.getType() == PB)
      count++;
  }
  
  cout << count << endl;
}


int main(int argc, char **argv)
{
  double tstart = clock(), tend;
  
  bool  cnf_output_only = false;

  int cpu_lim = -1; // Limit on CPU time allowed in seconds
  int mem_lim = -1; // Limit on memory usage in megabytes
  
  // Set limit on CPU-time: (copy and paste from glucose source code)
  if (cpu_lim != -1){
      rlimit rl;
      getrlimit(RLIMIT_CPU, &rl);
      if (rl.rlim_max == RLIM_INFINITY || (rlim_t)cpu_lim < rl.rlim_max){
	  rl.rlim_cur = cpu_lim;
	  if (setrlimit(RLIMIT_CPU, &rl) == -1)
	      printf("c WARNING! Could not set resource limit: CPU-time.\n");
      } }

  // Set limit on virtual memory:
  if (mem_lim != -1){
      rlim_t new_mem_lim = (rlim_t)mem_lim * 1024*1024;
      rlimit rl;
      getrlimit(RLIMIT_AS, &rl);
      if (rl.rlim_max == RLIM_INFINITY || new_mem_lim < rl.rlim_max){
	  rl.rlim_cur = new_mem_lim;
	  if (setrlimit(RLIMIT_AS, &rl) == -1)
	      printf("c WARNING! Could not set resource limit: Virtual memory.\n");
      } }
  // end of copy and paste
  
  if (argc < 2)
  {
    cout << "usage " << argv[0] << " inputfile [options]" << endl;
    return -1;
  }
  
  PBConfig config = make_shared<PBConfigClass>();
//     config->pb_encoder = PB_ENCODER::SORTINGNETWORKS;
//   config->pb_encoder = PB_ENCODER::ADDER;
//   config->pb_encoder = PB_ENCODER::BINARY_MERGE;
//   config->print_used_encodings = true;
  config->MAX_CLAUSES_PER_CONSTRAINT = 1000000;  
//   config->amo_encoder = AMO_ENCODER::COMMANDER;    
//   config->use_formula_cache=false;
//   config->check_for_dup_literals = true;
  config->print_used_encodings = false;
//   config->pb_encoder = PB_ENCODER::ADDER;
  
  
  //TODO refactor this
  unordered_map<string,string> options;
  
  if (argc > 2)
  {
    string tmp;
    for (int i = 2; i < argc; ++i)
    {
      tmp = string(argv[i]);
      while (tmp[0] == '-')
	tmp = tmp.substr(1);
      config->cmd_line_options.insert(tmp);
    }
    

    for (int i = 2; i < argc; ++i)
    {
	tmp = string(argv[i]);
	while (tmp[0] == '-')
	    tmp = tmp.substr(1);	

	options[tmp.substr(0,tmp.find("="))] = tmp.find("=") == string::npos ? "" : tmp.substr(tmp.find("=") + 1);
    }
  }
  
  if (config->cmd_line_options.find("valid") != config->cmd_line_options.end())
  {
    vector<int32_t> model;
    int64_t o_value = 0;
    bool has_o_value = false;
    int result = readValueVector(model, o_value, has_o_value);
    
    if (result < 0)
    {
      cout << "c no s line present" << endl;
      // below is the check for o-value
    }
    
    if (result == UNSAT)
    {
      if (has_o_value)
      {
	cout << "c error: UNSAT but o line present" << endl;
	cout << "Result: INVALID" << endl;
	return 0;
      }
      
      cout << "c UNSAT" << endl;
      
      cout << "Result: OK" << endl;
      return 0;
    }
    
    if (result == OPTIMUM && !has_o_value)
    {
      cout << "c error: OPTIMUM FOUND but no o line" << endl;
      cout << "Result: INVALID" << endl;
      return 0;
    }
    
    PBParser parser;

    
    vector<PBConstraint> constraints = parser.parseFile(argv[1]);
    
    if (!parser.isOk())
    {
      cout << "c [check valid] error: could not parse input file" << endl;
      cout << "Result: ERROR" << endl;
      return 0;
    }
    
    if(check_constraints(model, constraints))
    {
      cout << "c model is consistent with constraints" << endl;
    }
    else
    {
      cout << "c error: model is inconsistent with constraints" << endl;
      cout << "Result: INVALID" << endl;
      return 0;
    }
    
    if (has_o_value)
    {
      PBConstraint obj = parser.getObjConstraint();
      int64_t real_o_value = getSumFromModel(obj, model);
      
      if (real_o_value == o_value)
      {
	cout << "c o-value is consistent with model" << endl;
      }
      else
      {
	cout << "c error: o-value is inconsistent with model" << endl;
	cout << "Result: INVALID" << endl;
	return 0;
      }
    }
    
    
    cout << "Result: OK" << endl;
    return 0;
  }
  
  if (options.count("maxclauses") > 0)
  {
    config->MAX_CLAUSES_PER_CONSTRAINT = atol(options["maxclauses"].c_str());
    cout << "c setting max clauses per constraint to " <<  options["maxclauses"] << endl;
  }
  
  
  if (config->cmd_line_options.find("cnf") != config->cmd_line_options.end())
    cnf_output_only = true;
  
  if (config->cmd_line_options.find("norobdds") != config->cmd_line_options.end())
    config->use_real_robdds = false;
  
  if (config->cmd_line_options.find("rectest") != config->cmd_line_options.end())
    config->use_recursive_bdd_test = true;
  
  if (config->cmd_line_options.find("itertest") != config->cmd_line_options.end())
    config->use_recursive_bdd_test = false;
  
  
  if (config->cmd_line_options.find("nested") != config->cmd_line_options.end())
    config->amo_encoder = AMO_ENCODER::NESTED;
  
  if (config->cmd_line_options.find("amo_bdd") != config->cmd_line_options.end())
    config->amo_encoder = AMO_ENCODER::BDD;

  if (config->cmd_line_options.find("bimander") != config->cmd_line_options.end())
    config->amo_encoder = AMO_ENCODER::BIMANDER;
  
  if (config->cmd_line_options.find("commander") != config->cmd_line_options.end())
    config->amo_encoder = AMO_ENCODER::COMMANDER;
  
  if (config->cmd_line_options.find("kproduct") != config->cmd_line_options.end())
    config->amo_encoder = AMO_ENCODER::KPRODUCT;
  
  if (config->cmd_line_options.find("binary") != config->cmd_line_options.end())
    config->amo_encoder = AMO_ENCODER::BINARY;
  
  if (config->cmd_line_options.find("pairwise") != config->cmd_line_options.end())
    config->amo_encoder = AMO_ENCODER::PAIRWISE;
  
  if (config->cmd_line_options.find("amk_bdd") != config->cmd_line_options.end())
    config->amk_encoder = AMK_ENCODER::BDD;
  
  if (config->cmd_line_options.find("card") != config->cmd_line_options.end())
    config->amk_encoder = AMK_ENCODER::CARD;
  
  if (config->cmd_line_options.find("pb_bdd") != config->cmd_line_options.end())
    config->pb_encoder = PB_ENCODER::BDD;

  if (config->cmd_line_options.find("pb_adder") != config->cmd_line_options.end())
    config->pb_encoder = PB_ENCODER::ADDER;

  if (config->cmd_line_options.find("pb_sorter") != config->cmd_line_options.end())
    config->pb_encoder = PB_ENCODER::SORTINGNETWORKS;
  
  if (config->cmd_line_options.find("watchdog") != config->cmd_line_options.end())
  {
    config->pb_encoder = PB_ENCODER::BINARY_MERGE;
    config->use_watch_dog_encoding_in_binary_merger = true;
  }
  
  
  if (config->cmd_line_options.find("bin_merge") != config->cmd_line_options.end())
  {
    config->pb_encoder = PB_ENCODER::BINARY_MERGE;
    config->use_gac_binary_merge = true;
  }
  
  if (config->cmd_line_options.find("bin_merge_no_gac") != config->cmd_line_options.end())
    config->use_gac_binary_merge = false;
  
  if (config->cmd_line_options.find("watchdog_no_gac") != config->cmd_line_options.end())
    config->use_gac_binary_merge = false;
  
  if (config->cmd_line_options.find("bin_merge_all_support") != config->cmd_line_options.end())
    config->binary_merge_no_support_for_single_bits = false;
 
  
  if (config->cmd_line_options.find("non_gac_bdds") != config->cmd_line_options.end())
    config->debug_value = "non_gac_bdds";
  
  if (config->cmd_line_options.find("test") != config->cmd_line_options.end())
    config->debug_value = "test";
  
  if (config->cmd_line_options.find("analyse") != config->cmd_line_options.end())
  {
    analyse(argv);
    return 0;
  }
  
  BasicSATSolver satsolver;
  SATSolverClauseDatabase formula(config, &satsolver);
  

      
  PBParser parser;

  cout << "c start parsing ... "; cout.flush();
  
  vector<PBConstraint> constraints = parser.parseFile(argv[1]);
  
  if (!parser.isOk())
  {
    cout << "c error: could not parse input file" << endl;
    exit(-1);
  }
  
  satsolver.increseVariables(parser.getMaxVarID());

  cout << "done" << endl;

  basicSearch(cnf_output_only, tstart, constraints, parser, config);
  
  
  tend = clock();
  cout << "c wall time: " << (tend - tstart) / CLOCKS_PER_SEC << " sec" << endl;

  struct rusage rusage;
  getrusage( RUSAGE_SELF, &rusage );
  cout <<"c Memory usage: "<< (size_t)(rusage.ru_maxrss / 1024L) << " mbyte" << endl; 
  
  return 0;
}
