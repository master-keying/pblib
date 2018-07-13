// this is adapted code from minisat+

#ifndef FORMULA_H
#define FORMULA_H

#include <iostream>
#include <cstdint>
#include <memory>
#include <assert.h>
#include <algorithm>
#include <vector>
#include <map>
#include "PBConfig.h"

class FormulaClass;

typedef std::shared_ptr<FormulaClass> Formula;


typedef int32_t Lit;

void printFormulaBits(Formula f);


inline Formula true_branch(Formula const & f);
inline Formula false_branch(Formula const & f);
inline Formula selector(Formula const & f);
inline Formula left(Formula const & f);
inline Formula right(Formula const & f);
inline bool isAtom( Formula const & f );
inline bool isNeg( Formula const & f );
inline bool isPos(Formula const & f);
inline bool isAND(Formula const & f);
inline bool isEquiv(Formula const & f);
inline bool isITE(Formula const & f);
inline bool isMonotonicITE(Formula const & f);
inline bool isFAs(Formula const & f);
inline bool isFAc(Formula const & f);

inline Lit getLit(Formula const & f );
inline Lit getId(Formula const & f);

inline bool operator != (Formula const & f, Formula const & g);
inline bool operator == (Formula const & f, Formula const & g);
inline bool operator < (Formula const & f, Formula const & g);
inline bool operator > (Formula const & f, Formula const & g);




class FormulaClass
{

private:
  static std::map<uint64_t, std::vector<Formula>> formula_cache;
  static std::map<uint64_t, std::vector<Formula>>::iterator it;
  static int32_t id;
  static std::vector<Formula> nodes;
public:
  static PBConfig config;
  FormulaClass(int32_t flags, int32_t data, bool) : flags(flags), data(data)
  {
    assert( (flags & 2) != 0); // Do NOT use this ctor for compound formulas! You would copy the id!
    assert( flags != 0 || data == 0 || data == 1 || data == 4);
  };

  FormulaClass(int32_t flags, int32_t var = 0) : flags(flags)
  {
    init(var);
    assert( flags != 0 || data == 0 || data == 1 || data == 4);
  }

   FormulaClass(int32_t flags, int32_t data, std::vector<Formula> & input_nodes, bool) : flags(flags), data(data),  input_nodes(input_nodes)
  {
    assert( (flags & 2) == 0);
    assert( flags != 0 || data == 0 || data == 1 || data == 4);
  };

  FormulaClass(int32_t flags, std::vector<Formula> & input_nodes, int32_t var = 0) : flags(flags), input_nodes(input_nodes)
  {
    init(var);
    assert( flags != 0 || data == 0 || data == 1 || data == 4);
  };

  void init(int32_t var)
  {
    if (flags == 0)
    {
      assert (input_nodes.size() == 0);
      // const term
      if (var == 0)
      {
	// _false_
	data = 1; // 1 0 0
      } else if (var == 1)
      {
	// _true_
	data = 0; // 0 0 0
      } else if (var == 2)
      {
	// _undef_
	data = 4; // 0 0 1
      } else
      {
	assert ( var == 0 || var == 1 || var == 2);
	assert(false);
      }

      return;
    }

    if ( (flags & 2) != 0 )
    {
      // flag marked as atom
      assert (input_nodes.size() == 0);
      assert (var != 0);

      if (var < 0)
      {
	data = -var << 2;
	data = data ^ 3;
      } else
      {
	data = var << 2;
	data = data ^ 2;
      }
    }
    else
    {
      // flag marked not as atom
      assert (var == 0);
      data = id << 2;
      id++;
      assert(id > 3);
// std::cout << "id " << id <<  "  flags: " << flags << std::endl;

    }
  }
public:
  //   1      2     4    8    16      32        64
  // [FAs |isAtom|isAND|FAc |isEquiv|isITE| isMonoITE]
  uint32_t flags;

  // [neg|isAtom| data ..]
  //if secound bit = 1 then data = variable_id; else data = formula_id
  uint32_t data;

