/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
 * Copyright (c) 2009, 2010 Kai Wang
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
_dwarf_die_alloc(Dwarf_Die *ret_die, Dwarf_Error *error)
{
	Dwarf_Die die;

	assert(ret_die != NULL);

	if ((die = calloc(1, sizeof(struct _Dwarf_Die))) == NULL) {
		DWARF_SET_ERROR(error, DW_DLE_MEMORY);
		return (DW_DLE_MEMORY);
	}

	STAILQ_INIT(&die->die_attr);

	*ret_die = die;

	return (DW_DLE_NONE);
}

int
_dwarf_die_add(Dwarf_CU cu, uint64_t offset, uint64_t abnum, Dwarf_Abbrev ab,
    Dwarf_Die *diep, Dwarf_Error *error)
{
	Dwarf_Die die;
	uint64_t key;
	int ret;

	if (cu == NULL || ab == NULL) {
		DWARF_SET_ERROR(error, DW_DLE_ARGUMENT);
		return (DW_DLE_ARGUMENT);
	}

	if ((ret = _dwarf_die_alloc(&die, error)) != DW_DLE_NONE)
		return (ret);

	die->die_offset	= offset;
	die->die_abnum	= abnum;
	die->die_ab	= ab;
	die->die_cu	= cu;
	die->die_dbg	= cu->cu_dbg;

	STAILQ_INSERT_TAIL(&cu->cu_die, die, die_next);

	/* Add the die to the hash table in the compilation unit. */
	key = offset % DWARF_DIE_HASH_SIZE;
	STAILQ_INSERT_TAIL(&cu->cu_die_hash[key], die, die_hash);

	if (diep != NULL)
		*diep = die;

	return (DW_DLE_NONE);
}

/* Find die at offset 'off' within the same CU. */
Dwarf_Die
_dwarf_die_find(Dwarf_Die die, Dwarf_Unsigned off)
{
	Dwarf_CU cu;
	Dwarf_Die die1;

	cu = die->die_cu;
	STAILQ_FOREACH(die1, &cu->cu_die, die_next) {
		if (die1->die_offset == off)
			return (die1);
	}

	return (NULL);
}

int
_dwarf_die_parse(Dwarf_Debug dbg, Dwarf_Section *ds, Dwarf_CU cu,
    int dwarf_size, uint64_t offset, uint64_t next_offset, Dwarf_Error *error)
{
	Dwarf_Abbrev ab;
	Dwarf_AttrDef ad;
	Dwarf_Die die;
	Dwarf_Die parent;
	Dwarf_Die left;
	uint64_t abnum;
	uint64_t die_offset;
	int ret;

	die = NULL;
	parent = NULL;
	left = NULL;

	while (offset < next_offset && offset < ds->ds_size) {

		die_offset = offset;

		abnum = _dwarf_read_uleb128(ds->ds_data, &offset);

		if (abnum == 0) {
			/*
			 * Return to previous DIE level.
			 */
			left = parent;
			if (parent == NULL)
				break;

			parent = parent->die_parent;
			continue;
		}

		if ((ab = _dwarf_abbrev_find(cu, abnum)) == NULL) {
			DWARF_SET_ERROR(error, DW_DLE_DEBUG_ABBREV_NULL);
			return (DW_DLE_DEBUG_ABBREV_NULL);
		}

		if ((ret = _dwarf_die_add(cu, die_offset, abnum, ab, &die,
		    error)) != DW_DLE_NONE)
			return (ret);

		STAILQ_FOREACH(ad, &ab->ab_attrdef, ad_next) {
			if ((ret = _dwarf_attr_init(dbg, ds, &offset,
			    dwarf_size, cu, die, ad, ad->ad_form, 0,
			    error)) != DW_DLE_NONE)
				return (ret);
		}

		die->die_parent = parent;
		die->die_left = left;

		if (left)
			left->die_right = die;
		else if (parent)
			parent->die_child = die; /* First child. */

		left = die;

		if (ab->ab_children == DW_CHILDREN_yes) {
			/*
			 * Advance to next DIE level.
			 */
			parent = die;
			left = NULL;
		}
	}

	return (DW_DLE_NONE);
}

