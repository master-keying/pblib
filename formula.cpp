// this is adapted code from minisat+
#include "formula.h"

const Formula _false_ = FormulaClass::newFalse();
const Formula _true_ = FormulaClass::newTrue();
const Formula _undef_ = FormulaClass::newUndef();

int32_t FormulaClass::id = 3;
std::vector<Formula> FormulaClass::nodes;
std::map<uint64_t, std::vector<Formula>> FormulaClass::formula_cache;
std::map<uint64_t, std::vector<Formula>>::iterator FormulaClass::it;
PBConfig FormulaClass::config;

Formula FormulaClass::newITE(const Formula& s, const Formula& t, const Formula& f)
{
    uint64_t hash;
    if (config->use_formula_cache)
    {
      const uint64_t sd = s->data;
      const uint64_t td = t->data;
      const uint64_t fd = f->data;
      const uint64_t ha = sd * td;
      const uint64_t hb = td & fd;
      const uint64_t hc = fd ^ sd;
      hash = ((sd & 3) ^ (td & 3) ^ (fd & 3)) ^ (ha << 2) ^ (hb << 16) ^ (hc << 32);
      
      it = formula_cache.find(hash);
      if (it != formula_cache.end())
      {
	for (int i = 0; i < it->second.size(); ++i)
	{
	  if (isITE(it->second[i]) && selector(it->second[i]) == s && true_branch(it->second[i]) == t && false_branch(it->second[i]) == f)
	    return it->second[i];
	}
      }
    }
    
    nodes.clear();
    nodes.push_back(s);
    nodes.push_back(t);
    nodes.push_back(f);
    
    Formula result = std::make_shared<FormulaClass>(32, nodes);
    
    if (config->use_formula_cache)
      formula_cache[hash].push_back(result);
    return result;
}

void printFormula(Formula f)
{
  using namespace std;
  
  if (f == _true_)
    cout << "True";
  else if (f == _false_)
    cout << "False";
  else if (f == _undef_)
    cout << "_undef_";
  else if (isAND(f))
  {
    cout << "( ";
    printFormula(left(f));
    cout << " & ";
    printFormula(right(f));
    cout << " )";
  }
  else if (isEquiv(f))
  {
    cout << "( ";
    printFormula(left(f));
    cout << " = ";
    printFormula(right(f));
    cout << " )";
  }
  else if (isITE(f))
  {
    cout << "( if ";
    printFormula(selector(f));
    cout << " then ";
    printFormula(true_branch(f));
    cout << " else ";
    printFormula(false_branch(f));
    cout << " )";
  }
  else if (isMonotonicITE(f))
  {
    cout << "( if (Mono) ";
    printFormula(selector(f));
    cout << " then ";
    printFormula(true_branch(f));
    cout << " else ";
    printFormula(false_branch(f));
    cout << " )";
  }
  else if (isAtom(f))
  {
    cout << getLit(f);
  }
  else
  {
    cout << "CANNOT PRINT NODE";
  }
  
}

void printFormulaBits(Formula f)
{
  std::cout << "c flags" << std::endl << "c ";
  for (int i = 0; i < 32; ++i)
  {
    if ((f->flags & (1 << i)) != 0)
      std::cout << "1";
    else
      std::cout << "0";
    
  }
  std::cout << std::endl << "c data" << std::endl << "c ";
  for (int i = 0; i < 32; ++i)
  {
    if ((f->data & (1 << i)) != 0)
      std::cout << "1";
    else
      std::cout << "0";
  }
  std::cout << std::endl;
}

Formula operator ~ (Formula const & f) {
  if (f == _true_)
    return _false_;
  
  if (f == _false_)
    return _true_;
  
  if (f == _undef_)
    return _undef_;
  
  return FormulaClass::newNeg(f);
}

Formula AND (std::vector<Formula> & conjuncts)
{
  for (int i = 0; i < conjuncts.size(); ++i) {
    if (conjuncts[i] == _false_)
      return _false_;
    
    if (conjuncts[i] == _true_)
    {
      conjuncts[i] = conjuncts[conjuncts.size() - 1];
      conjuncts.pop_back();
      i--;
      continue;
    }
  }
  
  if (conjuncts.size() == 0)
    return _true_;
  
  if (conjuncts.size() == 1)
    return conjuncts[0];
  
  if (conjuncts.size() == 2)
    return AND(conjuncts[0], conjuncts[1]);
  
  return FormulaClass::newAND(conjuncts);
}

