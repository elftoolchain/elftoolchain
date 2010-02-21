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
_dwarf_section_init(Dwarf_Section **ds, const char *name, Dwarf_Error *error)
{

	assert(ds != NULL);

	if ((*ds = malloc(sizeof(*ds))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	(*ds)->ds_name = name;
	(*ds)->ds_size = 0;
	(*ds)->ds_cap = _SECTION_INIT_SIZE;
	if (((*ds)->ds_data = malloc((*ds)->ds_cap)) == NULL) {
		free(*ds);
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	return (DWARF_E_NONE);
}

void
_dwarf_section_free(Dwarf_Section **ds)
{

	assert(ds != NULL);
	if (*ds == NULL)
		return;
	if ((*ds)->ds_data)
		free((*ds)->ds_data);
	free(*ds);
	*ds = NULL;
}

int
_dwarf_generate_sections(Dwarf_P_Debug dbg, Dwarf_Error *error)
{
	Dwarf_CU cu;
	int i, ret;

	/*
	 * Create the single CU for this debugging object.
	 */
	if ((cu = calloc(1, sizeof(struct _Dwarf_CU))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	cu->cu_dbg = dbg;
	cu->cu_version = 2;	/* DWARF2 */
	cu->cu_pointer_size = dbg->dbg_pointer_size;
	STAILQ_INIT(&cu->cu_abbrev);
	STAILQ_INIT(&cu->cu_die);
	for (i = 0; i < DWARF_DIE_HASH_SIZE; i++)
		STAILQ_INIT(&cu->cu_die_hash[i]);
	STAILQ_INSERT_TAIL(&dbg->dbg_cu, cu, cu_next);

	if ((ret = _dwarf_info_gen(dbg, error)) != DWARF_E_NONE)
		goto fail_cleanup;

	return (DWARF_E_NONE);

fail_cleanup:

	STAILQ_REMOVE(&dbg->dbg_cu, cu, _Dwarf_CU, cu_next);
	free(cu);

	return (ret);
}
