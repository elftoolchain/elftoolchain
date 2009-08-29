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

#include <stdio.h>
#include <stdlib.h>
#include "_libdwarf.h"

static int
_dwarf_macinfo_parse(Dwarf_Debug dbg, Dwarf_Section *ds, uint64_t *off,
    Dwarf_Macro_Details *dmd, Dwarf_Unsigned *cnt, Dwarf_Error *error)
{
	Dwarf_Unsigned lineno, fileindex;
	char *p;
	int i, type;

	i = 0;
	while (*off < ds->ds_size) {

		if (dmd != NULL)
			dmd[i].dmd_offset = *off;

		type = dbg->read(ds->ds_data, off, 1);

		if (dmd != NULL) {
			dmd[i].dmd_type = type;
			dmd[i].dmd_fileindex = -1;
		}

		switch (type) {
		case 0:
			break;
		case DW_MACINFO_define:
		case DW_MACINFO_undef:
		case DW_MACINFO_vendor_ext:
			lineno = _dwarf_read_uleb128(ds->ds_data, off);
			p = (char *) ds->ds_data;
			if (dmd != NULL) {
				dmd[i].dmd_lineno = lineno;
				dmd[i].dmd_macro = p + *off;

			}
			while (p[(*off)++] != '\0')
				;
			break;
		case DW_MACINFO_start_file:
			lineno = _dwarf_read_uleb128(ds->ds_data, off);
			fileindex = _dwarf_read_uleb128(ds->ds_data, off);
			if (dmd != NULL) {
				dmd[i].dmd_lineno = lineno;
				dmd[i].dmd_fileindex = fileindex;
			}
			break;
		case DW_MACINFO_end_file:
			break;
		default:
			DWARF_SET_ERROR(error, DWARF_E_INVALID_MACINFO);
			return (DWARF_E_INVALID_MACINFO);
		}

		i++;

		if (type == 0)
			break;
	}

	if (cnt != NULL)
		*cnt = i;

	return (DWARF_E_NONE);
}

void
_dwarf_macinfo_cleanup(Dwarf_Debug dbg)
{
	Dwarf_MacroSet ms, tms;

	if (STAILQ_EMPTY(&dbg->dbg_mslist))
		return;

	STAILQ_FOREACH_SAFE(ms, &dbg->dbg_mslist, ms_next, tms) {
		STAILQ_REMOVE(&dbg->dbg_mslist, ms, _Dwarf_MacroSet, ms_next);
		if (ms->ms_mdlist)
			free(ms->ms_mdlist);
		free(ms);
	}
}

int
_dwarf_macinfo_init(Dwarf_Debug dbg, Dwarf_Section *ds, Dwarf_Error *error)
{
	Dwarf_MacroSet ms;
	Dwarf_Unsigned cnt;
	uint64_t offset, entry_off;
	int ret;

	offset = 0;
	while (offset < ds->ds_size) {

		entry_off = offset;

		ret = _dwarf_macinfo_parse(dbg, ds, &offset, NULL, &cnt, error);
		if (ret != DWARF_E_NONE)
			return (ret);

		if (cnt == 0)
			break;

		if ((ms = calloc(1, sizeof(struct _Dwarf_MacroSet))) == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			ret = DWARF_E_MEMORY;
			goto fail_cleanup;
		}
		STAILQ_INSERT_TAIL(&dbg->dbg_mslist, ms, ms_next);

		if ((ms->ms_mdlist = calloc(cnt, sizeof(Dwarf_Macro_Details)))
		    == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			ret = DWARF_E_MEMORY;
			goto fail_cleanup;
		}

		ms->ms_cnt = cnt;

		offset = entry_off;

		ret = _dwarf_macinfo_parse(dbg, ds, &offset, ms->ms_mdlist,
		    NULL, error);

		if (ret != DWARF_E_NONE) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			ret = DWARF_E_MEMORY;
			goto fail_cleanup;
		}
	}

	return (DWARF_E_NONE);

fail_cleanup:

	_dwarf_macinfo_cleanup(dbg);

	return (ret);
}
