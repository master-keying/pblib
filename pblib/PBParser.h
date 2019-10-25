#ifndef PBPARSER_H
#define PBPARSER_H

/*=============================================================================
 * parser for pseudo-Boolean instances
 *
 * Copyright (c) 2005-2007 Olivier ROUSSEL and Vasco MANQUINHO
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *=============================================================================
 */

// version 2.9.4

#include <algorithm>
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>
#include <vector>
#include <set>
#include <cassert>
#include <math.h>

#include "pbconstraint.h"





/**
 * defines callback that are used by the parser to transmit information
 * to the solver
 *
 * These functions must be redefined in a subclass.
 */

class DefaultCallback
{
private:
	std::vector<PBLib::WeightedLit> vars;
	int64_t maxVarId;
	std::vector<PBLib::PBConstraint> constraints;
	std::set<int32_t> units;
	PBLib::PBConstraint obt_constraint;
	bool isObtInstance;

	PBLib::Comparator comparator;
	int64_t k;
	int64_t k2;
	std::vector<int32_t>  x;
	std::vector<int64_t>  w;
	int64_t maxSum;
	std::set<int32_t> varsInPB;

	std::vector< std::vector<int32_t> > realPB;
	std::vector< PBLib::WeightedLit> tmpVars;

public:
	std::vector< PBLib::PBConstraint > & getConstraints() {return constraints;}
	std::set<int32_t> & getUnits() { return units;}
	int64_t getMaxVarId() { return maxVarId; }
	bool getIsObtInstance() { return isObtInstance; }
	PBLib::PBConstraint  getObtConstraint() {assert(isObtInstance); return obt_constraint; }
	/**
	 * callback called when we get the number of variables and the
	 * expected number of constraints
	 * @param nbvar: the number of variables
	 * @param nbconstr: the number of contraints
	 */
	void metaData(int nbvar, int /*nbconstr*/)
	{
		maxVarId = nbvar;
	}

	/**
	 * callback called before we read the objective function
	 */
	void beginObjective()
	{

		isObtInstance = true;
		k = 0;
		x.clear();
		w.clear();
	}

	/**
	 * callback called after we've read the objective function
	 */
	void endObjective()
	{
	  std::vector<PBLib::WeightedLit> vars;
	  for (int i = 0; i < (int)x.size(); ++i)
		  vars.push_back(PBLib::WeightedLit(x[i], w[i]));

	  obt_constraint = PBLib::PBConstraint(vars, PBLib::LEQ, 0);
	}


	/**
	 * callback called when we read a term of the objective function
	 *
	 * @param coeff: the coefficient of the term
	 * @param idVar: the numerical identifier of the variable
	 */
	void objectiveTerm(int64_t coeff, int idVar)
	{
		if (coeff == 0)
			return;

		w.push_back(coeff);
		x.push_back(idVar);
	}

	/**
	 * callback called when we read a term of the objective function
	 * which is a product of literals
	 *
	 * @param coeff: the coefficient of the term
	 * @param list: list of literals which appear in the product
	 */
	void objectiveProduct(int64_t coeff, std::vector<int> list)
	{
	  if (list.size() == 1)
	  {
	    if (coeff == 0)
	      return;

	    w.push_back(coeff);
	    x.push_back(list[0]);
	  }
	  else
	  {
	    std::cerr << "c cannot handle objective products" << std::endl;
	    exit(-1);
	  }
	}

	int32_t getNewSymbole()
	{
		maxVarId++;
		return maxVarId;
	}

	/**
	 * callback called before we read a constraint
	 */
	void beginConstraint()
	{
		w.clear();
		x.clear();
		k = 0;
		maxSum = 0;
	}

	/**
	 * callback called after we've read a constraint
	 */
	void endConstraint()
	{
		assert(x.size() == w.size());
		assert(x.size() != 0);

		vars.clear();
		for (int i = 0; i < (int)x.size(); ++i)
		  vars.push_back(PBLib::WeightedLit(x[i], w[i]));


		if (comparator == PBLib::BOTH)
		  constraints.push_back(PBLib::PBConstraint(vars, comparator, k, k2));
		else
		    constraints.push_back(PBLib::PBConstraint(vars, comparator, k));
	}

