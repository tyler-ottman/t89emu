#ifndef DWARFENCODINGS_H
#define DWARFENCODINGS_H

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
enum TagEncoding {
    DW_TAG_array_type                   = 0x01,
    DW_TAG_class_type,
    DW_TAG_entry_point,
    DW_TAG_enumeration_type,
    DW_TAG_formal_parameter,
    TagReserved0,
    TagReserved1,
    DW_TAG_imported_declaration,
    TagReserved2,
    DW_TAG_label,
    DW_TAG_lexical_block,
    TagReserved3,
    DW_TAG_member,
    TagReserved4,
    DW_TAG_pointer_type,
    DW_TAG_reference_type,
    DW_TAG_compile_unit,
    DW_TAG_string_type,
    DW_TAG_structure_type,
    TagReserved5,
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
    TagReserved6,
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

// Child Determination Encodings
enum ChildEncoding {
    DW_CHILDREN_no,
    DW_CHILDREN_yes
};

// Attribute Encodings
enum AttributeEncoding {
    DW_AT_sibling                       = 0x1,
    DW_AT_location,
    DW_AT_name,
    AtReserved0,
    AtReserved1,
    AtReserved2,
    AtReserved3,
    AtReserved4,
    DW_AT_ordering,
    AtReserved5,
    DW_AT_byte_size,
    AtReserved6,
    DW_AT_bit_size,
    AtReserved7,
    AtReserved8,
    DW_AT_stmt_list,
    DW_AT_low_pc,
    DW_AT_high_pc,
    DW_AT_language,
    AtReserved9,
    DW_AT_discr,
    DW_AT_discr_value,
    DW_AT_visibility,
    DW_AT_import,
    DW_AT_string_length,
    DW_AT_common_reference,
    DW_AT_comp_dir,
    DW_AT_const_value,
    DW_AT_containing_type,
    DW_AT_default_value,
    AtReserved10,
    DW_AT_inline,
    DW_AT_is_optional,
    DW_AT_lower_bound,
    AtReserved11,
    AtReserved12,
    DW_AT_producer,
    AtReserved13,
    DW_AT_prototyped,
    AtReserved14,
    AtReserved15,
    DW_AT_return_addr,
    AtReserved16,
    DW_AT_start_scope,
    AtReserved17,
    DW_AT_bit_stride,
    DW_AT_upper_bound,
    AtReserved18,
    DW_AT_abstract_origin,
    DW_AT_accessibility,
    DW_AT_address_class,
    DW_AT_artificial,
    DW_AT_base_types,
    DW_AT_calling_convention,
    DW_AT_count,
    DW_AT_data_member_location,
    DW_AT_decl_column,
    DW_AT_decl_file,
    DW_AT_decl_line,
    DW_AT_declaration,
    DW_AT_discr_list,
    DW_AT_encoding,
    DW_AT_external,
    DW_AT_frame_base,
    DW_AT_friend,
    DW_AT_identifier_case,
    AtReserved19,
    DW_AT_namelist_item,
    DW_AT_priority,
    DW_AT_segment,
    DW_AT_specification,
    DW_AT_static_link,
    DW_AT_type,
    DW_AT_use_location,
    DW_AT_variable_parameter,
    DW_AT_virtuality,
    DW_AT_vtable_elem_location,
    DW_AT_allocated,
    DW_AT_associated,
    DW_AT_data_location,
    DW_AT_byte_stride,
    DW_AT_entry_pc,
    DW_AT_use_UTF8,
    DW_AT_extension,
    DW_AT_ranges,
    DW_AT_trampoline,
    DW_AT_call_column,
    DW_AT_call_file,
    DW_AT_call_line,
    DW_AT_description,
    DW_AT_binary_scale,
    DW_AT_decimal_scale,
    DW_AT_small,
    DW_AT_decimal_sign,
    DW_AT_digit_count,
    DW_AT_picture_string,
    DW_AT_mutable,
    DW_AT_threads_scaled,
    DW_AT_explicit,
    DW_AT_object_pointer,
    DW_AT_endianity,
    DW_AT_elemental,
    DW_AT_pure,
    DW_AT_recursive,
    DW_AT_signature,
    DW_AT_main_subprogram,
    DW_AT_data_bit_offset,
    DW_AT_const_expr,
    DW_AT_enum_class,
    DW_AT_linkage_name,
    DW_AT_string_length_bit_size,
    DW_AT_string_length_byte_size,
    DW_AT_rank,
    DW_AT_str_offsets_base,
    DW_AT_addr_base,
    DW_AT_rnglists_base,
    AtReserved20,
    DW_AT_dwo_name,
    DW_AT_reference,
    DW_AT_rvalue_reference,
    DW_AT_macros,
    DW_AT_call_all_calls,
    DW_AT_call_all_source_calls,
    DW_AT_call_all_tail_calls,
    DW_AT_call_return_pc,
    DW_AT_call_value,
    DW_AT_call_origin,
    DW_AT_call_parameter,
    DW_AT_call_pc,
    DW_AT_call_tail_call,
    DW_AT_call_target,
    DW_AT_call_target_clobbered,
    DW_AT_call_data_location,
    DW_AT_call_data_value,
    DW_AT_noreturn,
    DW_AT_alignment,
    DW_AT_export_symbols,
    DW_AT_deleted,
    DW_AT_defaulted,
    DW_AT_loclists_base,
    DW_AT_lo_user                       = 0x2000,
    DW_AT_hi_user                       = 0x3fff,
};

// Attribute Form Encodings
enum FormEncoding {
    DW_FORM_addr                        = 0x01,
    FormReserved0,
    DW_FORM_block2,
    DW_FORM_block4,
    DW_FORM_data2,
    DW_FORM_data4,
    DW_FORM_data8,
    DW_FORM_string,
    DW_FORM_block,
    DW_FORM_block1,
    DW_FORM_data1,
    DW_FORM_flag,
    DW_FORM_sdata,
    DW_FORM_strp,
    DW_FORM_udata,
    DW_FORM_ref_addr,
    DW_FORM_ref1,
    DW_FORM_ref2,
    DW_FORM_ref4,
    DW_FORM_ref8,
    DW_FORM_ref_udata,
    DW_FORM_indirect,
    DW_FORM_sec_offset,
    DW_FORM_exprloc,
    DW_FORM_flag_present,
    DW_FORM_strx,
    DW_FORM_addrx,
    DW_FORM_ref_sup4,
    DW_FORM_strp_sup,
    DW_FORM_data16,
    DW_FORM_line_strp,
    DW_FORM_ref_sig8,
    DW_FORM_implicit_const,
    DW_FORM_loclistx,
    DW_FORM_rnglistx,
    DW_FORM_ref_sup8,
    DW_FORM_strx1,
    DW_FORM_strx2,
    DW_FORM_strx3,
    DW_FORM_strx4,
    DW_FORM_addrx1,
    DW_FORM_addrx2,
    DW_FORM_addrx3,
    DW_FORM_addrx4
};

#endif // DWARFENCODINGS_H