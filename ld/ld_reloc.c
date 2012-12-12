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

static void _scan_reloc(struct ld *ld, struct ld_input_section *is,
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

		if (li->li_type == LIT_DSO)
			continue;

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
		_scan_reloc(ld, is, GELF_R_SYM(r.r_info), lre);
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
		_scan_reloc(ld, is, GELF_R_SYM(r.r_info), lre);
		STAILQ_INSERT_TAIL(is->is_reloc, lre, lre_next);
	}
}

static void
_scan_reloc(struct ld *ld, struct ld_input_section *is, uint64_t sym,
    struct ld_reloc_entry *lre)
{
	struct ld_input *li;

	(void) ld;

	li = is->is_input;

	lre->lre_sym = li->li_symindex[sym];

	ld->ld_arch->scan_reloc(ld, lre);
}

void
ld_reloc_create_entry(struct ld *ld, const char *name, uint64_t type,
    struct ld_symbol *lsb, uint64_t offset, int64_t addend)
{
	struct ld_input_section *is;
	struct ld_reloc_entry *lre;

	/*
	 * List of internal sections to hold dynamic relocations:
	 *
	 * .rel.bss      contains copy relocations
	 * .rel.plt      contains PLT (*_JMP_SLOT) relocations
	 * .rel.got      contains GOT (*_GLOB_DATA) relocations
	 * .rel.data.*   contains *_RELATIVE and absolute relocations
	 */

	is = ld_input_find_internal_section(ld, name);
	if (is == NULL) {
		is = ld_input_add_internal_section(ld, name);
		is->is_dynrel = 1;
	}

	if (is->is_reloc == NULL) {
		is->is_reloc = calloc(1, sizeof(*is->is_reloc));
		if (is->is_reloc == NULL)
			ld_fatal_std(ld, "calloc");
		STAILQ_INIT(is->is_reloc);
	}

	if ((lre = malloc(sizeof(*lre))) == NULL)
		ld_fatal_std(ld, "calloc");

	lre->lre_type = type;
	lre->lre_sym = lsb;
	lre->lre_offset = offset;
	lre->lre_addend = addend;

	STAILQ_INSERT_TAIL(is->is_reloc, lre, lre_next);
}

int
ld_reloc_require_plt(struct ld *ld, struct ld_reloc_entry *lre)
{
	struct ld_symbol *lsb;

	lsb = ld_symbols_ref(lre->lre_sym);

	/* Only need PLT for functions. */
	if (lsb->lsb_type != STT_FUNC)
		return (0);

	/* Create PLT for functions in DSOs. */
	if (lsb->lsb_input != NULL && lsb->lsb_input->li_type == LIT_DSO)
		return (1);

	/*
	 * If the linker outputs a DSO, PLT entry is needed if the symbol
	 * if undefined or it can be overridden.
	 */
	if (ld->ld_dso &&
	    (lsb->lsb_shndx == SHN_UNDEF || ld_symbols_overridden(ld, lsb)))
		return (1);

	/* Otherwise, we do not create PLT entry. */
	return (0);
}

int
ld_reloc_require_copy_reloc(struct ld *ld, struct ld_reloc_entry *lre)
{
	struct ld_symbol *lsb;

	lsb = ld_symbols_ref(lre->lre_sym);

	/* Functions do not need copy reloc. */
	if (lsb->lsb_type == STT_FUNC)
		return (0);

	/*
	 * If we are generating a normal executable and the symbol is
	 * defined in a DSO, we need a copy reloc.
	 */
	if (ld->ld_exec && lsb->lsb_input != NULL &&
	    lsb->lsb_input->li_type == LIT_DSO)
		return (1);

	return (0);
}

int
ld_reloc_require_dynamic_reloc(struct ld *ld, struct ld_reloc_entry *lre)
{
	struct ld_symbol *lsb;

	lsb = ld_symbols_ref(lre->lre_sym);

	/*
	 * If the symbol is defined in a DSO, we create specific dynamic
	 * relocations when we create PLT, GOT or copy reloc.
	 */
	if (lsb->lsb_input != NULL && lsb->lsb_input->li_type == LIT_DSO)
		return (0);

	/*
	 * When we are creating a DSO, we create dynamic relocation if
	 * the symbol is undefined, or if the symbol can be overridden.
	 */
	if (ld->ld_dso && (lsb->lsb_shndx == SHN_UNDEF ||
	    ld_symbols_overridden(ld, lsb)))
		return (1);

	/*
	 * When we are creating a PIE/DSO (position-independent), if the
	 * relocation is referencing the absolute address of a symbol,
	 * we should create dynamic relocation.
	 */
	if ((ld->ld_pie || ld->ld_dso) &&
	    ld->ld_arch->is_absolute_reloc(lre->lre_type))
		return (1);

	/* Otherwise we do not generate dynamic relocation. */
	return (0);
}

int
ld_reloc_relative_relax(struct ld *ld, struct ld_reloc_entry *lre)
{

	struct ld_symbol *lsb;

	lsb = ld_symbols_ref(lre->lre_sym);

	/*
	 * We only use *_RELATIVE relocation when we create PIE/DSO.
	 */
	if (!ld->ld_pie && !ld->ld_dso)
		return (0);

	/*
	 * If the symbol is defined in a DSO, we can not relax the
	 * relocation.
	 */
	if (lsb->lsb_input != NULL && lsb->lsb_input->li_type == LIT_DSO)
		return (0);

	/*
	 * When we are creating a DSO, we can not relax dynamic relocation
	 * to *_RELATIVE relocation if the symbol is undefined, or if the
	 * symbol can be overridden.
	 */
	if (ld->ld_dso && (lsb->lsb_shndx == SHN_UNDEF ||
	    ld_symbols_overridden(ld, lsb)))
		return (0);

	/* Otherwise it's ok to use *_RELATIVE. */
	return (1);
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
		lsb = ld_symbols_ref(lre->lre_sym);

		/* Arch-specific relocation handling. */
		ld->ld_arch->process_reloc(ld, is, lre, lsb, buf);
	}
}
