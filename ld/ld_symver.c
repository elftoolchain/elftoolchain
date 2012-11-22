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
#include "ld_input.h"
#include "ld_symbols.h"
#include "ld_symver.h"

ELFTC_VCSID("$Id$");

/*
 * Symbol versioning sections are the same for 32bit and 64bit
 * ELF objects.
 */
#define Elf_Verdef	Elf32_Verdef
#define	Elf_Verdaux	Elf32_Verdaux
#define	Elf_Verneed	Elf32_Verneed
#define	Elf_Vernaux	Elf32_Vernaux

static void _add_version_name(struct ld *ld, struct ld_input *li, int ndx,
    const char *name);
static void _load_verdef(struct ld *ld, struct ld_input *li, Elf *e,
    Elf_Scn *verdef);
static void _load_verneed(struct ld *ld, struct ld_input *li, Elf *e,
    Elf_Scn *verneed);

void
ld_symver_load_symbol_version_info(struct ld *ld, struct ld_input *li, Elf *e,
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
