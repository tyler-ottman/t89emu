#include "DwarfParser.h"

StackMachine::StackMachine() {
    exprStream = new DataStream(nullptr);
}

StackMachine::~StackMachine() {}

DebugData StackMachine::parseExpr(DebugData *expr, RegisterFile *regs,
                                  CallFrameInfo *cfi, uint32_t pc) {
    return parseExpr(expr, regs, cfi, pc, nullptr);
}

DebugData StackMachine::parseExpr(DebugData *expr, RegisterFile *regs,
                                  CallFrameInfo *cfi, uint32_t pc,
                                  Variable *var) {
    DebugData res;
    exprStream->setData(expr->getData());
    exprStream->setLen(expr->getLen());

    while (exprStream->isStreamable()) {
        OperationEncoding opcode = (OperationEncoding)exprStream->decodeUInt8();

        switch (opcode) {
        case DW_OP_lit0 ... DW_OP_lit31:
            stack.push(opcode - DW_OP_lit0);
            break;

        case DW_OP_addr:
            stack.push(exprStream->decodeUInt32());
            break;

        case DW_OP_fbreg: {
            if (var == nullptr) {
                printf("Unexpected DW_OP_fbreg"); exit(1);
            }
            int32_t offset = exprStream->decodeLeb128();

            // DW_AT_frame_base is DWARF location expression
            DebugInfoEntry *parent = var->getParentEntry();
            DebugData *fbLoc = parent->getAttribute(DW_AT_frame_base);
            StackMachine fbStack;
            DebugData location = fbStack.parseExpr(fbLoc, regs, cfi, pc);
            stack.push(offset + location.getUInt());
            break;
        }

        case DW_OP_call_frame_cfa: {
            uint32_t cfa;
            bool ret = cfi->getCfaAtLocation(cfa, pc, regs);
            if (!ret) { printf("%s error\n", printOperation(opcode)); exit(1);}
            stack.push(cfa);
            break;
        }
            
        default:
            printf("StackMachine: Unknown OP: %s\n",
                   printOperation((OperationEncoding)opcode));
            exit(1);
        }
    }

    res.write((uint8_t *)&stack.top(), sizeof(GenericType));
    return res;
}

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

bool DataStream::isStreamable() { return data && (index < streamLen); }

const uint8_t *DataStream::getData() { return data; }

const uint8_t *DataStream::getData(size_t idx) {
    return (idx >= 0 && idx < streamLen) ? data + idx : nullptr;
}

size_t DataStream::getIndex() { return index; }

void DataStream::setData(const uint8_t *data) { this->data = data; }

void DataStream::setOffset(size_t offset) { index = offset; }

void DataStream::setLen(size_t len) { streamLen = len; }

DebugData::DebugData(FormEncoding form) : form(form) {}

DebugData::DebugData() : form(DW_FORM_data8) {}

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
        Variable::VarInfo varInfo;
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

SourceInfo::SourceInfo(std::string &path) : path(path) {
    size_t found = path.find_last_of("/");  // Source Name
    name = path.substr(++found);

    // Store bytes of source file
    std::ifstream fs;
    fs.open(path, std::ifstream::in);
    if (!fs.is_open()) {
        printf("Could not open file: %s\n", name.c_str());
        exit(EXIT_FAILURE);
    }

    std::string line;
    while (std::getline(fs, line)) {
        lines.push_back(line);
    }

    fs.close();
}

std::string &SourceInfo::getPath() { return path; }

std::string &SourceInfo::getName() { return name; }

std::vector<std::string> &SourceInfo::getLines() { return lines; }

LineNumberInfo::LineNumberInfo(CompileUnit *compileUnit,
        uint8_t *debugLineStart) : compileUnit(compileUnit) {
    stream = new DataStream(debugLineStart, 4 + *((uint32_t *)debugLineStart));

    parseProgramHeader();
    generateFilePaths();

    // Process line number information with state machine
    clearRegisters();
    generateLineNumberMatrix();
}

LineNumberInfo::~LineNumberInfo() {}

