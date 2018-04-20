#include <iostream>

#include "PBParser.h"
#include "pb2cnf.h"

using namespace std;
using namespace PBLib;


int main(int argc, char **argv)
{
  unordered_map<string,string> options;
  
  if (argc > 2)
  {
    string tmp;
    for (int i = 2; i < argc; ++i)
    {
	tmp = string(argv[i]);
	while (tmp[0] == '-')
	    tmp = tmp.substr(1);	

	options[tmp.substr(0,tmp.find("="))] = tmp.find("=") == string::npos ? "" : tmp.substr(tmp.find("=") + 1);
    }
  }
  

  if (argc < 2)
  {
    cout << "usage " << argv[0] << " inputfile [options]" << endl;
    return -1;
  }
  

  PBConfig config = make_shared<PBConfigClass>();
  VectorClauseDatabase formula(config);
  
  PBParser parser;
  
  vector<PBConstraint> constraints = parser.parseFile(argv[1]);
  
  if (!parser.isOk())
  {
    cout << endl << "c error: could not parse input file" << endl;
    exit(-1);
  }


  PB2CNF pb2cnf(config);
  AuxVarManager auxvars(parser.getMaxVarID() + 1);
  
  
  for(int i = 0; i < (int) constraints.size(); ++i)
  {
      pb2cnf.encode(constraints[i], formula, auxvars);
  }
  
  cout << "p cnf " <<  auxvars.getBiggestReturnedAuxVar() << " " << formula.getClauses().size() << endl;
  
  for (auto clause : formula.getClauses())
  {
    for (auto lit : clause)
      cout << lit << " ";
    cout << "0" << endl;
  }
}