  // vector of inputnodes
  std::vector<Formula> input_nodes;

  static Formula newFalse() { return std::make_shared<FormulaClass>(0, 0); }
  static Formula newTrue() { return std::make_shared<FormulaClass>(0, 1); }
  static Formula newUndef() { return std::make_shared<FormulaClass>(0, 2); }

  static Formula newNeg(Formula const & f) {
    uint64_t hash;
    if (config->use_formula_cache)
    {
      hash = ((uint64_t)(f->data)) << 32;
      it = formula_cache.find(hash);
      if (it != formula_cache.end())
      {
	for (size_t i = 0; i < it->second.size(); ++i)
	  if (it->second[i]->data == (f->data ^ 1))
	    return it->second[i];
      }
    }

    Formula result = (isAtom(f)) ? std::make_shared<FormulaClass>(f->flags, f->data ^ 1, true) : std::make_shared<FormulaClass>(f->flags, f->data ^ 1, f->input_nodes, true);

    if (config->use_formula_cache)
      formula_cache[hash].push_back(result);
    return result;
  }

  static Formula newAND(std::vector<Formula> & conjuncts) {
    assert(false && "multiple AND nodes are currently depracted (since the result in fewer fomrula history hits)"); // multiple AND nodes are currently depracted (since the result in fewer fomrula history hits)
    if (config->use_formula_cache)
      assert(false && "if this is implemented again .. add code here");

    std::sort(conjuncts.begin(), conjuncts.end());
    assert(conjuncts.size() > 2);
    uint64_t hash = (((uint64_t)conjuncts[0]->data) << 32) ^ (((uint64_t)conjuncts[1]->data));
    for (size_t i = 2; i < conjuncts.size(); ++i) {
      hash *= conjuncts[i]->data;
    }

    it = formula_cache.find(hash);
    if (it != formula_cache.end())
    {
      for (size_t i = 0; i < it->second.size(); ++i)
      {
	if (isAND(it->second[i]) && conjuncts.size() == it->second[i]->input_nodes.size())
	{
	  bool equal = true;
	  for (size_t j = 0; j < it->second[i]->input_nodes.size(); ++j) {
	    if (it->second[i]->input_nodes[j] != conjuncts[j])
	    {
	      equal = false;
	      break;
	    }
	  }

	  if (equal == true)
	    return it->second[i];
	}
      }
    }

    Formula result = std::make_shared<FormulaClass>(4, conjuncts);
    formula_cache[hash].push_back(result);
    return result;
  }

  static Formula newAND(Formula const & f, Formula const & g) {
    uint64_t hash;
    if (config->use_formula_cache)
    {
      hash = (((uint64_t)f->data) << 32) ^ (((uint64_t)g->data));
      it = formula_cache.find(hash);
      if (it != formula_cache.end())
      {
	for (size_t i = 0; i < it->second.size(); ++i)
	{
	  if (isAND(it->second[i]) && it->second[i]->input_nodes[0] == f && it->second[i]->input_nodes[1] == g)
	    return it->second[i];
	}
      }
    }

    nodes.clear();
    nodes.push_back(f);
    nodes.push_back(g);

    Formula result = std::make_shared<FormulaClass>(4, nodes);
    if (config->use_formula_cache)
      formula_cache[hash].push_back(result);
    return result;
  }

  static Formula newEquiv(Formula const & f, Formula const & g) {
    uint64_t hash;
    if (config->use_formula_cache)
    {
      hash = (((uint64_t)f->data) << 32) ^ (((uint64_t)g->data));
      it = formula_cache.find(hash);
      if (it != formula_cache.end())
      {
	for (size_t i = 0; i < it->second.size(); ++i)
	{
	  if (isEquiv(it->second[i]) && it->second[i]->input_nodes[0] == f && it->second[i]->input_nodes[1] == g)
	    return it->second[i];
	}
      }
    }

    nodes.clear();
    nodes.push_back(f);
    nodes.push_back(g);

    Formula result = std::make_shared<FormulaClass>(16, nodes);

    if (config->use_formula_cache)
      formula_cache[hash].push_back(result);
    return result;
  }

