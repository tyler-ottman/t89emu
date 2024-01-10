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

void DataStream::setOffset(size_t offset) { index = offset; }

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
    for (std::map<ContentEncoding, DebugData *> &file : infoHeader.fileNames) {
        uint directoryIndex = file[DW_LNCT_directory_index]->getUInt();
        std::string fileName = dirPaths[directoryIndex] + "/"
            + file[DW_LNCT_path]->getString();
        if (!isFileInFilePaths(fileName)) {
            filePaths.push_back(fileName);
        }
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

bool LineNumberInfo::isFileInFilePaths(std::string &filePath) {
    for (std::string &path : filePaths) {
        if (path == filePath) { return true; }
    }
    return false;
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
    return abbrevEntry->getDieTag() == DW_TAG_variable;
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

bool CompileUnit::isPcInRange(uint32_t pc) {
    return rootScope->isPcInRange(pc);
}

void CompileUnit::printScopes() {
    DebugData *nameAtt = root->getAttribute(DW_AT_name);
    const char *cuName = nameAtt ? nameAtt->getString() : "";
    printf("\nCU: %s\n", cuName);
    rootScope->printScopes(0);
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

    uint8_t *fileStart = elfFileInfo->elfData;

    uint8_t *debugInfoStart = fileStart + debugInfoHeader->offset;
    uint8_t *debugInfoEnd = debugInfoStart + debugInfoHeader->size;
    uint8_t *debugAbbrevStart = fileStart + debugAbbrevHeader->offset;
    uint8_t *debugStrStart = fileStart + debugStrHeader->offset;
    uint8_t *debugLineStrStart = fileStart + debugLineStrHeader->offset;
    uint8_t *debugLineStart = fileStart + debugLineHeader->offset;

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

CompileUnit *DwarfParser::getCompileUnit(size_t fileIdx) {
    return compileUnits[fileIdx];
}

size_t DwarfParser::getNumCompileUnits() {
    return compileUnits.size();
}