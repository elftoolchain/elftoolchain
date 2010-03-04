/*-
 * Copyright (c) 2009, 2010 Kai Wang
 * All rights reserved.
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
 *
 * $FreeBSD: src/lib/libdwarf/dwarf_abbrev.c,v 1.1 2008/05/22 02:14:23 jb Exp $
 */

#include "_libdwarf.h"

int
_dwarf_abbrev_add(Dwarf_CU cu, uint64_t entry, uint64_t tag, uint8_t children,
    uint64_t aboff, Dwarf_Abbrev *abp, Dwarf_Error *error)
{
	Dwarf_Abbrev ab;

	if ((ab = malloc(sizeof(struct _Dwarf_Abbrev))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
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

	return (DWARF_E_NONE);
}

int
_dwarf_attrdef_add(Dwarf_Abbrev ab, uint64_t attr, uint64_t form,
    uint64_t adoff, Dwarf_AttrDef *adp, Dwarf_Error *error)
{
	Dwarf_AttrDef ad;
	
	if (ab == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DWARF_E_ARGUMENT);
	}

	if ((ad = malloc(sizeof(struct _Dwarf_AttrDef))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
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

	return (DWARF_E_NONE);
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

	ret = DWARF_E_NONE;

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
		    &ab, error)) != DWARF_E_NONE)
			break;

		do {
			adoff = offset;
			attr = _dwarf_read_uleb128(ds->ds_data, &offset);
			form = _dwarf_read_uleb128(ds->ds_data, &offset);
			if (attr != 0)
				if ((ret = _dwarf_attrdef_add(ab, attr, form,
				    adoff, NULL, error)) != DWARF_E_NONE)
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
		return (DWARF_E_NONE);

	/* Create .debug_abbrev section. */
	if ((ret = _dwarf_section_init(dbg, &ds, ".debug_abbrev", 0, error)) !=
	    DWARF_E_NONE)
		return (ret);

	STAILQ_FOREACH(ab, &cu->cu_abbrev, ab_next) {
		/* Write abbrev code. */
		ret = _dwarf_write_uleb128_alloc(&ds->ds_data, &ds->ds_cap,
		    &ds->ds_size, ab->ab_entry, error);
		if (ret != DWARF_E_NONE)
			goto fail_cleanup;

		/* Write abbrev tag. */
		ret = _dwarf_write_uleb128_alloc(&ds->ds_data, &ds->ds_cap,
		    &ds->ds_size, ab->ab_tag, error);
		if (ret != DWARF_E_NONE)
			goto fail_cleanup;

		/* Write children flag. */
		ret = dbg->write_alloc(&ds->ds_data, &ds->ds_cap, &ds->ds_size,
		    ab->ab_children, 1, error);
		if (ret != DWARF_E_NONE)
			goto fail_cleanup;

		/* Write attribute specifications. */
		STAILQ_FOREACH(ad, &ab->ab_attrdef, ad_next) {
			/* Write attribute name. */
			ret = _dwarf_write_uleb128_alloc(&ds->ds_data,
			    &ds->ds_cap, &ds->ds_size, ad->ad_attrib, error);
			if (ret != DWARF_E_NONE)
				goto fail_cleanup;
			/* Write form. */
			ret = _dwarf_write_uleb128_alloc(&ds->ds_data,
			    &ds->ds_cap, &ds->ds_size, ad->ad_form, error);
			if (ret != DWARF_E_NONE)
				goto fail_cleanup;
		}

		/* Signal end of attribute spec list. */
		ret = _dwarf_write_uleb128_alloc(&ds->ds_data, &ds->ds_cap,
		    &ds->ds_size, 0, error);
		if (ret != DWARF_E_NONE)
			goto fail_cleanup;
		ret = _dwarf_write_uleb128_alloc(&ds->ds_data, &ds->ds_cap,
		    &ds->ds_size, 0, error);
		if (ret != DWARF_E_NONE)
			goto fail_cleanup;
	}

	/* End of abbreviation for this CU. */
	ret = _dwarf_write_uleb128_alloc(&ds->ds_data, &ds->ds_cap,
	    &ds->ds_size, 0, error);
	if (ret != DWARF_E_NONE)
		goto fail_cleanup;

	/* Notify the creation of .debug_abbrev ELF section. */
	ret = _dwarf_section_callback(dbg, ds, SHT_PROGBITS, 0, 0, 0, error);
	if (ret != DWARF_E_NONE)
		goto fail_cleanup;

	return (DWARF_E_NONE);

fail_cleanup:

	_dwarf_section_free(dbg, &ds);

	return (ret);
}
