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
#include "ld_dynamic.h"
#include "ld_file.h"
#include "ld_input.h"
#include "ld_output.h"
#include "ld_symbols.h"
#include "ld_symver.h"
#include "ld_script.h"
#include "ld_strtab.h"

ELFTC_VCSID("$Id$");

#define	_INIT_SYMTAB_SIZE	128

static void _load_symbols(struct ld *ld, struct ld_file *lf);
static void _load_archive_symbols(struct ld *ld, struct ld_file *lf);
static void _load_elf_symbols(struct ld *ld, struct ld_input *li, Elf *e);
static void _unload_symbols(struct ld_input *li);
static void _add_elf_symbol(struct ld *ld, struct ld_input *li, Elf *e,
    GElf_Sym *sym, size_t strndx, int i);
static void _add_to_dynsym_table(struct ld *ld, struct ld_symbol *lsb);
static void _write_to_dynsym_table(struct ld *ld, struct ld_symbol *lsb);
static void _add_to_symbol_table(struct ld *ld, struct ld_symbol *lsb);
static void _free_symbol_table(struct ld_symbol_table *symtab);
struct ld_symbol_table *_alloc_symbol_table(struct ld *ld);
static int _archive_member_extracted(struct ld_archive *la, off_t off);
static struct ld_archive_member * _extract_archive_member(struct ld *ld,
    struct ld_file *lf, struct ld_archive *la, off_t off);
static void _print_extracted_member(struct ld *ld,
    struct ld_archive_member *lam, struct ld_symbol *lsb);
static void _resolve_and_add_symbol(struct ld *ld, struct ld_symbol *lsb);
static int _resolve_multidef_symbol(struct ld *ld, struct ld_symbol *lsb,
    struct ld_symbol *_lsb);
static struct ld_symbol *_alloc_symbol(struct ld *ld);
static void _free_symbol(struct ld_symbol *lsb);
static struct ld_symbol *_find_symbol(struct ld_symbol *tbl, char *name);
static struct ld_symbol *_find_symbol_from_import(struct ld *ld, char *name);
static struct ld_symbol *_find_symbol_from_export(struct ld *ld, char *name);
static void _add_to_import(struct ld *ld, struct ld_symbol *lsb);
static void _remove_from_import(struct ld *ld, struct ld_symbol *lsb);
static void _add_to_export(struct ld *ld, struct ld_symbol *lsb);
static void _remove_from_export(struct ld *ld, struct ld_symbol *lsb);
static void _update_import_and_export(struct ld *ld, struct ld_symbol *_lsb,
    struct ld_symbol *lsb);
static void _update_export(struct ld *ld, struct ld_symbol *lsb, int add);
static void _update_symbol(struct ld_symbol *lsb);

#define	_add_symbol(tbl, s) do {				\
	HASH_ADD_KEYPTR(hh, (tbl), (s)->lsb_longname,		\
	    strlen((s)->lsb_longname), (s));			\
	_update_export(ld, (s), 1);				\
	} while (0)
#define _remove_symbol(tbl, s) do {				\
	HASH_DEL((tbl), (s));					\
	_update_export(ld, (s), 0);				\
	} while (0)
#define _resolve_symbol(_s, s) do {				\
	assert((_s) != (s));					\
	(s)->lsb_ref_dso |= (_s)->lsb_ref_dso;			\
	(s)->lsb_ref_ndso |= (_s)->lsb_ref_ndso;		\
	if ((s)->lsb_prev != NULL) {				\
		(s)->lsb_prev->lsb_ref = (_s);			\
		(_s)->lsb_prev = (s)->lsb_prev;			\
	}							\
	(s)->lsb_prev = (_s);					\
	(_s)->lsb_ref = (s);					\
	_update_import_and_export(ld, _s, s);			\
	} while (0)

void
ld_symbols_cleanup(struct ld *ld)
{
	struct ld_input *li;
	struct ld_symbol *lsb, *_lsb;

	HASH_CLEAR(hh, ld->ld_symtab_def);
	HASH_CLEAR(hh, ld->ld_symtab_undef);
	HASH_CLEAR(hh, ld->ld_symtab_common);

	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		_unload_symbols(li);
	}

	if (ld->ld_ext_symbols != NULL) {
		STAILQ_FOREACH_SAFE(lsb, ld->ld_ext_symbols, lsb_next, _lsb) {
			STAILQ_REMOVE(ld->ld_ext_symbols, lsb, ld_symbol,
			    lsb_next);
			_free_symbol(lsb);
		}
		free(ld->ld_ext_symbols);
		ld->ld_ext_symbols = NULL;
	}

	if (ld->ld_var_symbols != NULL) {
		STAILQ_FOREACH_SAFE(lsb, ld->ld_var_symbols, lsb_next, _lsb) {
			STAILQ_REMOVE(ld->ld_var_symbols, lsb, ld_symbol,
			    lsb_next);
			_free_symbol(lsb);
		}
		free(ld->ld_var_symbols);
		ld->ld_var_symbols = NULL;
	}

	if (ld->ld_dyn_symbols != NULL) {
		free(ld->ld_dyn_symbols);
		ld->ld_dyn_symbols = NULL;
	}

	if (ld->ld_symtab != NULL) {
		_free_symbol_table(ld->ld_symtab);
		ld->ld_symtab = NULL;
	}

	if (ld->ld_strtab != NULL) {
		ld_strtab_free(ld->ld_strtab);
		ld->ld_strtab = NULL;
	}
}

