#include "SATSolverClauseDatabase.h"

using namespace std;

void SATSolverClauseDatabase::addClauseIntern(const vector< int32_t >& clause)
{
//   for (int32_t lit : clause)
//     cout << lit << " ";
//   cout << "0" << endl;
  
  satsolver->addClause(clause);
}

SATSolverClauseDatabase::SATSolverClauseDatabase(PBConfig config, BasicSATSolver* satsolver): ClauseDatabase(config), satsolver(satsolver)
{

}
