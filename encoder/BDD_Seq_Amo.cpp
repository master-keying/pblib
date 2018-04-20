#include "BDD_Seq_Amo.h"

using namespace std;

BDD_Seq_Amo::BDD_Seq_Amo(PBConfig& config) : Encoder(config)
{

}




void BDD_Seq_Amo::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  formula.addConditionals(pbconstraint.getConditionals());

  if (config->print_used_encodings)
    cout << "c encode with BDD Seq Amo" << endl;

  _literals.clear();

  for (int i = 0; i < (int) pbconstraint.getN(); ++i)
  {
    _literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);
  }
  
    if (pbconstraint.getComparator() == PBLib::BOTH && (pbconstraint.getGeq() == 1) )
  {
    assert(pbconstraint.getGeq() == 1 && pbconstraint.getLeq() == 1);
    formula.addClause(_literals);
  }
 
  
  
  encode_intern(_literals, formula, auxvars);
  
  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}

int64_t BDD_Seq_Amo::encodingValue(const SimplePBConstraint& pbconstraint)
{
  int64_t clauses = (pbconstraint.getWeightedLiterals().size() - 2) * 3 + 2;
  int64_t auxvars = (pbconstraint.getWeightedLiterals().size() - 1) ;
  
  return valueFunction(clauses, auxvars);
}

void BDD_Seq_Amo::encode_intern(vector< Lit >& literals, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  if (literals.size() == 1)
    return;
  
  aux.clear();
  for (int i = 1; i < (int) literals.size();++i)
  {
    aux.push_back(auxvars.getVariable());
  }
  
  for (int i = 0; i < literals.size() - 2; ++i)
  {
    formula.addClause(aux[i], -literals[i]);
    formula.addClause(-aux[i], aux[i+1]);
    formula.addClause(-aux[i], -literals[i+1]);
  }
  
  assert(literals.size() > 1);
  formula.addClause(aux[literals.size() - 2], -literals[literals.size() - 2]);
  formula.addClause(-aux[literals.size() - 2], -literals[literals.size() - 1]);
}

BDD_Seq_Amo::~BDD_Seq_Amo()
{

}
