#include "PBFuzzer.h"

#include <stdlib.h>     
#include <time.h>       
#include <assert.h>

#include "helper.h"
#include "preencoder.h"
#include "VectorClauseDatabase.h"
#include <fstream>
#include <sstream>


using namespace PBLib;
using namespace std;



// copy constraints! DO NOT USE A REFERENCE HERE
void PBFuzzer::writeToPBFile(PBConstraint opt_constraint, vector< PBConstraint > constraints, int numVars, string file)
{
    fstream pbfile(file.c_str(), std::fstream::out);
    stringstream output;
    
    int numConstraints = 0;
    
    for (int i = 0; i < constraints.size(); ++i)
    {
      PBConstraint & constraint = constraints[i];
      
      if (constraint.getComparator() == BOTH && constraint.getLeq() != constraint.getGeq())
      {
	PBConstraint constraint = constraints[i];
	constraints.push_back(constraint.getGeqConstraint());
	constraints.push_back(constraint.getLeqConstraint());
	continue;
      }
      
      if (constraint.getComparator() == LEQ)
      {
	for (int j = 0; j < constraint.getN(); ++j)
	{
	  constraint.getWeightedLiterals()[j].weight *= -1;
	}
	constraint.setComparator(GEQ);
	constraint.setGeq(-constraint.getLeq());
      }
      
      if (constraint.getN() == 0)
      {
	continue;
      }
      
      for (int j = 0; j < constraint.getN(); ++j)
      {
	if (constraint.getWeightedLiterals()[j].weight < 0)
	  output << constraint.getWeightedLiterals()[j].weight;
	else
	  output << "+" << constraint.getWeightedLiterals()[j].weight;
	
	if (constraint.getWeightedLiterals()[j].lit < 0)
	  output << " ~x" << -constraint.getWeightedLiterals()[j].lit << " ";
	else
	  output << " x" << constraint.getWeightedLiterals()[j].lit << " ";
      }
      if (constraint.getComparator() == GEQ)
	output << ">= " << constraint.getGeq() << " ;"<< endl;
      else
	output << "= " << constraint.getGeq() << " ;"<< endl;
      
      numConstraints++;
    }
    
    
    pbfile << "* #variable= " << numVars << " #constraint= " << numConstraints<< endl;
    pbfile << "****************************************"<< endl;
    pbfile << "* begin normalizer comments"<< endl;
    pbfile << "* category= random generated"<< endl;
    pbfile << "* end normalizer comments"<< endl;
    pbfile << "****************************************" << endl;
    if (opt_constraint.getN() > 0)
    {
      pbfile << "min: ";
      for (int i = 0; i < opt_constraint.getN(); ++i)
      {
	if (opt_constraint.getWeightedLiterals()[i].weight < 0)
	  pbfile << opt_constraint.getWeightedLiterals()[i].weight;
	else
	  pbfile << "+" << opt_constraint.getWeightedLiterals()[i].weight;
	
	if (opt_constraint.getWeightedLiterals()[i].lit < 0)
	  pbfile << " ~x" << -opt_constraint.getWeightedLiterals()[i].lit << " ";
	else
	  pbfile << " x" << opt_constraint.getWeightedLiterals()[i].lit << " ";
      }
      pbfile << ";" << endl;
    }
    pbfile << output.str();
    pbfile.close();
}



void PBFuzzer::scramble(vector<PBConstraint> & constraints)
{
  assert(constraints.size() > 0);
  int rnd;
  int index;
  for (int i = 0; i < constraints.size(); ++i)
  {
    PBConstraint & constraint = constraints[i];
    if (RandomCounter::rand() % 100 + 1 <= 50)
    {
      // change to GEQ
      for (int j = 0; j < constraint.getN(); ++j)
      {
	constraint.getWeightedLiterals()[j].weight *= -1;
      }
      constraint.setComparator(GEQ);
      constraint.setGeq(-constraint.getLeq());
    }
    else if (RandomCounter::rand() % 100 + 1 <= pBOTH)
    {
      // change to BOTH
      constraint.setComparator(BOTH);
      if (RandomCounter::rand() % 100 + 1 <= 10)
	constraint.setGeq(constraint.getLeq()); // set to equal
      else
      {
	if (constraint.getLeq() > 0)
	  constraint.setGeq(RandomCounter::rand() % constraint.getLeq());
	else
	  constraint.setGeq(constraint.getLeq()); // set to equal
      }
    }
    
    rnd = RandomCounter::rand() % constraint.getN();
    for (int j = 0; j < rnd; ++j)
    {
      index = RandomCounter::rand() % constraint.getN();
      constraint.setGeq(constraint.getGeq() - constraint.getWeightedLiterals()[index].weight);
      constraint.setLeq(constraint.getLeq() - constraint.getWeightedLiterals()[index].weight);
      constraint.getWeightedLiterals()[index].weight *= -1;
      constraint.getWeightedLiterals()[index].lit *= -1;
    }
  }
}


