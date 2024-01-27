#include "Dwarf/StringTable.h"

StringTable::StringTable(char *tableStart) : tableStart(tableStart) {}

StringTable::~StringTable() {}

std::string StringTable::getString(size_t offset) {
    std::string res;
    for (int i = offset; tableStart[i] != '\0'; i++) {
        res += tableStart[i];
    }
    return res;
}