  static Formula newNoNeg(Formula const & f) {
    if (isNeg(f))
      return newNeg(f);
    else
      return f;
  }

  static Formula newFAs(Formula const & x, Formula const & y, Formula const & c) {
    uint64_t hash;
    if (config->use_formula_cache)
    {
      const uint64_t sd = x->data;
      const uint64_t td = y->data;
      const uint64_t fd = c->data;
      const uint64_t ha = sd * td;
      const uint64_t hb = td & fd;
      const uint64_t hc = fd ^ sd;
      hash = ((sd & 3) ^ (td & 3) ^ (fd & 3)) ^ (ha << 2) ^ (hb << 16) ^ (hc << 32);

      it = formula_cache.find(hash);
      if (it != formula_cache.end())
      {
	for (size_t i = 0; i < it->second.size(); ++i)
	{
	  if (isFAs(it->second[i]) && it->second[i]->input_nodes[0] == x && it->second[i]->input_nodes[1] == y && it->second[i]->input_nodes[2] == c)
	    return it->second[i];
	}
      }
    }

    nodes.clear();
    nodes.push_back(x);
    nodes.push_back(y);
    nodes.push_back(c);

    Formula result = std::make_shared<FormulaClass>(1, nodes);

    if (config->use_formula_cache)
      formula_cache[hash].push_back(result);
    return result;
  }

  static Formula newFAc(Formula const & x, Formula const & y, Formula const &c) {
    uint64_t hash;
    if (config->use_formula_cache)
    {
      const uint64_t sd = x->data;
      const uint64_t td = y->data;
      const uint64_t fd = c->data;
      const uint64_t ha = sd * td;
      const uint64_t hb = td & fd;
      const uint64_t hc = fd ^ sd;
      hash = ((sd & 3) ^ (td & 3) ^ (fd & 3)) ^ (ha << 2) ^ (hb << 16) ^ (hc << 32);

      it = formula_cache.find(hash);
      if (it != formula_cache.end())
      {
	for (size_t i = 0; i < it->second.size(); ++i)
	{
	  if (isFAc(it->second[i]) && it->second[i]->input_nodes[0] == x && it->second[i]->input_nodes[1] == y && it->second[i]->input_nodes[2] == c)
	    return it->second[i];
	}
      }
    }

    nodes.clear();
    nodes.push_back(x);
    nodes.push_back(y);
    nodes.push_back(c);

    Formula result = std::make_shared<FormulaClass>(8, nodes);

    if (config->use_formula_cache)
      formula_cache[hash].push_back(result);
    return result;
  }

  static Formula newITE(Formula const & s, Formula const & t, Formula const & f);

  static Formula newMonotonic_ITE(Formula const & s, Formula const & t, Formula const & f) {
    uint64_t hash;
    if (config->use_formula_cache)
    {
      const uint64_t sd = s->data;
      const uint64_t td = t->data;
      const uint64_t fd = f->data;
      const uint64_t ha = sd * td;
      const uint64_t hb = td & fd;
      const uint64_t hc = fd ^ sd;
      hash = (((sd & 3) ^ (td & 3) ^ (fd & 3)) ^ (ha << 2) ^ (hb << 16) ^ (hc << 32)) + 1;

      it = formula_cache.find(hash);
      if (it != formula_cache.end())
      {
	for (size_t i = 0; i < it->second.size(); ++i)
	{
	  if (isMonotonicITE(it->second[i]) && selector(it->second[i]) == s && true_branch(it->second[i]) == t && false_branch(it->second[i]) == f)
	    return it->second[i];
	}
      }
    }

    nodes.clear();
    nodes.push_back(s);
    nodes.push_back(t);
    nodes.push_back(f);

    Formula result = std::make_shared<FormulaClass>(64, nodes);

    if (config->use_formula_cache)
      formula_cache[hash].push_back(result);
    return result;

  }

