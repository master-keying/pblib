#ifndef BASICSATSOLVER_H
#define BASICSATSOLVER_H

#include "minisat/core/Solver.h"
#include <vector>
#include <cstdint>


class BasicSATSolver : public Minisat::Solver
{
private:
    Minisat::vec<Minisat::Lit> tmpLits;

    long lits_in_clauses = 0;
    long clauses2er = 0;
    long clauses3er = 0;
    long clausesRest = 0;
    long units = 0;
    int tmp;
public:
    BasicSATSolver();
    BasicSATSolver(const BasicSATSolver& other) = delete;
    virtual ~BasicSATSolver();
    virtual bool operator==(const BasicSATSolver& other) const = delete; 
    
    using Minisat::Solver::addClause;

    void print_added_clauses_stats();
    void increseVariables(int32_t max_var);
    bool addClause(std::vector<int32_t> const & clause);
    bool addClauses(std::vector<std::vector<int32_t> > const & clauses);
       
    void printModel();
    
    void getModel(std::vector<int32_t> & model);
    void getModel(std::vector<bool> & model);
        
    void print_solving_stats();
   
    using Minisat::Solver::propagate;
    bool propagate(std::vector<int32_t> const & units, std::vector<int32_t> & newUnits);

};

#endif // BASICSATSOLVER_H
