#ifndef DWARFENCODINGS_H
#define DWARFENCODINGS_H

#include <iostream>
#include <utility>

// Expression rules
#define DEFINE(name, value) name = value,
#define CASE(name, value) case name: return #name;

// Unit Header Unit Type Encodings
#define UNIT_TYPE(EXPR)                         \
    EXPR(DW_UT_compile, 0x01)                   \
    EXPR(DW_UT_type, 0x02)                      \
    EXPR(DW_UT_partial, 0x03)                   \
    EXPR(DW_UT_skeleton, 0x04)                  \
    EXPR(DW_UT_split_compile, 0x05)             \
    EXPR(DW_UT_split_type, 0x6)                 \
    EXPR(DW_UT_lo_user, 0x80)                   \
    EXPR(DW_UT_hi_user, 0xff)

enum UnitType {
    UNIT_TYPE(DEFINE)
};

// Tag Encodings
#define TAG_ENCODING(EXPR)                      \
    EXPR(DW_TAG_array_type, 0x01)               \
    EXPR(DW_TAG_class_type, 0x02)               \
    EXPR(DW_TAG_entry_point, 0x03)              \
    EXPR(DW_TAG_enumeration_type, 0x04)         \
    EXPR(DW_TAG_formal_parameter, 0x05)         \
    EXPR(TagReserved0, 0x06)                    \
    EXPR(TagReserved1, 0x07)                    \
    EXPR(DW_TAG_imported_declaration, 0x08)     \
    EXPR(TagReserved2, 0x09)                    \
    EXPR(DW_TAG_label, 0x0a)                    \
    EXPR(DW_TAG_lexical_block, 0x0b)            \
    EXPR(TagReserved3, 0x0c)                    \
    EXPR(DW_TAG_member, 0x0d)                   \
    EXPR(TagReserved4, 0x0e)                    \
    EXPR(DW_TAG_pointer_type, 0x0f)             \
    EXPR(DW_TAG_reference_type, 0x10)           \
    EXPR(DW_TAG_compile_unit, 0x11)             \
    EXPR(DW_TAG_string_type, 0x12)              \
    EXPR(DW_TAG_structure_type, 0x13)           \
    EXPR(TagReserved5, 0x14)                    \
    EXPR(DW_TAG_subroutine_type, 0x15)          \
    EXPR(DW_TAG_typedef, 0x16)                  \
    EXPR(DW_TAG_union_type, 0x17)               \
    EXPR(DW_TAG_unspecified_parameters, 0x18)   \
    EXPR(DW_TAG_variant, 0x19)                  \
    EXPR(DW_TAG_common_block, 0x1a)             \
    EXPR(DW_TAG_common_inclusion, 0x1b)         \
    EXPR(DW_TAG_inheritance, 0x1c)              \
    EXPR(DW_TAG_inlined_subroutine, 0x1d)       \
    EXPR(DW_TAG_module, 0x1e)                   \
    EXPR(DW_TAG_ptr_to_member_type, 0x1f)       \
    EXPR(DW_TAG_set_type, 0x20)                 \
    EXPR(DW_TAG_subrange_type, 0x21)            \
    EXPR(DW_TAG_with_stmt, 0x22)                \
    EXPR(DW_TAG_access_declaration, 0x23)       \
    EXPR(DW_TAG_base_type, 0x24)                \
    EXPR(DW_TAG_catch_block, 0x25)              \
    EXPR(DW_TAG_const_type, 0x26)               \
    EXPR(DW_TAG_constant, 0x27)                 \
    EXPR(DW_TAG_enumerator, 0x28)               \
    EXPR(DW_TAG_file_type, 0x29)                \
    EXPR(DW_TAG_friend, 0x2a)                   \
    EXPR(DW_TAG_namelist, 0x2b)                 \
    EXPR(DW_TAG_namelist_item, 0x2c)            \
    EXPR(DW_TAG_packed_type, 0x2d)              \
    EXPR(DW_TAG_subprogram, 0x2e)               \
    EXPR(DW_TAG_template_type_parameter, 0x2f)  \
    EXPR(DW_TAG_template_value_parameter, 0x30) \
    EXPR(DW_TAG_thrown_type, 0x31)              \
    EXPR(DW_TAG_try_block, 0x32)                \
    EXPR(DW_TAG_variant_part, 0x33)             \
    EXPR(DW_TAG_variable, 0x34)                 \
    EXPR(DW_TAG_volatile_type, 0x35)            \
    EXPR(DW_TAG_dwarf_procedure, 0x36)          \
    EXPR(DW_TAG_restrict_type, 0x37)            \
    EXPR(DW_TAG_interface_type, 0x38)           \
    EXPR(DW_TAG_namespace, 0x39)                \
    EXPR(DW_TAG_imported_module, 0x3a)          \
    EXPR(DW_TAG_unspecified_type, 0x3b)         \
    EXPR(DW_TAG_partial_unit, 0x3c)             \
    EXPR(DW_TAG_imported_unit, 0x3d)            \
    EXPR(TagReserved6, 0x3e)                    \
    EXPR(DW_TAG_condition, 0x3f)                \
    EXPR(DW_TAG_shared_type, 0x40)              \
    EXPR(DW_TAG_type_unit, 0x41)                \
    EXPR(DW_TAG_rvalue_reference_type, 0x42)    \
    EXPR(DW_TAG_template_alias, 0x43)           \
    EXPR(DW_TAG_coarray_type, 0x44)             \
    EXPR(DW_TAG_generic_subrange, 0x45)         \
    EXPR(DW_TAG_dynamic_type, 0x46)             \
    EXPR(DW_TAG_atomic_type, 0x47)              \
    EXPR(DW_TAG_call_site, 0x48)                \
    EXPR(DW_TAG_call_site_parameter, 0x49)      \
    EXPR(DW_TAG_skeleton_unit, 0x4a)            \
    EXPR(DW_TAG_immutable_type, 0x4b)           \
    EXPR(DW_TAG_lo_user, 0x4080)                \
    EXPR(DW_TAG_hi_user, 0xffff)

