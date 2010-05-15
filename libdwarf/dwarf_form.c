/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
 * Copyright (c) 2009 Kai Wang
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

#include "_libdwarf.h"

int
dwarf_hasform(Dwarf_Attribute at, Dwarf_Half form, Dwarf_Bool *return_hasform,
    Dwarf_Error *error)
{
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_hasform == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	*return_hasform = (at->at_form == form);

	return (DW_DLV_OK);
}

int
dwarf_whatform(Dwarf_Attribute at, Dwarf_Half *return_form, Dwarf_Error *error)
{
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_form == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	*return_form = at->at_form;

	return (DW_DLV_OK);
}

int
dwarf_whatform_direct(Dwarf_Attribute at, Dwarf_Half *return_form,
    Dwarf_Error *error)
{
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_form == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (at->at_indirect)
		*return_form = DW_FORM_indirect;
	else
		*return_form = (Dwarf_Half) at->at_form;

	return (DW_DLV_OK);
}

int
dwarf_whatattr(Dwarf_Attribute at, Dwarf_Half *return_attr, Dwarf_Error *error)
{
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_attr == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	*return_attr = (Dwarf_Half) at->at_attrib;

	return (DW_DLV_OK);
}

int
dwarf_formref(Dwarf_Attribute at, Dwarf_Off *return_offset, Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_offset == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	switch (at->at_form) {
	case DW_FORM_ref1:
	case DW_FORM_ref2:
	case DW_FORM_ref4:
	case DW_FORM_ref8:
	case DW_FORM_ref_udata:
		*return_offset = (Dwarf_Off) at->u[0].u64;
		ret = DW_DLV_OK;
		break;
	default:
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}

int
dwarf_global_formref(Dwarf_Attribute at, Dwarf_Off *return_offset,
    Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_offset == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	switch (at->at_form) {
	case DW_FORM_ref_addr:
		*return_offset = (Dwarf_Off) at->u[0].u64;
		ret = DW_DLV_OK;
		break;
	case DW_FORM_ref1:
	case DW_FORM_ref2:
	case DW_FORM_ref4:
	case DW_FORM_ref8:
	case DW_FORM_ref_udata:
		*return_offset = (Dwarf_Off) at->u[0].u64 +
			at->at_die->die_cu->cu_offset;
		ret = DW_DLV_OK;
		break;
	default:
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}

int
dwarf_formaddr(Dwarf_Attribute at, Dwarf_Addr *return_addr, Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_addr == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (at->at_form == DW_FORM_addr) {
		*return_addr = at->u[0].u64;
		ret = DW_DLV_OK;
	} else {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}

int
dwarf_formflag(Dwarf_Attribute at, Dwarf_Bool *return_bool, Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_bool == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (at->at_form == DW_FORM_flag) {
		*return_bool = (Dwarf_Bool) at->u[0].u64;
		ret = DW_DLV_OK;
	} else {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}

int
dwarf_formudata(Dwarf_Attribute at, Dwarf_Unsigned *return_uvalue,
    Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_uvalue == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	switch (at->at_form) {
	case DW_FORM_data1:
	case DW_FORM_data2:
	case DW_FORM_data4:
	case DW_FORM_data8:
	case DW_FORM_udata:
		*return_uvalue = at->u[0].u64;
		ret = DW_DLV_OK;
		break;
	default:
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}

int
dwarf_formsdata(Dwarf_Attribute at, Dwarf_Signed *return_svalue,
    Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_svalue == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	switch (at->at_form) {
	case DW_FORM_data1:
		*return_svalue = (int8_t) at->u[0].s64;
		ret = DW_DLV_OK;
		break;
	case DW_FORM_data2:
		*return_svalue = (int16_t) at->u[0].s64;
		ret = DW_DLV_OK;
		break;
	case DW_FORM_data4:
		*return_svalue = (int32_t) at->u[0].s64;
		ret = DW_DLV_OK;
		break;
	case DW_FORM_data8:
	case DW_FORM_sdata:
		*return_svalue = at->u[0].s64;
		ret = DW_DLV_OK;
		break;
	default:
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}

int
dwarf_formblock(Dwarf_Attribute at, Dwarf_Block **return_block,
    Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_block == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	switch (at->at_form) {
	case DW_FORM_block:
	case DW_FORM_block1:
	case DW_FORM_block2:
	case DW_FORM_block4:
		*return_block = &at->at_block;
		ret = DW_DLV_OK;
		break;
	default:
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}

int
dwarf_formsig8(Dwarf_Attribute at, Dwarf_Sig8 *return_sig8, Dwarf_Error *error)
{
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_sig8 == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}
	
	if (at->at_form != DW_FORM_ref_sig8) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		return (DW_DLV_ERROR);
	}

	assert(at->u[0].u64 == 8);
	memcpy(return_sig8->signature, at->u[1].u8p, at->u[0].u64);

	return (DW_DLV_OK);
}

int
dwarf_formstring(Dwarf_Attribute at, char **return_string,
    Dwarf_Error *error)
{
	int ret;
	Dwarf_Debug dbg;

	dbg = at != NULL ? at->at_die->die_dbg : NULL;

	if (at == NULL || return_string == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	switch (at->at_form) {
	case DW_FORM_string:
		*return_string = (char *) at->u[0].s;
		ret = DW_DLV_OK;
		break;
	case DW_FORM_strp:
		*return_string = (char *) at->u[1].s;
		ret = DW_DLV_OK;
		break;
	default:
		DWARF_SET_ERROR(dbg, error, DW_DLE_ATTR_FORM_BAD);
		ret = DW_DLV_ERROR;
	}

	return (ret);
}