	void addClause(std::vector<int> & lits)
	{
		tmpVars.clear();
		for (int i = 0; i < (int) lits.size(); ++i)
			tmpVars.push_back(PBLib::WeightedLit(-lits[i], 1));

		constraints.push_back(PBLib::PBConstraint(tmpVars, PBLib::LEQ, lits.size() - 1));
	}
	/**
	 * callback called when we read a term of a constraint
	 *
	 * @param coeff: the coefficient of the term
	 * @param idVar: the numerical identifier of the variable
	 */
	void constraintTerm(int64_t coeff, int idVar)
	{
		x.push_back(idVar);
		w.push_back(coeff);
	}

	/**
	 * callback called when we read a term of a constraint which is a
	 * product of literals
	 *
	 * @param coeff: the coefficient of the term
	 * @param list: list of literals which appear in the product
	 */
	void constraintProduct(int64_t coeff, std::vector<int> list)
	{
		int32_t lit = getNewSymbole();
		linearizeProduct(lit , list);

		constraintTerm(coeff, lit);
	}

	/**
	 * callback called when we read the relational operator of a constraint
	 *
	 * @param relop: the relational oerator (>= or =)
	 */
	void constraintRelOp(std::string relop)
	{
		if (relop == "=" || relop == "B")
			comparator = PBLib::BOTH;
		else if (relop == ">=")
			comparator = PBLib::GEQ;
		else
			assert(false);
	}

	/**
	 * callback called when we read the right term of a constraint (also
	 * known as the degree)
	 *
	 * @param val: the degree of the constraint
	 */
	void constraintRightTerm(int64_t val)
	{
		k = val;
		k2 = val;
	}

	void constraintSecRightTerm(int64_t val)
	{
		k2 = val;
	}

	/**
	 * add the necessary constraints to define newSymbol as equivalent
	 * to the product (conjunction) of literals in product.
	 */
	void linearizeProduct(int newSymbol, std::vector<int> product)
	{
		using uint = unsigned int;
		std::vector<int> lits;

		lits.push_back(newSymbol);
		for(uint i=0;i<product.size();++i)
		{
			lits.push_back(-product[i]);
		}

		addClause(lits);
		lits.clear();
		lits.push_back(-newSymbol);

		for(uint i=0;i<product.size();++i)
		{
			lits.push_back(product[i]);
			addClause(lits);
			lits.pop_back();
		}
	}

	DefaultCallback() : isObtInstance(false) {}

};

/**
 * this class stores products of literals (as a tree) in order to
 * associate unique identifiers to these product (for linearization)
 */
template <typename Callback>
class ProductStore
{
private:
	// we represent each node of a n-ary tree by a std::vector<ProductNode>
	struct ProductNode
	{
		int lit; // ID of the literal
		int productId; // identifier associated to the product of the
		// literals found from the root up to this node
		std::vector<ProductNode> *next; // list of next literals in a product

		ProductNode(int l)
		{
			lit=l;
			productId=0;
			next=NULL;
		}

		// if we define a destructor to free <next>, we'll have to define
		// a copy constructor and use reference counting. It's not worth it.
	};

	std::vector<ProductNode> root; // root of the n-ary tree
	int nextSymbol; // next available variable

	/**
	 * define an order on ProductNode based on the literal (used to
	 * speed up look up)
	 */
	class ProductNodeLessLit
	{
	public:
		bool operator () (const ProductNode &a, const ProductNode &b)
		{
			return a.lit<b.lit;
		}
	};

public:
	/**
	 * give the first extra variable that can be used
	 */
	void setFirstExtraSymbol(int id)
	{
		nextSymbol=id;
	}

