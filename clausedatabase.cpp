#include "clausedatabase.h"
#include "helper.h"

using namespace std;


void ClauseDatabase::addClauses(vector< vector< int32_t > > const & clauses)
{
  for (int i = 0; i < clauses.size(); ++i)
    addClause(clauses[i]);
}


void ClauseDatabase::addUnsat()
{
  isUnsat = true;
  clause.clear();
  addClause(clause);
}


void ClauseDatabase::addFormula(const Formula& formula, AuxVarManager& aux_vars)
{
  history.clear(); // make sure that history is cleared
  clause.clear();
  clause.push_back(polarityClausify(formula, aux_vars));
  addClause(clause);
  history.clear(); // save memory
}

void ClauseDatabase::deleteIsSetToUnsatFlag()
{
  isUnsat = false;
}


bool ClauseDatabase::isSetToUnsat()
{
  return isUnsat;
}


void ClauseDatabase::addConditional(int32_t lit)
{
  conditionals.push_back(lit);
}

void ClauseDatabase::addConditionals(vector< int32_t > lits)
{
  for (int32_t lit : lits)
    conditionals.push_back(lit);
}



vector< int32_t >& ClauseDatabase::getConditionals()
{
  return conditionals;
}

void ClauseDatabase::addClause(const vector< int32_t >& clause)
{
  if (config->check_for_dup_literals)
  {
    vector<int32_t> copy = clause;
	if (conditionals.size() > 0)
	{
	  for (int32_t lit : conditionals)
		copy.push_back(-lit);
	}
	
    copy.resize( distance(copy.begin(),unique (copy.begin(), copy.end()) ) );
    addClauseIntern(copy);
  }
  else
  {
	  if (conditionals.size() > 0)
	  {
			vector<int32_t> copy = clause;		  
			for (int32_t lit : conditionals)
			  copy.push_back(-lit);
			
			addClauseIntern(copy);
	  }
	  else
		addClauseIntern(clause);
  }
}


ClauseDatabase::ClauseDatabase(PBConfig config) : config(config)
{
  isUnsat = false;
  FormulaClass::config = config;
}

ClauseDatabase::~ClauseDatabase()
{

}


bool ClauseDatabase::operator==(const ClauseDatabase& other) const
{
  return false;
}

// this is adapted code from minisat+
int32_t ClauseDatabase::polarityClausify(const Formula& f, AuxVarManager& aux_vars )
{
    Lit result = 0;
    std::unordered_map<int32_t,Lit>::const_iterator it;
    
    assert (f != _undef_);
    
    if (isAtom(f))
    {
	result = getLit(f);
	assert(result != 0);
	return result;
    }
    else if ((it = history.find(f->data)) != history.end())
    {
      result = it->second;      
      assert(result != 0);
      return result;
    }
    else
    {
      assert (!isAtom(f));
      
    // TODO is this sound?
//       if ((it = history.find((~f)->data)) != history.end())
//       {
// 	result = -it->second;
// 	assert(result != 0);
//       }
//       else
	result = aux_vars.getVariable();
      
      if ((f == _true_) || (f == _false_))
      {
	addClause(result); // if f == _false_ ... result will be negated later on
      }
      else if (isAND(f))
      {
	assert(f->input_nodes.size() > 1);
	if (f->input_nodes.size() == 2)
	{
	  if (!isNeg(f))
	  {
	    addClause(-result, polarityClausify(left(f), aux_vars));
	    addClause(-result, polarityClausify(right(f), aux_vars));
	  }
	  else
	  {
	    addClause(result, polarityClausify( ~left(f), aux_vars), polarityClausify(~right(f), aux_vars));
	  }
	}
	else
	{
	  assert(f->input_nodes.size() > 2);
	  if (!isNeg(f))
	  {
	    for (int i = 0; i < f->input_nodes.size(); ++i)
	    {
	      addClause(-result, polarityClausify(f->input_nodes[i], aux_vars));
	    }
	  }
	  else
	  {
	    clause.clear();
	    clause.push_back(result);
	    for (int i = 0; i < f->input_nodes.size(); ++i)
	    {
	      clause.push_back(polarityClausify(~ (f->input_nodes[i]) , aux_vars));
	    }
	    addClause(clause);
	    clause.clear();
	  }
	}
      }
      else if (isITE(f))
      {
	    Lit  s = polarityClausify( selector(f), aux_vars);
            Lit ns = polarityClausify(~selector(f), aux_vars);

            if (!isNeg(f))
	    {
	      Lit  a = polarityClausify( true_branch(f), aux_vars);
	      Lit  b = polarityClausify( false_branch(f), aux_vars);
	      addClause(-result, ns, a);
	      addClause(-result, s, b);
	      addClause(-result, a, b);
            }
            else
	    {
	      Lit na = polarityClausify(~true_branch(f), aux_vars);
	      Lit nb = polarityClausify(~false_branch(f), aux_vars);
	      addClause( result, ns, na);
	      addClause( result,  s, nb);
	      addClause(na, nb,  result);
            }
      }
      else if (isEquiv(f))
      {
	Lit l  = polarityClausify( left (f), aux_vars);
	Lit r  = polarityClausify( right(f), aux_vars);
	Lit nl = polarityClausify(~left (f), aux_vars);
	Lit nr = polarityClausify(~right(f), aux_vars);

	if (!isNeg(f))
	{
	  addClause(-result, nl,  r);
	  addClause(-result,  l, nr);
	}
	else
	{
	  addClause( result, nl, nr);
	  addClause( result,  l,  r);
	}
      }
      else if (isMonotonicITE(f))
      {
	assert(!isNeg(f));
	Lit  s = polarityClausify( selector(f), aux_vars);
	Lit  a = polarityClausify( true_branch(f), aux_vars);
        Lit  b = polarityClausify( false_branch(f), aux_vars);
	addClause(b, -result, 0);
	addClause(a, -s, -result, 0);
      } 
      else
      {
	cerr << "Error cannot encode Formula: " << endl;
	printFormulaBits(f);
	exit(-1);
	assert(false);
      }
      
      if (isNeg(f))
	result = -result;
      
      history[f->data] = result;
    }

    assert(result != 0);
    return result;
}


CountingClauseDatabase::CountingClauseDatabase(PBConfig config) : ClauseDatabase(config), number_of_clauses(0)
{

}

void CountingClauseDatabase::addClauseIntern(vector< int32_t > const & clause)
{
  number_of_clauses++;
}

void CountingClauseDatabase::clearDataBase()
{
  number_of_clauses = 0;
}

int64_t CountingClauseDatabase::getNumberOfClauses()
{
  return number_of_clauses;
}

