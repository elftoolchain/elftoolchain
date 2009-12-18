/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include "_libdwarf.h"

int
dwarf_get_AT_name(unsigned attr, const char **s)
{

	assert(s != NULL);

	switch (attr) {
	case DW_AT_abstract_origin:
		*s = "DW_AT_abstract_origin"; break;
	case DW_AT_accessibility:
		*s = "DW_AT_accessibility"; break;
	case DW_AT_address_class:
		*s = "DW_AT_address_class"; break;
	case DW_AT_artificial:
		*s = "DW_AT_artificial"; break;
	case DW_AT_allocated:
		*s = "DW_AT_allocated"; break;
	case DW_AT_associated:
		*s = "DW_AT_associated"; break;
	case DW_AT_base_types:
		*s = "DW_AT_base_types"; break;
	case DW_AT_binary_scale:
		*s = "DW_AT_binary_scale"; break;
	case DW_AT_bit_offset:
		*s = "DW_AT_bit_offset"; break;
	case DW_AT_bit_size:
		*s = "DW_AT_bit_size"; break;
	case DW_AT_bit_stride:
		*s = "DW_AT_bit_stride"; break;
	case DW_AT_byte_size:
		*s = "DW_AT_byte_size"; break;
	case DW_AT_byte_stride:
		*s = "DW_AT_byte_stride"; break;
	case DW_AT_calling_convention:
		*s = "DW_AT_calling_convention"; break;
	case DW_AT_common_reference:
		*s = "DW_AT_common_reference"; break;
	case DW_AT_comp_dir:
		*s = "DW_AT_comp_dir"; break;
	case DW_AT_const_value:
		*s = "DW_AT_const_value"; break;
	case DW_AT_containing_type:
		*s = "DW_AT_containing_type"; break;
	case DW_AT_count:
		*s = "DW_AT_count"; break;
	case DW_AT_call_column:
		*s = "DW_AT_call_column"; break;
	case DW_AT_call_file:
		*s = "DW_AT_call_file"; break;
	case DW_AT_call_line:
		*s = "DW_AT_call_line"; break;
	case DW_AT_data_location:
		*s = "DW_AT_data_location"; break;
	case DW_AT_data_member_location:
		*s = "DW_AT_data_member_location"; break;
	case DW_AT_decl_column:
		*s = "DW_AT_decl_column"; break;
	case DW_AT_decl_file:
		*s = "DW_AT_decl_file"; break;
	case DW_AT_decl_line:
		*s = "DW_AT_decl_line"; break;
	case DW_AT_declaration:
		*s = "DW_AT_declaration"; break;
	case DW_AT_default_value:
		*s = "DW_AT_default_value"; break;
	case DW_AT_decimal_scale:
		*s = "DW_AT_decimal_scale"; break;
	case DW_AT_decimal_sign:
		*s = "DW_AT_decimal_sign"; break;
	case DW_AT_description:
		*s = "DW_AT_description"; break;
	case DW_AT_digit_count:
		*s = "DW_AT_digit_count"; break;
	case DW_AT_discr:
		*s = "DW_AT_discr"; break;
	case DW_AT_discr_list:
		*s = "DW_AT_discr_list"; break;
	case DW_AT_discr_value:
		*s = "DW_AT_discr_value"; break;
	case DW_AT_element_list:
		*s = "DW_AT_element_list"; break;
	case DW_AT_encoding:
		*s = "DW_AT_encoding"; break;
	case DW_AT_external:
		*s = "DW_AT_external"; break;
	case DW_AT_entry_pc:
		*s = "DW_AT_entry_pc"; break;
	case DW_AT_extension:
		*s = "DW_AT_extension"; break;
	case DW_AT_explicit:
		*s = "DW_AT_explicit"; break;
	case DW_AT_endianity:
		*s = "DW_AT_endianity"; break;
	case DW_AT_elemental:
		*s = "DW_AT_elemental"; break;
	case DW_AT_frame_base:
		*s = "DW_AT_frame_base"; break;
	case DW_AT_friend:
		*s = "DW_AT_friend"; break;
	case DW_AT_high_pc:
		*s = "DW_AT_high_pc"; break;
	case DW_AT_hi_user:
		*s = "DW_AT_hi_user"; break;
	case DW_AT_identifier_case:
		*s = "DW_AT_identifier_case"; break;
	case DW_AT_import:
		*s = "DW_AT_import"; break;
	case DW_AT_inline:
		*s = "DW_AT_inline"; break;
	case DW_AT_is_optional:
		*s = "DW_AT_is_optional"; break;
	case DW_AT_language:
		*s = "DW_AT_language"; break;
	case DW_AT_lo_user:
		*s = "DW_AT_lo_user"; break;
	case DW_AT_location:
		*s = "DW_AT_location"; break;
	case DW_AT_low_pc:
		*s = "DW_AT_low_pc"; break;
	case DW_AT_lower_bound:
		*s = "DW_AT_lower_bound"; break;
	case DW_AT_macro_info:
		*s = "DW_AT_macro_info"; break;
	case DW_AT_mutable:
		*s = "DW_AT_mutable"; break;
	case DW_AT_member:
		*s = "DW_AT_member"; break;
	case DW_AT_name:
		*s = "DW_AT_name"; break;
	case DW_AT_namelist_item:
		*s = "DW_AT_namelist_item"; break;
	case DW_AT_ordering:
		*s = "DW_AT_ordering"; break;
	case DW_AT_object_pointer:
		*s = "DW_AT_object_pointer"; break;
	case DW_AT_priority:
		*s = "DW_AT_priority"; break;
	case DW_AT_producer:
		*s = "DW_AT_producer"; break;
	case DW_AT_prototyped:
		*s = "DW_AT_prototyped"; break;
	case DW_AT_picture_string:
		*s = "DW_AT_picture_string"; break;
	case DW_AT_pure:
		*s = "DW_AT_pure"; break;
	case DW_AT_return_addr:
		*s = "DW_AT_return_addr"; break;
	case DW_AT_ranges:
		*s = "DW_AT_ranges"; break;
	case DW_AT_recursive:
		*s = "DW_AT_recursive"; break;
	case DW_AT_segment:
		*s = "DW_AT_segment"; break;
	case DW_AT_sibling:
		*s = "DW_AT_sibling"; break;
	case DW_AT_specification:
		*s = "DW_AT_specification"; break;
	case DW_AT_start_scope:
		*s = "DW_AT_start_scope"; break;
	case DW_AT_static_link:
		*s = "DW_AT_static_link"; break;
	case DW_AT_stmt_list:
		*s = "DW_AT_stmt_list"; break;
	case DW_AT_string_length:
		*s = "DW_AT_string_length"; break;
	case DW_AT_subscr_data:
		*s = "DW_AT_subscr_data"; break;
	case DW_AT_small:
		*s = "DW_AT_small"; break;
	case DW_AT_type:
		*s = "DW_AT_type"; break;
	case DW_AT_trampoline:
		*s = "DW_AT_trampoline"; break;
	case DW_AT_threads_scaled:
		*s = "DW_AT_threads_scaled"; break;
	case DW_AT_upper_bound:
		*s = "DW_AT_upper_bound"; break;
	case DW_AT_use_location:
		*s = "DW_AT_use_location"; break;
	case DW_AT_use_UTF8:
		*s = "DW_AT_use_UTF8"; break;
	case DW_AT_variable_parameter:
		*s = "DW_AT_variable_parameter"; break;
	case DW_AT_virtuality:
		*s = "DW_AT_virtuality"; break;
	case DW_AT_visibility:
		*s = "DW_AT_visibility"; break;
	case DW_AT_vtable_elem_location:
		*s = "DW_AT_vtable_elem_location"; break;
	default:
		return (DW_DLV_NO_ENTRY);
	}

	return (DW_DLV_OK);
}

