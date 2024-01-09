#include "DwarfParser.h"

StackMachine::StackMachine() : opType(DW_OP_null) {}

StackMachine::~StackMachine() {}

uint32_t StackMachine::processExpression(DebugData *expression) {
    DataStream exprStream(expression->getData(), expression->getLen());
    
    while (exprStream.isStreamable()) {
        uint8_t operation = exprStream.decodeUInt8();
        opType = opType == DW_OP_null ? (OperationEncoding)operation : opType;

        switch (operation) {
        case DW_OP_addr: // Location of static variable
            stack.push(exprStream.decodeUInt32());
            break;
        case DW_OP_fbreg:
            stack.push(exprStream.decodeLeb128());
            break;
        default:
            printf("StackMachine: Unknown OP: 0x%x\n", operation);
            exit(1);
        }
    }

    return stack.top();
}

OperationEncoding StackMachine::getType() { return opType; }

DataStream::DataStream(const uint8_t *data, size_t streamLen)
    : data(data), index(0), streamLen(streamLen) {}

DataStream::DataStream(const uint8_t *data)
    : data(data), index(0), streamLen(-1) {}

DataStream::~DataStream() {}

uint8_t DataStream::decodeUInt8() {
    uint8_t res = data[index];
    index++;
    return res;
}

uint16_t DataStream::decodeUInt16() {
    uint16_t res = data[index] | data[index + 1] << 8;
    index += 2;
    return res;
}

uint32_t DataStream::decodeUInt32() {
    uint32_t res = data[index] | data[index + 1] << 8 | data[index + 2] << 16 |
                   data[index + 3] << 24;
    index += 4;
    return res;
}

int64_t DataStream::decodeLeb128() {
    size_t result = 0;
    size_t shift = 0;
    uint8_t byte;
    
    for (;;) {
        byte = data[index++];
        result |= (0x7f & byte) << shift;
        shift += 7;
        if ((0x80 & byte) == 0) { break; }
    }
    
    if (0x40 & byte) {
        result |= -(1 << shift);
    }

    return result;
}

size_t DataStream::decodeULeb128() {
    size_t result = 0;
    size_t shift = 0;
    
    for (;;) {
        result |= (0x7f & data[index]) << shift;
        if (!(0x80 & data[index++])) { break; }
        shift += 7;
    }

    return result;
}

bool DataStream::isStreamable() { return index < streamLen; }

const uint8_t *DataStream::getData() { return data; }

DebugData::DebugData(FormEncoding form) : form(form) {}

DebugData::~DebugData() {}

void DebugData::write(uint8_t *buff, size_t len) {
    for (size_t i = 0; i < len; i++) {
        data.push_back(buff[i]);
    }
}

bool DebugData::isString() {
    return form == DW_FORM_string || form == DW_FORM_strp ||
           form == DW_FORM_line_strp;
}

const uint8_t *DebugData::getData() { return data.data(); }

FormEncoding DebugData::getForm() { return form; }

uint64_t DebugData::getUInt() {
    uint64_t res = 0;

    switch (form) {
    case DW_FORM_addr:
    case DW_FORM_block:
    case DW_FORM_exprloc:
    case DW_FORM_data1:
    case DW_FORM_flag:
    case DW_FORM_flag_present:
    case DW_FORM_ref1:
    case DW_FORM_strx1:
    case DW_FORM_addrx1:
    case DW_FORM_data2:
    case DW_FORM_ref2:
    case DW_FORM_strx2:
    case DW_FORM_addrx2:
    case DW_FORM_strx3:
    case DW_FORM_addrx3:
    case DW_FORM_data4:
    case DW_FORM_ref_addr:
    case DW_FORM_ref4:
    case DW_FORM_sec_offset:
    case DW_FORM_line_strp:
    case DW_FORM_strx4:
    case DW_FORM_addrx4:
    case DW_FORM_data8:
    case DW_FORM_ref8:
    case DW_FORM_ref_sig8:
    case DW_FORM_sdata:
    case DW_FORM_udata:
    case DW_FORM_ref_udata: 
    case DW_FORM_strx:
    case DW_FORM_addrx:
    case DW_FORM_loclistx:
    case DW_FORM_rnglistx:
    case DW_FORM_implicit_const: {
        for (size_t i = 0; i < data.size(); i++) {
            res |= (uint64_t)data[i] << (8*i);
        }
        break;
    }

    default: // FORM does not support reading as int
        std::cerr << "Unsupported form: " << form << std::endl;
        exit(1);
    }
    return res;
}

size_t DebugData::getLen() { return data.size(); }

const char *DebugData::getString() {
    return (new std::string(data.begin(), data.end()))->c_str();
}

