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

static void _load_symbols(struct ld *ld, struct ld_file *lf);
static void _load_archive_symbols(struct ld *ld, struct ld_file *lf);
static void _load_elf_symbols(struct ld *ld, struct ld_input *li, Elf *e);
static void _unload_symbols(struct ld_input *li);
static void _add_elf_symbol(struct ld *ld, struct ld_input *li, Elf *e,
    GElf_Sym *sym, size_t strndx, int i);
static void _add_to_symbol_table(struct ld *ld, struct ld_symbol_table *symtab,
    struct ld_strtab *strtab, struct ld_symbol *lsb);
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
static struct ld_symbol *_find_symbol_from_input(struct ld_symbol *tbl,
    char *name);
static void _add_to_import(struct ld *ld, struct ld_symbol *lsb);
static void _remove_from_import(struct ld *ld, struct ld_symbol *lsb);
static void _update_import(struct ld *ld, struct ld_symbol *_lsb,
    struct ld_symbol *lsb);
static void _update_export(struct ld *ld, struct ld_symbol *lsb, int add);
static void _update_symbol(struct ld *ld, struct ld_symbol *lsb);
static void _add_version_name(struct ld *ld, struct ld_input *li, int ndx,
    const char *name);
static void _load_verdef(struct ld *ld, struct ld_input *li, Elf *e,
    Elf_Scn *verdef);
static void _load_verneed(struct ld *ld, struct ld_input *li, Elf *e,
    Elf_Scn *verneed);
static void _load_symbol_version_info(struct ld *ld, struct ld_input *li,
    Elf *e, Elf_Scn *versym, Elf_Scn *verneed, Elf_Scn *verdef);

#define	_add_symbol(tbl, s) do {				\
	HASH_ADD_KEYPTR(hh, (tbl), (s)->lsb_longname,		\
	    strlen((s)->lsb_longname), (s));			\
	_update_export(ld, (s), 1);				\
	} while (0)
#define _add_symbol_to_input(tbl, s) 				\
	HASH_ADD_KEYPTR(hhi, (tbl), (s)->lsb_name,		\
	    strlen((s)->lsb_name), (s))
#define _remove_symbol(tbl, s) do {				\
	HASH_DEL((tbl), (s));					\
	_update_export(ld, (s), 0);				\
	} while (0)
#define _resolve_symbol(_s, s) do {				\
	assert((_s) != (s));					\
	if ((s)->lsb_prev != NULL) {				\
		(s)->lsb_prev->lsb_ref = (_s);			\
		(_s)->lsb_prev = (s)->lsb_prev;			\
	}							\
	(s)->lsb_prev = (_s);					\
	(_s)->lsb_ref = (s);					\
	_update_import(ld, _s, s);				\
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

	if (ld->ld_var_symbols == NULL) {
		ld->ld_var_symbols = malloc(sizeof(*ld->ld_var_symbols));
		if (ld->ld_var_symbols == NULL)
			ld_fatal_std(ld, "malloc");
		STAILQ_INIT(ld->ld_var_symbols);
	}
	STAILQ_INSERT_TAIL(ld->ld_var_symbols, lsb, lsb_next);

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

int
ld_symbols_get_value_from_input(struct ld_input *li, char *name, uint64_t *val)
{
	struct ld_symbol *lsb;

	if ((lsb = _find_symbol_from_input(li->li_nonlocal, name)) != NULL) {
		while (lsb->lsb_ref != NULL)
			lsb = lsb->lsb_ref;
		*val = lsb->lsb_value;
		return (0);
	}

	return (-1);
}

int
ld_symbols_get_value_from_input_local(struct ld_input *li, char *name,
    uint64_t *val)
{
	struct ld_symbol *lsb;

	if ((lsb = _find_symbol_from_input(li->li_local, name)) != NULL) {
		*val = lsb->lsb_value;
		return (0);
	}

	return (-1);
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

	if (HASH_COUNT(ld->ld_symtab_undef) > 0) {
		HASH_ITER(hh, ld->ld_symtab_undef, lsb, _lsb) {
			if (lsb->lsb_bind != STB_WEAK)
				ld_warn(ld, "undefined symbol: %s", lsb->lsb_name);
		}
	}
}