enum TagEncoding {
    TAG_ENCODING(DEFINE)
};

// Child Determination Encodings
#define CHILD_ENCODING(EXPR)                    \
    EXPR(DW_CHILDREN_no, 0x00)                  \
    EXPR(DW_CHILDREN_yes, 0x01)

enum ChildEncoding {
    CHILD_ENCODING(DEFINE)
};

// Attribute Encodings
#define ATTRIBUTE_ENCODING(EXPR)                \
    EXPR(DW_AT_sibling, 0x01)                   \
    EXPR(DW_AT_location, 0x02)                  \
    EXPR(DW_AT_name, 0x03)                      \
    EXPR(AtReserved0, 0x04)                     \
    EXPR(AtReserved1, 0x05)                     \
    EXPR(AtReserved2, 0x06)                     \
    EXPR(AtReserved3, 0x07)                     \
    EXPR(AtReserved4, 0x08)                     \
    EXPR(DW_AT_ordering, 0x09)                  \
    EXPR(AtReserved5, 0x0a)                     \
    EXPR(DW_AT_byte_size, 0x0b)                 \
    EXPR(AtReserved6, 0x0c)                     \
    EXPR(DW_AT_bit_size, 0x0d)                  \
    EXPR(AtReserved7, 0x0e)                     \
    EXPR(AtReserved8, 0x0f)                     \
    EXPR(DW_AT_stmt_list, 0x10)                 \
    EXPR(DW_AT_low_pc, 0x11)                    \
    EXPR(DW_AT_high_pc, 0x12)                   \
    EXPR(DW_AT_language, 0x13)                  \
    EXPR(AtReserved9, 0x14)                     \
    EXPR(DW_AT_discr, 0x15)                     \
    EXPR(DW_AT_discr_value, 0x16)               \
    EXPR(DW_AT_visibility, 0x17)                \
    EXPR(DW_AT_import, 0x18)                    \
    EXPR(DW_AT_string_length, 0x19)             \
    EXPR(DW_AT_common_reference, 0x1a)          \
    EXPR(DW_AT_comp_dir, 0x1b)                  \
    EXPR(DW_AT_const_value, 0x1c)               \
    EXPR(DW_AT_containing_type, 0x1d)           \
    EXPR(DW_AT_default_value, 0x1e)             \
    EXPR(AtReserved10, 0x1f)                    \
    EXPR(DW_AT_inline, 0x20)                    \
    EXPR(DW_AT_is_optional, 0x21)               \
    EXPR(DW_AT_lower_bound, 0x22)               \
    EXPR(AtReserved11, 0x23)                    \
    EXPR(AtReserved12, 0x24)                    \
    EXPR(DW_AT_producer, 0x25)                  \
    EXPR(AtReserved13, 0x26)                    \
    EXPR(DW_AT_prototyped, 0x27)                \
    EXPR(AtReserved14, 0x28)                    \
    EXPR(AtReserved15, 0x29)                    \
    EXPR(DW_AT_return_addr, 0x2a)               \
    EXPR(AtReserved16, 0x2b)                    \
    EXPR(DW_AT_start_scope, 0x2c)               \
    EXPR(AtReserved17, 0x2d)                    \
    EXPR(DW_AT_bit_stride, 0x2e)                \
    EXPR(DW_AT_upper_bound, 0x2f)               \
    EXPR(AtReserved18, 0x30)                    \
    EXPR(DW_AT_abstract_origin, 0x31)           \
    EXPR(DW_AT_accessibility, 0x32)             \
    EXPR(DW_AT_address_class, 0x33)             \
    EXPR(DW_AT_artificial, 0x34)                \
    EXPR(DW_AT_base_types, 0x35)                \
    EXPR(DW_AT_calling_convention, 0x036)       \
    EXPR(DW_AT_count, 0x037)                    \
    EXPR(DW_AT_data_member_location, 0x38)      \
    EXPR(DW_AT_decl_column, 0x39)               \
    EXPR(DW_AT_decl_file, 0x3a)                 \
    EXPR(DW_AT_decl_line, 0x3b)                 \
    EXPR(DW_AT_declaration, 0x3c)               \
    EXPR(DW_AT_discr_list, 0x3d)                \
    EXPR(DW_AT_encoding, 0x3e)                  \
    EXPR(DW_AT_external, 0x3f)                  \
    EXPR(DW_AT_frame_base, 0x40)                \
    EXPR(DW_AT_friend, 0x41)                    \
    EXPR(DW_AT_identifier_case, 0x42)           \
    EXPR(AtReserved19, 0x43)                    \
    EXPR(DW_AT_namelist_item, 0x44)             \
    EXPR(DW_AT_priority, 0x45)                  \
    EXPR(DW_AT_segment, 0x46)                   \
    EXPR(DW_AT_specification, 0x47)             \
    EXPR(DW_AT_static_link, 0x48)               \
    EXPR(DW_AT_type, 0x49)                      \
    EXPR(DW_AT_use_location, 0x4a)              \
    EXPR(DW_AT_variable_parameter, 0x4b)        \
    EXPR(DW_AT_virtuality, 0x4c)                \
    EXPR(DW_AT_vtable_elem_location, 0x4d)      \
    EXPR(DW_AT_allocated, 0x4e)                 \
    EXPR(DW_AT_associated, 0x4f)                \
    EXPR(DW_AT_data_location, 0x50)             \
    EXPR(DW_AT_byte_stride, 0x51)               \
    EXPR(DW_AT_entry_pc, 0x52)                  \
    EXPR(DW_AT_use_UTF8, 0x53)                  \
    EXPR(DW_AT_extension, 0x54)                 \
    EXPR(DW_AT_ranges, 0x55)                    \
    EXPR(DW_AT_trampoline, 0x56)                \
    EXPR(DW_AT_call_column, 0x57)               \
    EXPR(DW_AT_call_file, 0x58)                 \
    EXPR(DW_AT_call_line, 0x59)                 \
    EXPR(DW_AT_description, 0x5a)               \
    EXPR(DW_AT_binary_scale, 0x5b)              \
    EXPR(DW_AT_decimal_scale, 0x5c)             \
    EXPR(DW_AT_small, 0x5d)                     \
    EXPR(DW_AT_decimal_sign, 0x5e)              \
    EXPR(DW_AT_digit_count, 0x5f)               \
    EXPR(DW_AT_picture_string, 0x60)            \
    EXPR(DW_AT_mutable, 0x61)                   \
    EXPR(DW_AT_threads_scaled, 0x62)            \
    EXPR(DW_AT_explicit, 0x63)                  \
    EXPR(DW_AT_object_pointer, 0x64)            \
    EXPR(DW_AT_endianity, 0x65)                 \
    EXPR(DW_AT_elemental, 0x66)                 \
    EXPR(DW_AT_pure, 0x67)                      \
    EXPR(DW_AT_recursive, 0x68)                 \
    EXPR(DW_AT_signature, 0x69)                 \
    EXPR(DW_AT_main_subprogram, 0x6a)           \
    EXPR(DW_AT_data_bit_offset, 0x6b)           \
    EXPR(DW_AT_const_expr, 0x6c)                \
    EXPR(DW_AT_enum_class, 0x6d)                \
    EXPR(DW_AT_linkage_name, 0x6e)              \
    EXPR(DW_AT_string_length_bit_size, 0x6f)    \
    EXPR(DW_AT_string_length_byte_size, 0x70)   \
    EXPR(DW_AT_rank, 0x71)                      \
    EXPR(DW_AT_str_offsets_base, 0x72)          \
    EXPR(DW_AT_addr_base, 0x73)                 \
    EXPR(DW_AT_rnglists_base, 0x74)             \
    EXPR(AtReserved20, 0x75)                    \
    EXPR(DW_AT_dwo_name, 0x76)                  \
    EXPR(DW_AT_reference, 0x77)                 \
    EXPR(DW_AT_rvalue_reference, 0x78)          \
    EXPR(DW_AT_macros, 0x79)                    \
    EXPR(DW_AT_call_all_calls, 0x7a)            \
    EXPR(DW_AT_call_all_source_calls, 0x7b)     \
    EXPR(DW_AT_call_all_tail_calls, 0x7c)       \
    EXPR(DW_AT_call_return_pc, 0x7d)            \
    EXPR(DW_AT_call_value, 0x7e)                \
    EXPR(DW_AT_call_origin, 0x7f)               \
    EXPR(DW_AT_call_parameter, 0x80)            \
    EXPR(DW_AT_call_pc, 0x81)                   \
    EXPR(DW_AT_call_tail_call, 0x82)            \
    EXPR(DW_AT_call_target, 0x83)               \
    EXPR(DW_AT_call_target_clobbered, 0x84)     \
    EXPR(DW_AT_call_data_location, 0x85)        \
    EXPR(DW_AT_call_data_value, 0x86)           \
    EXPR(DW_AT_noreturn, 0x87)                  \
    EXPR(DW_AT_alignment, 0x88)                 \
    EXPR(DW_AT_export_symbols, 0x89)            \
    EXPR(DW_AT_deleted, 0x8a)                   \
    EXPR(DW_AT_defaulted, 0x8b)                 \
    EXPR(DW_AT_loclists_base, 0x8c)             \
    EXPR(DW_AT_lo_user, 0x2000)                 \
    EXPR(DW_AT_hi_user, 0x3fff)

