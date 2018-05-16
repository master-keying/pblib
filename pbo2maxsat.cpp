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
#include <sys/time.h>


using namespace std;
using namespace PBLib;


int main(int argc, char **argv)
{

  double tstart = clock(), tend;

  PBParser parser;


  vector<PBConstraint> constraints = parser.parseFile(argv[1]);

  if (!parser.isOk())
  {
    cout << "c error: could not parse input file" << endl;
    return -1;
  }


  PBConfig config = make_shared<PBConfigClass>();
//   config->pb_encoder = PB_ENCODER::BINARY_MERGE;

  PB2CNF pb2cnf(config);
  VectorClauseDatabase formula(config);
  AuxVarManager auxvars(parser.getMaxVarID()+1);


  for (auto c : constraints)
  {
    pb2cnf.encode(c, formula, auxvars);
  }

  int64_t offset = 0;
  int64_t top_weight = 1;

  if (parser.hasObjectiveFunction())
  {
    PBConstraint  obtConstraint = parser.getObjConstraint();

    for (WeightedLit & l : obtConstraint.getWeightedLiterals())
    {
      if (l.weight < 0)
      {
	top_weight += -l.weight;
	offset += l.weight;
      }
      else
	top_weight += l.weight;
    }


    assert(offset <= 0);
    if (offset < 0)
    {
      cout << "c warning: use offset " << offset << " to calculate real optimum" << endl;
    }

    cout << "p wcnf " << auxvars.getBiggestReturnedAuxVar() << " " << formula.getClauses().size() + parser.getObjConstraint().getN() << " " << top_weight << endl;

    //printing soft clauses
    for (WeightedLit & l : obtConstraint.getWeightedLiterals())
    {
      if (l.weight < 0)
      {
	cout << -l.weight << " " << l.lit << " 0" << endl;
      }
      else
	cout << l.weight << " " << -l.lit << " 0" << endl;
    }

    //printing hard clauses
    for (auto c : formula.getClauses())
    {
      cout << top_weight << " ";
      for (auto l : c)
	cout << l << " ";
      cout << "0" << endl;
    }
  }
  else
  {
    cout << "c warning this is a decision instance, not a maxsat instance" << endl;
    cout << "p cnf " << auxvars.getBiggestReturnedAuxVar() << " " << formula.getClauses().size() << endl;

    for (auto c : formula.getClauses())
    {
      for (auto l : c)
	cout << l << " ";
      cout << "0" << endl;
    }
  }




//   tend = clock();
//   cout << "wall time: " << (tend - tstart) / CLOCKS_PER_SEC << " sec" << endl;

//   struct rusage rusage;
//   getrusage( RUSAGE_SELF, &rusage );
//   cout <<"Memory usage: "<< (size_t)(rusage.ru_maxrss / 1024L) << " mbyte" << endl;

  return 0;
}