void
ld_symbols_update(struct ld *ld)
{
	struct ld_input *li;
	struct ld_symbol *lsb, *_lsb;

	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		HASH_ITER(hhi, li->li_local, lsb, _lsb)
			_update_symbol(ld, lsb);
	}	

	HASH_ITER(hh, ld->ld_symtab_def, lsb, _lsb) {
		_update_symbol(ld, lsb);
	}

	HASH_ITER(hh, ld->ld_symtab_common, lsb, _lsb) {
		_update_symbol(ld, lsb);
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
	struct ld_symbol *lsb, *tmp, _lsb;

	lo = ld->ld_output;

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
		_add_to_symbol_table(ld, ld->ld_symtab, ld->ld_strtab, &_lsb);
	}

	/* Copy local symbols from each input object. */
	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		HASH_ITER(hhi, li->li_local, lsb, tmp) {
#if 0
			li = lsb->lsb_input;
			is = &li->li_is[lsb->lsb_shndx];
			if (is->is_output == NULL) {
				printf("discard symbol: %s\n", lsb->lsb_name);
				continue;
			}
#endif
			_add_to_symbol_table(ld, ld->ld_symtab, ld->ld_strtab,
			    lsb);
		}
	}

	/* Copy resolved global symbols from hash table. */
	HASH_ITER(hh, ld->ld_symtab_def, lsb, tmp) {
		_add_to_symbol_table(ld, ld->ld_symtab, ld->ld_strtab, lsb);
	}

	/* Copy undefined weak symbols. */
	HASH_ITER(hh, ld->ld_symtab_undef, lsb, tmp) {
		_add_to_symbol_table(ld, ld->ld_symtab, ld->ld_strtab, lsb);
	}

	/* Copy common symbols. */
	HASH_ITER(hh, ld->ld_symtab_common, lsb, tmp) {
		_add_to_symbol_table(ld, ld->ld_symtab, ld->ld_strtab, lsb);
	}
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
_find_symbol_from_input(struct ld_symbol *tbl, char *name)
{
	struct ld_symbol *s;

	HASH_FIND(hhi, tbl, name, strlen(name), s);
	return (s);
}

static void
_add_to_import(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;

	HASH_FIND(hhimp, ld->ld_symtab_import, lsb->lsb_longname,
	    strlen(lsb->lsb_longname), _lsb);
	if (_lsb != NULL)
		return;
	assert(_lsb == NULL);
	HASH_ADD_KEYPTR(hhimp, ld->ld_symtab_import, lsb->lsb_longname,
	    strlen(lsb->lsb_longname), lsb);
}

static void
_remove_from_import(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_symbol *_lsb;

	HASH_FIND(hhimp, ld->ld_symtab_import, lsb->lsb_longname,
	    strlen(lsb->lsb_longname), _lsb);
	if (_lsb == NULL)
		return;
	HASH_DELETE(hhimp, ld->ld_symtab_import, _lsb);
}

static void
_update_import(struct ld *ld, struct ld_symbol *_lsb, struct ld_symbol *lsb)
{
	struct ld_symbol *__lsb;

	if (_lsb->lsb_input != NULL && _lsb->lsb_input->li_type == LIT_DSO &&
	    _lsb->lsb_shndx != SHN_UNDEF)
		_remove_from_import(ld, _lsb);

	if (lsb->lsb_input != NULL && lsb->lsb_input->li_type == LIT_DSO &&
	    lsb->lsb_shndx != SHN_UNDEF) {
		for (__lsb = lsb->lsb_prev; __lsb != NULL;
		     __lsb = __lsb->lsb_prev) {
			if (__lsb->lsb_input == NULL ||
			    __lsb->lsb_input->li_type == LIT_RELOCATABLE) {
				_add_to_import(ld, lsb);
				break;
			}
		}
	}
}

