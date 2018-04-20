#include "incpbconstraint.h"
using namespace PBLib;
using namespace std;


int IncPBConstraint::getN() const
{
  return weighted_literals.size();
}


PBConstraint IncPBConstraint::getNonIncConstraint()
{
  if (comparator == LEQ)
    return PBConstraint(weighted_literals, comparator, leq);
  else if (comparator == GEQ)
    return PBConstraint(weighted_literals, comparator, geq);
  else
    return PBConstraint(weighted_literals, comparator, leq, geq);
}


Comparator IncPBConstraint::getComparator() const
{
  return comparator;
}

IncPBConstraint::IncPBConstraint() : comparator(BOTH), leq(0), geq(0), init_leq(leq), init_geq(geq), isDualEncoded(false)
{

}



IncPBConstraint::IncPBConstraint(vector<WeightedLit> const & literals, Comparator comparator, int64_t less_eq, int64_t greater_eq) : weighted_literals(literals), comparator(comparator), leq(less_eq), geq(greater_eq), init_leq(leq), init_geq(geq), isDualEncoded(false)
{
  assert(comparator == BOTH);
}
IncPBConstraint::IncPBConstraint(vector<WeightedLit> const & literals, Comparator comparator, int64_t bound) : weighted_literals(literals), comparator(comparator), leq(0), geq(0), init_leq(leq), init_geq(geq), isDualEncoded(false)
{
  assert(comparator != BOTH);
  if (comparator == LEQ)
  {
    init_leq = leq = bound;
  }
  else 
  {
    assert(comparator == GEQ);
    init_geq = geq = bound;
  }
}

int64_t IncPBConstraint::getGeq() const
{
  return geq;
}

int64_t IncPBConstraint::getLeq() const
{
  return leq;
}

vector< WeightedLit > const & IncPBConstraint::getWeightedLiterals() const
{
  return weighted_literals;
}

void IncPBConstraint::setIncSimplePBConstraint(shared_ptr< IncSimplePBConstraint > incSimplePBConstraint)
{
  assert(isDualEncoded == false);
  inc_simple_pb_constraint = incSimplePBConstraint;
}

void IncPBConstraint::setGeqIncSimplePBConstraint(shared_ptr< IncSimplePBConstraint > geqIncSimplePBConstraint)
{
  isDualEncoded = true;
  geq_inc_simple_pb_constraint = geqIncSimplePBConstraint;
}

void IncPBConstraint::setLeqIncSimplePBConstraint(shared_ptr< IncSimplePBConstraint > leqIncSimplePBConstraint)
{
  isDualEncoded = true;
  leq_inc_simple_pb_constraint = leqIncSimplePBConstraint;
}


void IncPBConstraint::encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager & auxVars)
{
  if (newGeq <= init_geq)
    return;

  geq = newGeq;
  
  if (isDualEncoded)
  {
    assert(comparator == BOTH);
    assert(geq_inc_simple_pb_constraint->getComparator() == LEQ);
    geq_inc_simple_pb_constraint->encodeNewLeq(-geq, formula, auxVars); // we encode GEQ as LEQ internaly 
  }
  else
  {
    if (comparator == BOTH)
      inc_simple_pb_constraint->encodeNewGeq(geq, formula, auxVars);
    else
    {
      assert(comparator == GEQ);
      inc_simple_pb_constraint->encodeNewLeq(-geq, formula, auxVars); // we encode GEQ as LEQ internaly 
    }
  }
}

void IncPBConstraint::encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  if (newLeq >= init_leq)
    return;

  leq = newLeq;
  
  if (isDualEncoded)
  {
    assert(comparator == BOTH);
    assert(leq_inc_simple_pb_constraint->getComparator() == LEQ);
    leq_inc_simple_pb_constraint->encodeNewLeq(leq, formula, auxVars);
  }
  else
  {
    inc_simple_pb_constraint->encodeNewLeq(leq, formula, auxVars);
  }
}



IncPBConstraint::~IncPBConstraint()
{

}

bool IncPBConstraint::operator==(const IncPBConstraint& other) const
{
  return false;
}

IncPBConstraint IncPBConstraint::getGeqConstraint() const
{
  assert(comparator == BOTH);
  return IncPBConstraint(weighted_literals, GEQ, geq);
}


IncPBConstraint IncPBConstraint::getLeqConstraint() const
{
  assert(comparator == BOTH);
  return IncPBConstraint(weighted_literals, LEQ, leq);
}

shared_ptr< IncSimplePBConstraint > IncPBConstraint::getIncSimplePBConstraint()
{
  return inc_simple_pb_constraint;
}

void IncPBConstraint::print() const
{
  bool stderr = false;

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

void IncPBConstraint::addConditional(int32_t lit)
{
  conditionals.push_back(lit);
}

void IncPBConstraint::addConditionals(vector< int32_t > lits)
{
  for (int32_t lit : lits)
    conditionals.push_back(lit);
}

void IncPBConstraint::clearConditionals()
{
  conditionals.clear();
}

const vector< int32_t >& IncPBConstraint::getConditionals() const
{
  return conditionals;
}

void IncPBConstraint::setComparator(Comparator comp)
{
  comparator = comp;
}
