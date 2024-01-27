#pragma once

#include <string>

class StringTable {
public:
    StringTable(char *tableStart);
    ~StringTable();

    std::string getString(size_t offset);

private:
    char *tableStart;
};