  static Formula newLit(Lit lit) {
    return std::make_shared<FormulaClass>(2,lit);
  }

  ~FormulaClass() {

#if 0
    if (data  == 297) std::cout << "c destroing " << data << " / " << flags << std::endl;
//     if ( (data & 2) != 0 )
//       std::cout << "    Lit ";
//     else
//       std::cout << " nonLit ";
//     std::cout <<  ((data & 1) != 0 ?  -1 * (data >> 2) : (data >> 2)) << std::endl;
#endif
  }
};


extern const Formula _false_;
extern const Formula _true_;
extern const Formula _undef_;


inline bool isAtom( Formula const & f ) { return (f->flags & 2) != 0; }
inline bool isNeg( Formula const & f ) { return (f->data & 1) != 0; }
inline bool isPos(Formula const & f) { return (f->data & 1) == 0; }
inline bool isAND(Formula const & f) { return (f->flags & 4) != 0; }
inline bool isEquiv(Formula const & f) { return (f->flags & 16) != 0; }
inline bool isITE(Formula const & f) { return (f->flags & 32) != 0; }
inline bool isMonotonicITE(Formula const & f) { return (f->flags & 64) != 0; }
inline bool isFAs(Formula const & f) { return (f->flags & 1) != 0; }
inline bool isFAc(Formula const & f) { return (f->flags & 8) != 0; }
// inline bool is(Formula f) { return (f->flags & ) != 0; }

inline Lit getLit(Formula const & f ) { assert(isAtom(f)); return ((f->data & 1) != 0 ?  -1 * (f->data >> 2) : (f->data >> 2));}
inline Lit getId(Formula const & f) { assert(!isAtom(f)); return ((f->data & 1) != 0 ?  -1 * (f->data >> 2) : (f->data >> 2));}

inline bool operator != (Formula const & f, Formula const & g) { return f->data != g->data; }
inline bool operator == (Formula const & f, Formula const & g) { return f->data == g->data; }
inline bool operator < (Formula const & f, Formula const & g) {  return f->data < g->data; }
inline bool operator > (Formula const & f, Formula const & g) {  return f->data > g->data; }


inline Formula noNeg(Formula const & f) {
  return FormulaClass::newNoNeg(f);
}

Formula operator ~ (Formula const & f);

Formula AND (std::vector< Formula >& conjuncts);

Formula AND ( const Formula& f, const Formula& g );

inline Formula OR (Formula const & f, Formula const & g) { return ~(AND(~f, ~g)); }

Formula XOR (Formula f, Formula g);

Formula FAs(Formula x, Formula y, Formula c); // XOR of 3 arguments: x # y # c

Formula FAc(Formula x, Formula y, Formula c); // x + y + c >= 2

Formula ITE(Formula s, Formula t, Formula f);

Formula Monotonic_ITE(Formula s, Formula t, Formula f);

Formula Pure_ITE(Formula s, Formula t, Formula f);

inline Formula LIT(Lit lit) { return FormulaClass::newLit(lit);}



inline Formula true_branch(Formula const & f) { assert(isITE(f) || isMonotonicITE(f)); return f->input_nodes[1];}
inline Formula false_branch(Formula const & f) {assert(isITE(f) || isMonotonicITE(f)); return f->input_nodes[2];}
inline Formula selector(Formula const & f) {assert(isITE(f) || isMonotonicITE(f)); return f->input_nodes[0];}
inline Formula left(Formula const & f) { assert(isAND(f) || isEquiv(f)); return f->input_nodes[0];}
inline Formula right(Formula const & f) { assert(isAND(f) || isEquiv(f)); return f->input_nodes[1];}


// TODO optimize new gate functions with realNeg ??
// inline Formula realNeg(Formula  f) { assert(f.unique()); f->data = f->data ^ 1; return f;}

void printFormula(Formula f);
#endif // FORMULA_H
