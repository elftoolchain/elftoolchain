/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
 * Copyright (c) 2009,2010 Kai Wang
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
_dwarf_abbrev_add(Dwarf_CU cu, uint64_t entry, uint64_t tag, uint8_t children,
    uint64_t aboff, Dwarf_Abbrev *abp, Dwarf_Error *error)
{
	Dwarf_Abbrev ab;
	Dwarf_Debug dbg;

	dbg = cu != NULL ? cu->cu_dbg : NULL;

	if ((ab = malloc(sizeof(struct _Dwarf_Abbrev))) == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_MEMORY);
		return (DW_DLE_MEMORY);
	}

	/* Initialise the abbrev structure. */
	ab->ab_entry	= entry;
	ab->ab_tag	= tag;
	ab->ab_children	= children;
	ab->ab_offset	= aboff;
	ab->ab_length	= 0;	/* fill in later. */
	ab->ab_atnum	= 0;	/* fill in later. */

	/* Initialise the list of attribute definitions. */
	STAILQ_INIT(&ab->ab_attrdef);

	/* Add the abbrev to the list in the compilation unit. */
	STAILQ_INSERT_TAIL(&cu->cu_abbrev, ab, ab_next);

	if (abp != NULL)
		*abp = ab;

	return (DW_DLE_NONE);
}

int
_dwarf_attrdef_add(Dwarf_Debug dbg, Dwarf_Abbrev ab, uint64_t attr,
    uint64_t form, uint64_t adoff, Dwarf_AttrDef *adp, Dwarf_Error *error)
{
	Dwarf_AttrDef ad;

	if (ab == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_ARGUMENT);
		return (DW_DLE_ARGUMENT);
	}

	if ((ad = malloc(sizeof(struct _Dwarf_AttrDef))) == NULL) {
		DWARF_SET_ERROR(dbg, error, DW_DLE_MEMORY);
		return (DW_DLE_MEMORY);
	}

	/* Initialise the attribute definition structure. */
	ad->ad_attrib	= attr;
	ad->ad_form	= form;
	ad->ad_offset	= adoff;

	/* Add the attribute definition to the list in the abbrev. */
	STAILQ_INSERT_TAIL(&ab->ab_attrdef, ad, ad_next);

	/* Increase number of attribute counter. */
	ab->ab_atnum++;

	if (adp != NULL)
		*adp = ad;

	return (DW_DLE_NONE);
}

int
_dwarf_abbrev_init(Dwarf_Debug dbg, Dwarf_CU cu, Dwarf_Error *error)
{
	Dwarf_Abbrev ab;
	Dwarf_Section *ds;
	int ret;
	uint64_t attr;
	uint64_t entry;
	uint64_t form;
	uint64_t offset;
	uint64_t aboff;
	uint64_t adoff;
	uint64_t tag;
	u_int8_t children;

	ret = DW_DLE_NONE;

	ds = _dwarf_find_section(dbg, ".debug_abbrev");
	assert(ds != NULL);

	offset = cu->cu_abbrev_offset;
	while (offset < ds->ds_size) {
		aboff = offset;

		entry = _dwarf_read_uleb128(ds->ds_data, &offset);
		if (entry == 0) {
			/* Last entry. */
			ret = _dwarf_abbrev_add(cu, entry, 0, 0, aboff, &ab,
			    error);
			ab->ab_length = 1;
			break;
		}

		tag = _dwarf_read_uleb128(ds->ds_data, &offset);

		children = dbg->read(ds->ds_data, &offset, 1);

		if ((ret = _dwarf_abbrev_add(cu, entry, tag, children, aboff,
		    &ab, error)) != DW_DLE_NONE)
			break;

		do {
			adoff = offset;
			attr = _dwarf_read_uleb128(ds->ds_data, &offset);
			form = _dwarf_read_uleb128(ds->ds_data, &offset);
			if (attr != 0)
				if ((ret = _dwarf_attrdef_add(dbg, ab, attr,
				    form, adoff, NULL, error)) != DW_DLE_NONE)
					return (ret);
		} while (attr != 0);

		ab->ab_length = offset - aboff;
	}

	return (ret);
}

Dwarf_Abbrev
_dwarf_abbrev_find(Dwarf_CU cu, uint64_t entry)
{
	Dwarf_Abbrev ab;

	ab = NULL;
	STAILQ_FOREACH(ab, &cu->cu_abbrev, ab_next) {
		if (ab->ab_entry == entry)
			break;
	}

	return (ab);
}

void
_dwarf_abbrev_cleanup(Dwarf_CU cu)
{
	Dwarf_Abbrev ab, tab;
	Dwarf_AttrDef ad, tad;

	assert(cu != NULL);

	STAILQ_FOREACH_SAFE(ab, &cu->cu_abbrev, ab_next, tab) {
		STAILQ_REMOVE(&cu->cu_abbrev, ab, _Dwarf_Abbrev, ab_next);
		STAILQ_FOREACH_SAFE(ad, &ab->ab_attrdef, ad_next, tad) {
			STAILQ_REMOVE(&ab->ab_attrdef, ad, _Dwarf_AttrDef,
			    ad_next);
			free(ad);
		}
		free(ab);
	}
}

int
_dwarf_abbrev_gen(Dwarf_P_Debug dbg, Dwarf_Error *error)
{
	Dwarf_CU cu;
	Dwarf_Abbrev ab;
	Dwarf_AttrDef ad;
	Dwarf_P_Section ds;
	int ret;

	cu = STAILQ_FIRST(&dbg->dbg_cu);
	if (cu == NULL)
		return (DW_DLE_NONE);

	/* Create .debug_abbrev section. */
	if ((ret = _dwarf_section_init(dbg, &ds, ".debug_abbrev", 0, error)) !=
	    DW_DLE_NONE)
		return (ret);

	STAILQ_FOREACH(ab, &cu->cu_abbrev, ab_next) {
		RCHECK(WRITE_ULEB128(ab->ab_entry));
		RCHECK(WRITE_ULEB128(ab->ab_tag));
		RCHECK(WRITE_VALUE(ab->ab_children, 1));
		STAILQ_FOREACH(ad, &ab->ab_attrdef, ad_next) {
			RCHECK(WRITE_ULEB128(ad->ad_attrib));
			RCHECK(WRITE_ULEB128(ad->ad_form));
		}
		/* Signal end of attribute spec list. */
		RCHECK(WRITE_ULEB128(0));
		RCHECK(WRITE_ULEB128(0));
	}
	/* End of abbreviation for this CU. */
	RCHECK(WRITE_ULEB128(0));

	/* Notify the creation of .debug_abbrev ELF section. */
	RCHECK(_dwarf_section_callback(dbg, ds, SHT_PROGBITS, 0, 0, 0, error));

	return (DW_DLE_NONE);

gen_fail:

	_dwarf_section_free(dbg, &ds);

	return (ret);
}
