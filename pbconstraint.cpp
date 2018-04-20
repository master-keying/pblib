#include "pbconstraint.h"
#include <iostream>

using namespace PBLib;
using namespace std;

void PBConstraint::addConditional(int32_t lit)
{
  conditionals.push_back(lit);
}

void PBConstraint::addConditionals(vector< int32_t > lits)
{
  for (int32_t lit : lits)
    conditionals.push_back(lit);
}


void PBConstraint::clearConditionals()
{
  conditionals.clear();
}

const vector< int32_t >& PBConstraint::getConditionals() const
{
  return conditionals;
}


int PBConstraint::getN() const
{
  return weighted_literals.size();
}


void PBConstraint::setComparator(Comparator _comparator)
{
  comparator = _comparator;
}

void PBConstraint::setGeq(int64_t _geq)
{
    geq = _geq;
}

void PBConstraint::setLeq(int64_t _leq)
{
  leq = _leq;
}



int64_t PBConstraint::getMaxSum() const
{
  int64_t maxsum = 0;
  for (int i = 0; i < weighted_literals.size(); ++i)
    if (weighted_literals[i].weight >= 0)
      maxsum += weighted_literals[i].weight;
    
  return maxsum;
}

int64_t PBConstraint::getMinSum() const
{
  int64_t minsum = 0;
  for (int i = 0; i < weighted_literals.size(); ++i)
    if (weighted_literals[i].weight < 0)
      minsum += weighted_literals[i].weight;
    
  return minsum;
}



Comparator PBConstraint::getComparator() const
{
  return comparator;
}

PBConstraint PBConstraint::getGeqConstraint() const 
{
  assert(comparator == BOTH);
  PBConstraint c(weighted_literals, GEQ, geq);
  c.addConditionals(conditionals);
  return c;
}

PBConstraint PBConstraint::getLeqConstraint() const
{
  assert(comparator == BOTH);
  PBConstraint c(weighted_literals, LEQ, leq);
  c.addConditionals(conditionals);
  return c;
}

PBConstraint::PBConstraint() :  comparator(LEQ), leq(0), geq(0)
{
  
}

PBConstraint::PBConstraint(vector<WeightedLit> const & literals, Comparator comparator, int64_t less_eq, int64_t greater_eq) : weighted_literals(literals), comparator(comparator), leq(less_eq), geq(greater_eq)
{
  assert(comparator == BOTH);
}
PBConstraint::PBConstraint(vector<WeightedLit> const & literals, Comparator comparator, int64_t bound) : weighted_literals(literals), comparator(comparator), leq(0), geq(0)
{
  if (comparator == LEQ)
    leq = bound;
  else if (comparator == GEQ)
  {
    geq = bound;
  }
  else
  {
    assert(comparator == BOTH);
    geq = leq = bound;
  }
  
}

int64_t PBConstraint::getGeq() const
{
  return geq;
}

int64_t PBConstraint::getLeq() const
{
  return leq;
}

vector< WeightedLit > const & PBConstraint::getWeightedLiterals() const
{
  return weighted_literals;
}

vector< WeightedLit > & PBConstraint::getWeightedLiterals() 
{
  return weighted_literals;
}


PBConstraint::~PBConstraint()
{

}

bool PBConstraint::operator==(const PBConstraint& other) const
{
  return false;
}


//TODO rewrite print functions ... 

void PBConstraint::print(bool stderr) const
{

  if (getN() == 0)
  {
    if (stderr)
      cerr << "TRUE" << endl;
    else
      cout << "TRUE" << endl;
    return;
  }
  
  if (conditionals.size() > 0)
  {
      if (stderr)
	cerr << "[";
      else
	cout << "[";
      
    for (int i = 0; i < conditionals.size(); ++i)
    {
      if (stderr)
	cerr << conditionals[i] << ",";
      else
	cout << conditionals[i] << ",";
    }
    
    if (stderr)
	cerr << "] => ";
      else
	cout << "] => ";
  }
  
  for (int i = 0; i < getN(); ++i)
  {
    if (i < getN() - 1)
    {
      if (weighted_literals[i].lit < 0)
	if (stderr) cerr << weighted_literals[i].weight << " -x" << -weighted_literals[i].lit << " +";
	else  cout << weighted_literals[i].weight << " -x" << -weighted_literals[i].lit << " +";
      else
	if (stderr) cerr << weighted_literals[i].weight << " x" << weighted_literals[i].lit << " +";
	else cout << weighted_literals[i].weight << " x" << weighted_literals[i].lit << " +";
    }
    else
    {
      if (weighted_literals[getN() - 1].lit < 0)
	if (stderr) cerr << weighted_literals[getN() - 1].weight << " -x" << -weighted_literals[getN() - 1].lit;
	else cout << weighted_literals[getN() - 1].weight << " -x" << -weighted_literals[getN() - 1].lit;
      else
	if(stderr) cerr << weighted_literals[getN() - 1].weight << " x" << weighted_literals[getN() - 1].lit;
	else cout << weighted_literals[getN() - 1].weight << " x" << weighted_literals[getN() - 1].lit;
    }
  }
  
  
  if (comparator == LEQ)
    if(stderr) cerr << " =< " << leq << endl;
    else cout << " =< " << leq << endl;
  else if (comparator == GEQ)
    if(stderr) cerr << " >= " << geq << endl;
    else cout << " >= " << geq << endl;
  else
    if(stderr) cerr << " >= " << geq << " =< " << leq << endl;
    else cout << " >= " << geq << " =< " << leq << endl;
  
}