// stream should point to directoryEntryFormatCount or fileNameEntryFormatCount
void LineNumberInfo::generateFileFormatInfo(uint8_t &numEntries,
    std::vector<std::pair<ContentEncoding, FormEncoding>> &formatEntries) {
    numEntries = stream->decodeUInt8();
    for (size_t i = 0; i < numEntries; i++) {
        ContentEncoding dirContent = (ContentEncoding)stream->decodeULeb128();
        FormEncoding dirForm = (FormEncoding)stream->decodeULeb128();
        formatEntries.push_back({dirContent, dirForm});
    }
}

// stream should point to directoriesCount or fileNamesCount
void LineNumberInfo::generateFileInformation(uint &numEntries,
    std::vector<std::map<ContentEncoding, DebugData *>> &fileEntries,
    std::vector<std::pair<ContentEncoding, FormEncoding>> &fileFormat) {
    numEntries = stream->decodeULeb128();
    for (size_t i = 0; i < numEntries; i++) {
        std::map<ContentEncoding, DebugData *> dirEntry;
        for (std::pair<ContentEncoding, FormEncoding> &dirFormat : fileFormat) {
            AttributeEntry dirAttr(AtReserved0, dirFormat.second, 0);
            DebugData *dirData = compileUnit->decodeInfo(&dirAttr, stream);
            dirEntry[dirFormat.first] = dirData;
        }
        fileEntries.push_back(dirEntry);
    }
}

// Includes unitLength (4 bytes) field itself
size_t LineNumberInfo::getLength() { return 4 + infoHeader.unitLength; }

std::vector<SourceInfo *> &LineNumberInfo::getSourceInfo() {
    return sourceInfo;
}

uint LineNumberInfo::getLineNumberAtPc(uint32_t pc) {
    LineEntry *lineEntry = getLineEntryAtPc(pc);
    return lineEntry ? lineEntry->line : 0;
}

std::string temp = "";
std::string &LineNumberInfo::getSourceNameAtPc(uint32_t pc) {
    LineEntry *lineEntry = getLineEntryAtPc(pc);
    return lineEntry ? sourceInfo[lineEntry->file]->getName() : temp;
}

bool LineNumberInfo::containsPath(std::vector<SourceInfo *> &sources,
                         std::string &path) {
    for (SourceInfo *source : sources) {
        if (path == source->getPath()) {
            return true;
        }
    }
    return false;
}

void LineNumberInfo::clearRegisters() {
    basicBlock = endSequence = prologueEnd = epilogueBegin = false;
    address = opIndex = column = isa = discriminator = 0;
    isStmt = infoHeader.defaultIsStmt;
    file = line = 1;
}

void LineNumberInfo::generateFilePaths() {
    std::string compileDirectory =
        infoHeader.directories[0][DW_LNCT_path]->getString();
    
    // Generate full paths to directories
    std::vector<std::string> dirPaths;
    for (std::map<ContentEncoding, DebugData *> &directory :
         infoHeader.directories) {
        std::string dirPath = directory[DW_LNCT_path]->getString();
        if (dirPath[0] != '/') { // Make full path
            dirPath = compileDirectory + "/" + dirPath;
        }
        dirPaths.push_back(dirPath);
    }

    // Generate full paths to files
    std::vector<std::string> filePaths;
    for (std::map<ContentEncoding, DebugData *> &file : infoHeader.fileNames) {
        uint directoryIndex = file[DW_LNCT_directory_index]->getUInt();
        std::string filePath = dirPaths[directoryIndex] + "/"
            + file[DW_LNCT_path]->getString();
        sourceInfo.push_back(new SourceInfo(filePath));
    }
}

