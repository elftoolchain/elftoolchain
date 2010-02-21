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
_dwarf_section_init(Dwarf_Debug dbg, Dwarf_Section **dsp, const char *name,
    Dwarf_Error *error)
{
	Dwarf_Section *ds;

	assert(dbg != NULL && dsp != NULL && name != NULL);

	if ((ds = malloc(sizeof(struct _Dwarf_Section))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	ds->ds_name = name;
	ds->ds_size = 0;
	ds->ds_cap = _SECTION_INIT_SIZE;
	ds->ds_ndx = 0;
	if ((ds->ds_data = malloc(ds->ds_cap)) == NULL) {
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
_dwarf_section_free(Dwarf_Debug dbg, Dwarf_Section **dsp)
{
	Dwarf_Section *ds, *tds;

	assert(dbg != NULL && dsp != NULL);

	if (*dsp == NULL)
		return;

	STAILQ_FOREACH_SAFE(ds, &dbg->dbgp_seclist, ds_next, tds) {
		if (ds == *dsp) {
			STAILQ_REMOVE(&dbg->dbgp_seclist, ds, _Dwarf_Section,
			    ds_next);
			if (ds->ds_data)
				free(ds->ds_data);
			free(ds);
			*dsp = NULL;
			return;
		}
	}
}

Dwarf_Unsigned
_dwarf_pro_callback(Dwarf_P_Debug dbg, const char *name, int size,
    Dwarf_Unsigned type, Dwarf_Unsigned flags, Dwarf_Unsigned link,
    Dwarf_Unsigned info, int *error)
{
	Dwarf_Unsigned ndx;
	char *name0;
	int e, ret, indx;

	if ((name0 = strdup(name)) == NULL)
		return (0);
	
	if (dbg->dbgp_func_b)
		ret = dbg->dbgp_func_b(name0, size, type, flags, link, info,
		    &ndx, &e);
	else {
		ret = dbg->dbgp_func(name0, size, type, flags, link, info,
		    &indx, &e);
		ndx = indx;
	}

	if (ret < 0) {
		if (error)
			*error = e;
		ndx = 0;
	}

	free(name0);

	return (ndx);
}

int
_dwarf_generate_sections(Dwarf_P_Debug dbg, Dwarf_Error *error)
{
	int ret;

	/* Generate .debug_info section content. */
	if ((ret = _dwarf_info_gen(dbg, error)) != DWARF_E_NONE)
		return (ret);

	return (DWARF_E_NONE);
}
