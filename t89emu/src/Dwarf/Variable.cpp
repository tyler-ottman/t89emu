#include "Dwarf/Variable.h"

Variable::Variable(DebugInfoEntry *debugEntry) : debugEntry(debugEntry) {
    // Variable Name Attribute
    DebugData *attEntry = debugEntry->getAttribute(DW_AT_name);
    varInfo.name = attEntry ? attEntry->getString() : "???";

    // Variable Location Attribute
    locExpr = debugEntry->getAttribute(DW_AT_location);
    if (locExpr && locExpr->getForm() != DW_FORM_exprloc) {
        printf("Unsupported form: loclist\n"); exit(1);
    }

    // Variable Type Attribute
    attEntry = debugEntry->getAttribute(DW_AT_type);
    if (attEntry) {
        // Get Variable type
    }

    // Stack machine for evaluating locations/values
    stack = new StackMachine();
}

Variable::~Variable() {}

DebugData *Variable::getAttribute(AttributeEncoding attribute) {
    return debugEntry->getAttribute(attribute);
}

DebugInfoEntry *Variable::getParentEntry() {
    return debugEntry->getParent();
}

void Variable::getVarInfo(VarInfo &res, bool doUpdate, RegisterFile *regs,
                          CallFrameInfo *cfi, uint32_t pc) {
    if (!doUpdate) {
        res = varInfo;
        return;
    }

    // Re-calculate variable location and value
    if (!updateLocation(regs, cfi, pc) || !updateValue(regs)) {
        varInfo.isValid = false;
    }
    
    res = varInfo;
}

bool Variable::updateLocation(RegisterFile *regs, CallFrameInfo *cfi,
                              uint32_t pc) {
    if (!locExpr) { // Variable has no DW_AT_location
        return false;
    }
    
    DebugData location = stack->parseExpr(locExpr, regs, cfi, pc, this);
    varInfo.location = location.getUInt();
    
    return true;
}

bool Variable::updateValue(RegisterFile *regs) {
    varInfo.value = "???";

    return true;
}