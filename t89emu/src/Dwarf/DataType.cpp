#include "Architecture.h"
#include "Dwarf/DataType.h"

DataType::DataType(DebugInfoEntry *entry) : baseEncoding(AteReserved0),
    byteSize(4) {
    // Type name
    DebugData *attEntry = entry->getAttribute(DW_AT_name);
    name = attEntry ? attEntry->getString() : "";

    // If type has base encoding
    attEntry = entry->getAttribute(DW_AT_encoding);
    if (attEntry) {
        baseEncoding = (AteEncoding)attEntry->getUInt();
    }

    // Should be little endian
    attEntry = entry->getAttribute(DW_AT_endianity);
    ASSERT(!(attEntry && attEntry->getUInt() == DW_END_big),
             "Unexpected DW_END_big\n");
    
    // Byte size
    attEntry = entry->getAttribute(DW_AT_byte_size);
    if (attEntry) {
        byteSize = attEntry->getUInt();
    }


}

DataType::~DataType() {}