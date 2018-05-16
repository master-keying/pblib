// this is adapted code from minisat+
#include "adderencoding.h"
#include "../helper.h"

using namespace std;

AdderEncoding::AdderIncData::AdderIncData(vector< int32_t > result) : result(result)
{

}

void AdderEncoding::AdderIncData::encodeNewGeq(int64_t newGeq, ClauseDatabase& formula, AuxVarManager& auxVars, vector<int32_t> conditionals)
{
  formula.addConditionals(conditionals);

  numToBits ( kBits, result.size(), newGeq );
  assert(kBits.size() == result.size());
  for (int i = 0; i < kBits.size(); ++i) // negate everythink
  {
    kBits[i] = kBits[i] == 0 ? 1 : 0;
    result[i] = -result[i];
  }
  lessThanOrEqual ( result, kBits , formula);
  for (int i = 0; i < kBits.size(); ++i)
  {
    result[i] = -result[i]; // reset result vector
  }

  for (int i = 0; i < conditionals.size(); ++i)
      formula.getConditionals().pop_back();
}

void AdderEncoding::AdderIncData::encodeNewLeq(int64_t newLeq, ClauseDatabase& formula, AuxVarManager& auxVars, vector<int32_t> conditionals)
{
  formula.addConditionals(conditionals);

  numToBits ( kBits, result.size(), newLeq );
  assert(kBits.size() == result.size());
  lessThanOrEqual ( result, kBits , formula);

  for (int i = 0; i < conditionals.size(); ++i)
      formula.getConditionals().pop_back();
}

AdderEncoding::AdderIncData::~AdderIncData()
{

}


void AdderEncoding::FA_extra ( int32_t xc, int32_t xs, int32_t a, int32_t b, int32_t c )
{

  formula->addClause(-xc, -xs, a);
  formula->addClause(-xc, -xs, b);
  formula->addClause(-xc, -xs, c);

  formula->addClause(xc, xs, -a);
  formula->addClause(xc, xs, -b);
  formula->addClause(xc, xs, -c);

}


int32_t AdderEncoding::FA_carry ( int32_t a, int32_t b, int32_t c ) {

  int32_t x = auxVars->getVariable();

  formula->addClause( b,c,-x,0 );
  formula->addClause( a,c,-x,0 );
  formula->addClause( a,b,-x,0 );

  formula->addClause( -b,-c,x,0 );
  formula->addClause( -a,-c,x,0 );
  formula->addClause( -a,-b,x,0 );

  return x;
}

int32_t AdderEncoding::FA_sum ( int32_t a, int32_t b, int32_t c )
{
    int32_t x = auxVars->getVariable();

    formula->addClause( a,b,c,-x,0 );
    formula->addClause( a,-b,-c,-x,0 );
    formula->addClause( -a,b,-c,-x,0 );
    formula->addClause( -a,-b,c,-x,0 );

    formula->addClause( -a,-b,-c,x,0 );
    formula->addClause( -a,b,c,x,0 );
    formula->addClause( a,-b,c,x,0 );
    formula->addClause( a,b,-c,x,0 );

    return x;
}

int32_t AdderEncoding::HA_carry ( int32_t a, int32_t b ) // a AND b
{
  int32_t x = auxVars->getVariable();

  formula->addClause( a, -x, 0 );
  formula->addClause( b, -x, 0 );

  formula->addClause( -a, -b, x, 0 );
  return x;
}

int32_t AdderEncoding::HA_sum ( int32_t a, int32_t b ) // a XOR b
{
  int32_t x = auxVars->getVariable();

  formula->addClause( -a,-b,-x,0 );
  formula->addClause( a,b,-x,0 );

  formula->addClause( -a,b,x,0 );
  formula->addClause( a,-b,x,0 );

  return x;
}

void AdderEncoding::adderTree ( vector< queue< int32_t > > & buckets, vector< int32_t >& result ) {
  int32_t x,y,z;

  for ( int i = 0; i < buckets.size(); i++ ) {
      if ( buckets[i].size() == 0 )
	  continue;

      if ( i == buckets.size() - 1 && buckets[i].size() >= 2 ) {
	  buckets.push_back ( queue<int32_t>() );
	  result.push_back ( 0 );
	  }

      while ( buckets[i].size() >= 3 ) {
	  x = buckets[i].front();
	  buckets[i].pop();
	  y = buckets[i].front();
	  buckets[i].pop();
	  z = buckets[i].front();
	  buckets[i].pop();
	  int32_t xs = FA_sum ( x,y,z );
	  int32_t xc = FA_carry ( x,y,z );
	  buckets[i  ].push ( xs );
	  buckets[i+1].push ( xc );
	  FA_extra(xc, xs, x, y, z);
	  }

      if ( buckets[i].size() == 2 ) {
	  x = buckets[i].front();
	  buckets[i].pop();
	  y = buckets[i].front();
	  buckets[i].pop();
	  buckets[i  ].push ( HA_sum ( x,y ) );
	  buckets[i+1].push ( HA_carry ( x,y ) );
	  }


      result[i] = buckets[i].front();
      buckets[i].pop();
      }

  }

// Generates clauses for “xs <= ys”, assuming ys has only constant signals (0 or 1).
// xs and ys must have the same size

