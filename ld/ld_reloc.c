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
#include "ld_arch.h"
#include "ld_input.h"
#include "ld_output.h"
#include "ld_reloc.h"
#include "ld_symbols.h"

ELFTC_VCSID("$Id$");

/*
 * Support routines for relocation handling.
 */

static void _process_reloc(struct ld *ld, struct ld_input_section *is,
    uint64_t sym, struct ld_reloc_entry *lre);
static void _read_rel(struct ld *ld, struct ld_input_section *is,
    Elf_Data *d);
static void _read_rela(struct ld *ld, struct ld_input_section *is,
    Elf_Data *d);

void
ld_reloc_load(struct ld *ld)
{
	struct ld_input *li;
	struct ld_input_section *is;
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	int elferr, i;

	ld_input_link_objects(ld);

	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {

		ld_input_load(ld, li);
		e = li->li_elf;

		for (i = 0; (uint64_t) i < li->li_shnum - 1; i++) {
			is = &li->li_is[i];

			if (is->is_type != SHT_REL && is->is_type != SHT_RELA)
				continue;

			if ((scn = elf_getscn(e, is->is_index)) == NULL) {
				ld_warn(ld, "elf_getscn failed: %s",
				    elf_errmsg(-1));
				continue;
			}

			(void) elf_errno();
			if ((d = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0)
					ld_warn(ld, "elf_getdata failed: %s",
					    elf_errmsg(elferr));
				continue;
			}

			/*
			 * Load and process relocation entries.
			 */

			if ((is->is_reloc = malloc(sizeof(*is->is_reloc))) ==
			    NULL)
				ld_fatal(ld, "malloc");
			STAILQ_INIT(is->is_reloc);

			if (is->is_type == SHT_REL)
				_read_rel(ld, is, d);
			else
				_read_rela(ld, is, d);
		}

		ld_input_unload(ld, li);
	}
}

static void
_read_rel(struct ld *ld, struct ld_input_section *is, Elf_Data *d)
{
	struct ld_reloc_entry *lre;
	GElf_Rel r;
	int i, len;

	assert(is->is_reloc != NULL);

	len = d->d_size / is->is_entsize;
	for (i = 0; i < len; i++) {
		if (gelf_getrel(d, i, &r) != &r) {
			ld_warn(ld, "gelf_getrel failed: %s", elf_errmsg(-1));
			continue;
		}
		if ((lre = calloc(1, sizeof(*lre))) == NULL)
			ld_fatal(ld, "calloc");
		lre->lre_offset = r.r_offset;
		lre->lre_type = GELF_R_TYPE(r.r_info);
		_process_reloc(ld, is, GELF_R_SYM(r.r_info), lre);
		STAILQ_INSERT_TAIL(is->is_reloc, lre, lre_next);
	}
}

static void
_read_rela(struct ld *ld, struct ld_input_section *is, Elf_Data *d)
{
	struct ld_reloc_entry *lre;
	GElf_Rela r;
	int i, len;

	assert(is->is_reloc != NULL);

	len = d->d_size / is->is_entsize;
	for (i = 0; i < len; i++) {
		if (gelf_getrela(d, i, &r) != &r) {
			ld_warn(ld, "gelf_getrel failed: %s", elf_errmsg(-1));
			continue;
		}
		if ((lre = calloc(1, sizeof(*lre))) == NULL)
			ld_fatal(ld, "calloc");
		lre->lre_offset = r.r_offset;
		lre->lre_type = GELF_R_TYPE(r.r_info);
		lre->lre_addend = r.r_addend;
		_process_reloc(ld, is, GELF_R_SYM(r.r_info), lre);
		STAILQ_INSERT_TAIL(is->is_reloc, lre, lre_next);
	}
}

static void
_process_reloc(struct ld *ld, struct ld_input_section *is, uint64_t sym,
    struct ld_reloc_entry *lre)
{
	struct ld_input *li;

	(void) ld;

	li = is->is_input;

	lre->lre_sym = li->li_symindex[sym];
}

void
ld_reloc_process_input_section(struct ld *ld, struct ld_input_section *is,
    void *buf)
{
	struct ld_input *li;
	struct ld_input_section *ris;
	struct ld_reloc_entry *lre;
	struct ld_symbol *lsb;
	int i;

	if (is->is_type == SHT_REL || is->is_type == SHT_RELA)
		return;

	li = is->is_input;
	ris = NULL;
	for (i = 0; (uint64_t) i < li->li_shnum; i++) {
		if (li->li_is[i].is_type != SHT_REL &&
		    li->li_is[i].is_type != SHT_RELA)
			continue;
		if (li->li_is[i].is_info == is->is_index) {
			ris = &li->li_is[i];
			break;
		}
	}

	if (ris == NULL)
		return;

	assert(ris->is_reloc != NULL);

	STAILQ_FOREACH(lre, ris->is_reloc, lre_next) {
		lsb = lre->lre_sym;
		while (lsb->lsb_ref != NULL)
			lsb = lsb->lsb_ref;

		/* Arch-specific relocation handling. */
		ld->ld_arch->process_reloc(ld, is, lre, lsb, buf);
	}
}