enum AttributeEncoding {
    ATTRIBUTE_ENCODING(DEFINE)
};

// Form Encodings
#define FORM_ENCODING(EXPR)                     \
    EXPR(DW_FORM_addr, 0x01)                    \
    EXPR(FormReserved0, 0x02)                   \
    EXPR(DW_FORM_block2, 0x03)                  \
    EXPR(DW_FORM_block4, 0x04)                  \
    EXPR(DW_FORM_data2, 0x05)                   \
    EXPR(DW_FORM_data4, 0x06)                   \
    EXPR(DW_FORM_data8, 0x07)                   \
    EXPR(DW_FORM_string, 0x08)                  \
    EXPR(DW_FORM_block, 0x09)                   \
    EXPR(DW_FORM_block1, 0x0a)                  \
    EXPR(DW_FORM_data1, 0x0b)                   \
    EXPR(DW_FORM_flag, 0x0c)                    \
    EXPR(DW_FORM_sdata, 0x0d)                   \
    EXPR(DW_FORM_strp, 0x0e)                    \
    EXPR(DW_FORM_udata, 0x0f)                   \
    EXPR(DW_FORM_ref_addr, 0x10)                \
    EXPR(DW_FORM_ref1, 0x11)                    \
    EXPR(DW_FORM_ref2, 0x12)                    \
    EXPR(DW_FORM_ref4, 0x13)                    \
    EXPR(DW_FORM_ref8, 0x14)                    \
    EXPR(DW_FORM_ref_udata, 0x15)               \
    EXPR(DW_FORM_indirect, 0x16)                \
    EXPR(DW_FORM_sec_offset, 0x17)              \
    EXPR(DW_FORM_exprloc, 0x18)                 \
    EXPR(DW_FORM_flag_present, 0x19)            \
    EXPR(DW_FORM_strx, 0x1a)                    \
    EXPR(DW_FORM_addrx, 0x1b)                   \
    EXPR(DW_FORM_ref_sup4, 0x1c)                \
    EXPR(DW_FORM_strp_sup, 0x1d)                \
    EXPR(DW_FORM_data16, 0x1e)                  \
    EXPR(DW_FORM_line_strp, 0x1f)               \
    EXPR(DW_FORM_ref_sig8, 0x20)                \
    EXPR(DW_FORM_implicit_const, 0x21)          \
    EXPR(DW_FORM_loclistx, 0x22)                \
    EXPR(DW_FORM_rnglistx, 0x23)                \
    EXPR(DW_FORM_ref_sup8, 0x24)                \
    EXPR(DW_FORM_strx1, 0x25)                   \
    EXPR(DW_FORM_strx2, 0x26)                   \
    EXPR(DW_FORM_strx3, 0x27)                   \
    EXPR(DW_FORM_strx4, 0x28)                   \
    EXPR(DW_FORM_addrx1, 0x29)                  \
    EXPR(DW_FORM_addrx2, 0x2a)                  \
    EXPR(DW_FORM_addrx3, 0x2b)                  \
    EXPR(DW_FORM_addrx4, 0x2c)

