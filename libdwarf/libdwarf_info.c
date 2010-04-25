/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
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

int
_dwarf_info_init(Dwarf_Debug dbg, Dwarf_Section *ds, Dwarf_Error *error)
{
	Dwarf_CU cu;
	int dwarf_size, i, ret;
	uint64_t length;
	uint64_t next_offset;
	uint64_t offset;

	ret = DW_DLE_NONE;

	offset = 0;
	while (offset < ds->ds_size) {
		if ((cu = calloc(1, sizeof(struct _Dwarf_CU))) == NULL) {
			DWARF_SET_ERROR(error, DW_DLE_MEMORY);
			return (DW_DLE_MEMORY);
		}

		cu->cu_dbg = dbg;
		cu->cu_offset = offset;

		length = dbg->read(ds->ds_data, &offset, 4);
		if (length == 0xffffffff) {
			length = dbg->read(ds->ds_data, &offset, 8);
			dwarf_size = 8;
		} else
			dwarf_size = 4;

		/*
		 * Check if there is enough ELF data for this CU. This assumes
		 * that libelf gives us the entire section in one Elf_Data
		 * object.
		 */
		if (length > ds->ds_size - offset) {
			free(cu);
			DWARF_SET_ERROR(error, DW_DLE_CU_LENGTH_ERROR);
			return (DW_DLE_CU_LENGTH_ERROR);
		}

		/* Compute the offset to the next compilation unit: */
		next_offset = offset + length;

		/* Initialise the compilation unit. */
		cu->cu_length 		= length;
		cu->cu_length_size	= (dwarf_size == 4 ? 4 : 12);
		cu->cu_version		= dbg->read(ds->ds_data, &offset, 2);
		cu->cu_abbrev_offset	= dbg->read(ds->ds_data, &offset,
		    dwarf_size);
		cu->cu_pointer_size	= dbg->read(ds->ds_data, &offset, 1);
		cu->cu_next_offset	= next_offset;

		STAILQ_INIT(&cu->cu_abbrev);
		STAILQ_INIT(&cu->cu_die);

		/* Initialise the hash table of dies. */
		for (i = 0; i < DWARF_DIE_HASH_SIZE; i++)
			STAILQ_INIT(&cu->cu_die_hash[i]);

		/* Add the compilation unit to the list. */
		STAILQ_INSERT_TAIL(&dbg->dbg_cu, cu, cu_next);

		if (cu->cu_version != 2 && cu->cu_version != 3) {
			DWARF_SET_ERROR(error, DW_DLE_VERSION_STAMP_ERROR);
			ret = DW_DLE_VERSION_STAMP_ERROR;
			break;
		}

		/*
		 * Parse the .debug_abbrev info for this CU.
		 */
		if ((ret = _dwarf_abbrev_init(dbg, cu, error)) != DW_DLE_NONE)
			break;

		/*
		 * Parse the list of DIE for this CU.
		 */
		if ((ret = _dwarf_die_parse(dbg, ds, cu, dwarf_size, offset,
		    next_offset, error)) != DW_DLE_NONE)
			break;

		offset = next_offset;
	}

	return (ret);
}

void
_dwarf_info_cleanup(Dwarf_Debug dbg)
{
	Dwarf_CU cu, tcu;

	assert(dbg != NULL && dbg->dbg_mode == DW_DLC_READ);

	STAILQ_FOREACH_SAFE(cu, &dbg->dbg_cu, cu_next, tcu) {
		STAILQ_REMOVE(&dbg->dbg_cu, cu, _Dwarf_CU, cu_next);
		_dwarf_die_cleanup(dbg, cu);
		_dwarf_abbrev_cleanup(cu);
		if (cu->cu_lineinfo != NULL) {
			_dwarf_lineno_cleanup(cu->cu_lineinfo);
			cu->cu_lineinfo = NULL;
		}
		free(cu);
	}
}

