#include "VectorClauseDatabase.h"
#include "helper.h"

using namespace std;


VectorClauseDatabase::~VectorClauseDatabase()
{
	if (local_clauses)
		delete clauses;
}


VectorClauseDatabase::VectorClauseDatabase(PBConfig config): ClauseDatabase(config), local_clauses(true), clauses(new vector<vector<int32_t> >())
{

}

VectorClauseDatabase::VectorClauseDatabase(PBConfig config, vector< vector< int32_t > > * clauses): ClauseDatabase(config), local_clauses(false), clauses(clauses)
{

}


void VectorClauseDatabase::clearDatabase()
{
	assert(local_clauses);
	clauses->clear();
	resetInternalUnsatState();
}

void VectorClauseDatabase::resetInternalUnsatState()
{
	deleteIsSetToUnsatFlag();
}


void VectorClauseDatabase::addClauseIntern(const vector< int32_t >& clause)
{
  clauses->push_back(clause);
}


vector< vector< int32_t > > const & VectorClauseDatabase::getClauses() 
{
  return *clauses;
}



void VectorClauseDatabase::printFormula(ostream & output)
{
  for (int i = 0; i < clauses->size(); ++i)
  {
    cout << (*clauses)[i] << " 0" << endl;
  }
}