void LineNumberInfo::generateLineNumberMatrix() {
    // stream->setOffset(12 + infoHeader.headerLength);

    while (stream->isStreamable()) {
        uint8_t opcode = stream->decodeUInt8();
        uint adjustedOpcode;
        uint operationAdvance;

        if (opcode >= infoHeader.opcodeBase) { // Special
            adjustedOpcode = opcode - infoHeader.opcodeBase;
            operationAdvance = adjustedOpcode / infoHeader.lineRange;

            address = getNewAddress(operationAdvance);
            opIndex = getNewOpIndex(operationAdvance);
            line += infoHeader.lineBase + (adjustedOpcode % infoHeader.lineRange);
            lineMatrix.push_back(new LineEntry(address, line, column, file,
                                               isStmt, prologueEnd,
                                               epilogueBegin));
            basicBlock = prologueEnd = epilogueBegin = false;
            discriminator = 0;
        } else if (opcode && opcode < infoHeader.opcodeBase) { // Standard
            switch (opcode) {
            case DW_LNS_copy:
                lineMatrix.push_back(new LineEntry(address, line, column, file,
                                                   isStmt, prologueEnd,
                                                   epilogueBegin));
                discriminator = 0;
                basicBlock = prologueEnd = epilogueBegin = false;
                break;

            case DW_LNS_advance_pc:
                operationAdvance = stream->decodeULeb128();
                address = getNewAddress(operationAdvance);
                opIndex = getNewOpIndex(operationAdvance);
                break;

            case DW_LNS_advance_line: line += stream->decodeLeb128(); break;

            case DW_LNS_set_file: file = stream->decodeULeb128(); break;

            case DW_LNS_set_column: column = stream->decodeULeb128(); break;

            case DW_LNS_negate_stmt: isStmt = !isStmt;  break;

            case DW_LNS_set_basic_block: basicBlock = true; break;

            case DW_LNS_const_add_pc:
                adjustedOpcode = 255 - infoHeader.opcodeBase;
                operationAdvance = adjustedOpcode / infoHeader.lineRange;
                address = getNewAddress(operationAdvance);
                opIndex = getNewOpIndex(operationAdvance);
                break;

            case DW_LNS_fixed_advance_pc:
                address += stream->decodeUInt16();
                opIndex = 0;
                break;

            case DW_LNS_set_prologue_end: prologueEnd = true; break;

            case DW_LNS_set_epilogue_end: epilogueBegin = true; break;

            case DW_LNS_set_isa: isa = stream->decodeULeb128(); break;
            }
        } else { // Extended
            stream->decodeULeb128(); // instruction length
            opcode = stream->decodeUInt8();

            switch (opcode) {
            case DW_LNE_end_sequence:
                endSequence = true;
                lineMatrix.push_back(new LineEntry(address, line, column, file,
                                                   isStmt, prologueEnd,
                                                   epilogueBegin));
                clearRegisters();
                break;

            case DW_LNE_set_address:
                address = stream->decodeUInt32();
                opIndex = 0;
                break;

            case DW_LNE_set_discriminator:
                discriminator = stream->decodeULeb128();
                break;
            }
        }
    }
}

void LineNumberInfo::parseProgramHeader() {
    infoHeader = {
        .unitLength = stream->decodeUInt32(),
        .version = stream->decodeUInt16(),
        .addressSize = stream->decodeUInt8(),
        .segmentSelectorSize = stream->decodeUInt8(),
        .headerLength = stream->decodeUInt32(),
        .minimumInstructionLength = stream->decodeUInt8(),
        .maximumOperationsPerInstruction = stream->decodeUInt8(),
        .defaultIsStmt = stream->decodeUInt8(),
        .lineBase = (int8_t)stream->decodeUInt8(),
        .lineRange = stream->decodeUInt8(),
        .opcodeBase = stream->decodeUInt8()
    };

    for (int i = 0; i < infoHeader.opcodeBase - 1; i++) {
        infoHeader.standardOpcodeLengths.push_back(stream->decodeUInt8());
    }

    // Directory Information
    generateFileFormatInfo(infoHeader.directoryEntryFormatCount,
                           infoHeader.directoryEntryFormat);
    generateFileInformation(infoHeader.directoriesCount, infoHeader.directories,
                            infoHeader.directoryEntryFormat);

    // File Information
    generateFileFormatInfo(infoHeader.fileNameEntryFormatCount,
                           infoHeader.fileNameEntryFormat);
    generateFileInformation(infoHeader.fileNamesCount, infoHeader.fileNames,
                            infoHeader.fileNameEntryFormat);
}