void
ld_symbols_add_extern(struct ld *ld, char *name)
{
	struct ld_symbol *lsb;

	if (_find_symbol(ld->ld_symtab_undef, name) != NULL)
		return;

	lsb = _alloc_symbol(ld);
	if ((lsb->lsb_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");
	if ((lsb->lsb_longname = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");

	if (ld->ld_ext_symbols == NULL) {
		ld->ld_ext_symbols = malloc(sizeof(*ld->ld_ext_symbols));
		if (ld->ld_ext_symbols == NULL)
			ld_fatal_std(ld, "malloc");
		STAILQ_INIT(ld->ld_ext_symbols);
	}
	STAILQ_INSERT_TAIL(ld->ld_ext_symbols, lsb, lsb_next);

	_add_symbol(ld->ld_symtab_undef, lsb);
}

void
ld_symbols_add_variable(struct ld *ld, struct ld_script_variable *ldv,
    unsigned provide, unsigned hidden)
{
	struct ld_symbol *lsb;

	lsb = _alloc_symbol(ld);
	if ((lsb->lsb_name = strdup(ldv->ldv_name)) == NULL)
		ld_fatal_std(ld, "strdup");
	if ((lsb->lsb_longname = strdup(ldv->ldv_name)) == NULL)
		ld_fatal_std(ld, "strdup");
	lsb->lsb_var = ldv;
	lsb->lsb_bind = STB_GLOBAL;
	lsb->lsb_shndx = SHN_ABS;
	lsb->lsb_provide = provide;
	if (hidden)
		lsb->lsb_other = STV_HIDDEN;
	lsb->lsb_ref_ndso = 1;

	if (ld->ld_var_symbols == NULL) {
		ld->ld_var_symbols = malloc(sizeof(*ld->ld_var_symbols));
		if (ld->ld_var_symbols == NULL)
			ld_fatal_std(ld, "malloc");
		STAILQ_INIT(ld->ld_var_symbols);
	}
	STAILQ_INSERT_TAIL(ld->ld_var_symbols, lsb, lsb_next);

	_resolve_and_add_symbol(ld, lsb);
}

void
ld_symbols_add_internal(struct ld *ld, const char *name, uint64_t size,
    uint64_t value, uint16_t shndx, unsigned char bind, unsigned char type,
    unsigned char other, struct ld_input *input,
    struct ld_output_section *preset_os)
{
	struct ld_symbol *lsb;

	lsb = _alloc_symbol(ld);
	if ((lsb->lsb_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");
	if ((lsb->lsb_longname = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");
	lsb->lsb_size = size;
	lsb->lsb_value = value;
	lsb->lsb_shndx = shndx;
	lsb->lsb_bind = bind;
	lsb->lsb_type = type;
	lsb->lsb_other = other;
	lsb->lsb_preset_os = preset_os;
	lsb->lsb_ref_ndso = 1;
	lsb->lsb_input = input;

	_resolve_and_add_symbol(ld, lsb);
}

int
ld_symbols_get_value(struct ld *ld, char *name, uint64_t *val)
{
	struct ld_symbol *lsb;

	if ((lsb = _find_symbol(ld->ld_symtab_def, name)) != NULL)
		*val = lsb->lsb_value;
	else if ((lsb = _find_symbol(ld->ld_symtab_common, name)) != NULL)
		*val = lsb->lsb_value;
	else
		return (-1);

	return (0);
}

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
		if (ls->ls_arch_conflict) {
			ld_file_unload(ld, lf);
			return;
		}
		_load_symbols(ld, lf);
		ld_file_unload(ld, lf);
		lf = TAILQ_NEXT(lf, lf_next);
	}

	/* Print information regarding space allocated for common symbols. */
	if (ld->ld_print_linkmap && HASH_COUNT(ld->ld_symtab_common) > 0) {
		printf("\nCommon symbols:\n");
		printf("%-34s %-10s %s\n", "name", "size", "file");
		HASH_ITER(hh, ld->ld_symtab_common, lsb, _lsb) {
			printf("%-34s", lsb->lsb_name);
			if (strlen(lsb->lsb_name) > 34)
				printf("\n%-34s", "");
			printf(" %#-10jx %s\n", (uintmax_t) lsb->lsb_size,
			    ld_input_get_fullname(ld, lsb->lsb_input));
		}
	}
}

void
ld_symbols_warn_undefined(struct ld *ld)
{
	struct ld_symbol *lsb, *_lsb;

	if (ld->ld_dso)
		return;

	if (HASH_COUNT(ld->ld_symtab_undef) > 0) {
		HASH_ITER(hh, ld->ld_symtab_undef, lsb, _lsb) {
			if (lsb->lsb_bind != STB_WEAK)
				ld_warn(ld, "undefined symbol: %s",
				    lsb->lsb_name);
		}
	}
}

void
ld_symbols_update(struct ld *ld)
{
	struct ld_input *li;
	struct ld_symbol *lsb, *_lsb;

	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		if (li->li_local == NULL)
			continue;
		STAILQ_FOREACH(lsb, li->li_local, lsb_next)
			_update_symbol(lsb);
	}

	HASH_ITER(hh, ld->ld_symtab_def, lsb, _lsb) {
		/* Skip defined symbols from DSOs. */
		if (lsb->lsb_input != NULL &&
		    lsb->lsb_input->li_type == LIT_DSO)
			continue;

		_update_symbol(lsb);
	}

	HASH_ITER(hh, ld->ld_symtab_common, lsb, _lsb) {
		_update_symbol(lsb);
	}
}

void
ld_symbols_build_symtab(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_output_section *os;
	struct ld_input *li;
#if 0
	struct ld_input_section *is;
#endif
	struct ld_symbol *lsb, *lsb0, *tmp, _lsb;

	lo = ld->ld_output;

	ld->ld_symtab = _alloc_symbol_table(ld);
	ld->ld_strtab = ld_strtab_alloc(ld);

	/* Create an initial symbol at the beginning of symbol table. */
	_lsb.lsb_name = NULL;
	_lsb.lsb_size = 0;
	_lsb.lsb_value = 0;
	_lsb.lsb_shndx = SHN_UNDEF;
	_lsb.lsb_bind = STB_LOCAL;
	_lsb.lsb_type = STT_NOTYPE;
	_lsb.lsb_other = 0;
	_add_to_symbol_table(ld, &_lsb);

	/* Create STT_SECTION symbols. */
	STAILQ_FOREACH(os, &lo->lo_oslist, os_next) {
		if (os->os_empty)
			continue;
		_lsb.lsb_name = NULL;
		_lsb.lsb_size = 0;
		_lsb.lsb_value = os->os_addr;
		_lsb.lsb_shndx = elf_ndxscn(os->os_scn);
		_lsb.lsb_bind = STB_LOCAL;
		_lsb.lsb_type = STT_SECTION;
		_lsb.lsb_other = 0;
		_add_to_symbol_table(ld, &_lsb);
	}

	/* Copy local symbols from each input object. */
	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		if (li->li_local == NULL)
			continue;
		STAILQ_FOREACH(lsb, li->li_local, lsb_next) {
#if 0
			li = lsb->lsb_input;
			is = &li->li_is[lsb->lsb_shndx];
			if (is->is_output == NULL) {
				printf("discard symbol: %s\n", lsb->lsb_name);
				continue;
			}
#endif
			if (lsb->lsb_type != STT_SECTION &&
			    lsb->lsb_index != 0)
				_add_to_symbol_table(ld, lsb);
		}
	}

	/* Copy resolved global symbols from hash table. */
	HASH_ITER(hh, ld->ld_symtab_def, lsb, tmp) {
		if (lsb->lsb_input != NULL &&
		    lsb->lsb_input->li_type == LIT_DSO) {
			lsb0 = _find_symbol_from_import(ld, lsb->lsb_longname);
			if (lsb0 == NULL)
				continue;
			memcpy(&_lsb, lsb0, sizeof(_lsb));
			_lsb.lsb_value = 0;
			_lsb.lsb_shndx = SHN_UNDEF;
			_add_to_symbol_table(ld, &_lsb);
		} else
			_add_to_symbol_table(ld, lsb);
	}

	/* Copy undefined weak symbols. */
	HASH_ITER(hh, ld->ld_symtab_undef, lsb, tmp) {
		/* Skip weak undefined symbols from DSO. */
		if (lsb->lsb_input != NULL &&
		    lsb->lsb_input->li_type == LIT_DSO)
			continue;
		_add_to_symbol_table(ld, lsb);
	}

	/* Copy common symbols. */
	HASH_ITER(hh, ld->ld_symtab_common, lsb, tmp) {
		_add_to_symbol_table(ld, lsb);
	}
}

void
ld_symbols_create_dynsym(struct ld *ld)
{
	struct ld_symbol *lsb, *tmp;

	ld->ld_dynsym = _alloc_symbol_table(ld);
	if (ld->ld_dynstr == NULL)
		ld->ld_dynstr = ld_strtab_alloc(ld);

	/* Reserve space for the initial symbol. */
	ld->ld_dynsym->sy_size++;

	/* undefined weak symbols. */
	HASH_ITER(hh, ld->ld_symtab_undef, lsb, tmp) {
		/* Skip weak undefined symbols from DSO. */
		if (lsb->lsb_input != NULL &&
		    lsb->lsb_input->li_type == LIT_DSO)
			continue;
		_add_to_dynsym_table(ld, lsb);
	}

	/* import symbols. */
	HASH_ITER(hhimp, ld->ld_symtab_import, lsb, tmp) {
		lsb->lsb_import = 1;
		_add_to_dynsym_table(ld, lsb);
	}

	/* export symbols. */
	HASH_ITER(hhexp, ld->ld_symtab_export, lsb, tmp) {
		_add_to_dynsym_table(ld, lsb);
	}
}

void
ld_symbols_finalize_dynsym(struct ld *ld)
{
	struct ld_symbol *lsb, _lsb;

	/* Create an initial symbol at the beginning of symbol table. */
	_lsb.lsb_name = NULL;
	_lsb.lsb_nameindex = 0;
	_lsb.lsb_size = 0;
	_lsb.lsb_value = 0;
	_lsb.lsb_shndx = SHN_UNDEF;
	_lsb.lsb_bind = STB_LOCAL;
	_lsb.lsb_type = STT_NOTYPE;
	_lsb.lsb_other = 0;
	_write_to_dynsym_table(ld, &_lsb);

	assert(ld->ld_dyn_symbols != NULL);

	STAILQ_FOREACH(lsb, ld->ld_dyn_symbols, lsb_dyn) {
		if (lsb->lsb_import && lsb->lsb_type == STT_FUNC) {
			memcpy(&_lsb, lsb, sizeof(_lsb));
			_lsb.lsb_value = 0;
			_lsb.lsb_shndx = SHN_UNDEF;
			_write_to_dynsym_table(ld, &_lsb);
		} else
			_write_to_dynsym_table(ld, lsb);
	}
}

/*
 * Retrieve the resolved symbol.
 */
struct ld_symbol *
ld_symbols_ref(struct ld_symbol *lsb)
{

	while (lsb->lsb_ref != NULL)
		lsb = lsb->lsb_ref;

	return (lsb);
}

/*
 * Check if a symbol can be overriden (by symbols in main executable).
 */
int
ld_symbols_overridden(struct ld *ld, struct ld_symbol *lsb)
{

	/* Symbols can be overridden only when we are creating a DSO. */
	if (!ld->ld_dso)
		return (0);

	/* Only visible symbols can be overriden. */
	if (lsb->lsb_other != STV_DEFAULT)
		return (0);

	/* TODO: other cases. */

	/* Otherwise symbol can be overridden. */
	return (1);
}

static struct ld_symbol *
_alloc_symbol(struct ld *ld)
{
	struct ld_symbol *s;

	if ((s = calloc(1, sizeof(*s))) == NULL)
		ld_fatal_std(ld, "calloc");

	return (s);
}

static struct ld_symbol *
_find_symbol(struct ld_symbol *tbl, char *name)
{
	struct ld_symbol *s;

	HASH_FIND_STR(tbl, name, s);
	return (s);
}

static struct ld_symbol *
_find_symbol_from_import(struct ld *ld, char *name)
{
	struct ld_symbol *s;

	HASH_FIND(hhimp, ld->ld_symtab_import, name, strlen(name), s);
	return (s);
}

static struct ld_symbol *
_find_symbol_from_export(struct ld *ld, char *name)
{
	struct ld_symbol *s;

	HASH_FIND(hhexp, ld->ld_symtab_export, name, strlen(name), s);
	return (s);
}

static void
_add_to_import(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;

	_lsb = _find_symbol_from_import(ld, lsb->lsb_longname);
	if (_lsb != NULL)
		return;
	HASH_ADD_KEYPTR(hhimp, ld->ld_symtab_import, lsb->lsb_longname,
	    strlen(lsb->lsb_longname), lsb);
	lsb->lsb_input->li_dso_refcnt++;
	ld_symver_increase_verdef_refcnt(ld, lsb);
}

static void
_remove_from_import(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;

	_lsb = _find_symbol_from_import(ld, lsb->lsb_longname);
	if (_lsb == NULL)
		return;
	HASH_DELETE(hhimp, ld->ld_symtab_import, _lsb);
	_lsb->lsb_input->li_dso_refcnt--;
	ld_symver_decrease_verdef_refcnt(ld, lsb);
}

static void
_add_to_export(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;

	if (lsb->lsb_other == STV_HIDDEN)
		return;

	_lsb = _find_symbol_from_export(ld, lsb->lsb_longname);
	if (_lsb != NULL)
		return;
	HASH_ADD_KEYPTR(hhexp, ld->ld_symtab_export, lsb->lsb_longname,
	    strlen(lsb->lsb_longname), lsb);
}

static void
_remove_from_export(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;

	if (lsb->lsb_other == STV_HIDDEN)
		return;

	_lsb = _find_symbol_from_export(ld, lsb->lsb_longname);
	if (_lsb == NULL)
		return;
	HASH_DELETE(hhexp, ld->ld_symtab_export, _lsb);
}

static void
_update_import_and_export(struct ld *ld, struct ld_symbol *_lsb,
    struct ld_symbol *lsb)
{

	/*
	 * If a DSO symbol is resolved by a defined symbol, the latter should
	 * be added to the export list if it is *not* defined in a DSO and
	 * if it doesn't yet exist there.
	 *
	 * If a defined DSO symbol is resolved by another symbol, the DSO
	 * symbol should be removed from the import symbol table, if it exists
	 * there.
	 */
	if (_lsb->lsb_input != NULL && _lsb->lsb_input->li_type == LIT_DSO) {
		if (_lsb->lsb_shndx != SHN_UNDEF)
			_remove_from_import(ld, _lsb);
		if (lsb->lsb_shndx != SHN_UNDEF && (lsb->lsb_input == NULL ||
		    lsb->lsb_input->li_type == LIT_RELOCATABLE))
			_add_to_export(ld, lsb);
	}


	/*
	 * If some symbol resolved to a defined DSO symbol, and the former
	 * once appeared in a place other than a DSO, the DSO symbol
	 * should be added to import symbol table, if it doesn't yet exist
	 * there.
	 */
	if (lsb->lsb_input != NULL && lsb->lsb_input->li_type == LIT_DSO &&
	    lsb->lsb_shndx != SHN_UNDEF && lsb->lsb_ref_ndso)
		_add_to_import(ld, lsb);
}

static void
_update_export(struct ld *ld, struct ld_symbol *lsb, int add)
{

	/* Only defined symbols are exportable. */
	if (lsb->lsb_shndx == SHN_UNDEF)
		return;

	/*
	 * If the linker creates a executable, update the export table
	 * if the symbol is defined in a regular object and has been
	 * referenced by a DSO.
	 */
	if (ld->ld_exec || ld->ld_pie) {
		if (lsb->lsb_input != NULL &&
		    lsb->lsb_input->li_type != LIT_RELOCATABLE)
			return;

		if (!lsb->lsb_ref_dso)
			return;
	}

	/*
	 * If the linker creates a DSO, update the export table if the
	 * symbol is defined in a regualr object and is visible to the
	 * outside.
	 */
	if (ld->ld_dso) {
		if (lsb->lsb_input != NULL &&
		    lsb->lsb_input->li_type != LIT_RELOCATABLE)
			return;

		if (lsb->lsb_other != STV_DEFAULT)
			return;
	}

	if (add)
		_add_to_export(ld, lsb);
	else
		_remove_from_export(ld, lsb);
}

static int
_resolve_multidef_symbol(struct ld *ld, struct ld_symbol *lsb,
    struct ld_symbol *_lsb)
{

	if (_lsb->lsb_provide)
		_remove_symbol(ld->ld_symtab_def, _lsb);
	else if (lsb->lsb_input != NULL &&
	    lsb->lsb_input->li_type == LIT_DSO) {
		_resolve_symbol(lsb, _lsb);
		return (-1);
	} else if (_lsb->lsb_input != NULL &&
	    _lsb->lsb_input->li_type == LIT_DSO) {
		_resolve_symbol(_lsb, lsb);
		_remove_symbol(ld->ld_symtab_def, _lsb);
	} else
		ld_fatal(ld, "multiple definition of symbol "
		    "%s", lsb->lsb_longname);

	return (0);
}

static void
_resolve_and_add_symbol(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;
	struct ld_symbol_defver *dv;
	char *name, *sn;

	name = lsb->lsb_longname;
	sn = lsb->lsb_name;
	if (lsb->lsb_shndx == SHN_UNDEF) {
		/*
		 * Search the undefined, defined and common symbol hash
		 * table for symbol with the same name. If found, point this
		 * symbol to the symbol found.
		 */
		if ((_lsb = _find_symbol(ld->ld_symtab_def, name)) != NULL ||
		    (_lsb = _find_symbol(ld->ld_symtab_undef, name)) != NULL ||
		    (_lsb = _find_symbol(ld->ld_symtab_common, name)) !=
		    NULL) {
			_resolve_symbol(lsb, _lsb);
			return;
		}

		/*
		 * If the symbol version is not specified, also search the
		 * default-versioned defined symbol table.
		 */
		if (!strcmp(name, sn)){
			HASH_FIND_STR(ld->ld_defver, name, dv);
			if (dv != NULL) {
				if ((_lsb = _find_symbol(ld->ld_symtab_def,
				    dv->dv_longname)) != NULL) {
					_resolve_symbol(lsb, _lsb);
					return;
				}
			}
		}

		/*
		 * Otherwise add the new symbol to undefined symbol hash
		 * table.
		 */
		_add_symbol(ld->ld_symtab_undef, lsb);

	} else if (lsb->lsb_shndx == SHN_COMMON) {
		/*
		 * Search the defined symbol hash table for symbol with the
		 * same name. If found, resolve this symbol to the symbol
		 * found.
		 */
		if ((_lsb = _find_symbol(ld->ld_symtab_def, name)) != NULL) {
			_resolve_symbol(lsb, _lsb);
			return;
		}

		/*
		 * If the symbol version is not specified, also search the
		 * default-versioned defined symbol table.
		 */
		if (!strcmp(name, sn)){
			HASH_FIND_STR(ld->ld_defver, name, dv);
			if (dv != NULL) {
				if ((_lsb = _find_symbol(ld->ld_symtab_def,
				    dv->dv_longname)) != NULL) {
					_resolve_symbol(lsb, _lsb);
					return;
				}
			}
		}

		/*
		 * Search the undefined symbol hash table for symbol with the
		 * same name. If found, resolve the found symbol to this
		 * symbol.
		 */
		if ((_lsb = _find_symbol(ld->ld_symtab_undef, name)) != NULL) {
			_resolve_symbol(_lsb, lsb);
			_remove_symbol(ld->ld_symtab_undef, _lsb);
		}

		/*
		 * Search the common symbol hash table for symbol with the
		 * same name. If found, compare the size of the two symbols
		 * and prefer the one that has a bigger size.
		 */
		if ((_lsb = _find_symbol(ld->ld_symtab_common, name)) !=
		    NULL) {
			if (lsb->lsb_size > _lsb->lsb_size) {
				_resolve_symbol(_lsb, lsb);
				_remove_symbol(ld->ld_symtab_common, _lsb);
			} else {
				_resolve_symbol(lsb, _lsb);
				return;
			}
		}

		/* Add the new symbol to common symbol hash table. */
		_add_symbol(ld->ld_symtab_common, lsb);

	} else {
		/*
		 * Search in the defined symbol hash table for the symbol
		 * with the same "long name". If such symbol is found, pass
		 * it to function _resolve_multidef_symbol() for resolution.
		 * If the symbol is default-versioned, also search in the
		 * defined symbol hash table for unversioned symbol with the
		 * same "short name".
		 */
		if ((_lsb = _find_symbol(ld->ld_symtab_def, name)) != NULL) {
			if (_resolve_multidef_symbol(ld, lsb, _lsb) < 0)
				return;
		}
		if (lsb->lsb_default &&
		    (_lsb = _find_symbol(ld->ld_symtab_def, sn)) != NULL) {
			if (_resolve_multidef_symbol(ld, lsb, _lsb) < 0)
				return;
		}

		/*
		 * Search in the undefined symbol hash table for the symbol
		 * with the same "long name". If such symbol is found, we
		 * resolve the found symbol to this symbol. If the symbol
		 * is default-versioned, also search in the undefined symbol
		 * hash table for unversioned symbol with the same
		 * "short name".
		 */
		if ((_lsb = _find_symbol(ld->ld_symtab_undef, name)) != NULL) {
			_resolve_symbol(_lsb, lsb);
			_remove_symbol(ld->ld_symtab_undef, _lsb);
		}
		if (lsb->lsb_default &&
		    (_lsb = _find_symbol(ld->ld_symtab_undef, sn)) != NULL) {
			_resolve_symbol(_lsb, lsb);
			_remove_symbol(ld->ld_symtab_undef, _lsb);
		}

		/*
		 * Search in the common symbol hash table for the symbol with
		 * the same "long name". If such symbol is found, we resolve
		 * the found symbol to this symbol. If the symbol is
		 * default-versioned, also search in the common symbol hash
		 * table for unversioned symbol with the same "short name".
		 */
		if ((_lsb = _find_symbol(ld->ld_symtab_common, name)) !=
		    NULL) {
			_resolve_symbol(_lsb, lsb);
			_remove_symbol(ld->ld_symtab_common, _lsb);
		}
		if (lsb->lsb_default &&
		    (_lsb = _find_symbol(ld->ld_symtab_common, sn)) != NULL) {
			_resolve_symbol(_lsb, lsb);
			_remove_symbol(ld->ld_symtab_common, _lsb);
		}

		/* Add the new symbol to the defined symbol hash table. */
		_add_symbol(ld->ld_symtab_def, lsb);
	}
}

static void
_add_elf_symbol(struct ld *ld, struct ld_input *li, Elf *e, GElf_Sym *sym,
    size_t strndx, int i)
{
	struct ld_symbol *lsb;
	struct ld_symbol_defver *dv;
	char *name;
	int j, len, ndx;

	if ((name = elf_strptr(e, strndx, sym->st_name)) == NULL)
		return;

	lsb = _alloc_symbol(ld);

	if ((lsb->lsb_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");
	lsb->lsb_value = sym->st_value;
	lsb->lsb_size = sym->st_size;
	lsb->lsb_bind = GELF_ST_BIND(sym->st_info);
	lsb->lsb_type = GELF_ST_TYPE(sym->st_info);
	lsb->lsb_other = sym->st_other;
	lsb->lsb_shndx = sym->st_shndx;
	lsb->lsb_index = i;
	lsb->lsb_input = li;
	lsb->lsb_ver = NULL;

	if (li->li_type == LIT_DSO)
		lsb->lsb_ref_dso = 1;
	else
		lsb->lsb_ref_ndso = 1;

	/* Find out symbol version info. */
	j = 0;
	if (li->li_file->lf_type == LFT_DSO && li->li_vername != NULL &&
	    li->li_versym != NULL && (size_t) i < li->li_versym_sz) {
		j = li->li_versym[i];
		ndx = j & ~0x8000;
		if ((size_t) ndx < li->li_vername_sz) {
			lsb->lsb_ver = li->li_vername[ndx];
#if 0
			printf("symbol: %s ver: %s\n", lsb->lsb_name,
			    lsb->lsb_ver);
#endif
			if (j >= 2 && (j & 0x8000) == 0 &&
			    lsb->lsb_shndx != SHN_UNDEF)
				lsb->lsb_default = 1;
		}
	}

	/* Build "long" symbol name which is used for hash key. */
	if (lsb->lsb_ver == NULL || j < 2) {
		lsb->lsb_longname = strdup(lsb->lsb_name);
		if (lsb->lsb_longname == NULL)
			ld_fatal_std(ld, "strdup");
	} else {
		len = strlen(lsb->lsb_name) + strlen(lsb->lsb_ver) + 2;
		if ((lsb->lsb_longname = malloc(len)) == NULL)
			ld_fatal_std(ld, "malloc");
		snprintf(lsb->lsb_longname, len, "%s@%s", lsb->lsb_name,
		    lsb->lsb_ver);
	}

	/* Keep track of default versions. */
	if (lsb->lsb_default) {
		if ((dv = calloc(1, sizeof(*dv))) == NULL)
			ld_fatal(ld, "calloc");
		dv->dv_name = lsb->lsb_name;
		dv->dv_longname = lsb->lsb_longname;
		dv->dv_ver = lsb->lsb_ver;
		HASH_ADD_KEYPTR(hh, ld->ld_defver, dv->dv_name,
		    strlen(dv->dv_name), dv);
	}

	/*
	 * Insert symbol to input object internal symbol list and
	 * perform symbol resolving.
	 */
	ld_input_add_symbol(ld, li, lsb);
	if (lsb->lsb_bind != STB_LOCAL)
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

static struct ld_archive_member *
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

	lam->lam_ar_name = strdup(lf->lf_name);
	if (lam->lam_ar_name == NULL)
		ld_fatal_std(ld, "strdup");

	lam->lam_name = strdup(arhdr->ar_name);
	if (lam->lam_name == NULL)
		ld_fatal_std(ld, "strdup");

	lam->lam_off = off;

	HASH_ADD(hh, la->la_m, lam_off, sizeof(lam->lam_off), lam);

	/* Allocate input object for this member. */
	li = ld_input_alloc(ld, lf, lam->lam_name);
	li->li_lam = lam;
	lam->lam_input = li;

	/* Load the symbols of this member. */
	_load_elf_symbols(ld, li, e);

	elf_end(e);

	return (lam);
}

static void
_print_extracted_member(struct ld *ld, struct ld_archive_member *lam,
    struct ld_symbol *lsb)
{
	struct ld_state *ls;
	char *c1, *c2;

	ls = &ld->ld_state;

	if (!ls->ls_archive_mb_header) {
		printf("Extracted archive members:\n\n");
		ls->ls_archive_mb_header = 1;
	}

	c1 = ld_input_get_fullname(ld, lam->lam_input);
	c2 = ld_input_get_fullname(ld, lsb->lsb_input);

	printf("%-30s", c1);
	if (strlen(c1) >= 30) {
		printf("\n%-30s", "");
	}
	printf("%s (%s)\n", c2, lsb->lsb_name);
}

static void
_load_archive_symbols(struct ld *ld, struct ld_file *lf)
{
	struct ld_state *ls;
	struct ld_archive *la;
	struct ld_archive_member *lam;
	struct ld_symbol *lsb;
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
			if ((lsb = _find_symbol(ld->ld_symtab_undef,
			    as[i].as_name)) != NULL) {
				lam = _extract_archive_member(ld, lf, la,
				    as[i].as_off);
				extracted = 1;
				ls->ls_extracted[ls->ls_group_level] = 1;
				if (ld->ld_print_linkmap)
					_print_extracted_member(ld, lam, lsb);
			}
		}
	} while (extracted);
}

static void
_load_elf_symbols(struct ld *ld, struct ld_input *li, Elf *e)
{
	struct ld_input_section *is;
	Elf_Scn *scn, *scn_sym;
	Elf_Scn *scn_versym, *scn_verneed, *scn_verdef;
	Elf_Data *d;
	GElf_Sym sym;
	GElf_Shdr shdr;
	size_t strndx;
	int elferr, i;

	/* Load section list from input object. */
	ld_input_init_sections(ld, li, e);

	strndx = SHN_UNDEF;
	scn = scn_sym = scn_versym = scn_verneed = scn_verdef = NULL;

	for (i = 0; (uint64_t) i < li->li_shnum - 1; i++) {
		is = &li->li_is[i];
		if (li->li_type == LIT_DSO) {
			if (is->is_type == SHT_DYNSYM) {
				scn_sym = elf_getscn(e, is->is_index);
				strndx = is->is_link;
			}
			if (is->is_type == SHT_SUNW_versym)
				scn_versym = elf_getscn(e, is->is_index);
			if (is->is_type == SHT_SUNW_verneed)
				scn_verneed = elf_getscn(e, is->is_index);
			if (is->is_type == SHT_SUNW_verdef)
				scn_verdef = elf_getscn(e, is->is_index);
		} else {
			if (is->is_type == SHT_SYMTAB) {
				scn_sym = elf_getscn(e, is->is_index);
				strndx = is->is_link;
			}
		}
	}

	if (scn_sym == NULL || strndx == SHN_UNDEF)
		return;

	ld_symver_load_symbol_version_info(ld, li, e, scn_versym, scn_verneed,
	    scn_verdef);

	if (gelf_getshdr(scn_sym, &shdr) != &shdr)
		ld_fatal(ld, "%s: gelf_getshdr failed: %s", li->li_name,
		    elf_errmsg(-1));

	(void) elf_errno();
	if ((d = elf_getdata(scn_sym, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "%s: elf_getdata failed: %s", li->li_name,
			    elf_errmsg(elferr));
		/* Empty symbol table section? */
		return;
	}

	li->li_symnum = d->d_size / shdr.sh_entsize;
	for (i = 0; (uint64_t) i < li->li_symnum; i++) {
		if (gelf_getsym(d, i, &sym) != &sym)
			ld_fatal(ld, "%s: gelf_getsym failed: %s", li->li_name,
			    elf_errmsg(-1));
		_add_elf_symbol(ld, li, e, &sym, strndx, i);
	}
}

static void
_load_symbols(struct ld *ld, struct ld_file *lf)
{

	if (lf->lf_type == LFT_ARCHIVE)
		_load_archive_symbols(ld, lf);
	else {
		lf->lf_input = ld_input_alloc(ld, lf, lf->lf_name);
		_load_elf_symbols(ld, lf->lf_input, lf->lf_elf);
	}
}

static void
_unload_symbols(struct ld_input *li)
{
	int i;

	if (li->li_symindex == NULL)
		return;

	for (i = 0; (uint64_t) i < li->li_symnum; i++)
		_free_symbol(li->li_symindex[i]);
}

static void
_free_symbol(struct ld_symbol *lsb)
{

	free(lsb->lsb_name);
	free(lsb->lsb_longname);
	free(lsb);
}

static void
_update_symbol(struct ld_symbol *lsb)
{
	struct ld_input *li;
	struct ld_input_section *is;
	struct ld_output_section *os;

	if (lsb->lsb_preset_os != NULL) {
		lsb->lsb_value = lsb->lsb_preset_os->os_addr;
		lsb->lsb_shndx = elf_ndxscn(lsb->lsb_preset_os->os_scn);
		return;
	}

	if (lsb->lsb_shndx == SHN_ABS)
		return;

	if (lsb->lsb_input != NULL) {
		li = lsb->lsb_input;
		if (lsb->lsb_shndx == SHN_COMMON)
			is = &li->li_is[li->li_shnum - 1];
		else
			is = &li->li_is[lsb->lsb_shndx];
		if ((os = is->is_output) == NULL)
			return;
		lsb->lsb_value += os->os_addr + is->is_reloff;
		lsb->lsb_shndx = elf_ndxscn(os->os_scn);
	}
}

struct ld_symbol_table *
_alloc_symbol_table(struct ld *ld)
{
	struct ld_symbol_table *symtab;

	if ((symtab = calloc(1, sizeof(*ld->ld_symtab))) == NULL)
		ld_fatal_std(ld, "calloc");

	return (symtab);
}

static void
_add_to_dynsym_table(struct ld *ld, struct ld_symbol *lsb)
{

	assert(ld->ld_dynsym != NULL && ld->ld_dynstr != NULL);

	if (ld->ld_dyn_symbols == NULL) {
		ld->ld_dyn_symbols = malloc(sizeof(*ld->ld_dyn_symbols));
		if (ld->ld_dyn_symbols == NULL)
			ld_fatal_std(ld, "malloc");
		STAILQ_INIT(ld->ld_dyn_symbols);
	}
	STAILQ_INSERT_TAIL(ld->ld_dyn_symbols, lsb, lsb_dyn);

	lsb->lsb_nameindex = ld_strtab_insert_no_suffix(ld, ld->ld_dynstr,
	    lsb->lsb_name);

	lsb->lsb_dyn_index = ld->ld_dynsym->sy_size++;
}

static void
_write_to_dynsym_table(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_output *lo;
	struct ld_symbol_table *symtab;
	Elf32_Sym *s32;
	Elf64_Sym *s64;
	size_t es;

	assert(lsb != NULL);
	assert(ld->ld_dynsym != NULL && ld->ld_dynstr != NULL);
	symtab = ld->ld_dynsym;

	lo = ld->ld_output;
	assert(lo != NULL);

	es = (lo->lo_ec == ELFCLASS32) ? sizeof(Elf32_Sym) : sizeof(Elf64_Sym);

	/* Allocate buffer for the dynsym table. */
	if (symtab->sy_buf == NULL) {
		symtab->sy_buf = malloc(symtab->sy_size * es);
		symtab->sy_write_pos = 0;
	}

	if (lo->lo_ec == ELFCLASS32) {
		s32 = symtab->sy_buf;
		s32 += symtab->sy_write_pos;
		s32->st_name = lsb->lsb_nameindex;
		s32->st_info = ELF32_ST_INFO(lsb->lsb_bind, lsb->lsb_type);
		s32->st_other = lsb->lsb_other;
		s32->st_shndx = lsb->lsb_shndx;
		s32->st_value = lsb->lsb_value;
		s32->st_size = lsb->lsb_size;
	} else {
		s64 = symtab->sy_buf;
		s64 += symtab->sy_write_pos;
		s64->st_name = lsb->lsb_nameindex;
		s64->st_info = ELF64_ST_INFO(lsb->lsb_bind, lsb->lsb_type);
		s64->st_other = lsb->lsb_other;
		s64->st_shndx = lsb->lsb_shndx;
		s64->st_value = lsb->lsb_value;
		s64->st_size = lsb->lsb_size;
	}

	/* Remember the index for the first non-local symbol. */
	if (symtab->sy_first_nonlocal == 0 && lsb->lsb_bind != STB_LOCAL)
		symtab->sy_first_nonlocal = symtab->sy_write_pos;

	symtab->sy_write_pos++;
}

static void
_add_to_symbol_table(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_output *lo;
	struct ld_symbol_table *symtab;
	struct ld_strtab *strtab;
	Elf32_Sym *s32;
	Elf64_Sym *s64;
	size_t es;

	assert(lsb != NULL);
	assert(ld->ld_symtab != NULL && ld->ld_strtab != NULL);
	symtab = ld->ld_symtab;
	strtab = ld->ld_strtab;

	lo = ld->ld_output;
	assert(lo != NULL);

	es = (lo->lo_ec == ELFCLASS32) ? sizeof(Elf32_Sym) : sizeof(Elf64_Sym);

	/* Allocate/Reallocate buffer for the symbol table. */
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
	lsb->lsb_nameindex = ld_strtab_insert_no_suffix(ld, strtab,
	    lsb->lsb_name);
	if (lo->lo_ec == ELFCLASS32) {
		s32 = symtab->sy_buf;
		s32 += symtab->sy_size;
		s32->st_name = lsb->lsb_nameindex;
		s32->st_info = ELF32_ST_INFO(lsb->lsb_bind, lsb->lsb_type);
		s32->st_other = lsb->lsb_other;
		s32->st_shndx = lsb->lsb_shndx;
		s32->st_value = lsb->lsb_value;
		s32->st_size = lsb->lsb_size;
	} else {
		s64 = symtab->sy_buf;
		s64 += symtab->sy_size;
		s64->st_name = lsb->lsb_nameindex;
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

static void
_free_symbol_table(struct ld_symbol_table *symtab)
{

	if (symtab == NULL)
		return;

	free(symtab->sy_buf);
	free(symtab);
}