AttributeEntry::AttributeEntry(AttributeEncoding name, FormEncoding form,
                               size_t special)
    : name(name), form(form), special(special) {}

AttributeEntry::~AttributeEntry() {}

AttributeEncoding AttributeEntry::getName() { return name; }

FormEncoding AttributeEntry::getForm() { return form; }

size_t AttributeEntry::getSpecial() { return special; }

AbbrevEntry::AbbrevEntry(size_t dieCode, TagEncoding dieTag, bool hasChild)
    : dieCode(dieCode), dieTag(dieTag), hasChild(hasChild) {}

AbbrevEntry::~AbbrevEntry() {
    for (AttributeEntry *attEntry : attEntries) {
        delete attEntry;
    }

    attEntries.clear();
}

void AbbrevEntry::addAttributeEntry(AttributeEntry *attEntry) {
    attEntries.push_back(attEntry);
}

size_t AbbrevEntry::getNumAttributes(void) { return attEntries.size(); }

AttributeEntry *AbbrevEntry::getAttributeEntry(size_t index) {
    if (index < 0 || index >= attEntries.size()) {
        return nullptr;
    }
    return attEntries[index];
}

size_t AbbrevEntry::getDieCode() { return dieCode; }

TagEncoding AbbrevEntry::getDieTag() { return dieTag; }

bool AbbrevEntry::hasChildren() { return hasChild; }

AbbrevTable::AbbrevTable(uint8_t *abbrevTableStart) {
    abbrevData = new DataStream(abbrevTableStart);

    for (;;) {
        size_t dieCode = abbrevData->decodeULeb128();
        if (dieCode == 0) { // End of Compile Unit
            break;
        }

        TagEncoding dieTag = (TagEncoding)abbrevData->decodeULeb128();
        size_t hasChild = abbrevData->decodeULeb128();
        AbbrevEntry *abbrevEntry = new AbbrevEntry(dieCode, dieTag, hasChild);

        for (;;) {
            AttributeEncoding name =
                (AttributeEncoding)abbrevData->decodeULeb128();
            FormEncoding form = (FormEncoding)abbrevData->decodeULeb128();
            if (name == 0 && form == 0) { break; }
            size_t special = (form == DW_FORM_implicit_const)
                             ? abbrevData->decodeULeb128() : 0;

            abbrevEntry->addAttributeEntry(
                new AttributeEntry(name, form, special));
        }

        abbrevEntries.insert({dieCode, abbrevEntry});
    }
}

AbbrevTable::~AbbrevTable() {
    for (auto const &abbrevEntry : abbrevEntries) {
        delete abbrevEntry.second;
    }
    abbrevEntries.clear();

    delete abbrevData;
}

AbbrevEntry *AbbrevTable::getAbbrevEntry(size_t dieCode) {
    return abbrevEntries.find(dieCode) != abbrevEntries.end()
               ? abbrevEntries[dieCode]
               : nullptr;
}

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

Variable::Variable(DebugInfoEntry *debugEntry) : debugEntry(debugEntry) {
    // Check if variable has name
    DebugData *attEntry = debugEntry->getAttribute(DW_AT_name);
    name = attEntry ? attEntry->getString() : "";

    if(debugEntry->getAttribute(DW_AT_location)) {
        processLocation();
    }
}

Variable::~Variable() {}

uint32_t Variable::getLocation() { return location; }

std::string& Variable::getName() { return name; }

OperationEncoding Variable::getType() { return locType; }

void Variable::processLocation() {
    DebugData *locInfo = debugEntry->getAttribute(DW_AT_location);
    if (locInfo->getForm() == DW_FORM_exprloc) {
        StackMachine *exprloc = new StackMachine();

        // Hacky way to get the location type
        location = exprloc->processExpression(locInfo);
        locType = exprloc->getType();
    } else {
        // loclist
    }
}

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
        printf("V-%s, 0x%x\n", v->getName().c_str(), v->getLocation());
    }
    for (Scope *child : scopes) { child->printScopes(depth + 1); }
}

void Scope::getLocalVariables(std::vector<Variable *> &ret) {

}

void Scope::getGlobalVariables(std::vector<Variable *> &ret) {

}

const char *Scope::getName() { return name.c_str(); }

bool Scope::isPcInRange(uint32_t pc) { return pc >= lowPc && pc < highPc; }

DebugInfoEntry::DebugInfoEntry(CompileUnit *compileUnit, DebugInfoEntry *parent)
    : compileUnit(compileUnit), parent(parent), code(0) {}

DebugInfoEntry::~DebugInfoEntry() {
    for (DebugInfoEntry *debugEntry : children) {
        delete debugEntry;
    }
}

void DebugInfoEntry::addAttribute(AttributeEncoding encoding, DebugData *data) {
    attributes.insert({encoding, data});
}

