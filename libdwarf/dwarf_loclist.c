/*-
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

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include "_libdwarf.h"

int
dwarf_loclist_n(Dwarf_Attribute at, Dwarf_Locdesc ***llbuf,
    Dwarf_Signed *listlen, Dwarf_Error *error)
{
	Dwarf_Loclist ll;
	int ret;

	if (at == NULL || llbuf == NULL || listlen == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

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
			ret = loclist_find(at->at_cu->cu_dbg, at->u[0].u64,
			    &ll);
			if (ret == DWARF_E_NO_ENTRY) {
				/* Malformed Attr? */
				DWARF_SET_ERROR(error, ret);
				return (DW_DLV_NO_ENTRY);
			}
			if (ret != DWARF_E_NONE)
				assert(0); /* Internal error! */
			*llbuf = ll->ll_ldlist;
			*listlen = ll->ll_ldlen;
			return (DW_DLV_OK);
		case DW_FORM_block:
		case DW_FORM_block1:
		case DW_FORM_block2:
		case DW_FORM_block4:
			if (at->at_ld != NULL) {
				*llbuf = &at->at_ld;
				*listlen = 1;
				return (DW_DLV_OK);
			} else {
				/* Malformed Attr? */
				DWARF_SET_ERROR(error, DWARF_E_INVALID_ATTR);
				return (DW_DLV_NO_ENTRY);
			}
		default:
			/* Malformed Attr? */
			DWARF_SET_ERROR(error, DWARF_E_INVALID_ATTR);
			return (DW_DLV_NO_ENTRY);
		}
	default:
		/* Wrong attr supplied. */
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}
}

int
dwarf_loclist(Dwarf_Attribute at, Dwarf_Locdesc **llbuf,
    Dwarf_Signed *listlen, Dwarf_Error *error)
{
	Dwarf_Loclist ll;
	int ret;

	if (at == NULL || llbuf == NULL || listlen == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

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
			ret = loclist_find(at->at_cu->cu_dbg, at->u[0].u64,
			    &ll);
			if (ret == DWARF_E_NO_ENTRY) {
				DWARF_SET_ERROR(error, DWARF_E_INVALID_ATTR);
				return (DW_DLV_NO_ENTRY);
			}
			if (ret != DWARF_E_NONE)
				assert(0); /* Internal error! */
			*llbuf = ll->ll_ldlist[0];
			*listlen = 1;
			return (DW_DLV_OK);
		case DW_FORM_block:
		case DW_FORM_block1:
		case DW_FORM_block2:
		case DW_FORM_block4:
			if (at->at_ld != NULL) {
				*llbuf = at->at_ld;
				*listlen = 1;
				return (DW_DLV_OK);
			} else {
				DWARF_SET_ERROR(error, DWARF_E_INVALID_ATTR);
				return (DW_DLV_ERROR);
			}
		default:
			DWARF_SET_ERROR(error, DWARF_E_INVALID_ATTR);
			return (DW_DLV_ERROR);
		}
	default:
		/* Wrong attr supplied. */
		DWARF_SET_ERROR(error, DWARF_E_INVALID_ATTR);
		return (DW_DLV_ERROR);
	}
}

