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
#include "_libdwarf.h"

int
dwarf_get_ranges(Dwarf_Debug dbg, Dwarf_Off offset, Dwarf_Ranges **ranges,
    Dwarf_Signed *ret_cnt, Dwarf_Unsigned *ret_byte_cnt, Dwarf_Error *error)
{
	Dwarf_CU cu;
	Dwarf_Rangelist rl;

	if (dbg == NULL || ranges == NULL || ret_cnt == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	STAILQ_FOREACH(rl, &dbg->dbg_rllist, rl_next)
		if (rl->rl_offset == (Dwarf_Unsigned)offset)
			break;

	if (rl == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return (DW_DLV_NO_ENTRY);
	}

	*ranges = rl->rl_rgarray;
	*ret_cnt = rl->rl_rglen;

	if (ret_byte_cnt != NULL) {
		cu = rl->rl_cu;
		assert(cu != NULL);
		*ret_byte_cnt = cu->cu_pointer_size * rl->rl_rglen * 2;
	}

	return (DW_DLV_OK);
}

int
dwarf_get_ranges_a(Dwarf_Debug dbg, Dwarf_Off offset, Dwarf_Die die __unused,
    Dwarf_Ranges **ranges, Dwarf_Signed *ret_cnt, Dwarf_Unsigned *ret_byte_cnt,
    Dwarf_Error *error)
{

	return (dwarf_get_ranges(dbg, offset, ranges, ret_cnt, ret_byte_cnt,
	    error));
}

int
dwarf_ranges_dealloc(Dwarf_Debug dbg __unused, Dwarf_Ranges *ranges __unused,
    Dwarf_Signed range_count __unused)
{

	return (DW_DLV_OK);
}