uint LineNumberInfo::getNewAddress(uint operationAdvance) {
    return address + infoHeader.minimumInstructionLength *
                         ((opIndex + operationAdvance) /
                          infoHeader.maximumOperationsPerInstruction);
}

uint LineNumberInfo::getNewOpIndex(uint operationAdvance) {
    return (opIndex + operationAdvance) %
           infoHeader.maximumOperationsPerInstruction;
}

LineNumberInfo::LineEntry *LineNumberInfo::getLineEntryAtPc(uint32_t pc) {
    for (LineEntry *lineEntry : lineMatrix) {
        if (lineEntry->address == pc) {
            return lineEntry;
        }
    }
    return nullptr;
}

CallFrameInfo::CallFrameInfo(uint8_t *debugFrameStart, uint8_t *debugFrameEnd) {
    size_t debugFrameLen = (uintptr_t)(debugFrameEnd - debugFrameStart);
    cfiStream = new DataStream(debugFrameStart, debugFrameLen);
    generateFrameInfo();
}

CallFrameInfo::~CallFrameInfo() {}

bool CallFrameInfo::getCfaAtLocation(uint32_t &res, uint32_t l1,
                                     RegisterFile *regs) {
    std::vector<TableEntry> virtualTable(1);
    if (!generateVirtTable(virtualTable, l1, regs)) { return false; }

    TableEntry &entry = virtualTable.back();
    res = regs->read(entry.regNum) + entry.regOffset;
    return true;
}

bool CallFrameInfo::getRegAtLocation(uint32_t &res, uint column, uint32_t l1,
                                     RegisterFile *regs) {
    std::vector<TableEntry> virtualTable(1);
    if (!generateVirtTable(virtualTable, l1, regs)) { return false; }

    return getUnwindedValue(res, virtualTable, column, regs,
                            virtualTable.size() - 1);
}

bool CallFrameInfo::generateVirtTable(std::vector<TableEntry> &virtTable,
                                      uint32_t l1, RegisterFile *regs) {
    // Find the FDE contaning addr
    FrameDescriptEntry *fde = nullptr;
    for (FrameDescriptEntry *entry : fdes) {
        uint32_t baseAddr = entry->initialLocation;
        if (l1 >= baseAddr && l1 < baseAddr + entry->addressRange) {
            fde = entry;
        }
    }
    if (fde == nullptr || cies.find(fde->ciePointer) == cies.end()) {
        return false;
    }

    // 1. Initialize Register Set (Virtual Table) and read CIE's initial
    // instructions, then set L2 to initialLocation
    CommonInfoEntry *cie = cies[fde->ciePointer];
    DataStream cieStream(cie->initialInstructions, cie->instructionsBlockSize);
    uint32_t l2 = fde->initialLocation;
    for (uint i = 0; i < 32; i++) {
        virtTable.back().regRules[i] = {.cfiRule = cfiUndefined};
    }
    virtTable[0].location = fde->initialLocation;
    processInstructions(virtTable, cie, cieStream, regs, l1, l2);

    // 2. Process FDE instruction stream
    DataStream fdeStream(fde->instructions, fde->instructionsBlockSize);
    processInstructions(virtTable, cie, fdeStream, regs, l1, l2);

    return true;
}

bool CallFrameInfo::getUnwindedValue(uint32_t &res,
                                     std::vector<TableEntry> &virtTable,
                                     uint regCol, RegisterFile *regs,
                                     int depth) {
    return false;
}

void CallFrameInfo::generateFrameInfo() {
    while (cfiStream->isStreamable()) {
        size_t entryLen;
        size_t debugFrameOffset = cfiStream->getIndex();
        uint32_t length = cfiStream->decodeUInt32();
        uint32_t identifier = cfiStream->decodeUInt32();

        if (identifier == 0xffffffff) {
            // 2nd param specifies CIE's byte offset into .debug_frame
            CommonInfoEntry *cie = parseCie(length, identifier,
                                            cfiStream->getIndex() - 8);
            entryLen = cie->getLen();
        } else {
            FrameDescriptEntry *fde = parseFde(length, identifier);
            entryLen = fde->getLen();
        }

        cfiStream->setOffset(debugFrameOffset + entryLen);
    }
}