static void
_update_export(struct ld *ld, struct ld_symbol *lsb, int add)
{
	struct ld_symbol *_lsb;

	if (lsb->lsb_shndx == SHN_UNDEF)
		return;

	if (lsb->lsb_input != NULL &&
	    lsb->lsb_input->li_type != LIT_RELOCATABLE)
		return;

	/* TODO: DSOs export functions. */
	if (lsb->lsb_type != STT_OBJECT || lsb->lsb_other == STV_HIDDEN)
		return;

	if (add) {
		HASH_FIND(hhexp, ld->ld_symtab_export, lsb->lsb_longname,
		    strlen(lsb->lsb_longname), _lsb);
		assert(_lsb == NULL);
		HASH_ADD_KEYPTR(hhexp, ld->ld_symtab_export, lsb->lsb_longname,
		    strlen(lsb->lsb_longname), lsb);
	} else {
		HASH_FIND(hhexp, ld->ld_symtab_export, lsb->lsb_longname,
		    strlen(lsb->lsb_longname), _lsb);
		assert(_lsb != NULL);
		HASH_DELETE(hhexp, ld->ld_symtab_export, _lsb);
	}
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
	lsb->lsb_input = li;
	lsb->lsb_ver = NULL;

	/* Find out symbol version info. */
	if (li->li_file->lf_type == LFT_DSO && li->li_vername != NULL &&
	    li->li_versym != NULL && (size_t) i < li->li_versym_sz) {
		j = li->li_versym[i];
		ndx = j & ~0x8000;
		if ((size_t) ndx < li->li_vername_sz) {
			lsb->lsb_ver = li->li_vername[ndx];
			printf("symbol: %s ver: %s\n", lsb->lsb_name,
			    lsb->lsb_ver);
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
		HASH_ADD_KEYPTR(hh, ld->ld_defver, dv->dv_name,
		    strlen(dv->dv_name), dv);
	}

	/*
	 * Insert symbol to input object internal symbol list and
	 * perform symbol resolving.
	 */
	if (lsb->lsb_bind == STB_LOCAL) {
		if (lsb->lsb_type != STT_SECTION &&
		    (lsb->lsb_type != STT_NOTYPE ||
		    lsb->lsb_shndx != SHN_UNDEF))
			_add_symbol_to_input(li->li_local, lsb);
	} else {
		_add_symbol_to_input(li->li_nonlocal, lsb);
		_resolve_and_add_symbol(ld, lsb);
	}
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
	Elf_Scn *scn, *scn_sym;
	Elf_Scn *scn_versym, *scn_verneed, *scn_verdef;
	Elf_Data *d;
	GElf_Shdr shdr;
	GElf_Sym sym;
	size_t strndx;
	int elferr, len, i;

	strndx = SHN_UNDEF;
	scn = scn_sym = scn_versym = scn_verneed = scn_verdef = NULL;
	(void) elf_errno();
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &shdr) != &shdr)
			ld_fatal(ld, "%s: gelf_getshdr failed: %s", li->li_name,
			    elf_errmsg(-1));
		if (li->li_file->lf_type == LFT_DSO) {
			if (shdr.sh_type == SHT_DYNSYM) {
				scn_sym = scn;
				strndx = shdr.sh_link;
			}
			if (shdr.sh_type == SHT_SUNW_versym)
				scn_versym = scn;
			if (shdr.sh_type == SHT_SUNW_verneed)
				scn_verneed = scn;
			if (shdr.sh_type == SHT_SUNW_verdef)
				scn_verdef = scn;
		} else {
			if (shdr.sh_type == SHT_SYMTAB) {
				scn_sym = scn;
				strndx = shdr.sh_link;
				break;
			}
		}
	}

	if (scn_sym == NULL || strndx == SHN_UNDEF)
		return;

	_load_symbol_version_info(ld, li, e, scn_versym, scn_verneed, scn_verdef);

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

	len = d->d_size / shdr.sh_entsize;
	for (i = 0; i < len; i++) {
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
	struct ld_symbol *lsb, *_lsb;

	HASH_ITER(hhi, li->li_local, lsb, _lsb) {
		HASH_DELETE(hhi, li->li_local, lsb);
		_free_symbol(lsb);
	}

	HASH_ITER(hhi, li->li_nonlocal, lsb, _lsb) {
		HASH_DELETE(hhi, li->li_nonlocal, lsb);
		_free_symbol(lsb);
	}
}

static void
_free_symbol(struct ld_symbol *lsb)
{

	free(lsb->lsb_name);
	free(lsb->lsb_longname);
	free(lsb);
}