	/**
	 * get the identifier associated to a product term (update the list
	 * if necessary)
	 */
	int getProductVariable(std::vector<int> &list)
	{
		using uint = unsigned int;
		std::vector<ProductNode> *p=&root;
		typename std::vector<ProductNode>::iterator pos;

		// list must be sorted
		sort(list.begin(),list.end());

		// is this a known product ?
		for(uint i=0;i<list.size();++i)
		{
			assert(p!=NULL);

			// look for list[i] in *p
			pos=lower_bound(p->begin(),p->end(),list[i],ProductNodeLessLit());
			if (pos==p->end() || (*pos).lit!=list[i])
				pos=p->insert(pos,ProductNode(list[i])); // insert at the right place

			if (i!=list.size()-1 && (*pos).next==NULL)
				(*pos).next=new std::vector<ProductNode>;

			p=(*pos).next;
		}

		if ((*pos).productId==0)
			(*pos).productId=nextSymbol++;

		return (*pos).productId;
	}

	/**
	 * add the constraints which define all product terms
	 *
	 */
	void defineProductVariable(Callback &cb)
	{
		std::vector<int> list;

		defineProductVariableRec(cb,root,list);
	}


	/**
	 * free all allocated product data
	 *
	 */
	void freeProductVariable()
	{
		freeProductVariableRec(root);
	}

private:
	/**
	 * add the constraints which define all product terms
	 *
	 */
	void defineProductVariableRec(Callback &cb,
			std::vector<ProductNode> &nodes, std::vector<int> &list)
	{
		using uint = unsigned int;
		for(uint i=0;i<nodes.size();++i)
		{
			list.push_back(nodes[i].lit);
			if (nodes[i].productId)
				cb.linearizeProduct(nodes[i].productId,list);

			if (nodes[i].next)
				defineProductVariableRec(cb,*nodes[i].next,list);

			list.pop_back();
		}
	}


	/**
	 * free all allocated product data
	 *
	 */
	void freeProductVariableRec(std::vector<ProductNode> &nodes)
	{
		using uint = unsigned int;
		for(uint i=0;i<nodes.size();++i)
		{
			if (nodes[i].next)
			{
				freeProductVariableRec(*nodes[i].next);
				delete nodes[i].next;
			}
		}

		nodes.clear();
	}

};

template <typename Callback>
class SimpleParser
{
public:
	Callback cb;
	bool isOk() { return ok;}
private:
	bool ok = true;
	std::ifstream in; // the stream we're reading from
	int nbVars,nbConstr; // MetaData: #Variables and #Constraints in file.

	int nbProduct,sizeProduct; // MetaData for non linear format
	ProductStore<Callback> store;
	bool autoLinearize; // should the parser linearize constraints ?

	/**
	 * get the next character from the stream
	 */
	char get()
	{
		return in.get();
	}

	/**
	 * put back a character into the stream (only one chr can be put back)
	 */
	void putback(char c)
	{
		in.putback(c);
	}

	/**
	 * return true iff we've reached EOF
	 */
	bool eof()
	{
		return !in.good();
	}

	/**
	 * skip white spaces
	 */
	void skipSpaces()
	{
		char c;

		while(isspace(c=get()));

		putback(c);
	}

	/**
	 * read an identifier from stream and append it to the list "list"
	 * @param list: the current list of identifiers that were read
	 * @return true iff an identifier was correctly read
	 */
	bool readIdentifier(std::vector<int> &list)
	{
		char c;
		bool negated=false;

		skipSpaces();

		// first char (must be 'x')
		c=get();
		if (eof())
		{
			return false;
		}

		if (c=='~')
		{
			negated=true;
			c=get();
		}

		if (c!='x') {
			putback(c);
			return false;
		}

		int varID=0;

		// next chars (must be digits)
		while(true) {
			c=get();
			if (eof())
				break;

			if (isdigit(c))
				varID=varID*10+c-'0';
			else {
				putback(c);
				break;
			}
		}

		//Small check on the coefficient ID to make sure everything is ok
		if (varID > nbVars)
		{
		  std::cout << varID << std::endl;
			assert(false);
		}

		if (negated)
			varID=-varID;

		list.push_back(varID);

		return true;
	}

