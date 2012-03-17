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

static void _read_rel(struct ld *ld, struct ld_input_section *is, Elf_Data *d);
static void _read_rela(struct ld *ld, struct ld_input_section *is, Elf_Data *d);

void
ld_reloc_read(struct ld *ld)
{
	struct ld_input *li;
	struct ld_input_section *is;
	Elf *e;
	Elf_Data *d;
	Elf_Scn *scn;
	int i, elferr;

	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		e = NULL;
		for (i = 0; (uint64_t) i < li->li_shnum; i++) {
			is = &li->li_is[i];
			(void) elf_errno();
			if (is->is_type != SHT_REL && is->is_type != SHT_RELA)
				continue;
			if (e == NULL) {
				ld_input_load(ld, li);
				e = li->li_elf;
			}
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
			if (is->is_type == SHT_REL)
				_read_rel(ld, is, d);
			else
				_read_rela(ld, is, d);
		}
		if (e != NULL)
			ld_input_unload(ld, li);
	}
}

static void
_read_rel(struct ld *ld, struct ld_input_section *is, Elf_Data *d)
{
	struct ld_reloc_entry *lre;
	GElf_Rel r;
	int i, len;

	if ((is->is_reloc = malloc(sizeof(*is->is_reloc))) == NULL)
		ld_fatal(ld, "malloc");
	STAILQ_INIT(is->is_reloc);

	len = d->d_size / is->is_entsize;
	for (i = 0; i < len; i++) {
		if (gelf_getrel(d, i, &r) != &r) {
			ld_warn(ld, "gelf_getrel failed: %s", elf_errmsg(-1));
			continue;
		}
		if ((lre = calloc(1, sizeof(*lre))) == NULL)
			ld_fatal(ld, "calloc");
		lre->lre_offset = r.r_offset;
		lre->lre_sym = GELF_R_SYM(r.r_info);
		lre->lre_type = GELF_R_TYPE(r.r_info);
		STAILQ_INSERT_TAIL(is->is_reloc, lre, lre_next);
	}
}

static void
_read_rela(struct ld *ld, struct ld_input_section *is, Elf_Data *d)
{
	struct ld_reloc_entry *lre;
	GElf_Rela r;
	int i, len;

	if ((is->is_reloc = malloc(sizeof(*is->is_reloc))) == NULL)
		ld_fatal(ld, "malloc");
	STAILQ_INIT(is->is_reloc);

	len = d->d_size / is->is_entsize;
	for (i = 0; i < len; i++) {
		if (gelf_getrela(d, i, &r) != &r) {
			ld_warn(ld, "gelf_getrel failed: %s", elf_errmsg(-1));
			continue;
		}
		if ((lre = calloc(1, sizeof(*lre))) == NULL)
			ld_fatal(ld, "calloc");
		lre->lre_offset = r.r_offset;
		lre->lre_sym = GELF_R_SYM(r.r_info);
		lre->lre_type = GELF_R_TYPE(r.r_info);
		lre->lre_addend = r.r_addend;
		STAILQ_INSERT_TAIL(is->is_reloc, lre, lre_next);
	}
}

void
ld_reloc_process_input_section(struct ld *ld, struct ld_input_section *is,
    void *buf)
{
	struct ld_input *li;
	struct ld_input_section *ris, *sis;
	struct ld_reloc_entry *lre;
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Sym sym;
	char *name;
	size_t strndx;
	uint64_t vma, sym_val;
	int elferr, i;

	if (is->is_type == SHT_REL || is->is_type == SHT_RELA)
		return;

	li = is->is_input;
	ris = NULL;
	for (i = 0; (uint64_t) i < li->li_shnum; i++) {
		if (li->li_is[i].is_info == is->is_index) {
			ris = &li->li_is[i];
			break;
		}
	}

	if (ris == NULL)
		return;

	assert(ris->is_reloc != NULL);

	sis = &li->li_is[ris->is_link];
	if (sis->is_type != SHT_SYMTAB)
		return;

	strndx = sis->is_link;

	ld_input_load(ld, li);
	e = li->li_elf;

	if ((scn = elf_getscn(e, sis->is_index)) == NULL)
		ld_fatal(ld, "elf_getscn failed: %s", elf_errmsg(-1));

	(void) elf_errno();
	if ((d = elf_getdata(scn, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "elf_getdata failed: %s",
			    elf_errmsg(elferr));
		ld_input_unload(ld, li);
		return;
	}

	/*
	 * Calculate the corresponding output VMA address for
	 * the input section.
	 */
	vma = is->is_output->os_addr + is->is_reloff;

	STAILQ_FOREACH(lre, ris->is_reloc, lre_next) {
		if (gelf_getsym(d, lre->lre_sym, &sym) != &sym)
			ld_fatal(ld, "gelf_getsym failed: %s",
			    elf_errmsg(-1));

		/*
		 * Find out the value for the symbol assoicated
		 * with the relocation entry.
		 */
		if (GELF_ST_BIND(sym.st_info) == STB_GLOBAL) {
			name = elf_strptr(e, strndx, sym.st_name);
			if (ld_symbols_get_value(ld, name, &sym_val) < 0)
				sym_val = 0;
		} else if (GELF_ST_TYPE(sym.st_info) == STT_SECTION)
			sym_val = vma;
		else
			sym_val = 0;

		/* Arch-specific relocation handling. */
		ld->ld_arch->process_reloc(ld, is, lre, sym_val, buf);
	}

	ld_input_unload(ld, li);
}
