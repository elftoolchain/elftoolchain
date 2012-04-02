/*-
 * Copyright (c) 2010-2012 Kai Wang
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
#include "ld_output.h"
#include "ld_symbols.h"
#include "ld_script.h"
#include "ld_strtab.h"

ELFTC_VCSID("$Id$");

#define	_INIT_SYMTAB_SIZE	128

static void ld_symbols_load(struct ld *ld, struct ld_file *lf);
static void ld_symbols_load_archive(struct ld *ld, struct ld_file *lf);
static void ld_symbols_load_elf(struct ld *ld, struct ld_input *li, Elf *e);
static void _add_elf_symbol(struct ld *ld, struct ld_input *li, Elf *e,
    GElf_Sym *sym, size_t strndx);
void _add_to_symbol_table(struct ld *ld, struct ld_symbol_table *symtab,
    struct ld_strtab *strtab, struct ld_symbol *lsb);
struct ld_symbol_table *_alloc_symbol_table(struct ld *ld);
static int _archive_member_extracted(struct ld_archive *la, off_t off);
static void _extract_archive_member(struct ld *ld, struct ld_file *lf,
    struct ld_archive *la, off_t off);
static void _resolve_and_add_symbol(struct ld *ld, struct ld_symbol *lsb);
static struct ld_symbol *_symbol_alloc(struct ld *ld);
static struct ld_symbol *_symbol_find(struct ld_symbol *tbl, char *name);

void
ld_symbols_resolve(struct ld *ld)
{
	struct ld_state *ls;
	struct ld_file *lf;
	struct ld_symbol *lsb, *_lsb;

	if (TAILQ_EMPTY(&ld->ld_lflist))
		ld_fatal(ld, "no input files");

	ls = &ld->ld_state;
	lf = TAILQ_FIRST(&ld->ld_lflist);
	ls->ls_group_level = lf->lf_group_level;

	while (lf != NULL) {
		/* Process archive groups. */
		if (lf->lf_group_level < ls->ls_group_level &&
		    ls->ls_extracted[ls->ls_group_level]) {
			do {
				lf = TAILQ_PREV(lf, ld_file_head, lf_next);
			} while (lf->lf_group_level >= ls->ls_group_level);
			lf = TAILQ_NEXT(lf, lf_next);
			ls->ls_extracted[ls->ls_group_level] = 0;
		}
		ls->ls_group_level = lf->lf_group_level;

		/* Load symbols. */
		ld_file_load(ld, lf);
		ld_symbols_load(ld, lf);
		ld_file_unload(ld, lf);
		lf = TAILQ_NEXT(lf, lf_next);
	}

	if (HASH_COUNT(ld->ld_symtab_undef) > 0) {
		HASH_ITER(hh, ld->ld_symtab_undef, lsb, _lsb) {
			if (lsb->lsb_bind != STB_WEAK)
				ld_warn(ld, "undefined symbol: %s", lsb->lsb_name);
		}
	}
}

static struct ld_symbol *
_symbol_alloc(struct ld *ld)
{
	struct ld_symbol *s;

	if ((s = calloc(1, sizeof(*s))) == NULL)
		ld_fatal_std(ld, "calloc");

	return (s);
}

static struct ld_symbol *
_symbol_find(struct ld_symbol *tbl, char *name)
{
	struct ld_symbol *s;

	HASH_FIND_STR(tbl, name, s);
	return (s);
}

#define	_symbol_add(tbl, s) \
	HASH_ADD_KEYPTR(hh, (tbl), (s)->lsb_name, strlen((s)->lsb_name), (s))
#define _symbol_remove(tbl, s) HASH_DEL((tbl), (s));

static void
_resolve_and_add_symbol(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;
	char *name;

	name = lsb->lsb_name;
	if (lsb->lsb_shndx == SHN_UNDEF) {
		if (_symbol_find(ld->ld_symtab_def, name) != NULL)
			return;
		if (_symbol_find(ld->ld_symtab_undef, name) != NULL)
			return;
		_symbol_add(ld->ld_symtab_undef, lsb);
	} else {
		if ((_lsb = _symbol_find(ld->ld_symtab_def, name)) != NULL) {
			if (!lsb->lsb_provide)
				ld_fatal(ld, "multiple definition of symbol "
				    "%s", name);
			else if (_lsb->lsb_provide) {
				_symbol_remove(ld->ld_symtab_def, _lsb);
			}
		}
		if ((_lsb = _symbol_find(ld->ld_symtab_undef, name)) != NULL) {
			_symbol_remove(ld->ld_symtab_undef, _lsb);
		}
		_symbol_add(ld->ld_symtab_def, lsb);
	}
}