Formula AND ( const Formula& f, const Formula& g )
{
    if (f == _false_ || g == _false_) return _false_;
    else if (f == _true_) return g;
    else if (g == _true_) return f;
    else if (f == g ) return f;
    else if (f == ~g ) return _false_;

    if (g < f)
      return FormulaClass::newAND(f, g);
    else
      return FormulaClass::newAND(g, f);
}


Formula XOR (Formula f, Formula g)
{
    if (f == _false_) return g;
    else if (f == _true_) return ~g;
    else if (g == _false_) return f;
    else if (g == _true_) return ~f;
    else if (f == g ) return _false_;
    else if (f == ~g ) return _true_;

    if (g < f)
      swap(f,g);

      
    if (isNeg(f) == isNeg(g))
      return FormulaClass::newEquiv(noNeg(f), ~ noNeg(g));
    else
      return FormulaClass::newEquiv(noNeg(f), noNeg(g));
    
}

Formula FAs(Formula x, Formula y, Formula c) // XOR of 3 arguments: x # y # c
{
    bool sgn = isNeg(x) ^ isNeg(y) ^ isNeg(c);
    x = noNeg(x); y = noNeg(y); c = noNeg(c);

    if (sgn)
    {
      if (x == _true_) return XOR(y,c);
      if (y == _true_) return XOR(x,c);
      if (c == _true_) return XOR(x,y);
      if (x == y) return ~c;
      if (x == c) return ~y;
      if (y == c) return ~x;
    }
    else
    {
      if (x == _true_) return ~XOR(y,c);
      if (y == _true_) return ~XOR(x,c);
      if (c == _true_) return ~XOR(x,y);
      if (x == y) return c;
      if (x == c) return y;
      if (y == c) return x;
    }

    if (c < y) swap(c, y);
    if (y < x) swap(y, x);
    if (c < y) swap(c, y);

    if (sgn)
      return ~FormulaClass::newFAs(x, y, c);
    else
      return FormulaClass::newFAs(x, y, c);
    
}


Formula FAc(Formula x, Formula y, Formula c) // x + y + c >= 2
{
    if (x == _false_) return AND(y,x);
    if (x == _true_) return OR(y,c);
    if (y == _false_) return AND(x,c);
    if (y == _true_) return OR(x,c);
    if (c == _false_) return AND(x,y);
    if (c == _true_) return OR(x,y);

    if (x == y) return x;
    if (x == c) return c;
    if (y == c) return y;
    if (x == ~y) return c;
    if (x == ~c) return y;
    if (y == ~c) return x;

    if (c < y) swap(c, y);
    if (y < x) swap(y, x);
    if (c < y) swap(c, y);

    if (isNeg(c))
      return ~FormulaClass::newFAc(~x, ~y, noNeg(c));
    else
      return FormulaClass::newFAc(x, y, noNeg(c));
}


Formula ITE(Formula s, Formula t, Formula f)
{
    if (s == _false_) return f;
    if (s == _true_) return t;
    if (t == f) return t;
    if (t == ~f) return XOR(s,f);
    if (t == _false_ || t == ~s) return AND(~s,f);
    if (t == _true_ || t == s) return OR(s,f);
    if (f == _false_ || f == s) return AND(s,t);
    if (f == _true_ || f == ~s) return OR(~s,t);


    if (t < f)
    {
        swap(t, f);
        s = ~s;
    }
    

      if (isNeg(f))
	return ~FormulaClass::newITE(s, ~t, ~f);
      else
	return FormulaClass::newITE(s, t, f);
}

Formula Monotonic_ITE(Formula s, Formula t, Formula f)
{
    if (s == _false_) return f;
    if (s == _true_) return t;
    if (t == f) return t;
    if (t == ~f) return XOR(s,f);
    if (t == _false_ || t == ~s) return AND(~s,f);
    if (t == _true_ || t == s) return OR(s,f);
    if (f == _false_ || f == s) return AND(s,t);
    if (f == _true_ || f == ~s) return OR(~s,t);

    return FormulaClass::newMonotonic_ITE(s, t, f);
}

Formula Pure_ITE(Formula s, Formula t, Formula f)
{
    return FormulaClass::newITE(s, t, f);
}