void AdderEncoding::lessThanOrEqual ( vector< int32_t > & xs, vector< int32_t > & ys , ClauseDatabase & formula) {
  assert ( xs.size() == ys.size() );
  vector<Lit> clause;
  bool skip;
  for ( int i = 0; i < xs.size(); ++i ) {
      if ( ys[i] == 1 || xs[i] == 0 )
	  continue;

      clause.clear();

      skip = false;

      for ( int j = i + 1; j < xs.size(); ++j )
      {
	  if ( ys[j] == 1 )
	  {
	      if ( xs[j] == 0 )
	      {
		  skip = true;
		  break;
	      }

	      clause.push_back ( -xs[j] );
	  }
	  else
	  {
	      assert ( ys[j] == 0 );

	      if ( xs[j] == 0 )
		  continue;

	      clause.push_back ( xs[j] );
	  }
      }

      if ( skip )
	  continue;

      clause.push_back ( -xs[i] );

      formula.addClause( clause );
      }

}



void AdderEncoding::numToBits ( vector<int32_t> & bits, int64_t n, int64_t number ) {
  bits.clear();


  for ( int64_t i = n - 1; i >= 0; --i ) {
      int64_t tmp = ((int64_t)1) << i;
      if ( number < tmp ) {
	  bits.push_back ( 0 );
	  }
      else {
	  bits.push_back ( 1 );
	  number -= tmp;
	  }
      }

  reverse ( bits.begin(), bits.end() );
}

// Generates units for "result == kBits", assuming kBits of them has only constant signals (0 or 1).
// result and kBits must have the same size
void AdderEncoding::resultIsEqual ( vector< int32_t > & result, vector< int32_t > & kBits ) {
  assert (kBits.size() == result.size());

  for (int i = 0; i < result.size(); ++i) {
    if (kBits[i] == 1) {
      if (result[i] == 0) {
	formula->addUnsat();
	return;
      }
      formula->addClause(result[i],0);
    }
    else
    {
      assert (kBits[i] == 0);
      if (result[i] != 0) // if result[i] == 0 -> -0 = true -> we do not have to add this
	formula->addClause(-result[i],0);

    }
  }
}

void AdderEncoding::encode(const shared_ptr< IncSimplePBConstraint >& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxvars)
{
  if (config->print_used_encodings)
    cout << "c encode incremental with adder" << endl;

  isInc = true;
  encode(*pbconstraint, formula, auxvars);
  pbconstraint->setIncrementalData(make_shared<AdderIncData>(result));
  isInc = false;
}

int PBLib::ld64(const uint64_t x)
{
  return (sizeof(uint64_t) << 3) - __builtin_clzll (x);
//   cout << "x " << x << endl;
//   int ldretutn = 0;
//   for (int i = 0; i < 63; ++i)
//   {
//     if ((x & (1 << i)) > 0)
//     {
//       cout << "ldretutn " << ldretutn << endl;
//       ldretutn = i + 1;
//     }
//   }
//
//   return ldretutn;
}

void AdderEncoding::encode ( const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars ) {



    if (config->print_used_encodings && !isInc)
      cout << "c encode with adder" << endl;

    this->formula = &formula;
    this->auxVars = &auxvars;


    vector<queue<int32_t> > buckets;
    result.clear();
    vector<int32_t> rhs;

    int64_t nb = PBLib::ld64(pbconstraint.getLeq()); // number of bits


    for ( int iBit = 0; iBit < nb; ++iBit ) {
        buckets.push_back ( queue<int32_t>() );
        result.push_back ( 0 );
        for ( int iVar = 0; iVar < pbconstraint.getWeightedLiterals().size(); ++iVar ) {
            if ( ( ( ((int64_t)1) << iBit ) & pbconstraint.getWeightedLiterals()[iVar].weight ) != 0 )
                buckets.back().push ( pbconstraint.getWeightedLiterals()[iVar].lit );
            }
        }


    vector<int32_t> kBits;

    adderTree ( buckets, result );

    numToBits ( kBits, buckets.size(), pbconstraint.getLeq() );

    formula.addConditionals(pbconstraint.getConditionals());

    if (pbconstraint.getComparator() == PBLib::BOTH)
    {
      if (!isInc && pbconstraint.getLeq() == pbconstraint.getGeq())
	resultIsEqual(result, kBits);
      else
      {
	int32_t true_lit = auxvars.getVariable();
	formula.addClause(true_lit);
	lessThanOrEqual ( result, kBits , formula);
	numToBits ( kBits, buckets.size(), pbconstraint.getGeq() );
	assert(kBits.size() == result.size());
	for (int i = 0; i < kBits.size(); ++i) // negate everythink
	{
	  kBits[i] = kBits[i] == 0 ? 1 : 0;
	  result[i] = result[i] == 0 ? true_lit : -result[i];
	}
	lessThanOrEqual ( result, kBits , formula);
	for (int i = 0; i < kBits.size(); ++i) // negate everythink
	{
	  result[i] = -result[i]; // reset result vector
	}
      }
    }
    else
    {
      lessThanOrEqual ( result, kBits , formula);
    }

    for (int i = 0; i < pbconstraint.getConditionals().size(); ++i)
      formula.getConditionals().pop_back();

}


int64_t AdderEncoding::encodingValue(const shared_ptr< IncSimplePBConstraint >& pbconstraint)
{
  return config->MAX_CLAUSES_PER_CONSTRAINT - 1; // since adder encoding is not GAC we use this as fallback only
}

int64_t AdderEncoding::encodingValue(const SimplePBConstraint& pbconstraint)
{
  return config->MAX_CLAUSES_PER_CONSTRAINT - 1;
}





















