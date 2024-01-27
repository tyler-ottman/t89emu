#pragma once

#include <map>
#include <vector>

#include "Dwarf/CompileUnit.h"
#include "Dwarf/DataStream.h"
#include "Dwarf/DwarfEncodings.h"

class CompileUnit;

// Data associated with a DIE Attribute Entry
class DebugData {
public:
    DebugData(FormEncoding form);
    DebugData(void);
    ~DebugData();

    void write(uint8_t *buff, size_t len);
    bool isString(void);

    const uint8_t *getData(void);
    FormEncoding getForm(void);
    uint64_t getUInt(void);
    size_t getLen(void);
    const char *getString(void);

private:
    std::vector<uint8_t> data;
    FormEncoding form;
};

class AttributeEntry {
public:
    AttributeEntry(AttributeEncoding name, FormEncoding form, size_t special);
    ~AttributeEntry();

    AttributeEncoding getName(void);
    FormEncoding getForm(void);
    size_t getSpecial(void);

private:
    AttributeEncoding name;
    FormEncoding form;
    size_t special;
};

class AbbrevEntry {
public:
    AbbrevEntry(size_t dieCode, TagEncoding dieTag, bool hasChild);
    ~AbbrevEntry();

    void addAttributeEntry(AttributeEntry *attEntry);

    size_t getNumAttributes(void);
    AttributeEntry *getAttributeEntry(size_t index);
    size_t getDieCode(void);
    TagEncoding getDieTag(void);
    bool hasChildren(void);

private:
    size_t dieCode;
    TagEncoding dieTag;
    bool hasChild;

    std::vector<AttributeEntry *> attEntries;
};

class AbbrevTable {
public:
    AbbrevTable(uint8_t *abbrevTableStart);
    ~AbbrevTable();

    AbbrevEntry *getAbbrevEntry(size_t dieCode);

private:
    std::map<size_t, AbbrevEntry *> abbrevEntries;

    DataStream *abbrevData;
};

class DebugInfoEntry {
public:
    DebugInfoEntry(CompileUnit *compileUnit, DebugInfoEntry *parent);
    ~DebugInfoEntry();

    void addAttribute(AttributeEncoding encoding, DebugData *data);
    void addChild(DebugInfoEntry *child);
    void printEntry(void);

    AbbrevEntry *getAbbrevEntry(void);
    DebugData *getAttribute(AttributeEncoding attribute);
    size_t getCode(void);
    TagEncoding getTag(void);
    size_t getNumChildren(void);
    DebugInfoEntry *getChild(size_t index);
    DebugInfoEntry *getParent(void);

    void setAbbrevEntry(AbbrevEntry *abbrevEntry);
    void setCode(size_t dieCode);

    bool isScope(void);
    bool isType(void);
    bool isVariable(void);

private:
    CompileUnit *compileUnit;
    
    AbbrevEntry *abbrevEntry;
    std::map<AttributeEncoding, DebugData *> attributes; // {Attribute, Data}

    std::vector<DebugInfoEntry *> children;
    DebugInfoEntry *parent;

    size_t code;
    // std::string name;
};