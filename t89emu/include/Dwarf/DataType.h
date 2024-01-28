#pragma once

#include <string>

#include "Dwarf/DebugInfoEntry.h"
#include "Dwarf/DwarfEncodings.h"
#include "Dwarf/DwarfParser.h"

class DebugInfoEntry;
class DwarfParser;

class DataType {
public:
    DataType(DebugInfoEntry *entry);
    ~DataType();

private:
    std::string name;
    AteEncoding baseEncoding;
    size_t byteSize;
    size_t bitSize;
};