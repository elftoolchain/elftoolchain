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

		if (li->li_name == NULL)
			continue;

		if (li->li_type == LIT_DSO)
			continue;

		ld_input_load(ld, li);
		e = li->li_elf;

		for (i = 0; (uint64_t) i < li->li_shnum - 1; i++) {
			is = &li->li_is[i];

			if (is->is_type != SHT_REL && is->is_type != SHT_RELA)
				continue;

			if ((scn = elf_getscn(e, is->is_index)) == NULL) {
				ld_warn(ld, "%s(%s): elf_getscn failed: %s",
				    li->li_name, is->is_name, elf_errmsg(-1));
				continue;
			}

			(void) elf_errno();
			if ((d = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0)
					ld_warn(ld, "%s(%s): elf_getdata "
					    "failed: %s", li->li_name,
					    is->is_name, elf_errmsg(elferr));
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
		is->is_num_reloc++;
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
		is->is_num_reloc++;
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

	ld->ld_arch->scan_reloc(ld, is, lre);
}

void *
ld_reloc_serialize(struct ld *ld, struct ld_output_section *os, size_t *sz)
{
	struct ld_reloc_entry *lre;
	struct ld_symbol *lsb;
	Elf32_Rel *r32;
	Elf64_Rel *r64;
	Elf32_Rela *ra32;
	Elf64_Rela *ra64;
	uint8_t *p;
	void *b;
	size_t entsize;
	uint64_t sym;
	unsigned char is_64;
	unsigned char is_rela;

	is_64 = ld->ld_arch->reloc_is_64bit;
	is_rela = ld->ld_arch->reloc_is_rela;
	entsize = ld->ld_arch->reloc_entsize;

	b = malloc(ld->ld_arch->reloc_entsize * os->os_num_reloc);
	if (b == NULL)
		ld_fatal_std(ld, "malloc");

	p = b;
	STAILQ_FOREACH(lre, os->os_reloc, lre_next) {
		lsb = ld_symbols_ref(lre->lre_sym);
		if (os->os_dynrel)
			sym = lsb->lsb_dyn_index;
		else
			sym = lsb->lsb_index;
		
		if (is_64 && is_rela) {
			ra64 = (Elf64_Rela *) (uintptr_t) p;
			ra64->r_offset = lre->lre_offset;
			ra64->r_info = ELF64_R_INFO(sym, lre->lre_type);
			ra64->r_addend = lre->lre_addend;
		} else if (!is_64 && !is_rela) {
			r32 = (Elf32_Rel *) (uintptr_t) p;
			r32->r_offset = (uint32_t) lre->lre_offset;
			r32->r_info = (uint32_t) ELF32_R_INFO(sym,
			    lre->lre_type);
		} else if (!is_64 && is_rela) {
			ra32 = (Elf32_Rela *) (uintptr_t) p;
			ra32->r_offset = (uint32_t) lre->lre_offset;
			ra32->r_info = (uint32_t) ELF32_R_INFO(sym,
			    lre->lre_type);
			ra32->r_addend = (int32_t) lre->lre_addend;
		} else if (is_64 && !is_rela) {
			r64 = (Elf64_Rel *) (uintptr_t) p;
			r64->r_offset = lre->lre_offset;
			r64->r_info = ELF64_R_INFO(sym, lre->lre_type);
		}

		p += entsize;
	}

	*sz = entsize * os->os_num_reloc;
	assert((size_t) (p - (uint8_t *) b) == *sz);

	return (b);
}

void
ld_reloc_create_entry(struct ld *ld, const char *name, uint64_t type,
    struct ld_symbol *lsb, uint64_t offset, int64_t addend)
{
	struct ld_input_section *is;
	struct ld_reloc_entry *lre;
	int len;

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
		is->is_type = ld->ld_arch->reloc_is_rela ? SHT_RELA : SHT_REL;
		is->is_align = ld->ld_arch->reloc_is_64bit ? 8 : 4;
		is->is_entsize = ld->ld_arch->reloc_entsize;

		len = strlen(name);
		if (len > 3 && name[len - 1] == 't' && name[len - 2] == 'l' &&
		    name[len - 3] == 'p')
			is->is_pltrel = 1;
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
	is->is_num_reloc++;
	is->is_size += ld->ld_arch->reloc_entsize;
}

void
ld_reloc_finalize_sections(struct ld *ld)
{
	struct ld_input *li;
	struct ld_input_section *is;
	struct ld_output_section *os;
	struct ld_reloc_entry *lre;
	int i;

	li = STAILQ_FIRST(&ld->ld_lilist);
	assert(li != NULL);

	for (i = 0; (size_t) i < li->li_shnum; i++) {
		is = &li->li_is[i];

		if (!is->is_dynrel || is->is_reloc == NULL ||
		    is->is_output == NULL)
			continue;

		/* PLT relocation is handled in arch-specified code. */
		if (is->is_pltrel)
			continue;

		STAILQ_FOREACH(lre, is->is_reloc, lre_next) {
			if ((os = is->is_output) == NULL)
				continue;
			lre->lre_offset += os->os_addr + is->is_reloff;
		}
	}

	if (!ld->ld_emit_reloc)
		return;

	while ((li = STAILQ_NEXT(li, li_next)) != NULL) {
		is = &li->li_is[i];

		if (is->is_reloc == NULL || is->is_output == NULL)
			continue;

		STAILQ_FOREACH(lre, is->is_reloc, lre_next) {
			if ((os = is->is_output) == NULL)
				continue;
			lre->lre_offset += os->os_addr + is->is_reloff;
		}
	}
}

void
ld_reloc_join(struct ld *ld, struct ld_output_section *os,
    struct ld_input_section *is)
{

	assert(is->is_reloc != NULL);

	if (os->os_reloc == NULL) {
		if ((os->os_reloc = malloc(sizeof(*os->os_reloc))) == NULL)
			ld_fatal_std(ld, "malloc");
		STAILQ_INIT(os->os_reloc);
	}

	STAILQ_CONCAT(os->os_reloc, is->is_reloc);
	os->os_num_reloc += is->is_num_reloc;

	is->is_num_reloc = 0;
	free(is->is_reloc);
}

void
ld_reloc_sort(struct ld *ld, struct ld_output_section *os)
{

	/* TODO: Implement reloc sorting*/

	(void) ld;
	(void) os;
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