void DebugInfoEntry::addChild(DebugInfoEntry *child) {
    children.push_back(child);
}

void DebugInfoEntry::printEntry() {
    printf("\nCode: 0x%02lx %s\n", code, printTag(abbrevEntry->getDieTag()));
    for (size_t i = 0; i < abbrevEntry->getNumAttributes(); i++) {
        // Use abbrevEntry so attributes printed in correct order
        AttributeEntry *attEntry = abbrevEntry->getAttributeEntry(i);
        printf("\t%-30s%-23s", printAttribute(attEntry->getName()),
               printForm(attEntry->getForm()));
        DebugData *data = attributes[attEntry->getName()];
        if (data->isString()) { printf("(%s)\n", data->getString()); }
        else { printf("(%lx)\n", data->getUInt()); }
    }
}

AbbrevEntry *DebugInfoEntry::getAbbrevEntry() { return abbrevEntry; }

DebugData *DebugInfoEntry::getAttribute(AttributeEncoding attribute) {
    return (attributes.find(attribute) != attributes.end())
            ? attributes[attribute] : nullptr;
}

size_t DebugInfoEntry::getCode() { return code; }

TagEncoding DebugInfoEntry::getTag() { return abbrevEntry->getDieTag(); }

size_t DebugInfoEntry::getNumChildren() { return children.size(); }

DebugInfoEntry *DebugInfoEntry::getChild(size_t index) {
    return index < 0 || index >= children.size() ? nullptr : children[index];
}

DebugInfoEntry *DebugInfoEntry::getParent() { return parent; }

void DebugInfoEntry::setAbbrevEntry(AbbrevEntry *abbrevEntry) {
    this->abbrevEntry = abbrevEntry;
}

void DebugInfoEntry::setCode(size_t dieCode) { code = dieCode; }

bool DebugInfoEntry::isScope() {
    switch (abbrevEntry->getDieTag()) {
    case DW_TAG_subprogram:
    case DW_TAG_inlined_subroutine:
    case DW_TAG_entry_point: return true;
    default: return false;
    }
}

// Some types that aren't C/C++ aren't listed here, and some types can only be
// children to the ones below
bool DebugInfoEntry::isType() {
    switch (abbrevEntry->getDieTag()) {
    case DW_TAG_array_type:
    case DW_TAG_atomic_type:
    case DW_TAG_base_type:
    case DW_TAG_class_type:
    case DW_TAG_const_type:
    case DW_TAG_enumeration_type:
    case DW_TAG_pointer_type:
    case DW_TAG_ptr_to_member_type:
    case DW_TAG_reference_type:
    case DW_TAG_restrict_type:
    case DW_TAG_rvalue_reference_type:
    case DW_TAG_string_type:
    case DW_TAG_structure_type:
    case DW_TAG_typedef:
    case DW_TAG_union_type:
    case DW_TAG_unspecified_type:
    case DW_TAG_volatile_type: return true;
    default: return false;
    }
}

bool DebugInfoEntry::isVariable() {
    return abbrevEntry->getDieTag() == DW_TAG_variable;
}

CompileUnit::CompileUnit(uint8_t *debugInfoCUHeader, uint8_t *debugAbbrevStart,
                         uint8_t *debugStrStart, uint8_t *debugLineStrStart)
    : rootScope(nullptr) {
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
    debugStart = (uintptr_t)debugInfoCUHeader;

    generateDebugInfo(root);
}

CompileUnit::~CompileUnit() {
    delete compileUnitHeader;
    delete abbrevTable;
}

void CompileUnit::generateScopes() {    
    rootScope = generateScopes(root, nullptr);
}

AbbrevEntry *CompileUnit::getAbbrevEntry(size_t dieCode) {
    return abbrevTable->getAbbrevEntry(dieCode);
}

size_t CompileUnit::getAddrSize() {
    return compileUnitHeader->getAddressSize();
}

size_t CompileUnit::getLength() { return compileUnitLen; }

const char *CompileUnit::getUnitName() {
    return root->getAttribute(DW_AT_name)->getString();
}

const char *CompileUnit::getUnitDir() {
    return root->getAttribute(DW_AT_comp_dir)->getString();
}

bool CompileUnit::isPcInRange(uint32_t pc) {
    return rootScope->isPcInRange(pc);
}

