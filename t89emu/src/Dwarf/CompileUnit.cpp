#include <cstring>

#include "Architecture.h"
#include "Dwarf/CompileUnit.h"

CompileUnitHeader::CompileUnitHeader(uint8_t *debugInfoCompileUnitHeader) {
    memcpy(&header, debugInfoCompileUnitHeader, sizeof(FullCompileUnitHeader));
    ASSERT(getAddressSize() == 4, "addressSize error");
}

CompileUnitHeader::~CompileUnitHeader() {}

uint32_t CompileUnitHeader::getUnitLength() { return header.base.unitLength; }

uint16_t CompileUnitHeader::getUnitVersion() { return header.base.version; }

uint8_t CompileUnitHeader::getUnitType() { return header.base.unitType; }

uint8_t CompileUnitHeader::getAddressSize() { return header.base.addressSize; }

uint32_t CompileUnitHeader::getDebugAbbrevOffset() {
    return header.base.debugAbbrevOffset;
}

size_t CompileUnitHeader::getHeaderLen() {
    return sizeof(FullCompileUnitHeader);
}

CompileUnit::CompileUnit(uint8_t *debugInfoCUHeader, uint8_t *debugAbbrevStart,
                         uint8_t *debugStrStart, uint8_t *debugLineStrStart,
                         uint8_t *debugLineStart) : rootScope(nullptr) {
    compileUnitHeader = new CompileUnitHeader(debugInfoCUHeader);
    // size of length-field (4 bytes) + Rest of CU Header + DIEs
    compileUnitLen = 4 + compileUnitHeader->getUnitLength();
    headerLen = compileUnitHeader->getHeaderLen();

    abbrevTable = new AbbrevTable(debugAbbrevStart
                                  + compileUnitHeader->getDebugAbbrevOffset());
    debugStream = new DataStream(debugInfoCUHeader + headerLen,
                                 compileUnitLen - headerLen);
    root = new DebugInfoEntry(this, nullptr);
    debugStr = new StringTable((char *)debugStrStart);
    debugLineStr = new StringTable((char *)debugLineStrStart);
    debugLine = new LineNumberInfo(this, debugLineStart);

    generateDebugInfo(root);
}

CompileUnit::~CompileUnit() {
    delete compileUnitHeader;
    delete abbrevTable;
}

void CompileUnit::generateScopes() {    
    rootScope = generateScopes(root, nullptr);
}

void CompileUnit::printScopes() {
    DebugData *nameAtt = root->getAttribute(DW_AT_name);
    const char *cuName = nameAtt ? nameAtt->getString() : "";
    printf("\nCU: %s\n", cuName);
    rootScope->printScopes(0);
}

AbbrevEntry *CompileUnit::getAbbrevEntry(size_t dieCode) {
    return abbrevTable->getAbbrevEntry(dieCode);
}

size_t CompileUnit::getAddrSize() {
    return compileUnitHeader->getAddressSize();
}

size_t CompileUnit::getDebugInfoLength() { return compileUnitLen; }

size_t CompileUnit::getDebugLineLength() { return debugLine->getLength(); }

const char *CompileUnit::getUnitName() {
    return root->getAttribute(DW_AT_name)->getString();
}

const char *CompileUnit::getUnitDir() {
    return root->getAttribute(DW_AT_comp_dir)->getString();
}

std::vector<SourceInfo *> &CompileUnit::getSourceInfo() {
    return debugLine->getSourceInfo();
}

uint CompileUnit::getLineNumberAtPc(uint32_t pc) {
    return debugLine->getLineNumberAtPc(pc);
}

std::string &CompileUnit::getSourceNameAtPc(uint32_t pc) {
    return debugLine->getSourceNameAtPc(pc);
}

void CompileUnit::getLocalVariables(std::vector<Variable *> &variables,
                                    uint32_t pc, uint line) {
    rootScope->getLocalVariables(variables, pc, line);
}

void CompileUnit::getGlobalVariables(std::vector<Variable *> &variables,
                                     uint32_t pc, uint line) {
    rootScope->getGlobalVariables(variables, pc, line);
}

bool CompileUnit::isPcInRange(uint32_t pc) {
    return rootScope->isPcInRange(pc);
}