void PBConstraint::printGeq(bool stderr) const
{
  if (comparator != LEQ)
  {
    print();
    return;
  }
  
  if (getN() == 0)
  {
    if (stderr)
      cerr << "TRUE" << endl;
    else
      cout << "TRUE" << endl;
    return;
  }
  
  if (stderr) cerr << "-";
  else  cout << "-";
  
  for (int i = 0; i < getN(); ++i)
  {
    if (i < getN() - 1)
    {
      if (weighted_literals[i].lit < 0)
	if (stderr) cerr << weighted_literals[i].weight << " ~x" << -weighted_literals[i].lit << " -";
	else  cout << weighted_literals[i].weight << " ~x" << -weighted_literals[i].lit << " -";
      else
	if (stderr) cerr << weighted_literals[i].weight << " x" << weighted_literals[i].lit << " -";
	else cout << weighted_literals[i].weight << " x" << weighted_literals[i].lit << " -";
    }
    else
    {
      if (weighted_literals[getN() - 1].lit < 0)
	if (stderr) cerr << weighted_literals[getN() - 1].weight << " ~x" << -weighted_literals[getN() - 1].lit;
	else cout << weighted_literals[getN() - 1].weight << " ~x" << -weighted_literals[getN() - 1].lit;
      else
	if(stderr) cerr << weighted_literals[getN() - 1].weight << " x" << weighted_literals[getN() - 1].lit;
	else cout << weighted_literals[getN() - 1].weight << " x" << weighted_literals[getN() - 1].lit;
    }
  }
  
  
  assert(comparator == LEQ);
  
  if(stderr) cerr << " >= " << -leq << " ;" << endl;
  else cout << " >= " << -leq << " ;" << endl;
  
  
}

void PBConstraint::printNoNL(bool stderr) const
{
  if (getN() == 0)
  {
    if (stderr)
      cerr << "TRUE" << " ";
    else
      cout << "TRUE" << " ";
    return;
  }
  
  for (int i = 0; i < getN(); ++i)
  {
    if (i < getN() - 1)
    {
      if (weighted_literals[i].lit < 0)
	if (stderr) cerr << weighted_literals[i].weight << " -x" << -weighted_literals[i].lit << " +";
	else  cout << weighted_literals[i].weight << " -x" << -weighted_literals[i].lit << " +";
      else
	if (stderr) cerr << weighted_literals[i].weight << " x" << weighted_literals[i].lit << " +";
	else cout << weighted_literals[i].weight << " x" << weighted_literals[i].lit << " +";
    }
    else
    {
      if (weighted_literals[getN() - 1].lit < 0)
	if (stderr) cerr << weighted_literals[getN() - 1].weight << " -x" << -weighted_literals[getN() - 1].lit;
	else cout << weighted_literals[getN() - 1].weight << " -x" << -weighted_literals[getN() - 1].lit;
      else
	if(stderr) cerr << weighted_literals[getN() - 1].weight << " x" << weighted_literals[getN() - 1].lit;
	else cout << weighted_literals[getN() - 1].weight << " x" << weighted_literals[getN() - 1].lit;
    }
  }
  
  
  if (comparator == LEQ)
    if(stderr) cerr << " =< " << leq << " ";
    else cout << " =< " << leq << " ";
  else if (comparator == GEQ)
    if(stderr) cerr << " >= " << geq << " ";
    else cout << " >= " << geq << " ";
  else
    if(stderr) cerr << " >= " << geq << " =< " << leq << " ";
    else cout << " >= " << geq << " =< " << leq << " ";
  
}
