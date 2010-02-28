/*-
 * Copyright (c) 2010 Kai Wang
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

#define	_SECTION_INIT_SIZE	128

int
_dwarf_section_init(Dwarf_P_Debug dbg, Dwarf_P_Section *dsp, const char *name,
    Dwarf_Error *error)
{
	Dwarf_P_Section ds;

	assert(dbg != NULL && dsp != NULL && name != NULL);

	if ((ds = malloc(sizeof(struct _Dwarf_P_Section))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	if ((ds->ds_name = strdup(name)) == NULL) {
		free(ds);
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	ds->ds_size = 0;
	ds->ds_cap = _SECTION_INIT_SIZE;
	ds->ds_ndx = 0;
	if ((ds->ds_data = malloc(ds->ds_cap)) == NULL) {
		free(ds->ds_name);
		free(ds);
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	STAILQ_INSERT_TAIL(&dbg->dbgp_seclist, ds, ds_next);
	dbg->dbgp_seccnt++;
	*dsp = ds;

	return (DWARF_E_NONE);
}

void
_dwarf_section_free(Dwarf_P_Debug dbg, Dwarf_P_Section *dsp)
{
	Dwarf_P_Section ds, tds;

	assert(dbg != NULL && dsp != NULL);

	if (*dsp == NULL)
		return;

	STAILQ_FOREACH_SAFE(ds, &dbg->dbgp_seclist, ds_next, tds) {
		if (ds != *dsp)
			continue;
		STAILQ_REMOVE(&dbg->dbgp_seclist, ds, _Dwarf_P_Section,
		    ds_next);
		if (ds->ds_name)
			free(ds->ds_name);
		if (ds->ds_data)
			free(ds->ds_data);
		free(ds);
		*dsp = NULL;
		break;
	}
}

int
_dwarf_pro_callback(Dwarf_P_Debug dbg, char *name, int size,
    Dwarf_Unsigned type, Dwarf_Unsigned flags, Dwarf_Unsigned link,
    Dwarf_Unsigned info, Dwarf_Unsigned *symndx, int *error)
{
	int e, ret, isymndx;

	assert(dbg != NULL && name != NULL && symndx != NULL);

	if (dbg->dbgp_func_b)
		ret = dbg->dbgp_func_b(name, size, type, flags, link, info,
		    symndx, &e);
	else {
		ret = dbg->dbgp_func(name, size, type, flags, link, info,
		    &isymndx, &e);
		*symndx = isymndx;
	}
	if (ret < 0) {
		if (error)
			*error = e;
	}

	return (ret);
}

int
_dwarf_generate_sections(Dwarf_P_Debug dbg, Dwarf_Error *error)
{
	int ret;

	/* Produce .debug_info section. */
	if ((ret = _dwarf_info_gen(dbg, error)) != DWARF_E_NONE)
		return (ret);

	/* Produce .debug_abbrev section. */
	if ((ret = _dwarf_abbrev_gen(dbg, error)) != DWARF_E_NONE)
		return (ret);

	/* Produce .debug_str section. */
	if ((ret = _dwarf_strtab_gen(dbg, error)) != DWARF_E_NONE)
		return (ret);

	/* Set section/relocation iterator to the first element. */
	dbg->dbgp_secpos = STAILQ_FIRST(&dbg->dbgp_seclist);
	dbg->dbgp_drspos = STAILQ_FIRST(&dbg->dbgp_drslist);

	return (DWARF_E_NONE);
}
