#ifndef AUXVARMANAGER_H
#define AUXVARMANAGER_H

#include <algorithm>
#include <vector>
#include <cstdint>
#include <assert.h>

#include <unordered_set>


class AuxVarManager
{
// managed fresh auxiliary variables
private:
    AuxVarManager(const AuxVarManager& other) = delete;
    virtual bool operator==(const AuxVarManager& other) const = delete;

    int32_t variable_offset;
    std::unordered_set<int32_t> free_variables;
    std::vector<int32_t> * rememberedVariables;

public:
    // init the aux var manager with the first variable that is free
    AuxVarManager(int32_t first_free_variable);
    virtual ~AuxVarManager() = default;

    // returns an unused variables
    int32_t getVariable();

    // frees an used variable to refill a gap later on
    void freeVariable(int32_t var);

    // frees a range of variable
    void freeVariables(int32_t start, int32_t end);
    void freeVariables(std::vector<int32_t> & variables);

    int32_t getBiggestReturnedAuxVar();
    void startRememberReturnedVariables(std::vector<int32_t> * variables);
    void stopRememerReturnedVariables();
	void resetAuxVarsTo(int32_t new_first_free_variable);
};

#endif // AUXVARMANAGER_H