static void
_add_elf_symbol(struct ld *ld, struct ld_input *li, Elf *e, GElf_Sym *sym,
    size_t strndx)
{
	struct ld_symbol *lsb;
	char *name;

	if ((name = elf_strptr(e, strndx, sym->st_name)) == NULL)
		return;

	if (GELF_ST_BIND(sym->st_info) == STB_LOCAL)
		return;

	lsb = _symbol_alloc(ld);

	if ((lsb->lsb_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");
	lsb->lsb_value = sym->st_value;
	lsb->lsb_size = sym->st_size;
	lsb->lsb_bind = GELF_ST_BIND(sym->st_info);
	lsb->lsb_type = GELF_ST_TYPE(sym->st_info);
	lsb->lsb_shndx = sym->st_shndx;
	lsb->lsb_input = li;

	/* Insert symbol to input object internal symbol list. */
	if (lsb->lsb_bind == STB_LOCAL) {
		if (lsb->lsb_type != STT_SECTION)
			STAILQ_INSERT_TAIL(li->li_local, lsb, lsb_next);
	} else
		STAILQ_INSERT_TAIL(li->li_nonlocal, lsb, lsb_next);

	_resolve_and_add_symbol(ld, lsb);
}

static int
_archive_member_extracted(struct ld_archive *la, off_t off)
{
	struct ld_archive_member *_lam;

	HASH_FIND(hh, la->la_m, &off, sizeof(off), _lam);
	if (_lam != NULL)
		return (1);

	return (0);
}

static void
_extract_archive_member(struct ld *ld, struct ld_file *lf,
    struct ld_archive *la, off_t off)
{
	Elf *e;
	Elf_Arhdr *arhdr;
	struct ld_archive_member *lam;
	struct ld_input *li;

	if (elf_rand(lf->lf_elf, off) == 0)
		ld_fatal(ld, "%s: elf_rand failed: %s", lf->lf_name,
		    elf_errmsg(-1));

	if ((e = elf_begin(-1, ELF_C_READ, lf->lf_elf)) == NULL)
		ld_fatal(ld, "%s: elf_begin failed: %s", lf->lf_name,
		    elf_errmsg(-1));

	if ((arhdr = elf_getarhdr(e)) == NULL)
		ld_fatal(ld, "%s: elf_getarhdr failed: %s", lf->lf_name,
		    elf_errmsg(-1));

	/* Keep record of extracted members. */
	if ((lam = calloc(1, sizeof(*lam))) == NULL)
		ld_fatal_std(ld, "calloc");
	lam->lam_name = strdup(arhdr->ar_name);
	if (lam->lam_name == NULL)
		ld_fatal_std(ld, "strdup");
	lam->lam_off = off;
	HASH_ADD(hh, la->la_m, lam_off, sizeof(lam->lam_off), lam);

	/* Allocate input object for this member. */
	li = ld_input_alloc(ld, lf, lam->lam_name);
	li->li_moff = lam->lam_off;
	lam->lam_input = li;

	/* Load the symbols of this member. */
	ld_symbols_load_elf(ld, li, e);

	elf_end(e);
}

static void
ld_symbols_load_archive(struct ld *ld, struct ld_file *lf)
{
	struct ld_state *ls;
	struct ld_archive *la;
	Elf_Arsym *as;
	size_t c;
	int extracted, i;

	assert(lf != NULL && lf->lf_type == LFT_ARCHIVE);
	assert(lf->lf_ar != NULL);

	ls = &ld->ld_state;
	la = lf->lf_ar;
	if ((as = elf_getarsym(lf->lf_elf, &c)) == NULL)
		ld_fatal(ld, "%s: elf_getarsym failed: %s", lf->lf_name,
		    elf_errmsg(-1));
	do {
		extracted = 0;
		for (i = 0; (size_t) i < c; i++) {
			if (as[i].as_name == NULL)
				break;
			if (_archive_member_extracted(la, as[i].as_off))
				continue;
			if (_symbol_find(ld->ld_symtab_undef, as[i].as_name) !=
			    NULL) {
				_extract_archive_member(ld, lf, la,
				    as[i].as_off);
				extracted = 1;
				ls->ls_extracted[ls->ls_group_level] = 1;
			}
		}
	} while (extracted);
}

static void
ld_symbols_load_elf(struct ld *ld, struct ld_input *li, Elf *e)
{
	Elf_Scn *scn, *scn_sym;
	Elf_Data *d;
	GElf_Shdr shdr;
	GElf_Sym sym;
	size_t strndx;
	int elferr, len, i;

	strndx = SHN_UNDEF;
	scn = scn_sym = NULL;
	(void) elf_errno();
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &shdr) != &shdr)
			ld_fatal(ld, "%s: gelf_getshdr failed: %s", li->li_name,
			    elf_errmsg(-1));
		if (shdr.sh_type == SHT_SYMTAB)
			scn_sym = scn;
		else if (shdr.sh_type == SHT_STRTAB)
			strndx = elf_ndxscn(scn);
	}
	elferr = elf_errno();
	if (elferr != 0)
		ld_fatal(ld, "%s: elf_nextscn failed: %s", li->li_name,
		    elf_errmsg(elferr));
	if (scn_sym == NULL || strndx == SHN_UNDEF)
		return;

	if (gelf_getshdr(scn_sym, &shdr) != &shdr)
		ld_fatal(ld, "%s: gelf_getshdr failed: %s", li->li_name,
		    elf_errmsg(-1));

	(void) elf_errno();
	if ((d = elf_getdata(scn_sym, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "%s: elf_getdata failed: %s", li->li_name,
			    elf_errmsg(elferr));
		/* Empty .symtab section? */
		return;
	}

	len = d->d_size / shdr.sh_entsize;
	for (i = 0; i < len; i++) {
		if (gelf_getsym(d, i, &sym) != &sym)
			ld_fatal(ld, "%s: gelf_getsym failed: %s", li->li_name,
			    elf_errmsg(-1));
		_add_elf_symbol(ld, li, e, &sym, strndx);
	}
}