	/**
	 * read a relational operator from stream and store it in s
	 * @param s: the variable to hold the relational operator we read
	 * @return true iff a relational operator was correctly read
	 */
	bool readRelOp(std::string &s)
	{
		char c;

		skipSpaces();

		c=get();
		if (eof())
			return false;

		if (c=='=')
		{
			s="=";
			return true;
		}

		if (c=='>' && get()=='=')
		{
			s=">=";
			return true;
		}

		if (c=='B')
		{
			s="B";
			return true;
		}

		return false;
	}

	/**
	 * read the first comment line to get the number of variables and
	 * the number of constraints in the file
	 *
	 * calls metaData with the data that was read
	 */
	void readMetaData()
	{
		char c;
		std::string s;

		// get the number of variables and constraints
		c=get();
		assert (c=='*');

		in >> s;
		assert (!(eof() || s!="#variable="));

		in >> nbVars;
		store.setFirstExtraSymbol(nbVars+1);

		in >> s;
		assert  ( !(eof() || s!="#constraint="));

		in >> nbConstr;

		skipSpaces();

		c=get();
		putback(c);

		if (c=='#')
		{
			// assume non linear format
			in >> s;
			assert (!(eof() || s!="#product="));

			in >> nbProduct;

			in >> s;
			assert (!(eof() || s!="sizeproduct="));

			in >> sizeProduct;
		}

		// skip the rest of the line
		getline(in,s);

		// callback to transmit the data
		if (nbProduct && autoLinearize)
		{
#ifdef ONLYCLAUSES
			cb.metaData(nbVars+nbProduct,nbConstr+nbProduct+sizeProduct);
#else
			cb.metaData(nbVars+nbProduct,nbConstr+2*nbProduct);
#endif
		}
		else
			cb.metaData(nbVars,nbConstr);
	}


	/**
	 * skip the comments at the beginning of the file
	 */
	void skipComments()
	{
		std::string s;
		char c = ' ';

		// skip further comments

		while(!eof() && (c=get())=='*')
		{
			getline(in,s);
		}

		putback(c);
	}


	/**
	 * read a term into coeff and list
	 * @param coeff: the coefficient of the variable
	 * @param list: the list of literals identifiers in the product
	 */
	void readTerm(int64_t &coeff, std::vector<int> &list)
	{
		char c;

		list.clear();

		in >> coeff;

		assert ( in.good() && "error after reading 64bit value. Bigger coefficients cannot be handled yet." );
		if (!in.good())
		{
		  // for ndebug
		  ok = false;
		  std::cerr << "error after reading 64bit value. Bigger coefficients cannot be handled yet." << std::endl;
		}



		skipSpaces();

		while(readIdentifier(list));

		if (list.size()==0)
			assert(false);
	}

	/**
	 * read the objective line (if any)
	 *
	 * calls beginObjective, objectiveTerm and endObjective
	 */
	void readObjective()
	{
		char c;
		std::string s;

		int64_t coeff;
		std::vector<int> list;

		// read objective line (if any)

		skipSpaces();
		c=get();
		if (c!='m')
		{
			// no objective line
			putback(c);
			return;
		}

		if (get()=='i' && get()=='n' && get()==':')
		{
			cb.beginObjective(); // callback

			while(!eof())
			{
				readTerm(coeff,list);
				if (list.size()==1 && list[0]>0)
					cb.objectiveTerm(coeff,list[0]);
				else
					handleProduct(true,coeff,list);

				skipSpaces();
				c=get();
				if (c==';')
					break; // end of objective
				else
					if (c=='-' || c=='+' || isdigit(c))
						putback(c);
					else
						assert(false);
			}

			cb.endObjective();
		}
		else
			assert(false);
	}

