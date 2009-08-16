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
#include "_libdwarf.h"

void
arange_cleanup(Dwarf_Debug dbg)
{
	Dwarf_ArangeSet as, tas;
	Dwarf_Arange ar, tar;

	STAILQ_FOREACH_SAFE(as, &dbg->dbg_aslist, as_next, tas) {
		STAILQ_FOREACH_SAFE(ar, &as->as_arlist, ar_next, tar) {
			STAILQ_REMOVE(&as->as_arlist, ar, _Dwarf_Arange,
			    ar_next);
			free(ar);
		}
		STAILQ_REMOVE(&dbg->dbg_aslist, as, _Dwarf_ArangeSet, as_next);
		free(as);
	}

	if (dbg->dbg_arange_array)
		free(dbg->dbg_arange_array);

	dbg->dbg_arange_array = NULL;
	dbg->dbg_arange_cnt = 0;
}

int
arange_init(Dwarf_Debug dbg, Elf_Data *d, Dwarf_Error *error)
{
	Dwarf_CU cu;
	Dwarf_ArangeSet as;
	Dwarf_Arange ar;
	uint64_t offset, dwarf_size, length, addr, range;
	int i, ret;

	ret = DWARF_E_NONE;

	offset = 0;
	while (offset < d->d_size) {

		if ((as = malloc(sizeof(struct _Dwarf_ArangeSet))) == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			return (DWARF_E_MEMORY);
		}
		STAILQ_INIT(&as->as_arlist);
		STAILQ_INSERT_TAIL(&dbg->dbg_aslist, as, as_next);

		/* Read in the table header. */
		length = dbg->read(&d, &offset, 4);
		if (length == 0xffffffff) {
			dwarf_size = 8;
			length = dbg->read(&d, &offset, 8);
		} else
			dwarf_size = 4;

		as->as_length = length;
		as->as_version = dbg->read(&d, &offset, 2);
		if (as->as_version != 2) {
			DWARF_SET_ERROR(error, DWARF_E_INVALID_ARANGE);
			ret = DWARF_E_INVALID_ARANGE;
			goto fail_cleanup;
		}

		as->as_cu_offset = dbg->read(&d, &offset, dwarf_size);
		STAILQ_FOREACH(cu, &dbg->dbg_cu, cu_next) {
			if (cu->cu_offset == as->as_cu_offset)
				break;
		}
		if (cu == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_INVALID_ARANGE);
			ret = DWARF_E_INVALID_ARANGE;
			goto fail_cleanup;
		}
		as->as_cu = cu;

		as->as_addrsz = dbg->read(&d, &offset, 1);
		as->as_segsz = dbg->read(&d, &offset, 1);

		/* Skip the padding bytes.  */
		offset = roundup(offset, 2 * as->as_addrsz);

		/* Read in address range descriptors. */
		while (offset < d->d_size) {
			addr = dbg->read(&d, &offset, as->as_addrsz);
			range = dbg->read(&d, &offset, as->as_addrsz);
			if (addr == 0 && range == 0)
				break;
			if ((ar = malloc(sizeof(struct _Dwarf_Arange))) ==
			    NULL) {
				DWARF_SET_ERROR(error, DWARF_E_MEMORY);
				goto fail_cleanup;
			}
			ar->ar_as = as;
			ar->ar_address = addr;
			ar->ar_range = range;
			STAILQ_INSERT_TAIL(&as->as_arlist, ar, ar_next);
			dbg->dbg_arange_cnt++;
		}
	}

	/* Build arange array. */
	if (dbg->dbg_arange_cnt > 0) {
		if  ((dbg->dbg_arange_array = malloc(dbg->dbg_arange_cnt *
		    sizeof(struct _Dwarf_Arange))) == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			ret = DWARF_E_MEMORY;
			goto fail_cleanup;
		}

		i = 0;
		STAILQ_FOREACH(as, &dbg->dbg_aslist, as_next) {
			STAILQ_FOREACH(ar, &as->as_arlist, ar_next)
				dbg->dbg_arange_array[i++] = ar;
		}
		assert((Dwarf_Unsigned)i == dbg->dbg_arange_cnt);
	}

	return (DWARF_E_NONE);

fail_cleanup:

	arange_cleanup(dbg);

	return (ret);
}