PBConstraint PBFuzzer::generatePBConstraint()
{
  int tmp = number_of_constraints;
  number_of_constraints = 1;
  vector<PBConstraint> constraints;
  generatePBProblem(constraints);
  tmp = number_of_constraints;
  
  assert(constraints.size() == 1);
  
  return constraints[0];
}


PBConstraint PBFuzzer::generatePBProblem(std::vector< PBConstraint> & constraints)
{
  constraints.clear();
  
  
  PBConfig config = make_shared<PBConfigClass>();

  VectorClauseDatabase formula(config);
  PreEncoder preEncoder(config);

  int rnd;
  int constraintSize;
  int leq;
  int geq;
  
  int trivial_count = 0;
  bool retry = false;
  bool isBig;
  while ( constraints.size() < number_of_constraints + 1)
  {
    literals.clear(); 
    bool isAmk = false;
    bool isAmo = false;
    
    
    if (!retry)
    {
      rnd = RandomCounter::rand() % 100 + 1;
    
      if (RandomCounter::rand() % 100 + 1 <= pBIG)
      {
	isBig = true;
      }
      else
      {
	isBig = false;
      }
    }
    
    
    if (isBig)
    {
      if (constant_constraint_length)
	constraintSize = max_big_constraint_size;
      else
	constraintSize = RandomCounter::rand() % max_big_constraint_size + 1;
    }
    else
    {
      if (constant_constraint_length)
	constraintSize = max_constraint_size;
      else
	constraintSize = RandomCounter::rand() % max_constraint_size + 1;
    }
    
    
    retry = false;
    
    if (rnd <= pAMO)
    {
      isAmo = true;
      // AMO
      for (int i = 0 ; i < constraintSize; ++i)
      {
	if (RandomCounter::rand() % 100 + 1 <= polarity_ratio)
	  literals.push_back(WeightedLit(RandomCounter::rand() % numer_of_variables + 1, 1));
	else
	  literals.push_back(WeightedLit(-(RandomCounter::rand() % numer_of_variables + 1), 1));
      }
      constraints.push_back(PBConstraint(literals, LEQ, 1));
    }
    else if (rnd <= pAMO + pAMK)
    {
      isAmk = true;
      // AMK
      for (int i = 0 ; i < constraintSize; ++i)
      {
	if (RandomCounter::rand() % 100 + 1 <= polarity_ratio)
	  literals.push_back(WeightedLit(RandomCounter::rand() % numer_of_variables + 1, 1));
	else
	  literals.push_back(WeightedLit(-(RandomCounter::rand() % numer_of_variables + 1), 1));
      }
	constraints.push_back(PBConstraint(literals, LEQ, RandomCounter::rand() % (constraintSize) + 2));
    }
    else
    {
      // PB
      for (int i = 0 ; i < constraintSize; ++i)
      {
	if (isBig)
	{
	  if (RandomCounter::rand() % 100 + 1 <= polarity_ratio)
	    literals.push_back(WeightedLit(RandomCounter::rand() % numer_of_variables + 1, RandomCounter::rand() % max_big_weight_size + 1));
	  else
	    literals.push_back(WeightedLit(-(RandomCounter::rand() % numer_of_variables + 1), RandomCounter::rand() % max_big_weight_size + 1));
	}
	else
	{
	  if (RandomCounter::rand() % 100 + 1 <= polarity_ratio)
	    literals.push_back(WeightedLit(RandomCounter::rand() % numer_of_variables + 1, RandomCounter::rand() % max_weight_size + 1));
	  else
	    literals.push_back(WeightedLit(-(RandomCounter::rand() % numer_of_variables + 1), RandomCounter::rand() % max_weight_size + 1));
	}
      }
      if (isBig)
	constraints.push_back(PBConstraint(literals, LEQ, RandomCounter::rand() % (max_big_weight_size * constraintSize)));
      else  
	constraints.push_back(PBConstraint(literals, LEQ, RandomCounter::rand() % (max_weight_size * constraintSize)));
    }
    formula.clearDatabase();
    SimplePBConstraint preEncodedConstraint = preEncoder.preEncodePBConstraint(constraints.back(), formula);
    if (preEncodedConstraint.getType() == DONTCARE)
    {
      if (formula.isSetToUnsat())
      {
	retry = true;
	constraints.pop_back();
      }
      else
      {
	if (RandomCounter::rand() % 100 == 0 && trivial_count <= max_trivial_constraints)
	  trivial_count++;
	else
	{
	  retry = true;
	  constraints.pop_back();
	}
      }
    }
    else
    if (isAmk && preEncodedConstraint.getType() != AMK)
    {
      retry = true;
      constraints.pop_back();
    }
    else
    if (isAmo && preEncodedConstraint.getType() != AMO)
    {
      retry = true;
      constraints.pop_back();
    }
    
  }
  
  PBConstraint opt = constraints.back();
  opt.setComparator(LEQ);
  constraints.pop_back();
  
  if(doScramble)
    scramble(constraints);
  return opt;
}

PBFuzzer::PBFuzzer()
{

}

PBFuzzer::~PBFuzzer()
{

}