	/**
	 * read a constraint
	 *
	 * calls beginConstraint, constraintTerm and endConstraint
	 */
	void readConstraint()
	{

		std::string s;
		char c;

		int64_t coeff;
		std::vector<int> list;

		cb.beginConstraint();

		while(!eof())
		{
			readTerm(coeff,list);
			if (list.size()==1)
			{
				cb.constraintTerm(coeff,list[0]);
			}
			else
			{
				handleProduct(false,coeff,list);
			}

			skipSpaces();
			c=get();
			if (c=='>' || c=='=' || c=='B')
			{
				// relational operator found
				putback(c);
				break;
			}
			else
				if (c=='-' || c=='+' || isdigit(c))
					putback(c);
				else
					assert(false && "detected invalid symbol in input");
		}

		if (eof())
			assert(false);

		if (readRelOp(s))
			cb.constraintRelOp(s);
		else
			assert(false);

		in >> coeff;
		cb.constraintRightTerm(coeff);

		if (s == "B")
		{
		  in >> coeff;
		  cb.constraintSecRightTerm(coeff);
		}


		skipSpaces();
		c=get();
		if (eof() || c!=';')
			assert(false);

		cb.endConstraint();
	}

	/**
	 * passes a product term to the solver (first linearizes the product
	 * if this is wanted)
	 */
	void handleProduct(bool inObjective, int64_t coeff, std::vector<int> &list)
	{
		if (autoLinearize)
		{
			// get symbol corresponding to this product
			int var=store.getProductVariable(list);

			if (inObjective)
				cb.objectiveTerm(coeff,var);
			else
				cb.constraintTerm(coeff,var);
		}
		else
		{
			if (inObjective)
				cb.objectiveProduct(coeff, list);
			else
				cb.constraintProduct(coeff, list);
		}
	}

public:
	/**
	 * constructor which only opens the file
	 */
	SimpleParser(const char *filename)
	{
		in.open(filename,std::ios_base::in);

		if (!in.good())
		{
			std::cout << "error opening input file " << filename << std::endl;
			exit(-1);
		}



		autoLinearize=true;

		nbVars=0;
		nbConstr=0;
		nbProduct=0;
		sizeProduct=0;
	}

	~SimpleParser()
	{
		store.freeProductVariable();
	}

	/**
	 * ask the parser to linearize the constraints using a simple
	 * default method
	 */
	void setAutoLinearize(bool lin=true)
	{
		autoLinearize=lin;
	}

	/**
	 * parses the file and uses the callbacks to send the data
	 * back to the program
	 */
	void parse()
	{
		char c;

		readMetaData();

		skipComments();

		readObjective();

		// read constraints
		int nbConstraintsRead = 0;
		while(!eof() && ok) {
			skipSpaces();
			if (eof())
				break;

			putback(c=get());
			if(c=='*')
				skipComments();

			if (eof())
				break;

			readConstraint();
			nbConstraintsRead++;
		}

		if (!ok)
		{
		  std::cerr << "c error during parsing .. stoped" << std::endl;
		  return;
		}

		if (nbConstraintsRead != nbConstr)
		{
		  //Small check on the number of constraints
		  std::cerr << "c warning: number of constraints header dismatch" << std::endl;
		}


		if (autoLinearize)
		{
			store.defineProductVariable(cb);
		}
	}
}; // class SimpleParser



class PBParser
{
	private:
		bool hasObjective;
		int32_t maxVarID;
		PBLib::PBConstraint obt_constraint;
		bool ok = true;
	public:
		PBParser() : hasObjective(false) {};
		std::vector<PBLib::PBConstraint> parseFile(std::string fileName);
		int32_t getMaxVarID() const { return maxVarID; }
		PBLib::PBConstraint getObjConstraint() const { assert(hasObjective);  return obt_constraint; }
		bool hasObjectiveFunction() const { return hasObjective; }
		bool isOk() const { return ok; }
};

#endif // PBPARSER_H
