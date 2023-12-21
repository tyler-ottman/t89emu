#ifndef DWARFPARSER_H
#define DWARFPARSER_H

#include "ElfParser.h"

// Unit Header Unit Type Encodings
enum UnitType {
    DW_UT_compile                       = 0x01,
    DW_UT_type,
    DW_UT_partial,
    DW_UT_skeleton,
    DW_UT_split_compile,
    DW_UT_split_type,
    DW_UT_lo_user                       = 0x80,
    DW_UT_hi_user                       = 0xff
};

// Tag Encodings
enum TagEncodings {
    DW_TAG_array_type                   = 0x01,
    DW_TAG_class_type,
    DW_TAG_entry_point,
    DW_TAG_enumeration_type,
    DW_TAG_formal_parameter,
    Reserved,
    Reserved,
    DW_TAG_imported_declaration,
    Reserved,
    DW_TAG_label,
    DW_TAG_lexical_block,
    Reserved,
    DW_TAG_member,
    Reserved,
    DW_TAG_pointer_type,
    DW_TAG_reference_type,
    DW_TAG_compile_unit,
    DW_TAG_string_type,
    DW_TAG_structure_type,
    Reserved,
    DW_TAG_subroutine_type,
    DW_TAG_typedef,
    DW_TAG_union_type,
    DW_TAG_unspecified_parameters,
    DW_TAG_variant,
    DW_TAG_common_block,
    DW_TAG_common_inclusion,
    DW_TAG_inheritance,
    DW_TAG_inlined_subroutine,
    DW_TAG_module,
    DW_TAG_ptr_to_member_type,
    DW_TAG_set_type,
    DW_TAG_subrange_type,
    DW_TAG_with_stmt,
    DW_TAG_access_declaration,
    DW_TAG_base_type,
    DW_TAG_catch_block,
    DW_TAG_const_type,
    DW_TAG_constant,
    DW_TAG_enumerator,
    DW_TAG_file_type,
    DW_TAG_friend,
    DW_TAG_namelist,
    DW_TAG_namelist_item,
    DW_TAG_packed_type,
    DW_TAG_subprogram,
    DW_TAG_template_type_parameter,
    DW_TAG_template_value_parameter,
    DW_TAG_thrown_type,
    DW_TAG_try_block,
    DW_TAG_variant_part,
    DW_TAG_variable,
    DW_TAG_volatile_type,
    DW_TAG_dwarf_procedure,
    DW_TAG_restrict_type,
    DW_TAG_interface_type,
    DW_TAG_namespace,
    DW_TAG_imported_module,
    DW_TAG_unspecified_type,
    DW_TAG_partial_unit,
    DW_TAG_imported_unit,
    Reserved,
    DW_TAG_condition,
    DW_TAG_shared_type,
    DW_TAG_type_unit,
    DW_TAG_rvalue_reference_type,
    DW_TAG_template_alias,
    DW_TAG_coarray_type,
    DW_TAG_generic_subrange,
    DW_TAG_dynamic_type,
    DW_TAG_atomic_type,
    DW_TAG_call_site,
    DW_TAG_call_site_parameter,
    DW_TAG_skeleton_unit,
    DW_TAG_immutable_type,
    DW_TAG_lo_user                      = 0x4080,
    DW_TAG_hi_user                      = 0xffff
};

struct BaseUnitHeader {
    uint32_t unitLength;
    uint16_t version;
    uint8_t unitType;
    uint8_t addressSize;
    uint32_t debugAbbrevOffset;
};

struct CompileUnitHeader {
    struct BaseUnitHeader;
};

class DwarfParser : public ElfParser {
public:
    DwarfParser(const char *fileName);
    ~DwarfParser();

    void printDebugSection(const char *name);
    
private:
    uint8_t *debugAbbrev;
    uint8_t *debugInfo;
};

#endif // DWARFPARSER_H