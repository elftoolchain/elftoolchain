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
 *
 * $FreeBSD: src/lib/libdwarf/dwarf_die.c,v 1.1 2008/05/22 02:14:23 jb Exp $
 */

#include <stdlib.h>
#include "_libdwarf.h"

int
die_add(Dwarf_CU cu, int level, uint64_t offset, uint64_t abnum,
    Dwarf_Abbrev ab, Dwarf_Die *diep, Dwarf_Error *error)
{
	Dwarf_Die die;
	uint64_t key;

	if (cu == NULL || ab == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DWARF_E_ARGUMENT);
	}

	if ((die = malloc(sizeof(struct _Dwarf_Die))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	/* Initialise the abbrev structure. */
	die->die_level	= level;
	die->die_offset	= offset;
	die->die_abnum	= abnum;
	die->die_ab	= ab;
	die->die_cu	= cu;
	die->die_name	= NULL;
	die->die_attrarray = NULL;

	/* Initialise the list of attribute values. */
	STAILQ_INIT(&die->die_attr);

	/* Add the die to the list in the compilation unit. */
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
die_find(Dwarf_Die die, Dwarf_Unsigned off)
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
