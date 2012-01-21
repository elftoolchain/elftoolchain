/*-
 * Copyright (c) 2012 Kai Wang
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

#include "ld.h"
#include "ld_file.h"
#include "ld_input.h"
#include "ld_reloc.h"

ELFTC_VCSID("$Id$");

/*
 * Support routines for relocation handling.
 */

static void _read_rel(struct ld *ld, struct ld_input_section *is);
static void _read_rela(struct ld *ld, struct ld_input_section *is);

void
ld_reloc_read(struct ld *ld)
{
	struct ld_file *lf;
	struct ld_input *li;
	struct ld_input_section *is;
	Elf *e;
	Elf_Data *d;
	Elf_Scn *scn;
	int i, elferr;

	lf = NULL;
	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		e = NULL;
		for (i = 0; (uint64_t) i < li->li_shnum; i++) {
			is = &li->li_is[i];
			(void) elf_errno();
			if (is->is_type != SHT_REL && is->is_type != SHT_RELA)
				continue;
			if (lf == NULL || li->li_file != lf) {
				if (lf != NULL)
					ld_file_unload(ld, lf);
				ld_file_load(ld, li->li_file);
				lf = li->li_file;
			}
			if (e == NULL)
				e = ld_input_get_elf(ld, li);
			if ((scn = elf_getscn(e, is->is_index)) == NULL) {
				ld_warn(ld, "elf_getscn failed: %s",
				    elf_errmsg(-1));
				continue;
			}
			if ((d = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0)
					ld_warn(ld, "elf_getdata failed: %s",
					    elf_errmsg(elferr));
				continue;
			}
			if (is->is_type == SHT_REL)
				_read_rel(ld, is);
			else
				_read_rela(ld, is);
		}
		if (e != NULL)
			ld_input_end_elf(ld, li, e);
	}
}

static void
_read_rel(struct ld *ld, struct ld_input_section *is)
{

	(void) ld;
	(void) is;
}

static void
_read_rela(struct ld *ld, struct ld_input_section *is)
{

	(void) ld;
	(void) is;
}