enum FormEncoding {
    FORM_ENCODING(DEFINE)
};

#define OPERATION_ENCODING(EXPR)                \
    EXPR(DW_OP_null, 0x00)                      \
    EXPR(DW_OP_Reserved0, 0x01)                 \
    EXPR(DW_OP_Reserved1, 0x02)                 \
    EXPR(DW_OP_addr, 0x03)                      \
    EXPR(DW_OP_Reserved2, 0x04)                 \
    EXPR(DW_OP_Reserved3, 0x05)                 \
    EXPR(DW_OP_deref, 0x06)                     \
    EXPR(DW_OP_Reserved4, 0x07)                 \
    EXPR(DW_OP_const1u, 0x08)                   \
    EXPR(DW_OP_const1s, 0x09)                   \
    EXPR(DW_OP_const2u, 0x0a)                   \
    EXPR(DW_OP_const2s, 0x0b)                   \
    EXPR(DW_OP_const4u, 0x0c)                   \
    EXPR(DW_OP_const4s, 0x0d)                   \
    EXPR(DW_OP_const8u, 0x0e)                   \
    EXPR(DW_OP_const8s, 0x0f)                   \
    EXPR(DW_OP_constu, 0x10)                    \
    EXPR(DW_OP_consts, 0x11)                    \
    EXPR(DW_OP_dup, 0x12)                       \
    EXPR(DW_OP_drop, 0x13)                      \
    EXPR(DW_OP_over, 0x14)                      \
    EXPR(DW_OP_pick, 0x15)                      \
    EXPR(DW_OP_swap, 0x16)                      \
    EXPR(DW_OP_rot, 0x17)                       \
    EXPR(DW_OP_xderef, 0x18)                    \
    EXPR(DW_OP_abs, 0x19)                       \
    EXPR(DW_OP_and, 0x1a)                       \
    EXPR(DW_OP_div, 0x1b)                       \
    EXPR(DW_OP_minus, 0x1c)                     \
    EXPR(DW_OP_mod, 0x1d)                       \
    EXPR(DW_OP_mul, 0x1e)                       \
    EXPR(DW_OP_neg, 0x1f)                       \
    EXPR(DW_OP_not, 0x20)                       \
    EXPR(DW_OP_or, 0x21)                        \
    EXPR(DW_OP_plus, 0x22)                      \
    EXPR(DW_OP_plus_uconst, 0x23)               \
    EXPR(DW_OP_shl, 0x24)                       \
    EXPR(DW_OP_shr, 0x25)                       \
    EXPR(DW_OP_shra, 0x26)                      \
    EXPR(DW_OP_xor, 0x27)                       \
    EXPR(DW_OP_bra, 0x28)                       \
    EXPR(DW_OP_eq, 0x29)                        \
    EXPR(DW_OP_ge, 0x2a)                        \
    EXPR(DW_OP_gt, 0x2b)                        \
    EXPR(DW_OP_le, 0x2c)                        \
    EXPR(DW_OP_lt, 0x2d)                        \
    EXPR(DW_OP_ne, 0x2e)                        \
    EXPR(DW_OP_skip, 0x2f)                      \
    EXPR(DW_OP_lit0, 0x30)                      \
    EXPR(DW_OP_lit1, 0x31)                      \
    EXPR(DW_OP_lit2, 0x32)                      \
    EXPR(DW_OP_lit3, 0x33)                      \
    EXPR(DW_OP_lit4, 0x34)                      \
    EXPR(DW_OP_lit5, 0x35)                      \
    EXPR(DW_OP_lit6, 0x36)                      \
    EXPR(DW_OP_lit7, 0x37)                      \
    EXPR(DW_OP_lit8, 0x38)                      \
    EXPR(DW_OP_lit9, 0x39)                      \
    EXPR(DW_OP_lit10, 0x3a)                     \
    EXPR(DW_OP_lit11, 0x3b)                     \
    EXPR(DW_OP_lit12, 0x3c)                     \
    EXPR(DW_OP_lit13, 0x3d)                     \
    EXPR(DW_OP_lit14, 0x3e)                     \
    EXPR(DW_OP_lit15, 0x3f)                     \
    EXPR(DW_OP_lit16, 0x40)                     \
    EXPR(DW_OP_lit17, 0x41)                     \
    EXPR(DW_OP_lit18, 0x42)                     \
    EXPR(DW_OP_lit19, 0x43)                     \
    EXPR(DW_OP_lit20, 0x44)                     \
    EXPR(DW_OP_lit21, 0x45)                     \
    EXPR(DW_OP_lit22, 0x46)                     \
    EXPR(DW_OP_lit23, 0x47)                     \
    EXPR(DW_OP_lit24, 0x48)                     \
    EXPR(DW_OP_lit25, 0x49)                     \
    EXPR(DW_OP_lit26, 0x4a)                     \
    EXPR(DW_OP_lit27, 0x4b)                     \
    EXPR(DW_OP_lit28, 0x4c)                     \
    EXPR(DW_OP_lit29, 0x4d)                     \
    EXPR(DW_OP_lit30, 0x4e)                     \
    EXPR(DW_OP_lit31, 0x4f)                     \
    EXPR(DW_OP_reg0, 0x50)                      \
    EXPR(DW_OP_reg1, 0x51)                      \
    EXPR(DW_OP_reg2, 0x52)                      \
    EXPR(DW_OP_reg3, 0x53)                      \
    EXPR(DW_OP_reg4, 0x54)                      \
    EXPR(DW_OP_reg5, 0x55)                      \
    EXPR(DW_OP_reg6, 0x56)                      \
    EXPR(DW_OP_reg7, 0x57)                      \
    EXPR(DW_OP_reg8, 0x58)                      \
    EXPR(DW_OP_reg9, 0x59)                      \
    EXPR(DW_OP_reg10, 0x5a)                     \
    EXPR(DW_OP_reg11, 0x5b)                     \
    EXPR(DW_OP_reg12, 0x5c)                     \
    EXPR(DW_OP_reg13, 0x5d)                     \
    EXPR(DW_OP_reg14, 0x5e)                     \
    EXPR(DW_OP_reg15, 0x5f)                     \
    EXPR(DW_OP_reg16, 0x60)                     \
    EXPR(DW_OP_reg17, 0x61)                     \
    EXPR(DW_OP_reg18, 0x62)                     \
    EXPR(DW_OP_reg19, 0x63)                     \
    EXPR(DW_OP_reg20, 0x64)                     \
    EXPR(DW_OP_reg21, 0x65)                     \
    EXPR(DW_OP_reg22, 0x66)                     \
    EXPR(DW_OP_reg23, 0x67)                     \
    EXPR(DW_OP_reg24, 0x68)                     \
    EXPR(DW_OP_reg25, 0x69)                     \
    EXPR(DW_OP_reg26, 0x6a)                     \
    EXPR(DW_OP_reg27, 0x6b)                     \
    EXPR(DW_OP_reg28, 0x6c)                     \
    EXPR(DW_OP_reg29, 0x6d)                     \
    EXPR(DW_OP_reg30, 0x6e)                     \
    EXPR(DW_OP_reg31, 0x6f)                     \
    EXPR(DW_OP_breg0, 0x70)                     \
    EXPR(DW_OP_breg1, 0x71)                     \
    EXPR(DW_OP_breg2, 0x72)                     \
    EXPR(DW_OP_breg3, 0x73)                     \
    EXPR(DW_OP_breg4, 0x74)                     \
    EXPR(DW_OP_breg5, 0x75)                     \
    EXPR(DW_OP_breg6, 0x76)                     \
    EXPR(DW_OP_breg7, 0x77)                     \
    EXPR(DW_OP_breg8, 0x78)                     \
    EXPR(DW_OP_breg9, 0x79)                     \
    EXPR(DW_OP_breg10, 0x7a)                    \
    EXPR(DW_OP_breg11, 0x7b)                    \
    EXPR(DW_OP_breg12, 0x7c)                    \
    EXPR(DW_OP_breg13, 0x7d)                    \
    EXPR(DW_OP_breg14, 0x7e)                    \
    EXPR(DW_OP_breg15, 0x7f)                    \
    EXPR(DW_OP_breg16, 0x80)                    \
    EXPR(DW_OP_breg17, 0x81)                    \
    EXPR(DW_OP_breg18, 0x82)                    \
    EXPR(DW_OP_breg19, 0x83)                    \
    EXPR(DW_OP_breg20, 0x84)                    \
    EXPR(DW_OP_breg21, 0x85)                    \
    EXPR(DW_OP_breg22, 0x86)                    \
    EXPR(DW_OP_breg23, 0x87)                    \
    EXPR(DW_OP_breg24, 0x88)                    \
    EXPR(DW_OP_breg25, 0x89)                    \
    EXPR(DW_OP_breg26, 0x8a)                    \
    EXPR(DW_OP_breg27, 0x8b)                    \
    EXPR(DW_OP_breg28, 0x8c)                    \
    EXPR(DW_OP_breg29, 0x8d)                    \
    EXPR(DW_OP_breg30, 0x8e)                    \
    EXPR(DW_OP_breg31, 0x8f)                    \
    EXPR(DW_OP_regx, 0x90)                      \
    EXPR(DW_OP_fbreg, 0x91)                     \
    EXPR(DW_OP_bregx, 0x92)                     \
    EXPR(DW_OP_piece, 0x93)                     \
    EXPR(DW_OP_deref_size, 0x94)                \
    EXPR(DW_OP_xderef_size, 0x95)               \
    EXPR(DW_OP_nop, 0x96)                       \
    EXPR(DW_OP_push_object_address, 0x97)       \
    EXPR(DW_OP_call2, 0x98)                     \
    EXPR(DW_OP_call4, 0x99)                     \
    EXPR(DW_OP_call_ref, 0x9a)                  \
    EXPR(DW_OP_form_tls_address, 0x9b)          \
    EXPR(DW_OP_call_frame_cfa, 0x9c)            \
    EXPR(DW_OP_bit_piece, 0x9d)                 \
    EXPR(DW_OP_implicit_value, 0x9e)            \
    EXPR(DW_OP_stack_value, 0x9f)               \
    EXPR(DW_OP_implicit_pointer, 0xa0)          \
    EXPR(DW_OP_addrx, 0xa1)                     \
    EXPR(DW_OP_constx, 0xa2)                    \
    EXPR(DW_OP_entry_value, 0xa3)               \
    EXPR(DW_OP_const_type, 0xa4)                \
    EXPR(DW_OP_regval_type, 0xa5)               \
    EXPR(DW_OP_deref_type, 0xa6)                \
    EXPR(DW_OP_xderef_type, 0xa7)               \
    EXPR(DW_OP_convert, 0xa8)                   \
    EXPR(DW_OP_reinterpret, 0xa9)               \
    EXPR(DW_OP_lo_user, 0xe0)                   \
    EXPR(DW_OP_hi_user, 0xff)

