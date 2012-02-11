/*-
 * Copyright (c) 2011,2012 Kai Wang
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

ELFTC_VCSID("$Id$");

/*
 * Support routines for input section handling.
 */

static off_t _offset_sort(struct ld_archive_member *a,
    struct ld_archive_member *b);

void
ld_input_create_objects(struct ld *ld)
{
	struct ld_file *lf;
	struct ld_archive_member *lam, *tmp;
	struct ld_input *li;

	TAILQ_FOREACH(lf, &ld->ld_lflist, lf_next) {
		if (lf->lf_ar != NULL) {
			HASH_SORT(lf->lf_ar->la_m, _offset_sort);
			HASH_ITER(hh, lf->lf_ar->la_m, lam, tmp) {
				li = calloc(1, sizeof(*li));
				if (li == NULL)
					ld_fatal_std(ld, "calloc");
				li->li_name = strdup(lam->lam_name);
				if (li->li_name == NULL)
					ld_fatal_std(ld, "strdup");
				li->li_moff = lam->lam_off;
				li->li_file = lf;
				STAILQ_INSERT_TAIL(&ld->ld_lilist, li, li_next);
			}
		} else {
			if ((li = calloc(1, sizeof(*li))) == NULL)
				ld_fatal_std(ld, "calloc");
			if ((li->li_name = strdup(lf->lf_name)) == NULL)
				ld_fatal_std(ld, "strdup");
			li->li_file = lf;
			STAILQ_INSERT_TAIL(&ld->ld_lilist, li, li_next);
		}
	}
}

void *
ld_input_get_section_rawdata(struct ld *ld, struct ld_input_section *is)
{
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	struct ld_input *li;
	char *buf;
	int elferr;

	li = is->is_input;
	ld_input_load(ld, li);
	e = li->li_elf;

	if ((scn = elf_getscn(e, is->is_index)) == NULL)
		ld_fatal(ld, "elf_getscn failed: %s", elf_errmsg(-1));
	if ((d = elf_rawdata(scn, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "elf_getdata failed: %s",
			    elf_errmsg(elferr));
		return (NULL);
	}
	if (d->d_buf == NULL && d->d_size == 0)
		return (NULL);
	if ((buf = malloc(d->d_size)) == NULL)
		ld_fatal_std(ld, "malloc");
	memcpy(buf, d->d_buf, d->d_size);

	ld_input_unload(ld, li);

	return (buf);
}

void
ld_input_load(struct ld *ld, struct ld_input *li)
{
	struct ld_state *ls;
	struct ld_file *lf;

	assert(li->li_elf == NULL);
	ls = &ld->ld_state;
	if (li->li_file != ls->ls_file) {
		if (ls->ls_file != NULL)
			ld_file_unload(ld, ls->ls_file);
		ld_file_load(ld, li->li_file);
	}
	lf = li->li_file;
	if (lf->lf_ar != NULL) {
		assert(li->li_moff != 0);
		if (elf_rand(lf->lf_elf, li->li_moff) != li->li_moff)
			ld_fatal(ld, "%s: elf_rand: %s", lf->lf_name,
			    elf_errmsg(-1));
		if ((li->li_elf = elf_begin(-1, ELF_C_READ, lf->lf_elf)) ==
		    NULL)
			ld_fatal(ld, "%s: elf_begin: %s", lf->lf_name,
			    elf_errmsg(-1));
	} else
		li->li_elf = lf->lf_elf;
}

void
ld_input_unload(struct ld *ld, struct ld_input *li)
{
	struct ld_file *lf;

	(void) ld;
	assert(li->li_elf != NULL);
	lf = li->li_file;
	if (lf->lf_ar != NULL)
		(void) elf_end(li->li_elf);
	li->li_elf = NULL;
}

void
ld_input_init_sections(struct ld *ld, struct ld_input *li)
{
	struct ld_input_section *is;
	Elf *e;
	Elf_Scn *scn;
	const char *name;
	GElf_Shdr sh;
	size_t shstrndx, ndx;
	int elferr;

	ld_input_load(ld, li);
	e = li->li_elf;
	if (elf_getshdrnum(e, &li->li_shnum) < 0)
		ld_fatal(ld, "%s: elf_getshdrnum: %s", li->li_name,
		    elf_errmsg(-1));

	assert(li->li_is == NULL);
	if ((li->li_is = calloc(li->li_shnum, sizeof(*is))) == NULL)
		ld_fatal_std(ld, "%s: calloc: %s", li->li_name);

	if (elf_getshdrstrndx(e, &shstrndx) < 0)
		ld_fatal(ld, "%s: elf_getshdrstrndx: %s", li->li_name,
		    elf_errmsg(-1));

	(void) elf_errno();
	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) != &sh)
			ld_fatal(ld, "%s: gelf_getshdr: %s", li->li_name,
			    elf_errmsg(-1));

		if ((name = elf_strptr(e, shstrndx, sh.sh_name)) == NULL)
			ld_fatal(ld, "%s: elf_strptr: %s", li->li_name,
			    elf_errmsg(-1));

		if ((ndx = elf_ndxscn(scn)) == SHN_UNDEF)
			ld_fatal(ld, "%s: elf_ndxscn: %s", li->li_name,
			    elf_errmsg(-1));

		if (ndx >= li->li_shnum)
			ld_fatal(ld, "%s: section index of '%s' section is"
			    " invalid", li->li_name, name);

		is = &li->li_is[ndx];
		if ((is->is_name = strdup(name)) == NULL)
			ld_fatal_std(ld, "%s: calloc", li->li_name);
		is->is_off = sh.sh_offset;
		is->is_size = sh.sh_size;
		is->is_align = sh.sh_addralign;
		is->is_type = sh.sh_type;
		is->is_flags = sh.sh_flags;
		is->is_input = li;
		is->is_orphan = 1;
	}
	elferr = elf_errno();
	if (elferr != 0)
		ld_fatal(ld, "%s: elf_nextscn failed: %s", li->li_name,
		    elf_errmsg(elferr));

	ld_input_unload(ld, li);
}

static off_t
_offset_sort(struct ld_archive_member *a, struct ld_archive_member *b)
{

	return (a->lam_off - b->lam_off);
}