int
dwarf_get_FORM_name(unsigned form, const char **s)
{

	assert(s != NULL);

	switch (form) {
	case DW_FORM_addr:
		*s = "DW_FORM_addr"; break;
	case DW_FORM_block:
		*s = "DW_FORM_block"; break;
	case DW_FORM_block1:
		*s = "DW_FORM_block1"; break;
	case DW_FORM_block2:
		*s = "DW_FORM_block2"; break;
	case DW_FORM_block4:
		*s = "DW_FORM_block4"; break;
	case DW_FORM_data1:
		*s = "DW_FORM_data1"; break;
	case DW_FORM_data2:
		*s = "DW_FORM_data2"; break;
	case DW_FORM_data4:
		*s = "DW_FORM_data4"; break;
	case DW_FORM_data8:
		*s = "DW_FORM_data8"; break;
	case DW_FORM_flag:
		*s = "DW_FORM_flag"; break;
	case DW_FORM_indirect:
		*s = "DW_FORM_indirect"; break;
	case DW_FORM_ref1:
		*s = "DW_FORM_ref1"; break;
	case DW_FORM_ref2:
		*s = "DW_FORM_ref2"; break;
	case DW_FORM_ref4:
		*s = "DW_FORM_ref4"; break;
	case DW_FORM_ref8:
		*s = "DW_FORM_ref8"; break;
	case DW_FORM_ref_addr:
		*s = "DW_FORM_ref_addr"; break;
	case DW_FORM_ref_udata:
		*s = "DW_FORM_ref_udata"; break;
	case DW_FORM_sdata:
		*s = "DW_FORM_sdata"; break;
	case DW_FORM_string:
		*s = "DW_FORM_string"; break;
	case DW_FORM_strp:
		*s = "DW_FORM_strp"; break;
	case DW_FORM_udata:
		*s = "DW_FORM_udata"; break;
	default:
		return (DW_DLV_NO_ENTRY);
	}

	return (DW_DLV_OK);
}

