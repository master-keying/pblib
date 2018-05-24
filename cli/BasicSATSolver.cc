#include "BasicSATSolver.h"
#include <iostream>
#include <assert.h>

using namespace Minisat;
using namespace std;


void BasicSATSolver::print_solving_stats()
{
  cout << "c conflicts: " << conflicts << endl;
  cout << "c propagations: " << propagations << endl;
  cout << "c decisions: " << decisions << endl;
}


void BasicSATSolver::print_added_clauses_stats()
{
  long clauses_count = units + clauses2er + clauses3er + clausesRest;
  cout << endl;
  cout << "c literals: " << nVars() << endl;
  cout << "c lits in clauses: " << lits_in_clauses << endl;
  if (clauses_count > 0)
    cout << "c lits per clauses: " << (double)lits_in_clauses / clauses_count<< endl;
  else
    cout << "c lits per clauses: " << 0 << endl;
  cout << "c unit clauses: " << units<< endl;
  cout << "c binary clauses: " << clauses2er << endl;
  cout << "c ternary clauses: " << clauses3er << endl;
  cout << "c clauses with size >= 4: " << clausesRest << endl;
  cout << "c number of base clause: " << clauses_count << endl;
}

bool BasicSATSolver::addClauses(std::vector< std::vector< int32_t > > const & clauses)
{
  assert(decisionLevel() == 0 && "add clauses only at level 0" );

  if (!ok) return false;

  bool ret = true;

  using uint = unsigned int;
  for( uint ci = 0 ; ci < clauses.size(); ++ ci ) {
    tmp = clauses[ci].size();
    lits_in_clauses += tmp;
    if (tmp == 2)
      clauses2er++;
    else if (tmp == 3)
      clauses3er++;
    else if (tmp == 1)
      units++;
    else
      clausesRest++;

    if (!addClause(clauses[ci]))
    {
      ret = false;
    }
  }



  if (!ok) return false;

  return ret;
}

void BasicSATSolver::increseVariables(int32_t max_var)
{
    while( max_var >= nVars() ) newVar();
}


bool BasicSATSolver::addClause(std::vector< int32_t > const & clause)
{
    assert(decisionLevel() == 0 && "add clauses only at level 0" );

    tmp = clause.size();
    lits_in_clauses += tmp;
    if (tmp == 2)
      clauses2er++;
    else if (tmp == 3)
      clauses3er++;
    else if (tmp == 1)
      units++;
    else
      clausesRest++;
    Minisat::Var maxVar = 0;

    tmpLits.clear();

    using uint = unsigned int;
    for( uint j = 0 ; j < clause.size(); ++ j ) {
      const int l = clause[j];
      tmpLits.push( l > 0 ? mkLit(l - 1, false) : mkLit( -l -1, true) );
      maxVar = maxVar >= var(tmpLits.last()) ? maxVar : var(tmpLits.last());
    }

    // take care that solver knows about all variables!
    while( maxVar >= nVars() ) newVar();

    return addClause(tmpLits);
}

void BasicSATSolver::getModel(std::vector< int32_t >& model)
{
  model.clear();
  model.push_back(0); // dummy literal
  for( int v = 0 ; v < nVars(); v++ )
  {
    model.push_back( this->model[v] == l_True ? (v + 1) : (- v - 1) );
  }
}

void BasicSATSolver::getModel(std::vector< bool >& model)
{
  model.clear();
  model.push_back(0); // dummy literal
  for( int v = 0 ; v < nVars(); v++ )
  {
    model.push_back( this->model[v] == l_True ? (true) : (false) );
  }
}


void BasicSATSolver::printModel()
{
  cout << "v";
  for( int v = 0 ; v < nVars(); v++ )
  {
    cout << " " << (this->model[v] == l_True ? (v + 1) : (- v - 1));
  }
  cout << endl;
}
