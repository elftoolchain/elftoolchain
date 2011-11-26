/*-
 * Copyright (c) 2010,2011 Kai Wang
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
#include "ld_symbols.h"

ELFTC_VCSID("$Id$");

static void ld_symbols_add_symbol(struct ld *ld, struct ld_file *lf,
    Elf *e, GElf_Sym *sym, size_t strndx);
static void ld_symbols_load(struct ld *ld, struct ld_file *lf);
static void ld_symbols_load_archive(struct ld *ld, struct ld_file *lf);
static void ld_symbols_load_elf(struct ld *ld, struct ld_file *lf, Elf *e);
static void ld_symbols_warn_muldef(struct ld *ld, struct ld_symbol *lsb,
    GElf_Sym *sym);
static int _archive_member_extracted(struct ld_archive *la, off_t off);
static void _extract_archive_member(struct ld *ld, struct ld_file *lf,
    struct ld_archive *la, off_t off);
static struct ld_symbol *_symbol_find(struct ld_symbol *tbl, char *name);

void
ld_symbols_resolve(struct ld *ld)
{
	struct ld_file *lf;
	struct ld_symbol *lsb;

	TAILQ_FOREACH(lf, &ld->ld_lflist, lf_next){
		ld_symbols_load(ld, lf);
	}

	if (HASH_COUNT(ld->ld_symtab_undef) > 0) {
		for (lsb = ld->ld_symtab_undef; lsb != NULL;
		     lsb = lsb->hh.next) {
			ld_warn(ld, "undefined symbol: %s", lsb->lsb_name);
		}
		exit(1);
	}
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
ld_symbols_add_symbol(struct ld *ld, struct ld_file *lf, Elf *e, GElf_Sym *sym,
    size_t strndx)
{
	struct ld_symbol *lsb, *_lsb;
	char *name;

	(void) lf;

	if ((name = elf_strptr(e, strndx, sym->st_name)) == NULL)
		return;

	if (GELF_ST_BIND(sym->st_info) == STB_LOCAL)
		return;

	printf("process symbol: %s\n", name);

	if (sym->st_shndx == SHN_UNDEF) {
		if (_symbol_find(ld->ld_symtab_def, name) != NULL)
			return;
		if (_symbol_find(ld->ld_symtab_undef, name) != NULL)
			return;
	} else {
		if ((_lsb = _symbol_find(ld->ld_symtab_def, name)) != NULL) {
			ld_symbols_warn_muldef(ld, _lsb, sym);
			return;
		}
		if ((_lsb = _symbol_find(ld->ld_symtab_undef, name)) != NULL) {
			_symbol_remove(ld->ld_symtab_undef, _lsb);
			printf("resolved symbol: %s\n", _lsb->lsb_name);
			_symbol_add(ld->ld_symtab_def, _lsb);
			return;
		}
	}

	if ((lsb = calloc(1, sizeof(*lsb))) == NULL)
		ld_fatal_std(ld, "calloc");

	lsb->lsb_name = strdup(name);
	if (lsb->lsb_name == NULL)
		ld_fatal_std(ld, "strdup");
	lsb->lsb_size = sym->st_size;

	if (sym->st_shndx == SHN_UNDEF) {
		_symbol_add(ld->ld_symtab_undef, lsb);
	} else
		_symbol_add(ld->ld_symtab_def, lsb);
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

	assert(la->la_elf != NULL);

	if (elf_rand(la->la_elf, off) == 0)
		ld_fatal(ld, "%s: elf_rand failed: %s", lf->lf_name,
		    elf_errmsg(-1));

	if ((e = elf_begin(-1, ELF_C_READ, la->la_elf)) == NULL)
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
	printf("extracted member %s from archive %s\n", lam->lam_name,
	    lf->lf_name);

	/* Load the symbols of this member. */
	ld_symbols_load_elf(ld, lf, e);

	elf_end(e);
}

static void
ld_symbols_load_archive(struct ld *ld, struct ld_file *lf)
{
	struct ld_archive *la;
	Elf_Arsym *as;
	size_t c;
	int extracted, i;

	assert(lf != NULL && lf->lf_type == LFT_ARCHIVE);
	assert(lf->lf_ar != NULL);

	printf("load_archive: %s\n", lf->lf_name);
	la = lf->lf_ar;
	if ((as = elf_getarsym(la->la_elf, &c)) == NULL)
		ld_fatal(ld, "%s: elf_getarsym failed: %s", lf->lf_name,
		    elf_errmsg(-1));
	do {
		extracted = 0;
		for (i = 0; (size_t) i < c; i++) {
			if (as[i].as_name == NULL)
				break;
			if (_archive_member_extracted(la, as[i].as_off))
				continue;
			/* printf("as_name=%s\n", as[i].as_name); */
			if (_symbol_find(ld->ld_symtab_undef, as[i].as_name) !=
			    NULL) {
				_extract_archive_member(ld, lf, la,
				    as[i].as_off);
				extracted = 1;
			}
		}
	} while (extracted);
}

static void
ld_symbols_load_elf(struct ld *ld, struct ld_file *lf, Elf *e)
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
			ld_fatal(ld, "%s: gelf_getshdr failed: %s", lf->lf_name,
			    elf_errmsg(-1));
		if (shdr.sh_type == SHT_SYMTAB)
			scn_sym = scn;
		else if (shdr.sh_type == SHT_STRTAB)
			strndx = elf_ndxscn(scn);
	}
	elferr = elf_errno();
	if (elferr != 0)
		ld_fatal(ld, "%s: elf_nextscn failed: %s", lf->lf_name,
		    elf_errmsg(elferr));
	if (scn_sym == NULL || strndx == SHN_UNDEF)
		return;

	if (gelf_getshdr(scn_sym, &shdr) != &shdr)
		ld_fatal(ld, "%s: gelf_getshdr failed: %s", lf->lf_name,
		    elf_errmsg(-1));

	(void) elf_errno();
	if ((d = elf_getdata(scn_sym, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "%s: elf_getdata failed: %s", lf->lf_name,
			    elf_errmsg(elferr));
		/* Empty .symtab section? */
		return;
	}

	len = d->d_size / shdr.sh_entsize;
	for (i = 0; i < len; i++) {
		if (gelf_getsym(d, i, &sym) != &sym)
			ld_fatal(ld, "%s: gelf_getsym failed: %s", lf->lf_name,
			    elf_errmsg(-1));
		ld_symbols_add_symbol(ld, lf, e, &sym, strndx);
	}
}

static void
ld_symbols_load(struct ld *ld, struct ld_file *lf)
{

	if (lf->lf_type == LFT_ARCHIVE)
		ld_symbols_load_archive(ld, lf);
	else
		ld_symbols_load_elf(ld, lf, lf->lf_elf);
}

static void
ld_symbols_warn_muldef(struct ld *ld, struct ld_symbol *lsb, GElf_Sym *sym)
{

	(void) sym;

	ld_warn(ld, "multiple definition of symbol %s", lsb->lsb_name);
}