void
_dwarf_die_cleanup(Dwarf_Debug dbg, Dwarf_CU cu)
{
	Dwarf_Die die, tdie;
	Dwarf_Attribute at, tat;

	assert(dbg != NULL && dbg->dbg_mode == DW_DLC_READ);
	assert(cu != NULL);

	STAILQ_FOREACH_SAFE(die, &cu->cu_die, die_next, tdie) {
		STAILQ_REMOVE(&cu->cu_die, die, _Dwarf_Die, die_next);
		STAILQ_FOREACH_SAFE(at, &die->die_attr, at_next, tat) {
			STAILQ_REMOVE(&die->die_attr, at,
			    _Dwarf_Attribute, at_next);
			if (at->at_ld != NULL)
				free(at->at_ld);
			free(at);
		}
		if (die->die_attrarray)
			free(die->die_attrarray);
		free(die);
	}
}

void
_dwarf_die_link(Dwarf_P_Die die, Dwarf_P_Die parent, Dwarf_P_Die child,
    Dwarf_P_Die left_sibling, Dwarf_P_Die right_sibling)
{

	assert(die != NULL);

	if (parent) {

		/* Disconnect from old parent. */
		if (die->die_parent) {
			if (die->die_parent == parent)
				return;
			die->die_parent->die_child = NULL;
			die->die_parent = NULL;
		}

		/* Connect to new parent. */
		die->die_parent = parent;
		parent->die_child = die;
		return;
	}

	if (child) {

		/* Disconnect from old child. */
		if (die->die_child) {
			if (die->die_child == child)
				return;
			die->die_child->die_parent = NULL;
			die->die_child = NULL;
		}

		/* Connect to new child. */
		die->die_child = child;
		child->die_parent = die;
		return;
	}

	if (left_sibling) {

		/* Disconnect from old left sibling. */
		if (die->die_left) {
			if (die->die_left == left_sibling)
				return;
			die->die_left->die_right = NULL;
			die->die_left = NULL;
		}

		/* Connect to new right sibling. */
		die->die_left = left_sibling;
		left_sibling->die_right = die;
		return;
	}

	if (right_sibling) {

		/* Disconnect from old right sibling. */
		if (die->die_right) {
			if (die->die_right == right_sibling)
				return;
			die->die_right->die_left = NULL;
			die->die_right = NULL;
		}

		/* Connect to new right sibling. */
		die->die_right = right_sibling;
		right_sibling->die_left = die;
		return;
	}
}

int
_dwarf_die_count_links(Dwarf_P_Die parent, Dwarf_P_Die child,
    Dwarf_P_Die left_sibling, Dwarf_P_Die right_sibling)
{
	int count;

	count = 0;

	if (parent)
		count++;
	if (child)
		count++;
	if (left_sibling)
		count++;
	if (right_sibling)
		count++;

	return (count);
}