static void
_update_symbol(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_input *li;
	struct ld_input_section *is;
	struct ld_output *lo;
	struct ld_output_section *os;

	if (lsb->lsb_shndx == SHN_ABS)
		return;

	lo = ld->ld_output;
	assert(lo != NULL);

	li = lsb->lsb_input;
	if (lsb->lsb_shndx == SHN_COMMON)
		is = &li->li_is[li->li_shnum - 1];
	else
		is = &li->li_is[lsb->lsb_shndx];
	if ((os = is->is_output) == NULL)
		return;
	lsb->lsb_value += os->os_addr + is->is_reloff;
	lsb->lsb_shndx = elf_ndxscn(os->os_scn);
#if 0
	printf("symbol %s: %#jx\n", lsb->lsb_name,
	    (uintmax_t) lsb->lsb_value);
#endif
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

static void
_free_symbol_table(struct ld_symbol_table *symtab)
{

	if (symtab == NULL)
		return;

	free(symtab->sy_buf);
	free(symtab);
}

/*
 * Symbol versioning sections are the same for 32bit and 64bit
 * ELF objects.
 */
#define Elf_Verdef	Elf32_Verdef
#define	Elf_Verdaux	Elf32_Verdaux
#define	Elf_Verneed	Elf32_Verneed
#define	Elf_Vernaux	Elf32_Vernaux

static void
_add_version_name(struct ld *ld, struct ld_input *li, int ndx,
    const char *name)
{
	int i;

	assert(name != NULL);

	if (ndx <= 1)
		return;

	if (li->li_vername == NULL) {
		li->li_vername_sz = 10;
		li->li_vername = calloc(li->li_vername_sz,
		    sizeof(*li->li_vername));
		if (li->li_vername == NULL)
			ld_fatal_std(ld, "calloc");
	}

	if ((size_t) ndx >= li->li_vername_sz) {
		li->li_vername = realloc(li->li_vername,
		    sizeof(*li->li_vername) * li->li_vername_sz * 2);
		if (li->li_vername == NULL)
			ld_fatal_std(ld, "realloc");
		for (i = li->li_vername_sz; (size_t) i < li->li_vername_sz * 2;
		     i++)
			li->li_vername[i] = NULL;
		li->li_vername_sz *= 2;
	}

	if (li->li_vername[ndx] == NULL) {
		li->li_vername[ndx] = strdup(name);
		if (li->li_vername[ndx] == NULL)
			ld_fatal_std(ld, "strdup");
	}
}

static void
_load_verneed(struct ld *ld, struct ld_input *li, Elf *e, Elf_Scn *verneed)
{
	Elf_Data *d_vn;
	Elf_Verneed *vn;
	Elf_Vernaux *vna;
	GElf_Shdr sh_vn;
	uint8_t *buf, *end, *buf2;
	char *name;
	int elferr, i;

	if (gelf_getshdr(verneed, &sh_vn) != &sh_vn)
		ld_fatal(ld, "%s: gelf_getshdr failed: %s", li->li_name,
		    elf_errmsg(-1));

	(void) elf_errno();
	if ((d_vn = elf_getdata(verneed, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "%s: elf_getdata failed: %s", li->li_name,
			    elf_errmsg(elferr));
		return;
	}
	if (d_vn->d_size == 0)
		return;

	buf = d_vn->d_buf;
	end = buf + d_vn->d_size;
	while (buf + sizeof(Elf_Verneed) <= end) {
		vn = (Elf_Verneed *) (uintptr_t) buf;
		buf2 = buf + vn->vn_aux;
		i = 0;
		while (buf2 + sizeof(Elf_Vernaux) <= end && i < vn->vn_cnt) {
			vna = (Elf32_Vernaux *) (uintptr_t) buf2;
			name = elf_strptr(e, sh_vn.sh_link,
			    vna->vna_name);
			if (name != NULL)
				_add_version_name(ld, li, (int) vna->vna_other,
				    name);
			buf2 += vna->vna_next;
			i++;
		}
		if (vn->vn_next == 0)
			break;
		buf += vn->vn_next;
	}
}

static void
_load_verdef(struct ld *ld, struct ld_input *li, Elf *e, Elf_Scn *verdef)
{
	Elf_Data *d_vd;
	Elf_Verdef *vd;
	Elf_Verdaux *vda;
	GElf_Shdr sh_vd;
	uint8_t *buf, *end, *buf2;
	char *name;
	int elferr;

	if (gelf_getshdr(verdef, &sh_vd) != &sh_vd)
		ld_fatal(ld, "%s: gelf_getshdr failed: %s", li->li_name,
		    elf_errmsg(-1));

	(void) elf_errno();
	if ((d_vd = elf_getdata(verdef, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "%s: elf_getdata failed: %s", li->li_name,
			    elf_errmsg(elferr));
		return;
	}
	if (d_vd->d_size == 0)
		return;

	buf = d_vd->d_buf;
	end = buf + d_vd->d_size;
	while (buf + sizeof(Elf_Verdef) <= end) {
		vd = (Elf_Verdef *) (uintptr_t) buf;
		buf2 = buf + vd->vd_aux;
		vda = (Elf_Verdaux *) (uintptr_t) buf2;
		name = elf_strptr(e, sh_vd.sh_link, vda->vda_name);
		if (name != NULL)
			_add_version_name(ld, li, (int) vd->vd_ndx, name);
		if (vd->vd_next == 0)
			break;
		buf += vd->vd_next;
	}		
}

static void
_load_symbol_version_info(struct ld *ld, struct ld_input *li, Elf *e,
    Elf_Scn *versym, Elf_Scn *verneed, Elf_Scn *verdef)
{
	Elf_Data *d_vs;
	int elferr;

	if (versym == NULL)
		return;

	(void) elf_errno();
	if ((d_vs = elf_getdata(versym, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "%s: elf_getdata failed: %s", li->li_name,
			    elf_errmsg(elferr));
		return;
	}
	if (d_vs->d_size == 0)
		return;

	if ((li->li_versym = malloc(d_vs->d_size)) == NULL)
		ld_fatal_std(ld, "malloc");
	memcpy(li->li_versym, d_vs->d_buf, d_vs->d_size);
	li->li_versym_sz = d_vs->d_size / sizeof(uint16_t);

	_add_version_name(ld, li, 0, "*local*");
	_add_version_name(ld, li, 1, "*global*");

	if (verneed != NULL)
		_load_verneed(ld, li, e, verneed);

	if (verdef != NULL)
		_load_verdef(ld, li, e, verdef);
}

#undef	Elf_Verdef
#undef	Elf_Verdaux
#undef	Elf_Verneed
#undef	Elf_Vernaux