int
dwarf_get_loclist_entry(Dwarf_Debug dbg, Dwarf_Unsigned offset,
    Dwarf_Addr *hipc, Dwarf_Addr *lopc, Dwarf_Ptr *data,
    Dwarf_Unsigned *entry_len, Dwarf_Unsigned *next_entry,
    Dwarf_Error *error)
{
	Dwarf_Loclist ll, next_ll;
	Dwarf_Locdesc *ld;
	int i, ret;

	if (dbg == NULL || hipc == NULL || lopc == NULL || data == NULL ||
	    entry_len == NULL || next_entry == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	ret = loclist_find(dbg, offset, &ll);
	if (ret == DWARF_E_NO_ENTRY) {
		DWARF_SET_ERROR(error, DWARF_E_INVALID_ATTR);
		return (DW_DLV_NO_ENTRY);
	}
	
	*hipc = *lopc = 0;
	for (i = 0; i < ll->ll_ldlen; i++) {
		ld = ll->ll_ldlist[i];
		if (i == 0) {
			*hipc = ld->ld_hipc;
			*lopc = ld->ld_lopc;
		} else {
			if (ld->ld_lopc < *lopc)
				*lopc = ld->ld_lopc;
			if (ld->ld_hipc > *hipc)
				*hipc = ld->ld_hipc;
		}
	}

	assert(dbg->dbg_s[DWARF_debug_loc].s_data != NULL);
	*data = (uint8_t *)dbg->dbg_s[DWARF_debug_loc].s_data->d_buf +
	    ll->ll_offset;
	*entry_len = ll->ll_length;

	next_ll = TAILQ_NEXT(ll, ll_next);
	if (next_ll != NULL)
		*next_entry = next_ll->ll_offset;
	else
		*next_entry = dbg->dbg_s[DWARF_debug_loc].s_data->d_size;

	return (DW_DLV_OK);
}

int
dwarf_loclist_from_expr(Dwarf_Debug dbg, Dwarf_Ptr bytes_in,
    Dwarf_Unsigned bytes_len, Dwarf_Locdesc **llbuf, Dwarf_Signed *listlen,
    Dwarf_Error *error)
{
	Dwarf_Locdesc *ld;
	int ret;

	if (dbg == NULL || bytes_in == NULL || bytes_len == 0 ||
	    llbuf == NULL || listlen == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	ret = loc_fill_locexpr(dbg, &ld, bytes_in, bytes_len,
	    dbg->dbg_pointer_size, error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	*llbuf = ld;
	*listlen = 1;

	return (DW_DLV_OK);
}

int
dwarf_loclist_from_expr_a(Dwarf_Debug dbg, Dwarf_Ptr bytes_in,
    Dwarf_Unsigned bytes_len, Dwarf_Half addr_size, Dwarf_Locdesc **llbuf,
    Dwarf_Signed *listlen, Dwarf_Error *error)
{
	Dwarf_Locdesc *ld;
	int ret;

	if (dbg == NULL || bytes_in == NULL || bytes_len == 0 ||
	    llbuf == NULL || listlen == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (addr_size != 4 && addr_size != 8) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	ret = loc_fill_locexpr(dbg, &ld, bytes_in, bytes_len, addr_size, error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	*llbuf = ld;
	*listlen = 1;

	return (DW_DLV_OK);
}

int
dwarf_loclist_from_expr_dealloc(Dwarf_Locdesc *llbuf, Dwarf_Error *error)
{

	if (llbuf == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (llbuf->ld_s != NULL)
		free(llbuf->ld_s);

	free(llbuf);

	return (DW_DLV_OK);
}

/*
 * DEPRECATED old libdwarf APIs.
 */

int
dwarf_locdesc(Dwarf_Die die, uint64_t attr, Dwarf_Locdesc **llbuf,
    Dwarf_Signed *lenp, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	Dwarf_CU cu;
	Dwarf_Attribute at;
	Dwarf_Locdesc *lbuf;
	int ret;

	if (die == NULL || llbuf == NULL || lenp == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if ((at = attr_find(die, attr)) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return (DW_DLV_NO_ENTRY);
	}

	cu = die->die_cu;
	assert(cu != NULL);

	dbg = cu->cu_dbg;
	assert(dbg != NULL);

	*lenp = 0;
	switch (at->at_form) {
	case DW_FORM_block:
	case DW_FORM_block1:
	case DW_FORM_block2:
	case DW_FORM_block4:
		ret = loc_fill_locexpr(dbg, &lbuf, at->u[1].u8p, at->u[0].u64,
		    die->die_cu->cu_pointer_size, error);
		*lenp = 1;
		break;
	default:
		printf("%s(%d): form %s not handled\n",__func__,
		    __LINE__,get_form_desc(at->at_form));
		DWARF_SET_ERROR(error, DWARF_E_NOT_IMPLEMENTED);
		return (DW_DLV_ERROR);
	}

	if (ret == DWARF_E_NONE)
		*llbuf = lbuf;

	return (DW_DLV_OK);
}

int
dwarf_locdesc_free(Dwarf_Locdesc *lbuf, Dwarf_Error *error)
{

	if (lbuf == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (lbuf->ld_s != NULL)
		free(lbuf->ld_s);

	free(lbuf);

	return (DW_DLV_OK);
}