CallFrameInfo::CommonInfoEntry *CallFrameInfo::parseCie(
    uint32_t length, uint32_t cieId, uint32_t debugFrameOffset) {
    CommonInfoEntry *cie = new CommonInfoEntry();
    cie->length = length;
    cie->cieId = cieId;
    cie->version = cfiStream->decodeUInt8();
    
    char c;
    while ((c = cfiStream->decodeUInt8())) {
        cie->augmentation.push_back(c);
    }
    
    // cie->addressSize = cfiStream->decodeUInt8();
    // cie->segmentSelectorSize = cfiStream->decodeUInt8();
    cie->codeAlignmentFactor = cfiStream->decodeULeb128();
    cie->dataAlignmentFactor = cfiStream->decodeLeb128();
    cie->returnAddressRegister = cfiStream->decodeULeb128();
    cie->initialInstructions = cfiStream->getData(cfiStream->getIndex());
    cie->instructionsBlockSize =
        cie->getLen() - (cfiStream->getIndex() - debugFrameOffset);
    cies.insert({debugFrameOffset, cie});

    return cie;
}

CallFrameInfo::FrameDescriptEntry *CallFrameInfo::parseFde(
    uint32_t length, uint32_t ciePointer) {
    uint startIdx = cfiStream->getIndex() - 8; // First index into FDE
    FrameDescriptEntry *fde = new FrameDescriptEntry();
    fde->length = length;
    fde->ciePointer = ciePointer;
    fde->initialLocation = cfiStream->decodeUInt32();
    fde->addressRange = cfiStream->decodeUInt32();
    fde->instructions = cfiStream->getData(cfiStream->getIndex());
    fde->instructionsBlockSize =
        fde->getLen() - (cfiStream->getIndex() - startIdx);
    fdes.push_back(fde);

    return fde;
}

