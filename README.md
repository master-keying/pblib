# pblib: Encoding Pseudo-Boolean Constraints into CNF

Author: [Peter Steinke](http://www.wv.inf.tu-dresden.de/People/Steinke.html)
| [Manual](https://github.com/master-keying/pblib/blob/master/manual/pblib.pdf)

Many different encodings for Pseudo-Boolean (PB) constraints
into conjunctive normal form (CNF) have been proposed in the past.
The PBLib project starts to collect and implement these encodings
to be able to encode PB constraints in a very simple, but effective way.
The aim is not only to generate as few clauses as possible,
but also using encodings that maintain generalized arc consistency by unit propagation,
to speedup the run time of the SAT solver, solving the formula.

A major issue of the implementation is a high flexibility for the user.
Consequently it is not required to bring a PB constraint into a certain normal form.
The PBLib automatically normalizes the constraints and decides
which encoder provides the most effective translation.

The user can also define constraints with two comparators (less equal and greater equal)
and each PB constraint can be encoded in an incremental way:
After an initial encoding it is possible to add
a tighter bound with only a few additional clauses.
This mechanism allows the user to develop SAT-based solvers
for optimization problems with incremental strengthening
and to keep the learned clauses for incremental SAT solver calls.



## About this fork

[![Build Status](https://travis-ci.org/master-keying/pblib.svg?branch=master)](https://travis-ci.org/master-keying/pblib)
[![Build Status](https://ci.appveyor.com/api/projects/status/ctq1d74hvaf3m58q?svg=true)](https://ci.appveyor.com/project/cernoch/pblib)

The _vanilla_ PBlib is hosted at
[TU Dresden](http://tools.computational-logic.org/content/pblib.php).
If you find a more recent version there,
please let [me](https://github.com/cernoch) know.

The fork aims at keeping the PBlib as intact as possible,
but improving its usability in other projects.
Namely, we try to:
- be platform-independent,
- include a CMake that can be included from other projects “downstream”,
- simplify community contributions through pull requests, issue tracking, ...

And who are “we”? My name is Radomír Černoch and I needed the improved PBlib
for the [Lock-chart solving algorithm testbed](https://github.com/master-keying/mks).
I'm grateful for a lot of guidance from [Martin Hořeňovský](https://github.com/horenmar).



## Building

This fork strives for a “standard modern” CMake.
Ideally, you should be fine with
```bash
cmake -H. -Bbuild
cmake --build build
```
If not, please let us know by opening an issue.

**Installation** should be then as easy as
```bash
cd build
sudo make install
```

Do you use **Windows and [Microsoft Visual Studio](https://www.visualstudio.com/)**?
First, install [CMake](https://cmake.org/), then open the Command Prompt (`cmd.exe`) and type
```Batchfile
cmake -H. -Bbuild -G "Visual Studio 15 2017 Win64"
cmake --build build
```



## Usage
### Simple Example

Include the PBLib with `#include "PB2CNF.h"` and create an instance of the PB2CNF class:
```C++
PB2CNF pb2cnf;
```

And now we can start to encode the PBConstraint

![3\overline{x_1} - 2\overline{x_2} +7 x_3 \geq -4](http://latex.codecogs.com/gif.latex?%20$3\overline{x_1}%20-%202\overline{x_2}%20+7%20x_3%20\geq%20-4$)


with the following c++ code:

```C++
vector< int64_t > weights = {3,-2,7};
vector< int32_t > literals = {-1,-2,3};
vector< vector< int32_t > > formula;
int32_t firstFreshVariable = 4;
firstFreshVariable = pb2cnf.encodeGeq(weights, literals, -4, formula, firstFreshVariable) + 1;
```

If you only need a simple At-Most-k constraint
you can use the following code
(in this example at most 2 literals are true):
```C++
vector< int32_t > literals = {-1,-2,3};
vector< vector< int32_t > > formula;
int32_t firstFreshVariable = 4;
int k = 2;
firstFreshVariable = pb2cnf.encodeAtMostK(literals, k, formula, firstFreshVariable) + 1;
```

### Example Iterative Constraints

You can also add a less then or equal AND a greater then or equal comparator,
as well as incremental constraints. For the later one we need the additional
class `ClauseDatabase` (the generic formula container) and the `AuxVarManager`
which takes care of the fresh variables and we will also use the config class of the PBLib:

```C++
using namespace PBLib;
PBConfig config = make_shared< PBConfigClass >();
VectorClauseDatabase formula(config);
PB2CNF pb2cnf(config);
AuxVarManager auxvars(11);

vector< WeightedLit > literals
{WeightedLit(1,-7), WeightedLit(-2,5), WeightedLit(-3,9), WeightedLit(-10,-3), WeightedLit(10,7)};

IncPBConstraint constraint(literals, BOTH, 100, -5);

pb2cnf.encodeIncInital(constraint, formula, auxvars);
```

This encodes the constraint:

![-5 \leq -7 x_1 + 5\overline{x_2} +9 \overline{x_3} -3 \overline{x_{10}} +7 x_{10}  \leq 100](http://latex.codecogs.com/gif.latex?%20$-5%20\leq%20-7%20x_1%20+%205\overline{x_2}%20+9%20\overline{x_3}%20-3%20\overline{x_{10}}%20+7%20x_{10}%20%20\leq%20100%20$)

After adding more constraints and solve the formula
with a SAT solver we can encode new bounds based
on the previous constraint encoding:

```C++
constraint.encodeNewGeq(3, formula, auxvars);
constraint.encodeNewLeq(8, formula, auxvars);
```

This will - in combination with the formula encoded with encodeIncInital - encode the following constraint:

![-3 \leq -7 x_1 + 5\overline{x_2} +9 \overline{x_3} -3 \overline{x_{10}} +7 x_{10}  \leq 8](http://latex.codecogs.com/gif.latex?%20$-3%20\leq%20-7%20x_1%20+%205\overline{x_2}%20+9%20\overline{x_3}%20-3%20\overline{x_{10}}%20+7%20x_{10}%20%20\leq%208%20$)

### More Features

Besides the `VectorClauseDatabase` you can implement your own `ClauseDatabase` containers.
There are already a `CountingClauseDatabase`, which only counts the number of generated clauses,
and a `SATSolverClauseDatabase`, which directly add the generated clauses into
a [MiniSAT](http://github.com/master-keying/mks) like SAT solver
(therefore you have to include your SATsolver as a library).

You can add conditionals to each constraint (also to incremental constraints!), e.g.:

![(x_5 \land \overline{x_6}) \rightarrow (-3 \leq -7 x_1 + 5\overline{x_2} +9 \overline{x_3} -3 \overline{x_{10}} +7 x_{10}  \leq 8)](http://latex.codecogs.com/gif.latex?$(x_5%20\land%20\overline{x_6})%20\rightarrow%20(-3%20\leq%20-7%20x_1%20+%205\overline{x_2}%20+9%20\overline{x_3}%20-3%20\overline{x_{10}}%20+7%20x_{10}%20%20\leq%208)%20$)

Simply add the to your instance of the class PBConstraint (or IncPBConstraint):

```C++
constraint.addConditional(5);
constraint.addConditional(-6);
```

There is no specific normal form necessary.
Internally the PBlib automatically transform
any PBConstraint into a specific normal form
an applies various simplifications like merging multiple variable occurrences,
detecting internally if the constraint is a at-most-one constraint,
at-least-k constraint or a "real" PBConstraint and apply a appropriate encoder to it and much more.

With the PBConfig instance you have lots of options to customize the encoding at your specific needs.

### PB encoder
The PBLib also includes a `PBEncoder` which takes an `opb` input file and translate it into CNF using the PBLib. Usage:
```bash
./pbencoder inputfile
```

### Example PB Solver
The PBLib contains a folder `BasicPBSolver` with the implementations of an example PB Solver.
It uses minisat 2.2 as a backend SAT solver. Usage:
```bash
./pbsolver inputfile
```



## Implemented encodings

At most one:
- sequential*
- bimander
- commander
- k-product
- binary
- pairwise
- nested

At most _K_:
- BDD**
- cardinality networks
- adder networks
- todo: perfect hashing

PB:
- BDD
- adder networks
- watchdog
- sorting networks
- binary merge
- sequential weight counter

*) equivalent to BDD, latter and regular encoding

**) equivalent to sequential counter

Encodings labeled with todo are planed for the (near) future.



## License

And everything is **free, open source and under the MIT license.**
So please **do me a favor** and cite this work where ever you use it
and I would appreciate if you send me a short E-Mail
so that I can cite you as well as an application for the PBLib.



## How to cite the pblib?

```
@incollection{pblib.sat2015
    year={2015},
    isbn={978-3-319-24317-7},
    booktitle={Theory and Applications of Satisfiability Testing -- SAT 2015},
    volume={9340},
    series={Lecture Notes in Computer Science},
    editor={Heule, Marijn and Weaver, Sean},
    doi={10.1007/978-3-319-24318-4_2},
    title={PBLib -- A Library for Encoding Pseudo-Boolean Constraints into CNF},
    publisher={Springer International Publishing},
    author={Philipp, Tobias and Steinke, Peter},
    pages={9-16}
}
```

