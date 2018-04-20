#ifndef SORTING_NETWORKS_H
#define SORTING_NETWORKS_H
#include "../IncSimplePBConstraint.h"
#include "../SimplePBConstraint.h"
#include "../PBConfig.h"
#include "../clausedatabase.h"
#include "../auxvarmanager.h"
#include "../weightedlit.h"
#include "Encoder.h"

#include <climits>


// this is adapted code from minisat+
#define ExpensiveBigConstants
#define AllDigitsImportant

class SortingNetworks : public Encoder
{
private:
//   int true_lit;
  
  std::vector<int> primes = { 2, 3, 5, 7, 11, 13, 17 };

 inline void cmp2(std::vector<Formula>& fs, int begin)
{
    Formula a     = fs[begin];
    Formula b     = fs[begin + 1];

    fs[begin]     = OR(a,b);
    fs[begin + 1] = AND(a, b);

}

 void riffle(std::vector<Formula>& fs)
{
    std::vector<Formula> tmp; tmp = fs;
    for (int i = 0; i < fs.size() / 2; i++){
        fs[i*2]   = tmp[i];
        fs[i*2+1] = tmp[i+fs.size() / 2];
    }
}

 void unriffle(std::vector<Formula>& fs)
{
    std::vector<Formula> tmp; tmp = fs;
    for (int i = 0; i < fs.size() / 2; i++){
        fs[i]               = tmp[i*2];
        fs[i+fs.size() / 2] = tmp[i*2+1];
    }
}

