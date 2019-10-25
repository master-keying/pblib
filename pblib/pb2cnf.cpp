#include "pb2cnf.h"
#include <math.h>

#include <algorithm>
#include <memory>

using namespace PBLib;
using namespace std;

////////////////////////////////////////////////////////////////////
//        Encoding for incremental PB constraints                 //
////////////////////////////////////////////////////////////////////
void PB2CNF::encodeIncInital(IncPBConstraint& incPbconstraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{

  shared_ptr<IncSimplePBConstraint> constraint = pre_encoder.preEncodeIncPBConstraint(incPbconstraint, formula);

  vector<int32_t> geqOneClause; // for the amo constraint

  switch(constraint->getType())
  {
    case DONTCARE:
      constraint->setIncrementalData(make_shared<IncrementalDontCare>());
      break;
    case AMO:
      for (auto wlits : constraint->getWeightedLiterals())
	geqOneClause.push_back(wlits.lit);

      constraint->setIncrementalData(make_shared<AMOIncrementalData>(geqOneClause));
      encode_amo(*constraint, formula, auxVars);
      break;
    case AMK:
      encode_inc_amk(constraint, incPbconstraint, formula, auxVars);
      break;
    case PB:
      encode_inc_pb(constraint, incPbconstraint, formula, auxVars);
      break;
    default:
      assert(false && "this should never occure");
  }
}


void PB2CNF::encode_inc_with_adder(shared_ptr< IncSimplePBConstraint > constraint, ClauseDatabase & formula, AuxVarManager & auxVars)
{
  stats->num_adder_encodings++;
  adder_encoder.encode(constraint, formula, auxVars);
}

void PB2CNF::encode_inc_with_swc(shared_ptr< IncSimplePBConstraint > constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  swc_encoder.encode(constraint, formula, auxVars);
}



void PB2CNF::encode_inc_amk(shared_ptr< IncSimplePBConstraint > constraint, IncPBConstraint&, ClauseDatabase & formula, AuxVarManager & auxVars)
{
  if (card_encoder.encodingValue(constraint) < config->MAX_CLAUSES_PER_CONSTRAINT ) // aprox number of card clauses
    encode_inc_with_card(constraint, formula, auxVars);
  else
    encode_inc_with_adder(constraint, formula, auxVars);
}

void PB2CNF::encode_inc_pb(shared_ptr< IncSimplePBConstraint > constraint, IncPBConstraint&, ClauseDatabase & formula, AuxVarManager & auxVars)
{
  if(config->pb_encoder == PB_ENCODER::ADDER)
  {
    encode_inc_with_adder(constraint, formula, auxVars);
    return;
  }

  if(config->pb_encoder == PB_ENCODER::SWC)
  {
    encode_inc_with_swc(constraint, formula, auxVars);
    return;
  }

  if (swc_encoder.encodingValue(constraint) < config->MAX_CLAUSES_PER_CONSTRAINT ) // aprox number of card clauses
    encode_inc_with_swc(constraint, formula, auxVars);
  else
    encode_inc_with_adder(constraint, formula, auxVars);
}


void PB2CNF::encode_inc_with_card(shared_ptr< IncSimplePBConstraint > constraint, ClauseDatabase & formula, AuxVarManager & auxVars)
{
  stats->num_card_encodings++;
  card_encoder.encode(constraint, formula, auxVars);
}




////////////////////////////////////////////////////////////////////
//        Encoding for normal PB constraints                      //
////////////////////////////////////////////////////////////////////

bool PB2CNF::try_to_encode_in_threshold(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars, int clause_threshold)
{

  //TODO refactor this... and use encodingValue methode
	if (constraint.getType() == DONTCARE)
		return true;

	if (clause_threshold == 0)
		return false;

  if (constraint.getType() == AMO)
  {
	  if (constraint.getN() * 3 < clause_threshold)
	  {
		  encode_amo(constraint, formula, auxVars);
		  return true;
	  }
	  else
		  return false;
  }

  if (constraint.getType() == AMK)
  {
	  if ( (constraint.getN() * ceil(log2(constraint.getLeq())) * ceil(log2(constraint.getLeq()))) < clause_threshold)
	  {
		  encode_amk(constraint, formula, auxVars);
		  return true;
	  }
	  else
		  return false;
  }

  assert(constraint.getType() == PB);
	// since we cannot approx the number of clauses with BDD encoding on real PBs we try to encode the constraint within the limit,
  // if this is not possible we cancle the encoding and fallback to the adder encoding
  tmpFormula.clearDatabase();
  tmpVariables.clear();
  auxVars.startRememberReturnedVariables(&tmpVariables);
  bdd_encoder.bddEncode(constraint, tmpFormula, auxVars, false, clause_threshold);
  auxVars.stopRememerReturnedVariables();


  int64_t num_of_bin_merge_clauses = (constraint.getComparator() == BOTH ? 2 : 1) * constraint.getN() * ceil(log2(constraint.getLeq())) * ceil(log2(constraint.getLeq())) * ceil(log2(constraint.getMaxWeight()));
  if (bdd_encoder.wasToBig() || tmpFormula.getClauses().size() > num_of_bin_merge_clauses)
  {
    // BDD was to big
    auxVars.freeVariables(tmpVariables); // reset aux variables of tmp BDD encoding

    if ( num_of_bin_merge_clauses < clause_threshold)
	{
	  // use non gac variant
		bool tmp = config->use_gac_binary_merge;
		config->use_gac_binary_merge = false;
		encode_with_binary_merge(constraint, formula, auxVars);
		config->use_gac_binary_merge = tmp;
	}
    else
      return false;
  }
  else
  {
    // encode with BDD
    stats->num_bdd_gates_encodings++;
    formula.addClauses(tmpFormula.getClauses());
  }
  tmpFormula.clearDatabase();
  tmpVariables.clear();

	return false;
}


int32_t PB2CNF::encodeAtLeastK(const vector< int32_t >& literals, int64_t k, vector< vector< int32_t > >& formula, int32_t firstAuxiliaryVariable)
{
  vector<int64_t> weights;
  for (int i = 0; i < literals.size(); ++i)
    weights.push_back(1);

  return encodeGeq(weights, literals, k, formula, firstAuxiliaryVariable);
}


int32_t PB2CNF::encodeAtMostK(const vector< int32_t >& literals, int64_t k, vector< vector< int32_t > >& formula, int32_t firstAuxiliaryVariable)
{
  vector<int64_t> weights;
  for (int i = 0; i < literals.size(); ++i)
    weights.push_back(1);

  return encodeLeq(weights, literals, k, formula, firstAuxiliaryVariable);
}


int32_t PB2CNF::encodeGeq(const vector< int64_t >& weights, const vector< int32_t >& literals, int64_t geq, vector< vector< int32_t > >& formula, int32_t firstAuxiliaryVariable)
{
  assert(weights.size() == literals.size());

  if (literals.size() != weights.size())
  {
    cerr << "c [PBLib] error: size of weights differ from size of literals, can not encode constraint" << endl;
    return 0;
  }

  vector<WeightedLit> lits;
  for (int i = 0; i < weights.size(); ++i)
    lits.push_back(WeightedLit(literals[i], weights[i]));

  PBLib::PBConstraint constraint(lits, PBLib::GEQ, geq);
  VectorClauseDatabase clauseDatabase(config, &formula);
  AuxVarManager auxVars(firstAuxiliaryVariable);

  encode(constraint, clauseDatabase, auxVars);

  return auxVars.getBiggestReturnedAuxVar();
}

int32_t PB2CNF::encodeBoth(const vector< int64_t >& weights, const vector< int32_t >& literals, int64_t leq, int64_t geq, vector< vector< int32_t > >& formula, int32_t firstAuxiliaryVariable)
{
  assert(weights.size() == literals.size());

  if (literals.size() != weights.size())
  {
    cerr << "c [PBLib] error: size of weights differ from size of literals, can not encode constraint" << endl;
    return 0;
  }

  vector<WeightedLit> lits;
  for (int i = 0; i < weights.size(); ++i)
    lits.push_back(WeightedLit(literals[i], weights[i]));

  PBLib::PBConstraint constraint(lits, PBLib::BOTH, leq, geq);
  VectorClauseDatabase clauseDatabase(config, &formula);
  AuxVarManager auxVars(firstAuxiliaryVariable);

  encode(constraint, clauseDatabase, auxVars);

  return auxVars.getBiggestReturnedAuxVar();
}


int32_t PB2CNF::encodeLeq(const vector< int64_t >& weights, const vector< int32_t >& literals, int64_t leq, vector< vector< int32_t > >& formula, int32_t firstAuxiliaryVariable)
{
  assert(weights.size() == literals.size());

  if (literals.size() != weights.size())
  {
    cerr << "c [PBLib] error: size of weights differ from size of literals, can not encode constraint" << endl;
    return 0;
  }

  vector<WeightedLit> lits;
  for (int i = 0; i < weights.size(); ++i)
    lits.push_back(WeightedLit(literals[i], weights[i]));

  PBLib::PBConstraint constraint(lits, PBLib::LEQ, leq);
  VectorClauseDatabase clauseDatabase(config, &formula);
  AuxVarManager auxVars(firstAuxiliaryVariable);

  encode(constraint, clauseDatabase, auxVars);

  return auxVars.getBiggestReturnedAuxVar();
}



void PB2CNF::encode(const PBConstraint& pbconstraint, PBSATSolver& satsolver, AuxVarManager& auxVars, int clause_threshold)
{
	// encode with clause clause threshold
	VectorClauseDatabase formula(config);
	SimplePBConstraint constraint = pre_encoder.preEncodePBConstraint(pbconstraint, formula);

	for(auto clause : formula.getClauses())
		satsolver.addClause(clause);

	if (constraint.getType() == DONTCARE)
		return;

	if (try_to_encode_in_threshold(constraint, formula, auxVars, clause_threshold))
	{
		if (config->print_used_encodings)cout << "c debug: encoded in clauses" << endl;
		for(auto clause : formula.getClauses())
			satsolver.addClause(clause);
		return;
	}
	if (config->print_used_encodings) cout << "c debug: encoded native" << endl;




	if (constraint.getComparator() == BOTH)
	{
		encode(constraint.getLeqConstraint(), satsolver, auxVars, clause_threshold / 2);
		encode(constraint.getGeqConstraint(), satsolver, auxVars, clause_threshold / 2);
	}
	else
	{
	  if (constraint.getType() == AMO || constraint.getType() == AMK)
	  {
		  vector<int32_t> lits;
		  for (auto wlit : constraint.getWeightedLiterals())
			  lits.push_back(wlit.lit);

		  satsolver.addAtMost(lits, constraint.getLeq());
	  }
	  else
	  {
		  assert(constraint.getType() == PB);
		  vector<int32_t> lits;
		  vector<int64_t> weights;
		  for (auto wlit : constraint.getWeightedLiterals())
		  {
			  lits.push_back(wlit.lit);
			  weights.push_back(wlit.weight);
		  }

		  satsolver.addPBLeq(lits, weights, constraint.getLeq());
	  }
  }
}


void PB2CNF::encode(const PBConstraint& pbconstraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  SimplePBConstraint constraint = pre_encoder.preEncodePBConstraint(pbconstraint, formula);

  if (config->just_approximate)
  {
	  // experimental code
	  cout << "c warning, approximation is experimental code" << endl;
	  vector<WeightedLit> approx_lits;
	  long double new_max = config->approximate_max_value;
	  long double old_max = constraint.getMaxWeight();

	  for (WeightedLit lit : constraint.getWeightedLiterals())
	  {
		  approx_lits.push_back(WeightedLit(lit.lit,  floor((((long double)lit.weight) / old_max) * new_max)));
	  }


	  if (constraint.getComparator() == BOTH)
	  {
			long double new_geq = ceil(((long double)constraint.getGeq() / old_max) * new_max);
			long double new_leq = ceil(((long double)constraint.getLeq() / old_max) * new_max);
			constraint = pre_encoder.preEncodePBConstraint(PBConstraint(approx_lits, BOTH, new_leq, new_geq), formula);
	  }
	  else
	  {
			long double new_leq = ceil(((long double)constraint.getLeq() / old_max) * new_max);
			constraint = pre_encoder.preEncodePBConstraint(PBConstraint(approx_lits, LEQ, new_leq), formula);
	  }
  }

  switch(constraint.getType())
  {
    case DONTCARE:
      break;
    case AMO:
      encode_amo(constraint, formula, auxVars);
      break;
    case AMK:
      encode_amk(constraint, formula, auxVars);
      break;
    case PB:
      encode_pb(constraint, formula, auxVars);
      break;
    default:
      assert(false && "this should never occure");
  }
}

void PB2CNF::encode_amo(SimplePBConstraint & constraint, ClauseDatabase & formula, AuxVarManager & auxVars)
{
  stats->num_amo_encodings++;
  switch (config->amo_encoder)
  {
    using namespace AMO_ENCODER;
    case BDD:
      bdd_sec_amo.encode(constraint, formula, auxVars);
      break;
    case NESTED:
      amo_encoder.encode(constraint, formula, auxVars);
      break;
    case BIMANDER:
      bimander_amo_encoding.encode(constraint, formula, auxVars);
      break;
    case COMMANDER:
      commander_amo_encoding.encode(constraint, formula, auxVars);
      break;
    case KPRODUCT:
      k_product_encoer.encode(constraint, formula, auxVars);
      break;
    case BINARY:
      binary_amo_encoder.encode(constraint, formula, auxVars);
      break;
    case PAIRWISE:
      naive_amo_encoder.encode(constraint, formula, auxVars);
      break;
    case BEST:
      if (!encodeWithBestEncoder(vector<Encoder*>
			    {&bdd_sec_amo, &amo_encoder, &bimander_amo_encoding, &commander_amo_encoding, &k_product_encoer, &binary_amo_encoder, &naive_amo_encoder, &adder_encoder},
			    constraint, formula, auxVars)
      )
      {
	cerr << "c [pblib] error: could not encode a constraint" << endl;
	cout << "c [pblib] current constraint: "; constraint.printNoNL(); cout << endl;
	assert(false);
      }
      break;
    default:
      amo_encoder.encode(constraint, formula, auxVars);
  }
}


void PB2CNF::encode_amk(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{

  if (config->amk_encoder == AMK_ENCODER::CARD)
    encode_with_card(constraint, formula, auxVars);
  else
  if (config->amk_encoder == AMK_ENCODER::BDD)
    encode_with_bdd(constraint, formula, auxVars);
  else
  if (!encodeWithBestEncoder(vector<Encoder*>
			    {&card_encoder, &bdd_encoder, &adder_encoder},
			    constraint, formula, auxVars)
  )
  {
      cerr << "c [pblib] error: could not encode a constraint" << endl;
      cout << "c [pblib] current constraint: "; constraint.printNoNL(); cout << endl;
      assert(false);
  }


}

void PB2CNF::encode_with_binary_merge(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  if (constraint.getComparator() == BOTH)
  {
    // since the LEQ or GEQ could be a DONTCARE (result in an error) or a AMO / AMK constraint, we call the encode methode again (for an other preencoding)
    // and probably end up here again
    encode(constraint.getGeqConstraint(), formula, auxVars);
    encode(constraint.getLeqConstraint(), formula, auxVars);
  }
  else
  {
    binary_merge.encode(constraint, formula, auxVars);
  }
}



void PB2CNF::encode_pb(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  if(config->pb_encoder == PB_ENCODER::SORTINGNETWORKS)
  {
    sorting_networks.encode(constraint, formula, auxVars);
    return;
  }
  else
  if(config->pb_encoder == PB_ENCODER::ADDER)
  {
    encode_with_adder(constraint, formula, auxVars);
    return;
  }
  else
  if(config->pb_encoder == PB_ENCODER::BDD)
  {
    encode_with_bdd(constraint, formula, auxVars);
    return;
  }
  else
  if(config->pb_encoder == PB_ENCODER::BINARY_MERGE)
  {
    encode_with_binary_merge(constraint, formula, auxVars);
    return;
  }
  else
  if(config->pb_encoder == PB_ENCODER::SWC)
  {
    encode_with_swc(constraint, formula, auxVars);
    return;
  }
  else //TODO this way bdds are calculated twice: first time during clause counting, secound time if selected as best encoding
  if (!encodeWithBestEncoder(vector<Encoder*>
			    {&sorting_networks, &adder_encoder, &bdd_encoder, &binary_merge},
			    constraint, formula, auxVars)
  )
  {
      cerr << "c [pblib] error: could not encode a constraint" << endl;
      cout << "c [pblib] current constraint: "; constraint.printNoNL(); cout << endl;
      assert(false);
  }
}

void PB2CNF::encode_with_adder(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  stats->num_adder_encodings++;
  adder_encoder.encode(constraint, formula, auxVars);
}

void PB2CNF::encode_with_swc(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  swc_encoder.encode(constraint, formula, auxVars);
}


void PB2CNF::encode_with_bdd(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  stats->num_bdd_gates_encodings++;
  bdd_encoder.encode(constraint, formula, auxVars);
}

void PB2CNF::encode_with_card(SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  stats->num_card_encodings++;
  card_encoder.encode(constraint, formula, auxVars);
}



PB2CNF::PB2CNF() : PB2CNF(basic_default_config, nullptr)
{

}


PB2CNF::PB2CNF(PBConfig& config, statistic* _stats)
  : tmpFormula(config), pre_encoder(config, _stats), config(config), bdd_encoder(config), adder_encoder(config), amo_encoder(config), binary_amo_encoder(config), k_product_encoer(config), commander_amo_encoding(config), naive_amo_encoder(config), bimander_amo_encoding(config), bdd_sec_amo(config), card_encoder(config), sorting_networks(config),binary_merge(config), swc_encoder(config), stats(_stats)
{
  if (stats == 0)
  {
    stats = new statistic;
    private_stats = true;
  }
  else
  {
    private_stats = false;
  }
}



PB2CNF::~PB2CNF()
{
  if (private_stats)
    delete stats;
}



bool PB2CNF::encodeWithBestEncoder(vector< Encoder*> encoders, SimplePBConstraint& constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  if (encoders.size() == 0)
    return false;

  Encoder* bestEncoder = encoders[0];
  int64_t  bestValue= encoders[0]->encodingValue(constraint);

  for (int i = 1; i < encoders.size(); ++i)
  {
    int64_t encodingValue = encoders[i]->encodingValue(constraint);
    if (encodingValue < 0)
      continue;

    if ( (bestValue < 0) || (encodingValue < bestValue))
    {
      bestValue = encodingValue;
      bestEncoder = encoders[i];
    }
  }

  if (bestValue < 0)
    return false;

  bestEncoder->encode(constraint, formula, auxVars);

  return true;
}


bool PB2CNF::encodeWithBestEncoder(std::vector< Encoder* > encoders, shared_ptr< IncSimplePBConstraint > constraint, ClauseDatabase& formula, AuxVarManager& auxVars)
{
  if (encoders.size() == 0)
    return false;

  Encoder* bestEncoder = encoders[0];
  int64_t  bestValue= encoders[0]->encodingValue(constraint);

  for (int i = 1; i < encoders.size(); ++i)
  {
    int64_t encodingValue = encoders[i]->encodingValue(constraint);
    if (encodingValue < 0)
      continue;

    if ( (bestValue < 0) || (encodingValue < bestValue))
    {
      bestValue = encodingValue;
      bestEncoder = encoders[i];
    }
  }

  if (bestValue < 0)
    return false;

  bestEncoder->encode(constraint, formula, auxVars);

  return true;
}