void CompileUnit::printScopes() {
    DebugData *nameAtt = root->getAttribute(DW_AT_name);
    const char *cuName = nameAtt ? nameAtt->getString() : "";
    printf("\nCU: %s\n", cuName);
    rootScope->printScopes(0);
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
        DebugData *data = decodeInfo(attEntry);
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

DebugData *CompileUnit::decodeInfo(AttributeEntry *entry) {
    FormEncoding form = entry->getForm();
    DebugData *dieData = new DebugData(form);
    size_t streamLen = 0;

    switch (form) {    
    case DW_FORM_addr: // Read addr_size bytes from stream
        streamLen = compileUnitHeader->getAddressSize();
        break;
    
    case DW_FORM_block: // Read ULEB128 length, then block
    case DW_FORM_exprloc:
        streamLen = debugStream->decodeULeb128();
        break;
    case DW_FORM_block1: // Read 1-byte length, then block
        streamLen = debugStream->decodeUInt8();
        break;
    case DW_FORM_block2: // Read 2-byte length, then block
        streamLen = debugStream->decodeUInt16();
        break;
    case DW_FORM_block4: // Read 4-byte length, then block
        streamLen = debugStream->decodeUInt32();
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
        int64_t value = debugStream->decodeLeb128();
        dieData->write((uint8_t *)&value, sizeof(int64_t));
        break;
    }

    case DW_FORM_udata: // Read ULEB128
    case DW_FORM_ref_udata: 
    case DW_FORM_strx:
    case DW_FORM_addrx:
    case DW_FORM_loclistx:
    case DW_FORM_rnglistx: {
        uint64_t value = debugStream->decodeULeb128();
        dieData->write((uint8_t *)&value, sizeof(uint64_t));
        break;
    }

     // Read offset, copy string from either .debug_str or .debug_line_str
    case DW_FORM_strp: {
    case DW_FORM_line_strp:
        int offset = debugStream->decodeUInt32();

        StringTable *strTable = form == DW_FORM_strp ? debugStr : debugLineStr;
        std::string str = strTable->getString(offset);
        dieData->write((uint8_t *)str.c_str(), str.size());
        break;
    }

    case DW_FORM_string: { // Read string directly from DIE
        uint8_t c;
        while ((c = debugStream->decodeUInt8()) != '\0') {
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

    // If streamLen is used by a FORM, directly write bytes from debugStream
    for (; streamLen > 0; streamLen--) {
        uint8_t byte = debugStream->decodeUInt8();
        dieData->write(&byte, sizeof(byte));
    }

    return dieData;
}

StringTable::StringTable(char *tableStart) : tableStart(tableStart) {}

StringTable::~StringTable() {}

std::string StringTable::getString(size_t offset) {
    std::string res;

    for (int i = offset; tableStart[i] != '\0'; i++) {
        res += tableStart[i];
    }

    return res;
}

DwarfParser::DwarfParser(const char *fileName)
    : ElfParser::ElfParser(fileName) {
    const ElfSectionHeader *debugInfoHeader = getSectionHeader(".debug_info");
    const ElfSectionHeader *debugAbbrevHeader =
        getSectionHeader(".debug_abbrev");
    const ElfSectionHeader *debugStrHeader = getSectionHeader(".debug_str");
    const ElfSectionHeader *debugLineStrHeader =
        getSectionHeader(".debug_line_str");

    uint8_t *fileStart = elfFileInfo->elfData;

    uint8_t *debugInfoCUHeader = fileStart + debugInfoHeader->offset;
    uint8_t *debugInfoEnd = debugInfoCUHeader + debugInfoHeader->size;
    uint8_t *debugAbbrevStart = fileStart + debugAbbrevHeader->offset;
    uint8_t *debugStrStart = fileStart + debugStrHeader->offset;
    uint8_t *debugLineStrStart = fileStart + debugLineStrHeader->offset;

    // Initialize Compile Units
    for (;;) {
        // End of .debug_info section, CU parsing complete
        if ((uintptr_t)debugInfoCUHeader >= ((uintptr_t)debugInfoEnd)) {
            break;
        }

        CompileUnit *compileUnit = new CompileUnit(debugInfoCUHeader,
            debugAbbrevStart, debugStrStart, debugLineStrStart);
        compileUnits.push_back(compileUnit);
        compileUnit->generateScopes();
        compileUnit->printScopes();

        // Point to next CU Header
        debugInfoCUHeader += compileUnit->getLength();
    }
}

DwarfParser::~DwarfParser() {
    for (CompileUnit *compileUnit : compileUnits) {
        delete compileUnit;
    }
    compileUnits.clear();
}

Scope *DwarfParser::getScope(uint32_t pc) {
    for (CompileUnit *unit : compileUnits) {
        if (unit->isPcInRange(pc)) {
            // return unit->getScope(pc);
        }
    }
    return nullptr;
}

CompileUnit *DwarfParser::getCompileUnit(size_t fileIdx) {
    return compileUnits[fileIdx];
}

size_t DwarfParser::getNumCompileUnits() {
    return compileUnits.size();
}