 void oddEvenMerge(std::vector<Formula>& fs, int begin, int end)
{
    assert(end - begin > 1);
    if (end - begin == 2)
        cmp2(fs,begin);
    else {
        int          mid = (end - begin) / 2;
        std::vector<Formula> tmp;
        for (int i = 0; i < end - begin; i++)
            tmp.push_back(fs[begin+i]);
        unriffle(tmp);
        oddEvenMerge(tmp,0,mid);
        oddEvenMerge(tmp,mid,tmp.size());
        riffle(tmp);
        for (int i = 1; i < tmp.size() - 1; i += 2)
            cmp2(tmp,i);
        for (int i = 0; i < tmp.size(); i++)
            fs[i + begin] = tmp[i];
    }
}

// Inputs to the circuit is the formulas in fs, which is overwritten
// by the resulting outputs of the circuit.
// NOTE: The number of comparisons is bounded by: n * log n * (log n + 1)
void oddEvenSort(std::vector<Formula>& fs)
{
    int orig_sz = fs.size();
    int sz; for (sz = 1; sz < fs.size(); sz *= 2);
    
    if (fs.size() < sz)
      fs.resize(sz,_false_);
    

    for (int i = 1; i < fs.size(); i *= 2)
        for (int j = 0; j + 2*i <= fs.size(); j += 2*i)
            oddEvenMerge(fs,j,j+2*i);

    for (int i = 0; i < sz - orig_sz; ++i)
      fs.pop_back();
}

void optimizeBase(std::vector<int64_t>& seq, int carry_ins, std::vector<int64_t>& rhs, int cost, std::vector<int>& base, int& cost_bestfound, std::vector<int>& base_bestfound)
{
    if (cost >= cost_bestfound)
        return;

    // "Base case" -- don't split further, build sorting network for current sequence:
    int64_t final_cost = 0;
    for (int i = 0; i < seq.size(); i++){
        final_cost += seq[i];

        if (final_cost < 0)
            goto TooBig;
    }
    if (cost + final_cost < cost_bestfound){
        base_bestfound = base;
        cost_bestfound = cost + final_cost;
    }
  TooBig:;

    /**/ int depth = 0;

    // <<== could count 1:s here for efficiency

    std::vector<int64_t> new_seq;
    std::vector<int64_t> new_rhs;
#ifdef PickSmallest
    int p = -1;
    for (int i = 0; i < seq.size(); i++)
        if (seq[i] > 1){ p = seq[i]; break; }
    if (p != -1){
#else
    //int upper_lim = (seq.size() == 0) ? 1 : seq.back(); // <<== Check that sqRoot is an 'int' (no truncation of 'int64_t')
    //for (int i = 0; i < (int)elemsof(primes) && primes[i] <= upper_lim; i++){
    for (int i = 0; i < (int)primes.size(); i++){
        int p    = primes[i];
#endif
        int rest = carry_ins;   // Sum of all the remainders.
        int64_t div, rem;

        for (int j = 0; j < seq.size(); j++){
            rest += seq[j] % p;
            div = seq[j] / p;
            if (div > 0)
                //**/pf(" %d", div),
                new_seq.push_back(div);
        }

#ifdef AllDigitsImportant
        bool    digit_important = true;
#else
        bool    digit_important = false;
#endif
        for (int j = 0; j < rhs.size(); j++){
            div = rhs[j] / p;
            if (new_rhs.size() == 0 || div > new_rhs.back()){
                rem = rhs[j] % p;
                new_rhs.push_back(div);
                if (!(rem == 0 && rest < p) && !(rem > rest))
                    digit_important = true;
            }
            /* <<==
            om 'rhs' slutar på 0:a och 'rest' inte kan overflowa, då behövs inte det sorterande nätverket för 'rest' ("always TRUE")
            samma sak om 'rhs' sista siffra är strikt större än 'rest' ("never TRUE")
            */
        }

        base.push_back(p);
        /**/depth++;
        optimizeBase(new_seq, rest/p, new_rhs, cost+(digit_important ? rest : 0), base, cost_bestfound, base_bestfound);
        /**/depth--;
        base.pop_back();

        new_seq.clear();
        new_rhs.clear();
    }
}



void optimizeBase(std::vector<int64_t>& seq, std::vector<int64_t>& rhs, int& cost_bestfound, std::vector<int>& base_bestfound)
{
    std::vector<int>    base;
    cost_bestfound = INT_MAX;
    base_bestfound.clear();
    optimizeBase(seq, 0, rhs, 0, base, cost_bestfound, base_bestfound);
}


//=================================================================================================

#define lit2fml(p) id(var(var(p)),sign(p))



void buildSorter(std::vector<Formula>& ps, std::vector<int>& Cs, std::vector<Formula>& out_sorter)
{
    out_sorter.clear();
    for (int i = 0; i < ps.size(); i++)
        for (int j = 0; j < Cs[i]; j++)
            out_sorter.push_back(ps[i]);
    oddEvenSort(out_sorter); // (overwrites inputs)
}


void buildSorter(std::vector<Formula>& ps, std::vector<int64_t>& Cs, std::vector<Formula>& out_sorter)
{
    std::vector<int>    Cs_copy;
    for (int i = 0; i < Cs.size(); i++)
        Cs_copy.push_back(Cs[i]);
    buildSorter(ps, Cs_copy, out_sorter);
}


class Exception_TooBig {};


void buildConstraint(std::vector<Formula>& ps, std::vector<int64_t>& Cs, std::vector<Formula>& carry, std::vector<int>& base, int digit_no, std::vector<std::vector<Formula> >& out_digits)
{
    assert(ps.size() == Cs.size());

    if (digit_no == base.size()){
        // Final digit, build sorter for rest:
        // -- add carry bits:
        for (int i = 0; i < carry.size(); i++)
            ps.push_back(carry[i]),
            Cs.push_back(1);
        out_digits.push_back(std::vector<Formula>());
        buildSorter(ps, Cs, out_digits.back());

    }else{
        std::vector<Formula>    ps_rem;
        std::vector<int>        Cs_rem;
        std::vector<Formula>    ps_div;
        std::vector<int64_t>        Cs_div;

        // Split sum according to base:
        int B = base[digit_no];
        for (int i = 0; i < Cs.size(); i++){
            int64_t div = Cs[i] / int64_t(B);
            int rem = Cs[i] % B;
            if (div > 0){
                ps_div.push_back(ps[i]);
                Cs_div.push_back(div);
            }
            if (rem > 0){
                ps_rem.push_back(ps[i]);
                Cs_rem.push_back(rem);
            }
        }

        // Add carry bits:
        for (int i = 0; i < carry.size(); i++)
            ps_rem.push_back(carry[i]),
            Cs_rem.push_back(1);

        // Build sorting network:
        std::vector<Formula> result;
        buildSorter(ps_rem, Cs_rem, result);

        // Get carry bits:
        carry.clear();
        for (int i = B-1; i < result.size(); i += B)
            carry.push_back(result[i]);

        out_digits.push_back(std::vector<Formula>());
        for (int i = 0; i < B-1; i++){
            Formula out = _false_;
            for (int j = 0; j < result.size(); j += B){
                int n = j+B-1;
                if (j + i < result.size())
                    out = OR(out, AND(result[j + i] , ((n >= result.size()) ? _true_ : ~result[n])));
            }
            out_digits.back().push_back(out);
        }

        buildConstraint(ps_div, Cs_div, carry, base, digit_no+1, out_digits); // <<== change to normal loop
    }
}

/*
Naming:
  - a 'base' is a vector of integers, stating how far you count at that position before you wrap to the next digit (generalize base).
  - A 'dig' is an integer representing a digit in a number of some base.
  - A 'digit' is a vector of formulas, where the number of 1:s represents a digit in a number of some base.
*/



void convert(int64_t num, std::vector<int>& base, std::vector<int>& out_digs)
{
    for (int i = 0; i < base.size(); i++){
        out_digs.push_back(num % base[i]);
        num /= base[i];
    }
    out_digs.push_back(num);
}


// Compare number lexicographically to output digits from sorter networks.
// Formula is TRUE when 'sorter-digits >= num'.
//

Formula lexComp(int sz, std::vector<int>& num, std::vector<std::vector<Formula> >& digits)
{
    if (sz == 0)
        return _true_;
    else{
        sz--;
        std::vector<Formula>& digit = digits[sz];
        int           dig   = num[sz];

        Formula gt = (digit.size() > dig) ? digit[dig] : _false_;       // This digit is greater than the "dig" of 'num'.
        Formula ge = (dig == 0) ? _true_ :
                     (digit.size() > dig-1) ? digit[dig-1] : _false_;   // This digit is greater than or equal to the "dig" of 'num'.

        /**/if (sz == 0) return ge;
        return OR(gt , AND(ge , lexComp(sz, num, digits)));
    }
}

Formula lexComp(std::vector<int>& num, std::vector<std::vector<Formula> >& digits) {
    assert(num.size() == digits.size());
    return lexComp(num.size(), num, digits); }



Formula buildConstraint(std::vector<Formula>& ps, std::vector<int64_t>& Cs, std::vector<int>& base, int64_t lo, int64_t hi)
{
    std::vector<Formula> carry;
    std::vector<std::vector<Formula> > digits;
    buildConstraint(ps, Cs, carry, base, 0, digits);
    

    std::vector<int> lo_digs;
    std::vector<int> hi_digs;
    if (lo != INT_MIN)
        convert(lo, base, lo_digs);
    if (hi != INT_MAX)
        convert(hi+1, base, hi_digs);   // (+1 because we will change '<= x' to '!(... >= x+1)'


    /*DEBUG
    pf("Networks:");
    for (int i = 0; i < digits.size(); i++)
        pf(" %d", digits[i].size());
    pf("\n");

    if (lo != Int_MIN){
        pf("lo=%d :", lo); for (int i = 0; i < lo_digs.size(); i++) pf(" %d", lo_digs[i]); pf("\n"); }
    if (hi != Int_MAX){
        pf("hi+1=%d :", hi+1); for (int i = 0; i < hi_digs.size(); i++) pf(" %d", hi_digs[i]); pf("\n"); }
    END*/

/*
Base:  (1)    8    24   480
       aaa bbbbbb ccc ddddddd
Num:    2    0     5     6
*/

    Formula ret = AND(((lo == INT_MIN) ? _true_ :  lexComp(lo_digs, digits)),
                 ((hi == INT_MAX) ? _true_ : ~lexComp(hi_digs, digits)));

    return ret;
}


/*
a7...a1   b
0001111   001111111  00111
  ^^         ^        ^

a5 | (a4 & (b7 | b6 & (c3)))

a4
~a5 -> b6
~a6 & ~b7 -> c3
...

>= 404
*/



// Will return '_undef_' if 'cost_limit' is exceeded.
//
Formula buildConstraint(const SimplePBConstraint& c)
{
    std::vector<Formula>    ps;
    std::vector<int64_t>        Cs;

    for (int j = 0; j < c.getN(); j++)
        ps.push_back(LIT(c.getWeightedLiterals()[j].lit)),
        Cs.push_back(c.getWeightedLiterals()[j].weight);

    std::vector<int64_t> dummy;
    int      cost;
    std::vector<int> base;
    optimizeBase(Cs, dummy, cost, base);
    

    Formula ret;
    if (c.getComparator() == PBLib::BOTH)
      ret = buildConstraint(ps, Cs, base, c.getGeq(), c.getLeq());
    else
      ret = buildConstraint(ps, Cs, base, INT_MIN, c.getLeq());
    
    return ret;
}
  
  
  
public:
  SortingNetworks(PBConfig & config);
  int64_t encodingValue(const SimplePBConstraint& pbconstraint);
  void encode(const SimplePBConstraint& pbconstraint, ClauseDatabase & formula, AuxVarManager & auxvars);
};

#endif // SORTING_NETWORKS_H
