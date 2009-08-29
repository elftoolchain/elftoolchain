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
#include <stdlib.h>
#include <string.h>
#include "_libdwarf.h"

Dwarf_Section *
_dwarf_find_section(Dwarf_Debug dbg, const char *name)
{
	Dwarf_Section *ds;
	Dwarf_Half i;

	assert(name != NULL);

	for (i = 0; i < dbg->dbg_seccnt; i++) {
		ds = &dbg->dbg_section[i];
		if (ds->ds_name != NULL && !strcmp(ds->ds_name, name))
		    return (ds);
	}

	return (NULL);
}

static int
_dwarf_consumer_init(Dwarf_Debug dbg, Dwarf_Error *error)
{
	const Dwarf_Obj_Access_Methods *m;
	Dwarf_Obj_Access_Section sec;
	void *obj;
	Dwarf_Section *s;
	Dwarf_Unsigned cnt;
	Dwarf_Half i;
	int ret;

	assert(dbg != NULL);
	assert(dbg->dbg_iface != NULL);

	m = dbg->dbg_iface->methods;
	obj = dbg->dbg_iface->object;

	assert(m != NULL);
	assert(obj != NULL);

	if (m->get_byte_order(obj) == DW_OBJECT_MSB) {
		dbg->read = _dwarf_read_msb;
		dbg->write = _dwarf_write_msb;
		dbg->decode = _dwarf_decode_msb;
	} else {
		dbg->read = _dwarf_read_lsb;
		dbg->write = _dwarf_write_lsb;
		dbg->decode = _dwarf_decode_lsb;
	}

	dbg->dbg_pointer_size = m->get_pointer_size(obj);

	cnt = m->get_section_count(obj);

	if (cnt == 0) {
		DWARF_SET_ERROR(error, DWARF_E_DEBUG_INFO);
		return (DWARF_E_DEBUG_INFO);
	}

	dbg->dbg_seccnt = cnt;

	if ((dbg->dbg_section = calloc(cnt, sizeof(Dwarf_Section))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	for (i = 0; i < cnt; i++) {
		if (m->get_section_info(obj, i, &sec, &ret) != DW_DLV_OK) {
			free(dbg->dbg_section);
			DWARF_SET_ERROR(error, ret);
			return (ret);
		}

		dbg->dbg_section[i].ds_size = sec.size;
		dbg->dbg_section[i].ds_name = sec.name;
		
		if (m->load_section(obj, i, &dbg->dbg_section[i].ds_data, &ret)
		    != DW_DLV_OK) {
			free(dbg->dbg_section);
			DWARF_SET_ERROR(error, ret);
			return (ret);
		}			
	}

	if (_dwarf_find_section(dbg, ".debug_abbrev") == NULL ||
	    ((s = _dwarf_find_section(dbg, ".debug_info")) == NULL)) {
		free(dbg->dbg_section);
		DWARF_SET_ERROR(error, DWARF_E_DEBUG_INFO);
		return (DWARF_E_DEBUG_INFO);
	}

	ret = _dwarf_info_init(dbg, s, error);
	if (ret != DWARF_E_NONE)
		return (ret);

#define	INIT_NAMETBL(NDX, TBL)						\
	do {								\
		if ((s = _dwarf_find_section(dbg, ".debug_##NDX")) !=	\
		    NULL) {						\
			ret = _dwarf_nametbl_init(dbg, &dbg->dbg_##TBL,	\
			    s, error);					\
			if (ret != DWARF_E_NONE)			\
				return (ret);				\
		}							\
	} while (0)


	/*
	 * Initialise name lookup sections, if exist.
	 */

	INIT_NAMETBL(pubnames, globals);
	INIT_NAMETBL(pubtypes, pubtypes);
	INIT_NAMETBL(weaknames, weaks);
	INIT_NAMETBL(static_func, funcs);
	INIT_NAMETBL(static_vars, vars);
	INIT_NAMETBL(types, types);

#undef	INIT_NAMETBL

	/*
	 * Initialise call frame data.
	 */
	ret = _dwarf_frame_init(dbg, error);
	if (ret != DWARF_E_NONE)
		return (ret);

	/*
	 * Initialise address range data.
	 */
	if ((s = _dwarf_find_section(dbg, ".debug_aranges")) != NULL) {
		ret = _dwarf_arange_init(dbg, s, error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}

	/*
	 * Initialise macinfo data.
	 */
	if ((s = _dwarf_find_section(dbg, ".debug_macinfo")) != NULL) {
		ret = _dwarf_macinfo_init(dbg, s, error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}

	return (ret);
}

int
_dwarf_init(Dwarf_Debug dbg, Dwarf_Error* error)
{
	int ret;

	ret = DWARF_E_NONE;

	STAILQ_INIT(&dbg->dbg_cu);
	STAILQ_INIT(&dbg->dbg_rllist);
	STAILQ_INIT(&dbg->dbg_aslist);
	STAILQ_INIT(&dbg->dbg_mslist);
	TAILQ_INIT(&dbg->dbg_loclist);

	if (dbg->dbg_mode == DW_DLC_READ || dbg->dbg_mode == DW_DLC_RDWR) {
		ret = _dwarf_consumer_init(dbg, error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}

	return (DWARF_E_NONE);
}

#if 0
Dwarf_P_Debug
_dwarf_producer_init(Dwarf_Error *error)
{
	Dwarf_P_Debug dbg;

	dbg = calloc(1, sizeof(struct _Dwarf_Debug));
	if (dbg == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (NULL);
	}

	return (dbg);
}
#endif

