/*-
 * Copyright (c) 2009 Kai Wang
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
 * $FreeBSD: src/lib/libdwarf/dwarf_die.c,v 1.1 2008/05/22 02:14:23 jb Exp $
 */

#include <assert.h>
#include <stdlib.h>
#include "_libdwarf.h"

int
_dwarf_die_alloc(Dwarf_Die *ret_die, Dwarf_Error *error)
{
	Dwarf_Die die;

	assert(ret_die != NULL);

	if ((die = calloc(1, sizeof(struct _Dwarf_Die))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	STAILQ_INIT(&die->die_attr);

	*ret_die = die;

	return (DWARF_E_NONE);
}

int
_dwarf_die_add(Dwarf_CU cu, uint64_t offset, uint64_t abnum, Dwarf_Abbrev ab,
    Dwarf_Die *diep, Dwarf_Error *error)
{
	Dwarf_Die die;
	uint64_t key;
	int ret;

	if (cu == NULL || ab == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DWARF_E_ARGUMENT);
	}

	if ((ret = _dwarf_die_alloc(&die, error)) != DWARF_E_NONE)
		return (ret);

	die->die_offset	= offset;
	die->die_abnum	= abnum;
	die->die_ab	= ab;
	die->die_cu	= cu;

	STAILQ_INSERT_TAIL(&cu->cu_die, die, die_next);

	/* Add the die to the hash table in the compilation unit. */
	key = offset % DWARF_DIE_HASH_SIZE;
	STAILQ_INSERT_TAIL(&cu->cu_die_hash[key], die, die_hash);

	if (diep != NULL)
		*diep = die;

	return (DWARF_E_NONE);
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
			DWARF_SET_ERROR(error, DWARF_E_MISSING_ABBREV);
			return (DWARF_E_MISSING_ABBREV);
		}

		if ((ret = _dwarf_die_add(cu, die_offset, abnum, ab, &die,
		    error)) != DWARF_E_NONE)
			return (ret);

		STAILQ_FOREACH(ad, &ab->ab_attrdef, ad_next) {
			if ((ret = _dwarf_attr_init(dbg, ds, &offset,
			    dwarf_size, cu, die, ad, ad->ad_form, 0,
			    error)) != DWARF_E_NONE)
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

	return (DWARF_E_NONE);
}

void
_dwarf_die_link(Dwarf_P_Die die, Dwarf_P_Die parent, Dwarf_P_Die child,
    Dwarf_P_Die left_sibling, Dwarf_P_Die right_sibling)
{

	assert(die != NULL);

	if (parent) {

		/*
		 * Disconnect from old parent.
		 */
		if (die->die_parent) {
			if (die->die_parent == parent)
				return;
			die->die_parent->die_child = NULL;
			die->die_parent = NULL;
		}

		/*
		 * Connect to new parent.
		 */
		die->die_parent = parent;
		parent->die_child = die;

		return;
	}

	if (child) {

		/*
		 * Disconnect from old child.
		 */
		if (die->die_child) {
			if (die->die_child == child)
				return;
			die->die_child->die_parent = NULL;
			die->die_child = NULL;
		}

		/*
		 * Connect to new child.
		 */
		die->die_child = child;
		child->die_parent = die;

		return;
	}

	if (left_sibling) {

		/*
		 * Disconnect from old left sibling.
		 */
		if (die->die_left) {
			if (die->die_left == left_sibling)
				return;
			die->die_left->die_right = NULL;
			die->die_left = NULL;
		}

		/*
		 * Connect to new right sibling.
		 */
		die->die_left = left_sibling;
		left_sibling->die_right = die;

		return;
	}

	if (right_sibling) {

		/*
		 * Disconnect from old right sibling.
		 */
		if (die->die_right) {
			if (die->die_right == right_sibling)
				return;
			die->die_right->die_left = NULL;
			die->die_right = NULL;
		}

		/*
		 * Connect to new right sibling.
		 */
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