int
dwarf_get_TAG_name(unsigned tag, const char **s)
{

	assert(s != NULL);

	switch (tag) {
	case DW_TAG_access_declaration:
		*s = "DW_TAG_access_declaration"; break;
	case DW_TAG_array_type:
		*s = "DW_TAG_array_type"; break;
	case DW_TAG_base_type:
		*s = "DW_TAG_base_type"; break;
	case DW_TAG_catch_block:
		*s = "DW_TAG_catch_block"; break;
	case DW_TAG_class_type:
		*s = "DW_TAG_class_type"; break;
	case DW_TAG_common_block:
		*s = "DW_TAG_common_block"; break;
	case DW_TAG_common_inclusion:
		*s = "DW_TAG_common_inclusion"; break;
	case DW_TAG_compile_unit:
		*s = "DW_TAG_compile_unit"; break;
	case DW_TAG_condition:
		*s = "DW_TAG_condition"; break;
	case DW_TAG_const_type:
		*s = "DW_TAG_const_type"; break;
	case DW_TAG_constant:
		*s = "DW_TAG_constant"; break;
	case DW_TAG_dwarf_procedure:
		*s = "DW_TAG_dwarf_procedure"; break;
	case DW_TAG_entry_point:
		*s = "DW_TAG_entry_point"; break;
	case DW_TAG_enumeration_type:
		*s = "DW_TAG_enumeration_type"; break;
	case DW_TAG_enumerator:
		*s = "DW_TAG_enumerator"; break;
	case DW_TAG_formal_parameter:
		*s = "DW_TAG_formal_parameter"; break;
	case DW_TAG_friend:
		*s = "DW_TAG_friend"; break;
	case DW_TAG_imported_declaration:
		*s = "DW_TAG_imported_declaration"; break;
	case DW_TAG_imported_module:
		*s = "DW_TAG_imported_module"; break;
	case DW_TAG_imported_unit:
		*s = "DW_TAG_imported_unit"; break;
	case DW_TAG_inheritance:
		*s = "DW_TAG_inheritance"; break;
	case DW_TAG_inlined_subroutine:
		*s = "DW_TAG_inlined_subroutine"; break;
	case DW_TAG_interface_type:
		*s = "DW_TAG_interface_type"; break;
	case DW_TAG_label:
		*s = "DW_TAG_label"; break;
	case DW_TAG_lexical_block:
		*s = "DW_TAG_lexical_block"; break;
	case DW_TAG_member:
		*s = "DW_TAG_member"; break;
	case DW_TAG_module:
		*s = "DW_TAG_module"; break;
	case DW_TAG_namelist:
		*s = "DW_TAG_namelist"; break;
	case DW_TAG_namelist_item:
		*s = "DW_TAG_namelist_item"; break;
	case DW_TAG_namespace:
		*s = "DW_TAG_namespace"; break;
	case DW_TAG_packed_type:
		*s = "DW_TAG_packed_type"; break;
	case DW_TAG_partial_unit:
		*s = "DW_TAG_partial_unit"; break;
	case DW_TAG_pointer_type:
		*s = "DW_TAG_pointer_type"; break;
	case DW_TAG_ptr_to_member_type:
		*s = "DW_TAG_ptr_to_member_type"; break;
	case DW_TAG_reference_type:
		*s = "DW_TAG_reference_type"; break;
	case DW_TAG_restrict_type:
		*s = "DW_TAG_restrict_type"; break;
	case DW_TAG_set_type:
		*s = "DW_TAG_set_type"; break;
	case DW_TAG_shared_type:
		*s = "DW_TAG_shared_type"; break;
	case DW_TAG_string_type:
		*s = "DW_TAG_string_type"; break;
	case DW_TAG_structure_type:
		*s = "DW_TAG_structure_type"; break;
	case DW_TAG_subprogram:
		*s = "DW_TAG_subprogram"; break;
	case DW_TAG_subrange_type:
		*s = "DW_TAG_subrange_type"; break;
	case DW_TAG_subroutine_type:
		*s = "DW_TAG_subroutine_type"; break;
	case DW_TAG_template_type_parameter:
		*s = "DW_TAG_template_type_parameter"; break;
	case DW_TAG_template_value_parameter:
		*s = "DW_TAG_template_value_parameter"; break;
	case DW_TAG_thrown_type:
		*s = "DW_TAG_thrown_type"; break;
	case DW_TAG_try_block:
		*s = "DW_TAG_try_block"; break;
	case DW_TAG_typedef:
		*s = "DW_TAG_typedef"; break;
	case DW_TAG_union_type:
		*s = "DW_TAG_union_type"; break;
	case DW_TAG_unspecified_parameters:
		*s = "DW_TAG_unspecified_parameters"; break;
	case DW_TAG_unspecified_type:
		*s = "DW_TAG_unspecified_type"; break;
	case DW_TAG_variable:
		*s = "DW_TAG_variable"; break;
	case DW_TAG_variant:
		*s = "DW_TAG_variant"; break;
	case DW_TAG_variant_part:
		*s = "DW_TAG_variant_part"; break;
	case DW_TAG_volatile_type:
		*s = "DW_TAG_volatile_type"; break;
	case DW_TAG_with_stmt:
		*s = "DW_TAG_with_stmt"; break;
	default:
		return (DW_DLV_NO_ENTRY);
	}

	return (DW_DLV_OK);
}