static int
_dwarf_die_gen_recursive(Dwarf_P_Debug dbg, Dwarf_CU cu, Dwarf_Rel_Section drs,
    Dwarf_P_Die die, int pass2, Dwarf_Error *error) {
	Dwarf_P_Section ds;
	Dwarf_Abbrev ab;
	Dwarf_Attribute at;
	Dwarf_AttrDef ad;
	int match, ret;

	ds = dbg->dbgp_info;
	assert(ds != NULL);

	if (pass2)
		goto attr_gen;

	if (STAILQ_EMPTY(&die->die_attr))
		goto null_die;

	/*
	 * Search abbrev list to find a matching entry.
	 */
	die->die_ab = NULL;
	STAILQ_FOREACH(ab, &cu->cu_abbrev, ab_next) {
		if (die->die_tag != ab->ab_tag)
			continue;
		if (ab->ab_children == DW_CHILDREN_no && die->die_child != NULL)
			continue;
		if (ab->ab_children == DW_CHILDREN_yes &&
		    die->die_child == NULL)
			continue;
		at = STAILQ_FIRST(&die->die_attr);
		ad = STAILQ_FIRST(&ab->ab_attrdef);
		assert(at != NULL && ad != NULL);
		match = 1;
		do {
			if (at->at_attrib != ad->ad_attrib ||
			    at->at_form != ad->ad_form) {
				match = 0;
				break;
			}
			at = STAILQ_NEXT(at, at_next);
			ad = STAILQ_NEXT(ad, ad_next);
			if ((at == NULL && ad != NULL) ||
			    (at != NULL && ad == NULL))
				match = 0;
		} while (at != NULL && ad != NULL);
		if (match) {
			die->die_ab = ab;
			break;
		}
	}

	/*
	 * Create a new abbrev entry if we can not reuse any existing one.
	 */
	if (die->die_ab == NULL) {
		ret = _dwarf_abbrev_add(cu, ++cu->cu_abbrev_cnt, die->die_tag,
		    die->die_child != NULL ? 1 : 0, 0, &ab, error);
		if (ret != DW_DLE_NONE)
			return (ret);
		STAILQ_FOREACH(at, &die->die_attr, at_next) {
			ret = _dwarf_attrdef_add(ab, at->at_attrib, at->at_form,
			    0, NULL, error);
			if (ret != DW_DLE_NONE)
				return (ret);
		}
		die->die_ab = ab;
	}

	die->die_offset = ds->ds_size;

	/*
	 * Transform the DIE to bytes stream.
	 */
	ret = _dwarf_write_uleb128_alloc(&ds->ds_data, &ds->ds_cap,
	    &ds->ds_size, die->die_ab->ab_entry, error);
	if (ret != DW_DLE_NONE)
		return (ret);

attr_gen:

	/* Transform the attributes of this DIE. */
	ret = _dwarf_attr_gen(dbg, ds, drs, cu, die, pass2, error);
	if (ret != DW_DLE_NONE)
		return (ret);

	/* Proceed to child DIE. */
	if (die->die_child != NULL) {
		ret = _dwarf_die_gen_recursive(dbg, cu, drs, die->die_child,
		    pass2, error);
		if (ret != DW_DLE_NONE)
			return (ret);
	}

	/* Proceed to sibling DIE. */
	if (die->die_right != NULL) {
		ret = _dwarf_die_gen_recursive(dbg, cu, drs, die->die_right,
		    pass2, error);
		if (ret != DW_DLE_NONE)
			return (ret);
	}

null_die:
	/* Write a null DIE indicating the end of current level. */
	ret = _dwarf_write_uleb128_alloc(&ds->ds_data, &ds->ds_cap,
	    &ds->ds_size, 0, error);
	if (ret != DW_DLE_NONE)
		return (ret);

	return (DW_DLE_NONE);
}

int
_dwarf_die_gen(Dwarf_P_Debug dbg, Dwarf_CU cu, Dwarf_Rel_Section drs,
    Dwarf_Error *error)
{
	Dwarf_Abbrev ab, tab;
	Dwarf_AttrDef ad, tad;
	Dwarf_Die die;
	int ret;

	assert(dbg != NULL && cu != NULL);
	assert(dbg->dbgp_root_die != NULL);

	die = dbg->dbgp_root_die;

	/*
	 * Insert a DW_AT_stmt_list attribute into root DIE, if there are
	 * line number information.
	 */
	if (!STAILQ_EMPTY(&dbg->dbgp_lineinfo->li_lnlist))
		RCHECK(_dwarf_add_AT_dataref(dbg, die, DW_AT_stmt_list, 0, 0,
		    ".debug_line", NULL, error));

	RCHECK(_dwarf_die_gen_recursive(dbg, cu, drs, die, 0, error));

	if (cu->cu_pass2)
		RCHECK(_dwarf_die_gen_recursive(dbg, cu, drs, die, 1, error));

	return (DW_DLE_NONE);

gen_fail:

	STAILQ_FOREACH_SAFE(ab, &cu->cu_abbrev, ab_next, tab) {
		STAILQ_FOREACH_SAFE(ad, &ab->ab_attrdef, ad_next, tad) {
			STAILQ_REMOVE(&ab->ab_attrdef, ad, _Dwarf_AttrDef,
			    ad_next);
			free(ad);
		}
		STAILQ_REMOVE(&cu->cu_abbrev, ab, _Dwarf_Abbrev, ab_next);
		free(ab);
	}

	return (ret);
}

void
_dwarf_die_pro_cleanup(Dwarf_P_Debug dbg)
{
	Dwarf_P_Die die, tdie;
	Dwarf_P_Attribute at, tat;

	assert(dbg != NULL && dbg->dbg_mode == DW_DLC_WRITE);

	STAILQ_FOREACH_SAFE(die, &dbg->dbgp_dielist, die_next, tdie) {
		STAILQ_FOREACH_SAFE(at, &die->die_attr, at_next, tat) {
			STAILQ_REMOVE(&die->die_attr, at, _Dwarf_Attribute,
			    at_next);
			free(at);
		}
	}
}
