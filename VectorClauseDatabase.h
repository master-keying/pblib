#ifndef VECTORCLAUSEDATABASE_H
#define VECTORCLAUSEDATABASE_H

#include <vector>
#include "clausedatabase.h"

class VectorClauseDatabase : public ClauseDatabase
{
private:
	bool local_clauses;
    std::vector<std::vector<int32_t> > * clauses;
    virtual void addClauseIntern(std::vector< int32_t > const & clause);
public:
    
	~VectorClauseDatabase();
    VectorClauseDatabase(PBConfig config);
    VectorClauseDatabase(PBConfig config, std::vector<std::vector<int32_t> > * clauses);
    std::vector<std::vector<int32_t> > const & getClauses();
    void printFormula(std::ostream & output = std::cout);
    
    void clearDatabase();
	void resetInternalUnsatState();
};

#endif // VECTORCLAUSEDATABASE_H
