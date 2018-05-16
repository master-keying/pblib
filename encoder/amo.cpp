#include "amo.h"

using namespace std;

void AMO_Encoder::encode_intern(vector< Lit >& literals, ClauseDatabase& formula, AuxVarManager& auxvars)
{
    if( literals.size() <= 4 )
    {
	for( uint32_t i = 0 ; i + 1 < literals.size(); ++ i )
	{
	  for( uint32_t j = i+1; j < literals.size(); ++ j )
	  {
	    formula.addClause(-literals[i], -literals[j]);
	  }
	}
    }
    else
    {
	vector< Lit > l1;
	vector< Lit > l2;

	uint32_t i = 0;
	for( ; i < literals.size() / 2; ++ i )
	{
	  l1.push_back( literals[i] );
	}
	for( ; i < literals.size() ; ++ i )
	{
	  l2.push_back( literals[i] );
	}

	Lit newVariable = auxvars.getVariable();
	l1.push_back( newVariable );
	l2.push_back( -newVariable );
	encode_intern( l1 , formula, auxvars);
	encode_intern( l2 , formula, auxvars);
    }
}

int64_t AMO_Encoder::clauseCount(int32_t n)
{
  if (n <= 1)
    return 0;

  if (n == 2)
    return 1;

  if (n == 3)
    return 3;

  if (n == 4)
    return 6;

  litCount++;
  return clauseCount(floor((double)n/2)+1) + clauseCount(ceil((double)n/2)+1);
}



int64_t AMO_Encoder::encodingValue(const SimplePBConstraint& pbconstraint)
{
  // clauses:  3*n - 6
  // aux vars: 0.5*n
  litCount = 0;
  int64_t value = valueFunction(clauseCount(pbconstraint.getN()), litCount);
  litCount = 0;

  return value;
}



void AMO_Encoder::encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars)
{
  formula.addConditionals(pbconstraint.getConditionals());

  if (config->print_used_encodings)
    cout << "c encode with nested amo encoder" << endl;
  if (pbconstraint.getComparator() == PBLib::BOTH && (pbconstraint.getGeq() == 1) )
  {
    assert(pbconstraint.getGeq() == 1 && pbconstraint.getLeq() == 1);

    encodeEq(pbconstraint, formula, auxvars);
  }
  _literals.clear();

  for (int i = 0; i < (int) pbconstraint.getN(); ++i)
    _literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);

  encode_intern(_literals, formula, auxvars);

  for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
    formula.getConditionals().pop_back();
}
void AMO_Encoder::encodeEq(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars)
{
	_literals.clear();

	for(uint i = 0;i < pbconstraint.getWeightedLiterals().size();++i){
		_literals.push_back(pbconstraint.getWeightedLiterals()[i].lit);
	}
	formula.addClause(_literals);
}

AMO_Encoder::AMO_Encoder(PBConfig& config) : Encoder(config)
{

}

AMO_Encoder::~AMO_Encoder()
{

}