void CallFrameInfo::processInstructions(std::vector<TableEntry> &virtTable,
                             CommonInfoEntry *cie, DataStream &instrStream,
                             RegisterFile *regs, uint32_t l1, uint32_t l2) {
    while (instrStream.isStreamable()) {
        uint8_t opcode = instrStream.decodeUInt8();

        uint8_t primaryOpcode = opcode & 0xc0;
        if (primaryOpcode) {
            uint8_t operand = opcode & 0x4f;

            switch (primaryOpcode) {
            case DW_CFA_advance_loc: {
                TableEntry entry(virtTable.back());
                entry.location = virtTable.back().location +
                                 operand * cie->codeAlignmentFactor;
                virtTable.push_back(entry);
                break;
            }

            case DW_CFA_offset:
                virtTable.back().regRules[operand].value =
                    instrStream.decodeULeb128() * cie->dataAlignmentFactor;
                break;

            case DW_CFA_restore:
                virtTable.back().regRules[operand].cfiRule =
                    virtTable.front().regRules[operand].cfiRule;
                break;

            default:
                printf("Illegal CFA: %x\n", primaryOpcode); exit(1);
            }
            continue;
        }

        switch (opcode) {
        case DW_CFA_nop: break;
        
        case DW_CFA_advance_loc1: {
            uint32_t delta = instrStream.decodeUInt8();
            TableEntry entry(virtTable.back());
            entry.location = virtTable.back().location + delta;
            virtTable.push_back(entry);
            break;
        }

        case DW_CFA_def_cfa: {
            virtTable.back().regNum = instrStream.decodeULeb128();
            virtTable.back().regOffset = instrStream.decodeULeb128();
            break;
        }

        case DW_CFA_def_cfa_register:
            if (virtTable.back().isCfaLocExpr) {
                printf("%s error\n", printCfa((CfaEncoding)opcode)); exit(1);
            }
            virtTable.back().regNum = instrStream.decodeULeb128();
            break;

        case DW_CFA_def_cfa_offset:
            if (virtTable.back().isCfaLocExpr) {
                printf("%s error\n", printCfa((CfaEncoding)opcode)); exit(1);
            }
            virtTable.back().regOffset = instrStream.decodeULeb128();
            break;

        default:
            printf("Unsupported CFA: %s\n",
                   printCfa((CfaEncoding)opcode));
            exit(EXIT_FAILURE);
        }
    }
}

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
    switch (abbrevEntry->getDieTag()) {
    case DW_TAG_variable:
    case DW_TAG_formal_parameter: return true;
    default: return false;
    }
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
    const ElfSectionHeader *debugLineHeader = getSectionHeader(".debug_line");
    const ElfSectionHeader *debugFrameHeader = getSectionHeader(".debug_frame");

    uint8_t *fileStart = elfFileInfo->elfData;

    uint8_t *debugInfoStart = fileStart + debugInfoHeader->offset;
    uint8_t *debugInfoEnd = debugInfoStart + debugInfoHeader->size;
    uint8_t *debugAbbrevStart = fileStart + debugAbbrevHeader->offset;
    uint8_t *debugStrStart = fileStart + debugStrHeader->offset;
    uint8_t *debugLineStrStart = fileStart + debugLineStrHeader->offset;
    uint8_t *debugLineStart = fileStart + debugLineHeader->offset;
    uint8_t *debugFrameStart = fileStart + debugFrameHeader->offset;
    uint8_t *debugFrameEnd = debugFrameStart + debugFrameHeader->size;    

    // Initialize Call Frame Information
    debugFrame = new CallFrameInfo(debugFrameStart, debugFrameEnd);

    // Initialize Compile Units
    for (;;) {
        // End of .debug_info section, CU parsing complete
        if ((uintptr_t)debugInfoStart >= ((uintptr_t)debugInfoEnd)) {
            break;
        }

        CompileUnit *compileUnit = new CompileUnit(debugInfoStart,
            debugAbbrevStart, debugStrStart, debugLineStrStart, debugLineStart);
        compileUnits.push_back(compileUnit);
        
        compileUnit->generateScopes();
        // compileUnit->printScopes();

        // Point to next .debug_info CU Header
        debugInfoStart += compileUnit->getDebugInfoLength();

        // Point to next .debug_line CU header
        debugLineStart += compileUnit->getDebugLineLength();
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

CompileUnit *DwarfParser::getCompileUnitAtPc(uint32_t pc) {
    // for (CompileUnit *compileUnit : compileUnits) {
    //     if (compileUnit->isPcInRange(pc)) {
    //         return compileUnit;
    //     }
    // }
    for (size_t i = compileUnits.size() - 1; i >= 0; i--) {
        if (compileUnits[i]->isPcInRange(pc)) {
            return compileUnits[i];
        }
    }
    return nullptr;
}

size_t DwarfParser::getNumCompileUnits() {
    return compileUnits.size();
}

// Get source information from all compile units
std::vector<SourceInfo *> &DwarfParser::getSourceInfo() {
    if (sourceInfo.empty()) {
        for (CompileUnit *cu : compileUnits) {
            for (SourceInfo *source : cu->getSourceInfo()) {
                if (!LineNumberInfo::containsPath(sourceInfo,
                                                  source->getPath())) {
                    sourceInfo.push_back(source);
                }
            }
        }
    }
    return sourceInfo;
}

uint DwarfParser::getLineNumberAtPc(uint32_t pc) {
    return getCompileUnitAtPc(pc)->getLineNumberAtPc(pc);
}

std::string &DwarfParser::getSourceNameAtPc(uint32_t pc) {
    return getCompileUnitAtPc(pc)->getSourceNameAtPc(pc);
}

void DwarfParser::getLocalVariables(std::vector<Variable *> &variables,
                                    uint32_t pc, uint line) {
    getCompileUnitAtPc(pc)->getLocalVariables(variables, pc, line);
}

void DwarfParser::getGlobalVariables(std::vector<Variable *> &variables,
                                     uint32_t pc, uint line) {
    getCompileUnitAtPc(pc)->getGlobalVariables(variables, pc, line);
}

void DwarfParser::getVarInfo(Variable::VarInfo &res, bool doUpdate,
                             Variable *var, RegisterFile *regs, uint32_t pc) {
    var->getVarInfo(res, doUpdate, regs, debugFrame, pc);
}