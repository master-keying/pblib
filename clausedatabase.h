#ifndef CLAUSEDATABASE_H
#define CLAUSEDATABASE_H

#include <vector>
#include <cstdint>
#include <assert.h>
#include <unordered_map>
#include "auxvarmanager.h"
#include "formula.h"
#include "PBConfig.h"

class ClauseDatabase
{
private:
  bool isUnsat;
  std::vector<int32_t> tmp_clause_wc;
  std::unordered_map<int32_t,Lit> history;
  std::vector<int32_t> conditionals;
  
  ClauseDatabase(const ClauseDatabase& other) = delete;
  virtual bool operator==(const ClauseDatabase& other) const;
  
  virtual int32_t polarityClausify(Formula const & f, AuxVarManager & aux_vars);
  
protected:
  std::vector<int32_t> clause;
  PBConfig config;
  virtual void addClauseIntern(std::vector< int32_t > const & clause) = 0;
  
  
public:
  void addConditional(int32_t lit);
  void addConditionals(std::vector<int32_t> lits);
  std::vector<int32_t> & getConditionals();

  ClauseDatabase(PBConfig config);
  virtual ~ClauseDatabase();

  // add a clause to database
  virtual void addClause(std::vector<int32_t> const & clause);
  // add a Formula (gate) to the database (which is converted to clause, using the addClause method)
  virtual void addFormula(Formula const & formula, AuxVarManager & aux_vars);

  // add the empty clause to database and mark hole formula as unsat (by setting isUnsat to true and add empty clause with addClause)
  // WARNING: be sure to set isUnsat in you own implentation
  virtual void addUnsat();

  // check if empty clause was added manually
  // WARNING: if this is used to stop encoding at an early state,
  // be sure that you _really_ don't want to add/encode all other
  // clauses (e.g. if you are using tagging)!
  bool isSetToUnsat();
  void deleteIsSetToUnsatFlag();
  void addClauses(std::vector<std::vector<int32_t> > const & clauses);
  
  template < typename First >
  void addClause ( First first )
  {
      if (first != 0)
      {
	  tmp_clause_wc.push_back(first);
      }
      addClause(tmp_clause_wc);
      tmp_clause_wc.clear();
  }
  
  template < typename First, typename ... Liste >
  void addClause ( First first, Liste ... rest )
  {
      assert(first != 0);
      tmp_clause_wc.push_back(first);
      addClause ( rest ... );
  }
};

class CountingClauseDatabase : public ClauseDatabase
{
  private:
    int64_t number_of_clauses;
    
    virtual void addClauseIntern(std::vector< int32_t > const & clause);
  public:
    CountingClauseDatabase(PBConfig config);
    
    int64_t getNumberOfClauses();
    void clearDataBase();
};

#endif // CLAUSEDATABASE_H