int
dwarf_get_OP_name(unsigned op, const char **s)
{

	assert(s != NULL);

	switch (op) {
	case DW_OP_deref:
		*s = "DW_OP_deref"; break;
	case DW_OP_reg0:
		*s = "DW_OP_reg0"; break;
	case DW_OP_reg1:
		*s = "DW_OP_reg1"; break;
	case DW_OP_reg2:
		*s = "DW_OP_reg2"; break;
	case DW_OP_reg3:
		*s = "DW_OP_reg3"; break;
	case DW_OP_reg4:
		*s = "DW_OP_reg4"; break;
	case DW_OP_reg5:
		*s = "DW_OP_reg5"; break;
	case DW_OP_reg6:
		*s = "DW_OP_reg6"; break;
	case DW_OP_reg7:
		*s = "DW_OP_reg7"; break;
	case DW_OP_reg8:
		*s = "DW_OP_reg8"; break;
	case DW_OP_reg9:
		*s = "DW_OP_reg9"; break;
	case DW_OP_reg10:
		*s = "DW_OP_reg10"; break;
	case DW_OP_reg11:
		*s = "DW_OP_reg11"; break;
	case DW_OP_reg12:
		*s = "DW_OP_reg12"; break;
	case DW_OP_reg13:
		*s = "DW_OP_reg13"; break;
	case DW_OP_reg14:
		*s = "DW_OP_reg14"; break;
	case DW_OP_reg15:
		*s = "DW_OP_reg15"; break;
	case DW_OP_reg16:
		*s = "DW_OP_reg16"; break;
	case DW_OP_reg17:
		*s = "DW_OP_reg17"; break;
	case DW_OP_reg18:
		*s = "DW_OP_reg18"; break;
	case DW_OP_reg19:
		*s = "DW_OP_reg19"; break;
	case DW_OP_reg20:
		*s = "DW_OP_reg20"; break;
	case DW_OP_reg21:
		*s = "DW_OP_reg21"; break;
	case DW_OP_reg22:
		*s = "DW_OP_reg22"; break;
	case DW_OP_reg23:
		*s = "DW_OP_reg23"; break;
	case DW_OP_reg24:
		*s = "DW_OP_reg24"; break;
	case DW_OP_reg25:
		*s = "DW_OP_reg25"; break;
	case DW_OP_reg26:
		*s = "DW_OP_reg26"; break;
	case DW_OP_reg27:
		*s = "DW_OP_reg27"; break;
	case DW_OP_reg28:
		*s = "DW_OP_reg28"; break;
	case DW_OP_reg29:
		*s = "DW_OP_reg29"; break;
	case DW_OP_reg30:
		*s = "DW_OP_reg30"; break;
	case DW_OP_reg31:
		*s = "DW_OP_reg31"; break;
	case DW_OP_lit0:
		*s = "DW_OP_lit0"; break;
	case DW_OP_lit1:
		*s = "DW_OP_lit1"; break;
	case DW_OP_lit2:
		*s = "DW_OP_lit2"; break;
	case DW_OP_lit3:
		*s = "DW_OP_lit3"; break;
	case DW_OP_lit4:
		*s = "DW_OP_lit4"; break;
	case DW_OP_lit5:
		*s = "DW_OP_lit5"; break;
	case DW_OP_lit6:
		*s = "DW_OP_lit6"; break;
	case DW_OP_lit7:
		*s = "DW_OP_lit7"; break;
	case DW_OP_lit8:
		*s = "DW_OP_lit8"; break;
	case DW_OP_lit9:
		*s = "DW_OP_lit9"; break;
	case DW_OP_lit10:
		*s = "DW_OP_lit10"; break;
	case DW_OP_lit11:
		*s = "DW_OP_lit11"; break;
	case DW_OP_lit12:
		*s = "DW_OP_lit12"; break;
	case DW_OP_lit13:
		*s = "DW_OP_lit13"; break;
	case DW_OP_lit14:
		*s = "DW_OP_lit14"; break;
	case DW_OP_lit15:
		*s = "DW_OP_lit15"; break;
	case DW_OP_lit16:
		*s = "DW_OP_lit16"; break;
	case DW_OP_lit17:
		*s = "DW_OP_lit17"; break;
	case DW_OP_lit18:
		*s = "DW_OP_lit18"; break;
	case DW_OP_lit19:
		*s = "DW_OP_lit19"; break;
	case DW_OP_lit20:
		*s = "DW_OP_lit20"; break;
	case DW_OP_lit21:
		*s = "DW_OP_lit21"; break;
	case DW_OP_lit22:
		*s = "DW_OP_lit22"; break;
	case DW_OP_lit23:
		*s = "DW_OP_lit23"; break;
	case DW_OP_lit24:
		*s = "DW_OP_lit24"; break;
	case DW_OP_lit25:
		*s = "DW_OP_lit25"; break;
	case DW_OP_lit26:
		*s = "DW_OP_lit26"; break;
	case DW_OP_lit27:
		*s = "DW_OP_lit27"; break;
	case DW_OP_lit28:
		*s = "DW_OP_lit28"; break;
	case DW_OP_lit29:
		*s = "DW_OP_lit29"; break;
	case DW_OP_lit30:
		*s = "DW_OP_lit30"; break;
	case DW_OP_lit31:
		*s = "DW_OP_lit31"; break;
	case DW_OP_dup:
		*s = "DW_OP_dup"; break;
	case DW_OP_drop:
		*s = "DW_OP_drop"; break;
	case DW_OP_over:
		*s = "DW_OP_over"; break;
	case DW_OP_swap:
		*s = "DW_OP_swap"; break;
	case DW_OP_rot:
		*s = "DW_OP_rot"; break;
	case DW_OP_xderef:
		*s = "DW_OP_xderef"; break;
	case DW_OP_abs:
		*s = "DW_OP_abs"; break;
	case DW_OP_and:
		*s = "DW_OP_and"; break;
	case DW_OP_div:
		*s = "DW_OP_div"; break;
	case DW_OP_minus:
		*s = "DW_OP_minus"; break;
	case DW_OP_mod:
		*s = "DW_OP_mod"; break;
	case DW_OP_mul:
		*s = "DW_OP_mul"; break;
	case DW_OP_neg:
		*s = "DW_OP_neg"; break;
	case DW_OP_not:
		*s = "DW_OP_not"; break;
	case DW_OP_or:
		*s = "DW_OP_or"; break;
	case DW_OP_plus:
		*s = "DW_OP_plus"; break;
	case DW_OP_shl:
		*s = "DW_OP_shl"; break;
	case DW_OP_shr:
		*s = "DW_OP_shr"; break;
	case DW_OP_shra:
		*s = "DW_OP_shra"; break;
	case DW_OP_xor:
		*s = "DW_OP_xor"; break;
	case DW_OP_eq:
		*s = "DW_OP_eq"; break;
	case DW_OP_ge:
		*s = "DW_OP_ge"; break;
	case DW_OP_gt:
		*s = "DW_OP_gt"; break;
	case DW_OP_le:
		*s = "DW_OP_le"; break;
	case DW_OP_lt:
		*s = "DW_OP_lt"; break;
	case DW_OP_ne:
		*s = "DW_OP_ne"; break;
	case DW_OP_nop:
		*s = "DW_OP_nop"; break;
	case DW_OP_const1u:
		*s = "DW_OP_const1u"; break;
	case DW_OP_const1s:
		*s = "DW_OP_const1s"; break;
	case DW_OP_pick:
		*s = "DW_OP_pick"; break;
	case DW_OP_deref_size:
		*s = "DW_OP_deref_size"; break;
	case DW_OP_xderef_size:
		*s = "DW_OP_xderef_size"; break;
	case DW_OP_const2u:
		*s = "DW_OP_const2u"; break;
	case DW_OP_const2s:
		*s = "DW_OP_const2s"; break;
	case DW_OP_bra:
		*s = "DW_OP_bra"; break;
	case DW_OP_skip:
		*s = "DW_OP_skip"; break;
	case DW_OP_const4u:
		*s = "DW_OP_const4u"; break;
	case DW_OP_const4s:
		*s = "DW_OP_const4s"; break;
	case DW_OP_const8u:
		*s = "DW_OP_const8u"; break;
	case DW_OP_const8s:
		*s = "DW_OP_const8s"; break;
	case DW_OP_constu:
		*s = "DW_OP_constu"; break;
	case DW_OP_plus_uconst:
		*s = "DW_OP_plus_uconst"; break;
	case DW_OP_regx:
		*s = "DW_OP_regx"; break;
	case DW_OP_piece:
		*s = "DW_OP_piece"; break;
	case DW_OP_consts:
		*s = "DW_OP_consts"; break;
	case DW_OP_breg0:
		*s = "DW_OP_breg0"; break;
	case DW_OP_breg1:
		*s = "DW_OP_breg1"; break;
	case DW_OP_breg2:
		*s = "DW_OP_breg2"; break;
	case DW_OP_breg3:
		*s = "DW_OP_breg3"; break;
	case DW_OP_breg4:
		*s = "DW_OP_breg4"; break;
	case DW_OP_breg5:
		*s = "DW_OP_breg5"; break;
	case DW_OP_breg6:
		*s = "DW_OP_breg6"; break;
	case DW_OP_breg7:
		*s = "DW_OP_breg7"; break;
	case DW_OP_breg8:
		*s = "DW_OP_breg8"; break;
	case DW_OP_breg9:
		*s = "DW_OP_breg9"; break;
	case DW_OP_breg10:
		*s = "DW_OP_breg10"; break;
	case DW_OP_breg11:
		*s = "DW_OP_breg11"; break;
	case DW_OP_breg12:
		*s = "DW_OP_breg12"; break;
	case DW_OP_breg13:
		*s = "DW_OP_breg13"; break;
	case DW_OP_breg14:
		*s = "DW_OP_breg14"; break;
	case DW_OP_breg15:
		*s = "DW_OP_breg15"; break;
	case DW_OP_breg16:
		*s = "DW_OP_breg16"; break;
	case DW_OP_breg17:
		*s = "DW_OP_breg17"; break;
	case DW_OP_breg18:
		*s = "DW_OP_breg18"; break;
	case DW_OP_breg19:
		*s = "DW_OP_breg19"; break;
	case DW_OP_breg20:
		*s = "DW_OP_breg20"; break;
	case DW_OP_breg21:
		*s = "DW_OP_breg21"; break;
	case DW_OP_breg22:
		*s = "DW_OP_breg22"; break;
	case DW_OP_breg23:
		*s = "DW_OP_breg23"; break;
	case DW_OP_breg24:
		*s = "DW_OP_breg24"; break;
	case DW_OP_breg25:
		*s = "DW_OP_breg25"; break;
	case DW_OP_breg26:
		*s = "DW_OP_breg26"; break;
	case DW_OP_breg27:
		*s = "DW_OP_breg27"; break;
	case DW_OP_breg28:
		*s = "DW_OP_breg28"; break;
	case DW_OP_breg29:
		*s = "DW_OP_breg29"; break;
	case DW_OP_breg30:
		*s = "DW_OP_breg30"; break;
	case DW_OP_breg31:
		*s = "DW_OP_breg31"; break;
	case DW_OP_fbreg:
		*s = "DW_OP_fbreg"; break;
	case DW_OP_bregx:
		*s = "DW_OP_bregx"; break;
	case DW_OP_addr:
		*s = "DW_OP_addr"; break;
	default:
		return (DW_DLV_NO_ENTRY);
	}

	return (DW_DLV_OK);
}
