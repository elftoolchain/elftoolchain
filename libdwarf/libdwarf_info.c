/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
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
	int dwarf_size, i, ret, level;
	uint64_t length;
	uint64_t next_offset;
	uint64_t offset;

	ret = DWARF_E_NONE;
	
	offset = 0;
	while (offset < ds->ds_size) {
		if ((cu = calloc(1, sizeof(struct _Dwarf_CU))) == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			return (DWARF_E_MEMORY);
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
			DWARF_SET_ERROR(error, DWARF_E_INVALID_CU);
			return (DWARF_E_INVALID_CU);
		}

		/* Compute the offset to the next compilation unit: */
		next_offset = offset + length;

		/* Initialise the compilation unit. */
		cu->cu_length 		= length;
		cu->cu_header_length	= (dwarf_size == 4) ? 4 : 12;
		cu->cu_version		= dbg->read(ds->ds_data, &offset, 2);
		cu->cu_abbrev_offset	= dbg->read(ds->ds_data, &offset,
		    dwarf_size);
		cu->cu_pointer_size	= dbg->read(ds->ds_data, &offset, 1);
		cu->cu_next_offset	= next_offset;

		/* Initialise the list of abbrevs. */
		STAILQ_INIT(&cu->cu_abbrev);

		/* Initialise the list of dies. */
		STAILQ_INIT(&cu->cu_die);

		/* Initialise the hash table of dies. */
		for (i = 0; i < DWARF_DIE_HASH_SIZE; i++)
			STAILQ_INIT(&cu->cu_die_hash[i]);

		/* Add the compilation unit to the list. */
		STAILQ_INSERT_TAIL(&dbg->dbg_cu, cu, cu_next);

		if (cu->cu_version != 2 && cu->cu_version != 3) {
			DWARF_SET_ERROR(error, DWARF_E_CU_VERSION);
			ret = DWARF_E_CU_VERSION;
			break;
		}

		/* Parse the .debug_abbrev info for this CU: */
		if ((ret = _dwarf_abbrev_init(dbg, cu, error)) != DWARF_E_NONE)
			break;

		level = 0;

		while (offset < next_offset && offset < ds->ds_size) {
			Dwarf_Abbrev ab;
			Dwarf_AttrDef ad;
			Dwarf_Die die;
			uint64_t abnum;
			uint64_t die_offset = offset;

			abnum = _dwarf_read_uleb128(ds->ds_data, &offset);

			if (abnum == 0) {
				level--;
				continue;
			}

			if ((ab = _dwarf_abbrev_find(cu, abnum)) == NULL) {
				DWARF_SET_ERROR(error, DWARF_E_MISSING_ABBREV);
				return DWARF_E_MISSING_ABBREV;
			}

			if ((ret = _dwarf_die_add(cu, level, die_offset, abnum,
			    ab, &die, error)) != DWARF_E_NONE)
				return ret;

			STAILQ_FOREACH(ad, &ab->ab_attrdef, ad_next) {
				if ((ret = _dwarf_attr_init(dbg, ds, &offset,
				    dwarf_size, cu, die, ad, ad->ad_form, 0,
				    error)) != DWARF_E_NONE)
					return ret;
			}

			if (ab->ab_children == DW_CHILDREN_yes)
				level++;
		}

		offset = next_offset;
	}

	return (ret);
}
