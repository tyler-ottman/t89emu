#include "Dwarf/Scope.h"

Scope::Scope(DebugInfoEntry *debugEntry, Scope *parent) : parent(parent) {
    DebugData *nameAtt = debugEntry->getAttribute(DW_AT_name);
    name = nameAtt ? nameAtt->getString() : "null";

    DebugData *lowPcAtt = debugEntry->getAttribute(DW_AT_low_pc);
    DebugData *highPcAtt = debugEntry->getAttribute(DW_AT_high_pc);

    lowPc = lowPcAtt ? lowPcAtt->getUInt() : 0;
    highPc = highPcAtt ? lowPc + highPcAtt->getUInt() : 0;
}

Scope::~Scope() {}

void Scope::addScope(Scope *childScope) { scopes.push_back(childScope); }

void Scope::addVariable(Variable *childVar) { variables.push_back(childVar); }

void Scope::printScopes(int depth) {
    for (int i = 0; i < depth; i++) { printf("   "); }
    printf("S-%s, 0x%x - 0x%x\n", name.c_str(), lowPc, highPc);
    for (Variable *v : variables) {
        for (int i = 0; i < depth+1; i++) { printf("   "); }
        VarInfo varInfo;
        v->getVarInfo(varInfo, false, nullptr, nullptr, 0);
        printf("V-%s, 0x%x\n", varInfo.name.c_str(), 0);
    }
    for (Scope *child : scopes) { child->printScopes(depth + 1); }
}

void Scope::getLocalVariables(std::vector<Variable *> &ret, uint32_t pc,
                              uint line) {
    for (Scope *childScope : scopes) {
        if (childScope->isPcInRange(pc)) {
            childScope->getLocalVariables(ret, pc, line);
            return;
        }
    }

    getVariablesAboveLine(ret, line);
}

void Scope::getGlobalVariables(std::vector<Variable *> &ret, uint32_t pc,
                               uint line) {
    // Reached local scope, stop adding to globals
    if (isLocalScope(pc)) { return; }

    getVariablesAboveLine(ret, line);

    for (Scope *childScope : scopes) {
        if (childScope->isPcInRange(pc)) {
            childScope->getGlobalVariables(ret, pc, line);
            return;
        }
    }
}

const char *Scope::getName() { return name.c_str(); }

bool Scope::isLocalScope(uint32_t pc) {
    for (Scope *childScope : scopes) {
        if (childScope->isPcInRange(pc)) {
            return false;
        }
    }
    return true;
}

bool Scope::isPcInRange(uint32_t pc) { return pc >= lowPc && pc < highPc; }

void Scope::getVariablesAboveLine(std::vector<Variable *> &ret, uint line) {
    for (Variable *variable : variables) {
        DebugData *varEntry = variable->getAttribute(DW_AT_decl_line);
        if (varEntry && varEntry->getUInt() <= line) {
            ret.push_back(variable);
        }
    }
}