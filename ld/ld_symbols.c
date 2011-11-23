/*-
 * Copyright (c) 2010 Kai Wang
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
    GElf_Sym *sym, size_t strndx);
static void ld_symbols_load(struct ld *ld, struct ld_file *lf);
static void ld_symbols_warn_muldef(struct ld *ld, struct ld_symbol *lsb,
    GElf_Sym *sym);

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

static void
ld_symbols_add_symbol(struct ld *ld, struct ld_file *lf, GElf_Sym *sym,
    size_t strndx)
{
	struct ld_symbol *lsb, *lsb0;
	char *name;

	(void) lf;

	if ((name = elf_strptr(lf->lf_elf, strndx, sym->st_name)) == NULL)
		return;

	if (GELF_ST_BIND(sym->st_info) == STB_LOCAL)
		return;

	printf("process symbol: %s\n", name);

	if (sym->st_shndx == SHN_UNDEF) {
		HASH_FIND_STR(ld->ld_symtab_def, name, lsb0);
		if (lsb0 != NULL)
			return;
		HASH_FIND_STR(ld->ld_symtab_undef, name, lsb0);
		if (lsb0 != NULL)
			return;
	} else {
		HASH_FIND_STR(ld->ld_symtab_def, name, lsb0);
		if (lsb0 != NULL) {
			ld_symbols_warn_muldef(ld, lsb0, sym);
			return;
		}
		HASH_FIND_STR(ld->ld_symtab_undef, name, lsb0);
		if (lsb0 != NULL) {
			HASH_DEL(ld->ld_symtab_undef, lsb0);
			printf("resolved symbol: %s\n", lsb0->lsb_name);
			HASH_ADD_KEYPTR(hh, ld->ld_symtab_def, lsb0->lsb_name,
			    strlen(lsb0->lsb_name), lsb0);
			return;
		}
	}

	if ((lsb = calloc(1, sizeof(*lsb))) == NULL)
		ld_fatal_std(ld, "calloc failed");

	lsb->lsb_name = name;
	lsb->lsb_size = sym->st_size;

	if (sym->st_shndx == SHN_UNDEF)
		HASH_ADD_KEYPTR(hh, ld->ld_symtab_undef, lsb->lsb_name,
		    strlen(lsb->lsb_name), lsb);
	else
		HASH_ADD_KEYPTR(hh, ld->ld_symtab_def, lsb->lsb_name,
		    strlen(lsb->lsb_name), lsb);
}

static void
ld_symbols_load(struct ld *ld, struct ld_file *lf)
{
	Elf_Scn *scn, *scn_sym;
	Elf_Data *d;
	GElf_Shdr shdr;
	GElf_Sym sym;
	size_t strndx;
	int elferr, len, i;

	assert(lf != NULL && lf->lf_elf != NULL);

	strndx = SHN_UNDEF;
	scn = scn_sym = NULL;
	(void) elf_errno();
	while ((scn = elf_nextscn(lf->lf_elf, scn)) != NULL) {
		if (gelf_getshdr(scn, &shdr) != &shdr)
			ld_fatal(ld, "%s: gelf_getshdr failed: %s", lf->lf_name,
			    elf_errmsg(elf_errno()));
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
			    elf_errmsg(elf_errno()));
		ld_symbols_add_symbol(ld, lf, &sym, strndx);
	}
}

static void
ld_symbols_warn_muldef(struct ld *ld, struct ld_symbol *lsb, GElf_Sym *sym)
{

	(void) sym;

	ld_warn(ld, "multiple definition of symbol %s", lsb->lsb_name);
}
