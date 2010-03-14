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
		DWARF_SET_ERROR(error, DW_DLE_DEBUG_INFO_NULL);
		return (DW_DLE_DEBUG_INFO_NULL);
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
		DWARF_SET_ERROR(error, DW_DLE_DEBUG_INFO_NULL);
		return (DW_DLE_DEBUG_INFO_NULL);
	}

	ret = _dwarf_info_init(dbg, s, error);
	if (ret != DW_DLE_NONE)
		return (ret);

#define	INIT_NAMETBL(NDX, TBL)						\
	do {								\
		if ((s = _dwarf_find_section(dbg, ".debug_" NDX)) !=	\
		    NULL) {						\
			ret = _dwarf_nametbl_init(dbg, &dbg->dbg_##TBL,	\
			    s, error);					\
			if (ret != DW_DLE_NONE)			\
				return (ret);				\
		}							\
	} while (0)


	/*
	 * Initialise name lookup sections, if exist.
	 */

	INIT_NAMETBL("pubnames", globals);
	INIT_NAMETBL("pubtypes", pubtypes);
	INIT_NAMETBL("weaknames", weaks);
	INIT_NAMETBL("static_func", funcs);
	INIT_NAMETBL("static_vars", vars);
	INIT_NAMETBL("types", types);

#undef	INIT_NAMETBL

	/*
	 * Initialise call frame data.
	 */
	ret = _dwarf_frame_init(dbg, error);
	if (ret != DW_DLE_NONE)
		return (ret);

	/*
	 * Initialise address range data.
	 */
	if ((s = _dwarf_find_section(dbg, ".debug_aranges")) != NULL) {
		ret = _dwarf_arange_init(dbg, s, error);
		if (ret != DW_DLE_NONE)
			return (ret);
	}

	/*
	 * Initialise macinfo data.
	 */
	if ((s = _dwarf_find_section(dbg, ".debug_macinfo")) != NULL) {
		ret = _dwarf_macinfo_init(dbg, s, error);
		if (ret != DW_DLE_NONE)
			return (ret);
	}

	return (ret);
}

