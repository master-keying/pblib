#include "sorting_merging.h"

#include <climits>
#include <cmath>

using namespace std;

namespace PBLib
{
    vector<vector<int32_t> > Sorting::s_auxs;

    map< pair<int32_t,int32_t>, int64_t> Sorting::recursive_sorter_values;
    map< tuple<int32_t,int32_t,int32_t>, int64_t> Sorting::recursive_sorter_l_values;
    map< tuple<int32_t,int32_t,int32_t>, int64_t> Sorting::recursive_merger_values;



void Sorting::recursive_sorter(int m, int l, const vector< int32_t >& input, ClauseDatabase& formula, AuxVarManager& auxvars, vector< int32_t >& output, ImplicationDirection direction)
{
  int n = input.size();

  assert(output.size() == 0);
  assert(n > 1);
  assert(m <= n);

  vector<int32_t> tmp_lits_a;
  vector<int32_t> tmp_lits_b;
  vector<int32_t> tmp_lits_o1;
  vector<int32_t> tmp_lits_o2;

  for (int i = 0; i < l; ++i)
    tmp_lits_a.push_back(input[i]);

  for (int i = l; i < n; ++i)
    tmp_lits_b.push_back(input[i]);

  assert(tmp_lits_a.size() + tmp_lits_b.size() == n);

  sort(m,tmp_lits_a, formula, auxvars, tmp_lits_o1, direction);
  sort(m,tmp_lits_b, formula, auxvars, tmp_lits_o2, direction);


  merge(m, tmp_lits_o1, tmp_lits_o2, formula, auxvars, output, direction);

  assert(tmp_lits_o1.size() == min(l,m));
  assert(tmp_lits_o2.size() == min(n-l,m));
  assert(output.size() == m);
}

void Sorting::recursive_sorter(int m, const vector< int32_t >& input, ClauseDatabase& formula, AuxVarManager& auxvars, vector< int32_t >& output, ImplicationDirection direction)
{
  assert(m > 0);
  assert(input.size() > 0);

  output.clear();

  int n = input.size();
  assert(n > 1);



  int l = 1;

  if (n > 100) // avoid long calculations and stack overflows
    l = n / 2;
  else
  {
    int64_t min_value = recursive_sorter_value(m,n, l, direction);
    for (int i = 2; i < n; ++i)
    {
      int64_t value = recursive_sorter_value(m,n, i, direction);
      if (value < min_value)
      {
	l = i;
	min_value = value;
      }
    }
  }

  recursive_sorter(m,l,input, formula, auxvars, output, direction);
}

void Sorting::counter_sorter(int k, const vector< int32_t >& x, ClauseDatabase& formula, AuxVarManager& auxvars, vector< int32_t >& output, ImplicationDirection direction)
{
  int n = x.size();
  s_auxs.clear(); // TODO remove
  s_auxs.resize(n, vector<int32_t>(k));

  for (int j = 0; j < k; ++j)
  {
    for (int i = j; i < n; ++i)
    {
      s_auxs[i][j] = auxvars.getVariable();
    }
  }

  vector<vector<int32_t> > const & s = s_auxs;

  if (direction == INPUT_TO_OUTPUT || direction == BOTH)
  {
    for (int i = 0; i < n; ++i)
    {
      formula.addClause(-x[i], s[i][0]);
      if (i > 0)
	formula.addClause(-s[i-1][0], s[i][0]);
    }


    for (int j = 1; j < k; ++j)
    {
      for (int i = j; i < n; ++i)
      {
	formula.addClause(-x[i], -s[i-1][j-1], s[i][j]);
	if (i > j)
	  formula.addClause(-s[i-1][j], s[i][j]);
      }
    }
  }

  assert(direction == INPUT_TO_OUTPUT);
//   if (direction == OUTPUT_TO_INPUT || direction == BOTH)
//   {
//     for (int j = 1; j < k; ++j)
//     {
//       for (int i = j; i < n; ++i)
//       {
// 	formula.addClause(-s[i][j], s[i-1][j], x[i]);
// 	formula.addClause(-s[i][j], s[i-1][j], s[i-1][j-1]);
//       }
//     }
//
//     formula.addClause(-s[0][0],x[0]);
//   }

  output.clear();

  for (int i = 0; i < k; ++i)
    output.push_back(s[n-1][i]);
}

void Sorting::direct_sorter(int m, const vector< int32_t >& input, ClauseDatabase& formula, AuxVarManager& auxvars, vector< int32_t >& output, ImplicationDirection direction)
{
  assert(direction == INPUT_TO_OUTPUT);

  int n = input.size();

  assert(n < 20);

  int bitmask = 1;
  vector<int32_t> clause;

  output.clear();
  for (int i = 0; i < m; ++i)
    output.push_back(auxvars.getVariable());


  while (bitmask < pow(2,n))
  {
    int count = 0;
    clause.clear();
    for (int i = 0; i < n; ++i)
    {
	if ( (1 << i) & bitmask )
	{
	  count++;
	  if (count > m)
	    break;

	  clause.push_back(-input[i]);
	}
    }
    assert(count > 0);
    if (count <= m)
    {
      clause.push_back(output[count-1]);
      formula.addClause(clause);
    }

    bitmask++;
  }

}


void Sorting::sort(int m, const vector< int32_t >& input, ClauseDatabase& formula, AuxVarManager& auxvars, vector< int32_t >& output, ImplicationDirection direction)
{
  assert(m >= 0);
  if (m == 0)
  {
    output.clear();
    return;
  }

  int n = input.size();
  if (m > n)
    m = n; // normalize

  if (n == 0)
  {
    output.clear();
    return;
  }

  if (n == 1)
  {
    output.clear();
    output.push_back(input[0]);
    return;
  }

  if (n == 2)
  {
    output.clear();
    int32_t o1 = auxvars.getVariable();
    if (m == 2)
    {
      int32_t o2 = auxvars.getVariable();
      comparator(input[0], input[1], o1, o2, formula, direction);
      output.push_back(o1);
      output.push_back(o2);
    }
    else
    {
      assert(m == 1);
      comparator(input[0], input[1], o1, formula, direction);
      output.push_back(o1);
    }

    return;
  }

  if (direction != INPUT_TO_OUTPUT)
  {
    recursive_sorter(m, input, formula, auxvars, output, direction);
    return;
  }

  int64_t counter = counter_sorter_value(m,n, direction);
  int64_t direct = direct_sorter_value(m,n, direction);
  int64_t recursive = recursive_sorter_value(m,n, direction);


  if (counter < direct && counter < recursive)
  {
    counter_sorter(m, input, formula, auxvars, output, direction);
  }
  else
  if (direct < counter && direct < recursive)
  {
    direct_sorter(m, input, formula, auxvars, output, direction);
  }
  else
  {
    recursive_sorter(m, input, formula, auxvars, output, direction);
  }
}

void Sorting::recursive_merger(int c, vector<int32_t> const & input_a, int a, vector<int32_t> const & input_b, int b, ClauseDatabase & formula, AuxVarManager & auxvars, vector<int32_t> & output, ImplicationDirection direction)
{
  assert(input_a.size() > 0);
  assert(input_b.size() > 0);
  assert(c > 0);

  output.clear();



  if (a > c)
    a = c;
  if (b > c)
    b = c;

  if (c == 1)
  {
    int32_t y = auxvars.getVariable();
    comparator(input_a[0], input_b[0], y, formula, direction);
    output.push_back(y);
    return;
  }

  if (a == 1 && b == 1)
  {
    assert(c == 2);
    int32_t y1 = auxvars.getVariable();
    int32_t y2 = auxvars.getVariable();
    comparator(input_a[0], input_b[0], y1, y2, formula, direction);
    output.push_back(y1);
    output.push_back(y2);
    return;
  }

  vector<int32_t> odd_merge;
  vector<int32_t> even_merge;
  vector<int32_t> tmp_lits_odd_a;
  vector<int32_t> tmp_lits_odd_b;
  vector<int32_t> tmp_lits_even_a;
  vector<int32_t> tmp_lits_even_b;

  for (int i = 0; i < a; i = i + 2)
    tmp_lits_odd_a.push_back(input_a[i]);
  for (int i = 0; i < b; i = i + 2)
    tmp_lits_odd_b.push_back(input_b[i]);

  for (int i = 1; i < a; i = i + 2)
    tmp_lits_even_a.push_back(input_a[i]);
  for (int i = 1; i < b; i = i + 2)
    tmp_lits_even_b.push_back(input_b[i]);


  merge(c/2 + 1, tmp_lits_odd_a, tmp_lits_odd_b, formula, auxvars, odd_merge, direction);
  merge(c/2, tmp_lits_even_a, tmp_lits_even_b, formula, auxvars, even_merge, direction);

  assert(odd_merge.size() > 0);

  output.push_back(odd_merge[0]);

  int i = 1;
  int j = 0;
  while (true)
  {
    if (i < odd_merge.size() && j < even_merge.size())
    {

      if (output.size() + 2 <= c)
      {
	int32_t z0 = auxvars.getVariable();
	int32_t z1 = auxvars.getVariable();

	comparator(odd_merge[i], even_merge[j], z0, z1, formula, direction);
	output.push_back(z0);
	output.push_back(z1);

	if (output.size() == c)
	  break;
      }
      else
      if (output.size() + 1 == c)
      {
	int32_t z0 = auxvars.getVariable();
	comparator(odd_merge[i], even_merge[j], z0, formula, direction);
	output.push_back(z0);
	break;
      }
    }
    else
    if (i >= odd_merge.size() && j >= even_merge.size())
      break; // should never occure since we catch: if (output.size() == c) break; (with c at most n)
    else
    if (i >= odd_merge.size())
    {
      assert(j == even_merge.size() - 1);
      output.push_back(even_merge.back());
      break;
    }
    else
    {
      assert(i == odd_merge.size() - 1);
      output.push_back(odd_merge.back());
      break;
    }

    i++;
    j++;
  }

  assert(output.size() == a + b || output.size() == c);
}

void Sorting::direct_merger(int m, const vector< int32_t >& input_a, const vector< int32_t >& input_b, ClauseDatabase& formula, AuxVarManager& auxvars, vector< int32_t >& output, ImplicationDirection direction)
{
  assert(direction == INPUT_TO_OUTPUT);

  int a = input_a.size();
  int b = input_b.size();

  int n = a + b;

  for (int i = 0; i < m; ++i)
    output.push_back(auxvars.getVariable());


  int j = m < a  ? m : a;

  for (int i = 0; i < j; ++i)
  {
    formula.addClause(-input_a[i], output[i]);
  }

  j = m < b  ? m : b;

  for (int i = 0; i < j; ++i)
  {
    formula.addClause(-input_b[i], output[i]);
  }

  for (int i = 0; i < a; ++i)
  {
    for (int j = 0; j < b; ++j)
    {
      if (i+j+1 < m)
	formula.addClause(-input_a[i], -input_b[j], output[i+j+1]);
    }
  }



}


void Sorting::merge(int m, const vector< int32_t >& input_a, const vector< int32_t >& input_b, ClauseDatabase& formula, AuxVarManager& auxvars, vector< int32_t >& output, PBLib::Sorting::ImplicationDirection direction)
{
  assert(m >= 0);
  if (m == 0)
  {
    output.clear();
    return;
  }

  int a = input_a.size();
  int b = input_b.size();

  int n = a + b;
  if (m > n)
    m = n; // normalize


  if (a == 0 || b == 0)
  {
    output.clear();
    output = a == 0 ? input_b : input_a;
    return;
  }

  if (direction != INPUT_TO_OUTPUT)
  {
    recursive_merger(m, input_a, input_a.size(), input_b, input_b.size(), formula, auxvars, output, direction);
    return;
  }

  int64_t direct = direct_merger_value(m,a,b, direction);
  int64_t recursive = recursive_merger_value(m,a,b, direction);


  if (direct < recursive)
  {
    direct_merger(m, input_a, input_b, formula, auxvars, output, direction);
  }
  else
  {
    recursive_merger(m, input_a, input_a.size(), input_b, input_b.size(), formula, auxvars, output, direction);
  }
}

void Sorting::comparator(const int32_t x1, const int32_t x2, const int32_t y, ClauseDatabase& formula, ImplicationDirection direction)
{
  assert(x1 != x2);

  if (direction == INPUT_TO_OUTPUT || direction == BOTH)
  {
    formula.addClause(-x1, y);
    formula.addClause(-x2, y);
  }

  if (direction == OUTPUT_TO_INPUT || direction == BOTH)
  {
    formula.addClause(-y, x1, x2);
  }
}


void Sorting::comparator(const int32_t x1, const int32_t x2, const int32_t y1, const int32_t y2, ClauseDatabase& formula, ImplicationDirection direction)
{
  assert(x1 != x2);
  assert(y1 != y2);

  if (direction == INPUT_TO_OUTPUT || direction == BOTH)
  {
    formula.addClause(-x1, y1);
    formula.addClause(-x2, y1);
    formula.addClause(-x1,-x2, y2);
  }

  if (direction == OUTPUT_TO_INPUT || direction == BOTH)
  {
    formula.addClause(-y1, x1, x2);
    formula.addClause(-y2, x1);
    formula.addClause(-y2, x2);
  }

}

int64_t Sorting::value_function(int num_clauses, int num_variables)
{
  return num_clauses;
}


int64_t Sorting::direct_merger_value(int m, int a, int b, ImplicationDirection direction)
{
  return value_function((a+b)*m-(m*m-m)/2-(a*a-a)/2-(b*b-b)/2, m);
}

int64_t Sorting::recursive_sorter_value(int m, int n, int l, ImplicationDirection direction)
{
  auto entry = recursive_sorter_l_values.find(tuple<int32_t,int32_t,int32_t>(m,n,l));

  if (entry != recursive_sorter_l_values.end())
    return entry->second;

  PBConfig config = make_shared<PBConfigClass>();
  CountingClauseDatabase formula(config);
  AuxVarManager auxvars(n+1);

  vector<int32_t> input, output;
  for (int i = 0; i < n; ++i)
    input.push_back(i+1);

  recursive_sorter(m,l,input, formula, auxvars, output, direction);

  int64_t value = value_function(formula.getNumberOfClauses(), auxvars.getBiggestReturnedAuxVar() - n);
  recursive_sorter_l_values[tuple<int32_t,int32_t,int32_t>(m,n,l)] = value;

  return value;
}


int64_t Sorting::recursive_merger_value(int m, int a, int b, ImplicationDirection direction)
{
  auto entry = recursive_merger_values.find(tuple<int32_t,int32_t,int32_t>(m,a,b));

  if (entry != recursive_merger_values.end())
    return entry->second;

  PBConfig config = make_shared<PBConfigClass>();
  CountingClauseDatabase formula(config);
  AuxVarManager auxvars(a+b+1);

  vector<int32_t> input_a, input_b, output;
  for (int i = 0; i < a; ++i)
    input_a.push_back(i+1);

  for (int i = 0; i < b; ++i)
    input_b.push_back(i+a+1);

  recursive_merger(m,input_a, a, input_b, b, formula, auxvars, output, direction);

  int64_t value = value_function(formula.getNumberOfClauses(), auxvars.getBiggestReturnedAuxVar() - a -b);
  recursive_merger_values[tuple<int32_t,int32_t,int32_t>(m,a,b)] = value;

  return value;
}


int64_t Sorting::recursive_sorter_value(int m, int n, ImplicationDirection direction)
{
  auto entry = recursive_sorter_values.find(pair<int32_t,int32_t>(m,n));

  if (entry != recursive_sorter_values.end())
    return entry->second;

  PBConfig config = make_shared<PBConfigClass>();
  CountingClauseDatabase formula(config);
  AuxVarManager auxvars(n+1);

  vector<int32_t> input, output;
  for (int i = 0; i < n; ++i)
    input.push_back(i+1);

  recursive_sorter(m,input, formula, auxvars, output, direction);

  int64_t value = value_function(formula.getNumberOfClauses(), auxvars.getBiggestReturnedAuxVar() - n);
  recursive_sorter_values[pair<int32_t,int32_t>(m,n)] = value;

  return value;
}



int64_t Sorting::counter_sorter_value(int m, int n, ImplicationDirection direction)
{
  return value_function(2*n+(m-1)*(2*(n-1)-1)-(m-2)-2*((m-1)*(m-2)/2), m*n-m*(m-1)/2);
}

int64_t Sorting::direct_sorter_value(int m, int n, ImplicationDirection direction)
{
  if (n > 30)
    return LLONG_MAX;

  return value_function(pow(2,n) - 1, m);
}




}