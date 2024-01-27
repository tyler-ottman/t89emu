#pragma once

#include <vector>

#include "Dwarf/DebugInfoEntry.h"
// #include "Dwarf/Variable.h"

class DebugInfoEntry;
class Variable;

class Scope {
public:
    Scope(DebugInfoEntry *debugEntry, Scope *parent);
    ~Scope();

    void addScope(Scope *childScope);
    void addVariable(Variable *childVar);
    void printScopes(int depth);

    const char *getName(void);
    void getLocalVariables(std::vector<Variable *> &res, uint32_t pc, uint line);
    void getGlobalVariables(std::vector<Variable *> &res, uint32_t pc, uint line);
    bool isLocalScope(uint32_t pc);
    bool isPcInRange(uint32_t pc);

private:
    void getVariablesAboveLine(std::vector<Variable *> &ret, uint line);

    std::string name;

    Scope *parent;
    std::vector<Scope *> scopes;
    std::vector<Variable *> variables;

    uint32_t lowPc;
    uint32_t highPc;
};