DebugData *CompileUnit::decodeInfo(AttributeEntry *entry, DataStream *stream) {
    FormEncoding form = entry->getForm();
    DebugData *dieData = new DebugData(form);
    size_t streamLen = 0;

    switch (form) {    
    case DW_FORM_addr: // Read addr_size bytes from stream
        streamLen = compileUnitHeader->getAddressSize();
        break;
    
    case DW_FORM_block: // Read ULEB128 length, then block
    case DW_FORM_exprloc:
        streamLen = stream->decodeULeb128();
        break;
    case DW_FORM_block1: // Read 1-byte length, then block
        streamLen = stream->decodeUInt8();
        break;
    case DW_FORM_block2: // Read 2-byte length, then block
        streamLen = stream->decodeUInt16();
        break;
    case DW_FORM_block4: // Read 4-byte length, then block
        streamLen = stream->decodeUInt32();
        break;

    case DW_FORM_flag_present: break; // Read 0-byte integer

    case DW_FORM_data1: // Read 1-byte integer
    case DW_FORM_flag:
    case DW_FORM_ref1:
    case DW_FORM_strx1:
    case DW_FORM_addrx1: streamLen = 1; break;

    case DW_FORM_data2: // Read 2-byte integer
    case DW_FORM_ref2:
    case DW_FORM_strx2:
    case DW_FORM_addrx2: streamLen = 2; break;
    
    case DW_FORM_strx3:
    case DW_FORM_addrx3: streamLen = 3; break; // Read 3-byte integer

    case DW_FORM_data4: // Read 4-byte integer
    case DW_FORM_ref_addr:
    case DW_FORM_ref4:
    case DW_FORM_sec_offset:
    case DW_FORM_strx4:
    case DW_FORM_addrx4: streamLen = 4; break;
    
    case DW_FORM_data8: // Read 8-byte integer
    case DW_FORM_ref8:
    case DW_FORM_ref_sig8: streamLen = 8; break;
    
    case DW_FORM_data16: streamLen = 16; break; // Read 16-byte integer

    case DW_FORM_sdata: { // Read LEB128
        int64_t value = stream->decodeLeb128();
        dieData->write((uint8_t *)&value, sizeof(int64_t));
        break;
    }

    case DW_FORM_udata: // Read ULEB128
    case DW_FORM_ref_udata: 
    case DW_FORM_strx:
    case DW_FORM_addrx:
    case DW_FORM_loclistx:
    case DW_FORM_rnglistx: {
        uint64_t value = stream->decodeULeb128();
        dieData->write((uint8_t *)&value, sizeof(uint64_t));
        break;
    }

     // Read offset, copy string from either .debug_str or .debug_line_str
    case DW_FORM_strp: {
    case DW_FORM_line_strp:
        int offset = stream->decodeUInt32();

        StringTable *strTable = form == DW_FORM_strp ? debugStr : debugLineStr;
        std::string str = strTable->getString(offset);
        dieData->write((uint8_t *)str.c_str(), str.size());
        break;
    }

    case DW_FORM_string: { // Read string directly from DIE
        uint8_t c;
        while ((c = stream->decodeUInt8()) != '\0') {
            dieData->write(&c, sizeof(uint8_t));
        }
        break;
    }

    case DW_FORM_implicit_const: {// special value is attribute value
        size_t value = entry->getSpecial();
        dieData->write((uint8_t *)&value, sizeof(size_t));
        break;
    }

    default: // indirect, ref_sup4, strp_sup, ref_sup8
        std::cerr << "Unsupported form: " << form << std::endl;
        exit(0);
        break;
    }

    // If streamLen is used by a FORM, directly write bytes from stream
    for (; streamLen > 0; streamLen--) {
        uint8_t byte = stream->decodeUInt8();
        dieData->write(&byte, sizeof(byte));
    }

    return dieData;
}

DebugInfoEntry *CompileUnit::generateDebugInfo(DebugInfoEntry *node) {
    size_t code = debugStream->decodeULeb128();
    if (code == 0) { // End of siblings, travel back up tree
        delete node;
        return nullptr;
    }
    
    node->setCode(code);
    node->setAbbrevEntry(getAbbrevEntry(node->getCode()));
    
    AbbrevEntry *dieAbbrevEntry = node->getAbbrevEntry();
    ASSERT(dieAbbrevEntry, "dieAbbrevEntry not found\n");
    for (size_t i = 0; i < dieAbbrevEntry->getNumAttributes(); i++) {
        AttributeEntry *attEntry = dieAbbrevEntry->getAttributeEntry(i);
        DebugData *data = decodeInfo(attEntry, debugStream);
        node->addAttribute(attEntry->getName(), data);
    }

    // node->printEntry();

    // Check if end of CU reached or if leaf node
    if (!debugStream->isStreamable()) {
        return nullptr;
    } else if (!dieAbbrevEntry->hasChildren()) {
        return node;
    }

    // Traverse child subtrees
    DebugInfoEntry *child;
    while ((child = generateDebugInfo(new DebugInfoEntry(this, node)))) {
        node->addChild(child);
    }

    return node;    
}

Scope *CompileUnit::generateScopes(DebugInfoEntry *node, Scope *parent) {
    Scope *scope = new Scope(node, parent);

    // Add discovered variables/scopes to current scope
    for (uint i = 0; i < node->getNumChildren(); i++) {
        DebugInfoEntry *childEntry = node->getChild(i);
        if (childEntry->isVariable()) {
            scope->addVariable(new Variable(childEntry));
        } else if (childEntry->isScope()) {
            scope->addScope(generateScopes(childEntry, scope));
        }
    }

    return scope;
}