enum OperationEncoding {
    OPERATION_ENCODING(DEFINE)
};

// Line Number Header Entry Format Encodings
#define CONTENT_ENCODING(EXPR)                  \
    EXPR(DW_LNCT_path, 0x01)                    \
    EXPR(DW_LNCT_directory_index, 0x02)         \
    EXPR(DW_LNCT_timestamp, 0x03)               \
    EXPR(DW_LNCT_size, 0x04)                    \
    EXPR(DW_LNCT_MD5, 0x05)                     \
    EXPR(DW_LNCT_lo_user, 0x2000)               \
    EXPR(DW_LNCT_hi_user, 0x3fff)

enum ContentEncoding {
    CONTENT_ENCODING(DEFINE)
};

// Line Number Standard Opcode Encodings
#define STANDARD_OPCODE_ENCODING(EXPR)          \
    EXPR(DW_LNS_copy, 0x01)                     \
    EXPR(DW_LNS_advance_pc, 0x02)               \
    EXPR(DW_LNS_advance_line, 0x03)             \
    EXPR(DW_LNS_set_file, 0x04)                 \
    EXPR(DW_LNS_set_column, 0x05)               \
    EXPR(DW_LNS_negate_stmt, 0x06)              \
    EXPR(DW_LNS_set_basic_block, 0x07)          \
    EXPR(DW_LNS_const_add_pc, 0x08)             \
    EXPR(DW_LNS_fixed_advance_pc, 0x09)         \
    EXPR(DW_LNS_set_prologue_end, 0x0a)         \
    EXPR(DW_LNS_set_epilogue_end, 0x0b)         \
    EXPR(DW_LNS_set_isa, 0x0c)