int
_dwarf_info_gen(Dwarf_P_Debug dbg, Dwarf_Error *error)
{
	Dwarf_P_Section ds;
	Dwarf_Rel_Section drs;
	Dwarf_Unsigned offset;
	Dwarf_CU cu;
	int i, ret;

	assert(dbg != NULL && dbg->write_alloc != NULL);

	if (dbg->dbgp_root_die == NULL)
		return (DW_DLE_NONE);

	/* Create the single CU for this debugging object. */
	if ((cu = calloc(1, sizeof(struct _Dwarf_CU))) == NULL) {
		DWARF_SET_ERROR(error, DW_DLE_MEMORY);
		return (DW_DLE_MEMORY);
	}
	cu->cu_dbg = dbg;
	cu->cu_version = 2;	/* DWARF2 */
	cu->cu_pointer_size = dbg->dbg_pointer_size;
	STAILQ_INIT(&cu->cu_abbrev);
	STAILQ_INIT(&cu->cu_die);
	for (i = 0; i < DWARF_DIE_HASH_SIZE; i++)
		STAILQ_INIT(&cu->cu_die_hash[i]);
	STAILQ_INSERT_TAIL(&dbg->dbg_cu, cu, cu_next);

	/* Create .debug_init section. */
	if ((ret = _dwarf_section_init(dbg, &dbg->dbgp_info, ".debug_init", 0,
	    error)) != DW_DLE_NONE)
		goto gen_fail1;
	ds = dbg->dbgp_info;

	/* Create relocation section for .debug_init */
	if ((ret = _dwarf_reloc_section_init(dbg, &drs, ds, error)) !=
	    DW_DLE_NONE)
		goto gen_fail0;

	/* Length placeholder. (We only use 32-bit DWARF format) */
	RCHECK(WRITE_VALUE(cu->cu_length, 4));

	/* Write CU version */
	RCHECK(WRITE_VALUE(cu->cu_version, 2));

	/*
	 * Write abbrev offset. (always 0, we only support single CU)
	 * Also generate a relocation entry for this offset.
	 */
	RCHECK(_dwarf_reloc_entry_add(dbg, drs, ds, dwarf_drt_data_reloc, 4,
	    ds->ds_size, 0, cu->cu_abbrev_offset, ".debug_abbrev", error));

	/* Pointer size. */
	RCHECK(WRITE_VALUE(cu->cu_pointer_size, 1));

	/* Transform the DIE(s) of this CU. */
	RCHECK(_dwarf_die_gen(dbg, cu, drs, error));

	/* Now we can fill in the length of this CU. */
	cu->cu_length = ds->ds_size - 4;
	offset = 0;
	dbg->write(ds->ds_data, &offset, cu->cu_length, 4);

	/* Inform application the creation of .debug_info ELF section. */
	RCHECK(_dwarf_section_callback(dbg, ds, SHT_PROGBITS, 0, 0, 0, error));

	/*
	 * Inform application the creation of relocation section for
	 * .debug_info.
	 */
	RCHECK(_dwarf_reloc_section_finalize(dbg, drs, error));

	return (DW_DLE_NONE);

gen_fail:
	_dwarf_reloc_section_free(dbg, &drs);

gen_fail0:
	_dwarf_section_free(dbg, &dbg->dbgp_info);

gen_fail1:
	STAILQ_REMOVE(&dbg->dbg_cu, cu, _Dwarf_CU, cu_next);
	free(cu);

	return (ret);
}

void
_dwarf_info_pro_cleanup(Dwarf_P_Debug dbg)
{
	Dwarf_CU cu;

	assert(dbg != NULL && dbg->dbg_mode == DW_DLC_WRITE);

	cu = STAILQ_FIRST(&dbg->dbg_cu);
	STAILQ_REMOVE(&dbg->dbg_cu, cu, _Dwarf_CU, cu_next);
	_dwarf_abbrev_cleanup(cu);
	free(cu);
}

