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
#include <stdlib.h>
#include <string.h>
#include "_libdwarf.h"

static int
_dwarf_attr_add(Dwarf_Die die, Dwarf_Attribute atref, Dwarf_Attribute *atp,
    Dwarf_Error *error)
{
	Dwarf_Attribute at;
	int ret;

	if ((at = malloc(sizeof(struct _Dwarf_Attribute))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	memcpy(at, atref, sizeof(struct _Dwarf_Attribute));

	/* Add the attribute value to the list in the die. */
	STAILQ_INSERT_TAIL(&die->die_attr, at, at_next);

	/* Save a pointer to the attribute name if this is one. */
	if (at->at_attrib == DW_AT_name)
		switch (at->at_form) {
		case DW_FORM_strp:
			die->die_name = at->u[1].s;
			break;
		case DW_FORM_string:
			die->die_name = at->u[0].s;
			break;
		default:
			break;
		}

	/*
	 * If current die is DW_TAG_compile_unit and current attr
	 * is DW_AT_stmt_list, then this CU has line number infomation
	 * needs to be initialized.
	 */
	if (die->die_ab->ab_tag == DW_TAG_compile_unit &&
	    at->at_attrib == DW_AT_stmt_list) {
		ret = _dwarf_lineno_init(die, at->u[0].u64, error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}

	/*
	 * If the attribute points to a loclist or the attribute
	 * contains a locdesc, find and save it.
	 */
	switch (at->at_attrib) {
	case DW_AT_location:
	case DW_AT_string_length:
	case DW_AT_return_addr:
	case DW_AT_data_member_location:
	case DW_AT_frame_base:
	case DW_AT_segment:
	case DW_AT_static_link:
	case DW_AT_use_location:
	case DW_AT_vtable_elem_location:
		switch (at->at_form) {
		case DW_FORM_data4:
		case DW_FORM_data8:
			ret = _dwarf_loclist_add(die->die_cu->cu_dbg,
			    die->die_cu, at->u[0].u64, error);
			if (ret != DWARF_E_NONE)
				return (ret);
			break;
		case DW_FORM_block:
		case DW_FORM_block1:
		case DW_FORM_block2:
		case DW_FORM_block4:
			ret = _dwarf_loc_add(die, at, error);
			if (ret != DWARF_E_NONE)
				return (ret);
			break;
		default:
			break;
		}
	default:
		break;
	}

	/* If the attribute points to a range list, find and save it. */
	if (at->at_attrib == DW_AT_ranges) {
		ret = _dwarf_ranges_add(die->die_cu->cu_dbg, die->die_cu,
		    at->u[0].u64, error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}

	if (atp != NULL)
		*atp = at;

	return (DWARF_E_NONE);
}

Dwarf_Attribute
_dwarf_attr_find(Dwarf_Die die, Dwarf_Half attr)
{
	Dwarf_Attribute at;

	STAILQ_FOREACH(at, &die->die_attr, at_next) {
		if (at->at_attrib == attr)
			break;
	}

	return (at);
}

int
_dwarf_attr_init(Dwarf_Debug dbg, Dwarf_Section *ds, uint64_t *offsetp,
    int dwarf_size, Dwarf_CU cu, Dwarf_Die die, Dwarf_AttrDef ad,
    uint64_t form, int indirect, Dwarf_Error *error)
{
	struct _Dwarf_Attribute atref;
	Dwarf_Section *str;
	int ret;

	ret = DWARF_E_NONE;
	memset(&atref, 0, sizeof(atref));
	atref.at_cu = cu;
	atref.at_ad = ad;
	atref.at_indirect = indirect;
	atref.at_ld = NULL;

	switch (form) {
	case DW_FORM_addr:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp,
		    cu->cu_pointer_size);
		break;
	case DW_FORM_block:
		atref.u[0].u64 = _dwarf_read_uleb128(ds->ds_data, offsetp);
		atref.u[1].u8p = _dwarf_read_block(ds->ds_data, offsetp,
		    atref.u[0].u64);
		break;
	case DW_FORM_block1:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, 1);
		atref.u[1].u8p = _dwarf_read_block(ds->ds_data, offsetp,
		    atref.u[0].u64);
		break;
	case DW_FORM_block2:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, 2);
		atref.u[1].u8p = _dwarf_read_block(ds->ds_data, offsetp,
		    atref.u[0].u64);
		break;
	case DW_FORM_block4:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, 4);
		atref.u[1].u8p = _dwarf_read_block(ds->ds_data, offsetp,
		    atref.u[0].u64);
		break;
	case DW_FORM_data1:
	case DW_FORM_flag:
	case DW_FORM_ref1:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, 1);
		break;
	case DW_FORM_data2:
	case DW_FORM_ref2:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, 2);
		break;
	case DW_FORM_data4:
	case DW_FORM_ref4:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, 4);
		break;
	case DW_FORM_data8:
	case DW_FORM_ref8:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, 8);
		break;
	case DW_FORM_indirect:
		form = _dwarf_read_uleb128(ds->ds_data, offsetp);
		return (_dwarf_attr_init(dbg, ds, offsetp, dwarf_size, cu, die,
		    ad, form, 1, error));
	case DW_FORM_ref_addr:
		if (cu->cu_version == 2)
			atref.u[0].u64 = dbg->read(ds->ds_data, offsetp,
			    cu->cu_pointer_size);
		else if (cu->cu_version == 3)
			atref.u[0].u64 = dbg->read(ds->ds_data, offsetp,
			    dwarf_size);
		break;
	case DW_FORM_ref_udata:
	case DW_FORM_udata:
		atref.u[0].u64 = _dwarf_read_uleb128(ds->ds_data, offsetp);
		break;
	case DW_FORM_sdata:
		atref.u[0].s64 = _dwarf_read_sleb128(ds->ds_data, offsetp);
		break;
	case DW_FORM_string:
		atref.u[0].s = _dwarf_read_string(ds->ds_data, ds->ds_size,
		    offsetp);
		break;
	case DW_FORM_strp:
		atref.u[0].u64 = dbg->read(ds->ds_data, offsetp, dwarf_size);
		str = _dwarf_find_section(dbg, ".debug_str");
		assert(str != NULL);
		atref.u[1].s = (char *) ds->ds_data + atref.u[0].u64;
		break;
	default:
		DWARF_SET_ERROR(error, DWARF_E_NOT_IMPLEMENTED);
		ret = DWARF_E_NOT_IMPLEMENTED;
		break;
	}

	if (ret == DWARF_E_NONE)
		ret = _dwarf_attr_add(die, &atref, NULL, error);

	return (ret);
}