enum StandardOpcodeEncoding {
    STANDARD_OPCODE_ENCODING(DEFINE)
};

// Line Number Extended Opcode Encodings
#define EXTENDED_OPCODE_ENCODING(EXPR)          \
    EXPR(DW_LNE_end_sequence, 0x01)             \
    EXPR(DW_LNE_set_address, 0x02)              \
    EXPR(ExtOpReserved0, 0x03)                  \
    EXPR(DW_LNE_set_discriminator, 0x04)        \
    EXPR(DW_LNE_lo_user, 0x05)                  \
    EXPR(DW_LNE_hi_user, 0x06)

enum ExtendedOpcodeEncoding {
    EXTENDED_OPCODE_ENCODING(DEFINE)
};

// Call Frame Instruction Encodings
#define CFA_ENCODING(EXPR)                     \
    EXPR(DW_CFA_advance_loc, 0x40)              \
    EXPR(DW_CFA_offset, 0x80)                   \
    EXPR(DW_CFA_restore, 0xc0)                  \
    EXPR(DW_CFA_nop, 0x00)                      \
    EXPR(DW_CFA_set_loc, 0x01)                  \
    EXPR(DW_CFA_advance_loc1, 0x02)             \
    EXPR(DW_CFA_advance_loc2, 0x03)             \
    EXPR(DW_CFA_advance_loc4, 0x04)             \
    EXPR(DW_CFA_offset_extended, 0x05)          \
    EXPR(DW_CFA_restore_extended, 0x06)         \
    EXPR(DW_CFA_undefined, 0x07)                \
    EXPR(DW_CFA_same_value, 0x08)               \
    EXPR(DW_CFA_register, 0x09)                 \
    EXPR(DW_CFA_remember_state, 0x0a)           \
    EXPR(DW_CFA_restore_state, 0x0b)            \
    EXPR(DW_CFA_def_cfa, 0x0c)                  \
    EXPR(DW_CFA_def_cfa_register, 0x0d)         \
    EXPR(DW_CFA_def_cfa_offset, 0x0e)           \
    EXPR(DW_CFA_def_cfa_expression, 0x0f)       \
    EXPR(DW_CFA_expression, 0x10)               \
    EXPR(DW_CFA_offset_extended_sf, 0x11)       \
    EXPR(DW_CFA_def_cfa_sf, 0x12)               \
    EXPR(DW_CFA_def_cfa_offset_sf, 0x13)        \
    EXPR(DW_CFA_val_offset, 0x14)               \
    EXPR(DW_CFA_val_offset_sf, 0x15)            \
    EXPR(DW_CFA_val_expression, 0x16)           \
    EXPR(DW_CFA_lo_user, 0x1c)                  \
    EXPR(DW_CFA_hi_user, 0x3f)      

enum CfaEncoding {
    CFA_ENCODING(DEFINE)
};

#define PRINT_ENUM(NAME, ENUM, ENUM_LIST)       \
    static inline const char *NAME(ENUM type) { \
        switch (type) {                         \
        ENUM_LIST(CASE);                        \
        default: return nullptr;                \
        }                                       \
    }

PRINT_ENUM(printUnitType, UnitType, UNIT_TYPE)
PRINT_ENUM(printTag, TagEncoding, TAG_ENCODING)
PRINT_ENUM(printChild, ChildEncoding, CHILD_ENCODING)
PRINT_ENUM(printAttribute, AttributeEncoding, ATTRIBUTE_ENCODING)
PRINT_ENUM(printForm, FormEncoding, FORM_ENCODING)
PRINT_ENUM(printOperation, OperationEncoding, OPERATION_ENCODING)
PRINT_ENUM(printCfa, CfaEncoding, CFA_ENCODING)

#endif // DWARFENCODINGS_H