static int
_dwarf_producer_init(Dwarf_Debug dbg, Dwarf_Unsigned pf, Dwarf_Error *error)
{

	if (pf & DW_DLC_SIZE_32 && pf & DW_DLC_SIZE_64) {
		DWARF_SET_ERROR(error, DW_DLE_ARGUMENT);
		return (DW_DLE_ARGUMENT);
	}

	if (pf & DW_DLC_SIZE_64)
		dbg->dbg_pointer_size = 8;
	else
		dbg->dbg_pointer_size = 4;

	if (pf & DW_DLC_ISA_IA64 && pf & DW_DLC_ISA_MIPS) {
		DWARF_SET_ERROR(error, DW_DLE_ARGUMENT);
		return (DW_DLE_ARGUMENT);
	}

	if (pf & DW_DLC_ISA_IA64)
		dbg->dbgp_isa = DW_DLC_ISA_IA64;
	else
		dbg->dbgp_isa = DW_DLC_ISA_MIPS;

	if (pf & DW_DLC_TARGET_BIGENDIAN && pf & DW_DLC_TARGET_LITTLEENDIAN) {
		DWARF_SET_ERROR(error, DW_DLE_ARGUMENT);
		return (DW_DLE_ARGUMENT);
	}

	if (pf & DW_DLC_TARGET_BIGENDIAN) {
		dbg->write = _dwarf_write_msb;
		dbg->write_alloc = _dwarf_write_msb_alloc;
	} else if (pf & DW_DLC_TARGET_LITTLEENDIAN) {
		dbg->write = _dwarf_write_lsb;
		dbg->write_alloc = _dwarf_write_lsb_alloc;
	} else {
#if ELFTC_BYTE_ORDER == ELFTC_BYTE_ORDER_BIG_ENDIAN
		dbg->write = _dwarf_write_msb;
		dbg->write_alloc = _dwarf_write_msb_alloc;
#else  /* ELFTC_BYTE_ORDER != ELFTC_BYTE_ORDER_BIG_ENDIAN */
		dbg->write = _dwarf_write_lsb;
		dbg->write_alloc = _dwarf_write_lsb_alloc;
#endif	/* ELFTC_BYTE_ORDER == ELFTC_BYTE_ORDER_BIG_ENDIAN */
	}

	if (pf & DW_DLC_STREAM_RELOCATIONS &&
	    pf & DW_DLC_SYMBOLIC_RELOCATIONS) {
		DWARF_SET_ERROR(error, DW_DLE_ARGUMENT);
		return (DW_DLE_ARGUMENT);
	}

	dbg->dbgp_flags = pf;

	STAILQ_INIT(&dbg->dbgp_dielist);
	STAILQ_INIT(&dbg->dbgp_pelist);
	STAILQ_INIT(&dbg->dbgp_seclist);
	STAILQ_INIT(&dbg->dbgp_drslist);

	if ((dbg->dbgp_lineinfo = calloc(1, sizeof(struct _Dwarf_LineInfo))) ==
	    NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	STAILQ_INIT(&dbg->dbgp_lineinfo->li_lflist);
	STAILQ_INIT(&dbg->dbgp_lineinfo->li_lnlist);

	if ((dbg->dbgp_as = calloc(1, sizeof(struct _Dwarf_ArangeSet))) ==
	    NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	STAILQ_INIT(&dbg->dbgp_as->as_arlist);

	return (DW_DLE_NONE);
}

int
_dwarf_init(Dwarf_Debug dbg, Dwarf_Unsigned pro_flags, Dwarf_Error *error)
{
	int ret;

	ret = DW_DLE_NONE;

	STAILQ_INIT(&dbg->dbg_cu);
	STAILQ_INIT(&dbg->dbg_rllist);
	STAILQ_INIT(&dbg->dbg_aslist);
	STAILQ_INIT(&dbg->dbg_mslist);
	TAILQ_INIT(&dbg->dbg_loclist);

	if (dbg->dbg_mode == DW_DLC_READ || dbg->dbg_mode == DW_DLC_RDWR) {
		ret = _dwarf_consumer_init(dbg, error);
		if (ret != DW_DLE_NONE) {
			_dwarf_deinit(dbg);
			return (ret);
		}
	}

	if (dbg->dbg_mode == DW_DLC_WRITE || dbg->dbg_mode == DW_DLC_RDWR) {
		ret = _dwarf_producer_init(dbg, pro_flags, error);
		if (ret != DW_DLE_NONE) {
			_dwarf_deinit(dbg);
			return (ret);
		}
	}

	/*
	 * Initialise internal string table.
	 */
	if ((ret = _dwarf_strtab_init(dbg, error)) != DW_DLE_NONE)
		return (ret);

	return (DW_DLE_NONE);
}

void
_dwarf_deinit(Dwarf_Debug dbg)
{
	Dwarf_Abbrev ab;
	Dwarf_Abbrev tab;
	Dwarf_AttrDef ad;
	Dwarf_AttrDef tad;
	Dwarf_Attribute at;
	Dwarf_Attribute tat;
	Dwarf_CU cu;
	Dwarf_CU tcu;
	Dwarf_Die die;
	Dwarf_Die tdie;
	Dwarf_Loclist ll;
	Dwarf_Loclist tll;
	Dwarf_LineInfo li;
	Dwarf_LineFile lf, tlf;
	Dwarf_Line ln, tln;

	assert(dbg != NULL);

	/* Free entries in the compilation unit list. */
	STAILQ_FOREACH_SAFE(cu, &dbg->dbg_cu, cu_next, tcu) {
		/* Free entries in the die list */
		STAILQ_FOREACH_SAFE(die, &cu->cu_die, die_next, tdie) {
			/* Free entries in the attribute list */
			STAILQ_FOREACH_SAFE(at, &die->die_attr, at_next, tat) {
				STAILQ_REMOVE(&die->die_attr, at,
				    _Dwarf_Attribute, at_next);
				if (at->at_ld != NULL)
					free(at->at_ld);
				free(at);
			}

			if (die->die_attrarray)
				free(die->die_attrarray);

			STAILQ_REMOVE(&cu->cu_die, die, _Dwarf_Die, die_next);

			free(die);

		}

		/* Free entries in the abbrev list */
		STAILQ_FOREACH_SAFE(ab, &cu->cu_abbrev, ab_next, tab) {
			/* Free entries in the attribute list */
			STAILQ_FOREACH_SAFE(ad, &ab->ab_attrdef, ad_next, tad) {
				STAILQ_REMOVE(&ab->ab_attrdef, ad,
				    _Dwarf_AttrDef, ad_next);
				free(ad);
			}

			STAILQ_REMOVE(&cu->cu_abbrev, ab, _Dwarf_Abbrev,
			    ab_next);
			free(ab);
		}

		/* Free lineinfo. */
		if (cu->cu_lineinfo != NULL) {
			li = cu->cu_lineinfo;
			STAILQ_FOREACH_SAFE(lf, &li->li_lflist, lf_next, tlf) {
				STAILQ_REMOVE(&li->li_lflist, lf,
				    _Dwarf_LineFile, lf_next);
				if (lf->lf_fullpath)
					free(lf->lf_fullpath);
				free(lf);
			}
			STAILQ_FOREACH_SAFE(ln, &li->li_lnlist, ln_next, tln) {
				STAILQ_REMOVE(&li->li_lnlist, ln, _Dwarf_Line,
				    ln_next);
				free(ln);
			}
			if (li->li_oplen)
				free(li->li_oplen);
			if (li->li_incdirs)
				free(li->li_incdirs);
			if (li->li_lnarray)
				free(li->li_lnarray);
			free(li);
		}

		STAILQ_REMOVE(&dbg->dbg_cu, cu, _Dwarf_CU, cu_next);
		free(cu);
	}

	/* Free loclist list. */
	TAILQ_FOREACH_SAFE(ll, &dbg->dbg_loclist, ll_next, tll) {
		TAILQ_REMOVE(&dbg->dbg_loclist, ll, ll_next);
		_dwarf_loclist_cleanup(ll);
	}

	_dwarf_ranges_cleanup(dbg);

	if (dbg->dbg_globals)
		_dwarf_nametbl_cleanup(dbg->dbg_globals);
	if (dbg->dbg_pubtypes)
		_dwarf_nametbl_cleanup(dbg->dbg_pubtypes);
	if (dbg->dbg_weaks)
		_dwarf_nametbl_cleanup(dbg->dbg_weaks);
	if (dbg->dbg_funcs)
		_dwarf_nametbl_cleanup(dbg->dbg_funcs);
	if (dbg->dbg_vars)
		_dwarf_nametbl_cleanup(dbg->dbg_vars);
	if (dbg->dbg_types)
		_dwarf_nametbl_cleanup(dbg->dbg_types);

	_dwarf_frame_cleanup(dbg);

	_dwarf_arange_cleanup(dbg);

	_dwarf_macinfo_cleanup(dbg);

	_dwarf_strtab_cleanup(dbg);
}

int
_dwarf_alloc(Dwarf_Debug *ret_dbg, int mode, Dwarf_Error *error)
{
	Dwarf_Debug dbg;

	if ((dbg = calloc(sizeof(struct _Dwarf_Debug), 1)) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	dbg->dbg_mode = mode;

	*ret_dbg = dbg;

	return (DW_DLE_NONE);
}
