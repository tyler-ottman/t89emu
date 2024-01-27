#pragma once

#include <stack>

#include "Dwarf/Variable.h"

class Variable;

// For processing Dwarf Expressions
class StackMachine {
public:
    typedef uint32_t GenericType;
    typedef OperationEncoding ExprType;
    struct Element {
        Element(ExprType type, GenericType value)
            : type(type), value(value) {}
        ExprType type;
        GenericType value;
    };

    StackMachine(void);
    ~StackMachine();

    DebugData parseExpr(DebugData *expr, RegisterFile *regs, CallFrameInfo *cfi,
                        uint32_t pc);
    DebugData parseExpr(DebugData *expr, RegisterFile *regs, CallFrameInfo *cfi,
                        uint32_t pc, Variable *var);

private:
    std::stack<GenericType> stack;
    DataStream *exprStream;
};