static void
ld_symbols_load(struct ld *ld, struct ld_file *lf)
{

	if (lf->lf_type == LFT_ARCHIVE)
		ld_symbols_load_archive(ld, lf);
	else {
		lf->lf_input = ld_input_alloc(ld, lf, lf->lf_name);
		ld_symbols_load_elf(ld, lf->lf_input, lf->lf_elf);
	}
}

void
ld_symbols_add_extern(struct ld *ld, char *name)
{
	struct ld_symbol *lsb;

	if (_symbol_find(ld->ld_symtab_undef, name) != NULL)
		return;

	lsb = _symbol_alloc(ld);
	if ((lsb->lsb_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");

	_symbol_add(ld->ld_symtab_undef, lsb);
}

void
ld_symbols_add_variable(struct ld *ld, struct ld_script_variable *ldv,
    unsigned provide, unsigned hidden)
{
	struct ld_symbol *lsb;

	lsb = _symbol_alloc(ld);
	if ((lsb->lsb_name = strdup(ldv->ldv_name)) == NULL)
		ld_fatal_std(ld, "strdup");
	lsb->lsb_var = ldv;
	lsb->lsb_bind = STB_GLOBAL;
	lsb->lsb_shndx = SHN_ABS;
	lsb->lsb_provide = provide;
	if (hidden)
		lsb->lsb_other = STV_HIDDEN;
	_resolve_and_add_symbol(ld, lsb);
}

int
ld_symbols_get_value(struct ld *ld, char *name, uint64_t *val)
{
	struct ld_symbol *lsb;

	if ((lsb = _symbol_find(ld->ld_symtab_def, name)) == NULL)
		return (-1);

	*val = lsb->lsb_value;

	return (0);
}

void
ld_symbols_update_value(struct ld *ld)
{
	struct ld_input *li;
	struct ld_input_section *is;
	struct ld_output *lo;
	struct ld_output_section *os;
	struct ld_symbol *lsb, *_lsb;
	char *name;

	lo = ld->ld_output;
	assert(lo != NULL);

	HASH_ITER(hh, ld->ld_symtab_def, lsb, _lsb) {
		if (lsb->lsb_shndx != SHN_ABS && lsb->lsb_shndx != SHN_COMMON) {
			li = lsb->lsb_input;
			is = &li->li_is[lsb->lsb_shndx];
			name = is->is_name;
			HASH_FIND_STR(lo->lo_ostbl, name, os);
			lsb->lsb_value += os->os_addr + is->is_reloff;
			printf("symbol %s: %#jx\n", lsb->lsb_name,
			    (uintmax_t) lsb->lsb_value);
		}
	}
}

void
ld_symbols_build_symtab(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_input *li;
	struct ld_symbol _lsb;

	(void) lo;
	(void) li;

	ld->ld_symtab = _alloc_symbol_table(ld);
	ld->ld_strtab = ld_strtab_alloc(ld);

	/*
	 * Always create the special symbol at the beginning of the
	 * symbol table.
	 */
	_lsb.lsb_name = NULL;
	_lsb.lsb_size = 0;
	_lsb.lsb_value = 0;
	_lsb.lsb_shndx = SHN_UNDEF;
	_lsb.lsb_bind = STB_LOCAL;
	_lsb.lsb_type = STT_NOTYPE;
	_lsb.lsb_other = 0;
	_add_to_symbol_table(ld, ld->ld_symtab, ld->ld_strtab, &_lsb);

	/* TODO: Create STT_SECTION symbols. */
}

struct ld_symbol_table *
_alloc_symbol_table(struct ld *ld)
{
	struct ld_symbol_table *symtab;

	if ((symtab = calloc(1, sizeof(*ld->ld_symtab))) == NULL)
		ld_fatal_std(ld, "calloc");

	return (symtab);
}

void
_add_to_symbol_table(struct ld *ld, struct ld_symbol_table *symtab,
    struct ld_strtab *strtab, struct ld_symbol *lsb)
{
	struct ld_output *lo;
	Elf32_Sym *s32;
	Elf64_Sym *s64;
	size_t es;

	assert(symtab != NULL && lsb != NULL);

	lo = ld->ld_output;
	assert(lo != NULL);

	es = (lo->lo_ec == ELFCLASS32) ? sizeof(Elf32_Sym) : sizeof(Elf64_Sym);

	/*
	 * Allocate/Reallocate buffer for the symbol table.
	 */
	if (symtab->sy_buf == NULL) {
		symtab->sy_size = 0;
		symtab->sy_cap = _INIT_SYMTAB_SIZE;
		symtab->sy_buf = malloc(symtab->sy_cap * es);
		if (symtab->sy_buf == NULL)
			ld_fatal_std(ld, "malloc");
	} else if (symtab->sy_size >= symtab->sy_cap) {
		symtab->sy_cap *= 2;
		symtab->sy_buf = realloc(symtab->sy_buf, symtab->sy_cap * es);
		if (symtab->sy_buf == NULL)
			ld_fatal_std(ld, "relloc");
	}

	/*
	 * Insert the symbol into the symbol table and the symbol name to
	 * the assoicated name string table.
	 */
	if (lo->lo_ec == ELFCLASS32) {
		s32 = symtab->sy_buf;
		s32 += symtab->sy_size;
		s32->st_name = ld_strtab_insert_no_suffix(ld, strtab,
		    lsb->lsb_name);
		s32->st_info = ELF32_ST_INFO(lsb->lsb_bind, lsb->lsb_type);
		s32->st_other = lsb->lsb_other;
		s32->st_shndx = lsb->lsb_shndx;
		s32->st_value = lsb->lsb_value;
		s32->st_size = lsb->lsb_size;
	} else {
		s64 = symtab->sy_buf;
		s64 += symtab->sy_size;
		s64->st_name = ld_strtab_insert_no_suffix(ld, strtab,
		    lsb->lsb_name);
		s64->st_info = ELF64_ST_INFO(lsb->lsb_bind, lsb->lsb_type);
		s64->st_other = lsb->lsb_other;
		s64->st_shndx = lsb->lsb_shndx;
		s64->st_value = lsb->lsb_value;
		s64->st_size = lsb->lsb_size;
	}

	/* Remember the index for the first non-local symbol. */
	if (symtab->sy_first_nonlocal == 0 && lsb->lsb_bind != STB_LOCAL)
		symtab->sy_first_nonlocal = symtab->sy_size;

	symtab->sy_size++;
}
