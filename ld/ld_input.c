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
#include "ld_symbols.h"

ELFTC_VCSID("$Id$");

/*
 * Support routines for input section handling.
 */

static off_t _offset_sort(struct ld_archive_member *a,
    struct ld_archive_member *b);

void
ld_input_cleanup(struct ld *ld)
{
	struct ld_input *li, *_li;
	int i;

	STAILQ_FOREACH_SAFE(li, &ld->ld_lilist, li_next, _li) {
		STAILQ_REMOVE(&ld->ld_lilist, li, ld_input, li_next);
		if (li->li_versym)
			free(li->li_versym);
		if (li->li_vername) {
			for (i = 0; (size_t) i < li->li_vername_sz; i++)
				if (li->li_vername[i])
					free(li->li_vername[i]);
			free(li->li_vername);
		}
		if (li->li_fullname)
			free(li->li_fullname);
		free(li->li_name);
		free(li);
	}
}

struct ld_input *
ld_input_alloc(struct ld *ld, struct ld_file *lf, const char *name)
{
	struct ld_input *li;

	if ((li = calloc(1, sizeof(*li))) == NULL)
		ld_fatal_std(ld, "calloc");

	if ((li->li_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");

	/*
	 * TODO: Do not allocate memory for symbo lists if ld(1) is
	 * going to strip the symbol table.
	 */

	if ((li->li_local = malloc(sizeof(*li->li_local))) == NULL)
		ld_fatal_std(ld, "malloc");

	if ((li->li_nonlocal = malloc(sizeof(*li->li_nonlocal))) == NULL)
		ld_fatal_std(ld, "malloc");

	STAILQ_INIT(li->li_local);
	STAILQ_INIT(li->li_nonlocal);

	li->li_file = lf;

	return (li);
}

char *
ld_input_get_fullname(struct ld *ld, struct ld_input *li)
{
	struct ld_archive_member *lam;
	size_t len;

	if (li->li_fullname != NULL)
		return (li->li_fullname);

	if (li->li_lam == NULL)
		return (li->li_name);

	lam = li->li_lam;
	len = strlen(lam->lam_ar_name) + strlen(lam->lam_name) + 3;
	if ((li->li_fullname = malloc(len)) == NULL)
		ld_fatal_std(ld, "malloc");
	snprintf(li->li_fullname, len, "%s(%s)", lam->lam_ar_name,
	    lam->lam_name);

	return  (li->li_fullname);
}

void
ld_input_link_objects(struct ld *ld)
{
	struct ld_file *lf;
	struct ld_archive_member *lam, *tmp;
	struct ld_input *li;

	TAILQ_FOREACH(lf, &ld->ld_lflist, lf_next) {
		if (lf->lf_ar != NULL) {
			HASH_SORT(lf->lf_ar->la_m, _offset_sort);
			HASH_ITER(hh, lf->lf_ar->la_m, lam, tmp) {
				li = lam->lam_input;
				if (li != NULL)
					STAILQ_INSERT_TAIL(&ld->ld_lilist, li,
					    li_next);
			}
		} else {
			li = lf->lf_input;
			if (li != NULL)
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
	e = li->li_elf;
	assert(e != NULL);

	if ((scn = elf_getscn(e, is->is_index)) == NULL)
		ld_fatal(ld, "elf_getscn failed: %s", elf_errmsg(-1));

	(void) elf_errno();
	if ((d = elf_rawdata(scn, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			ld_fatal(ld, "elf_rawdata failed: %s",
			    elf_errmsg(elferr));
		return (NULL);
	}

	if (d->d_buf == NULL || d->d_size == 0)
		return (NULL);

	if ((buf = malloc(d->d_size)) == NULL)
		ld_fatal_std(ld, "malloc");

	memcpy(buf, d->d_buf, d->d_size);

	return (buf);
}

void
ld_input_load(struct ld *ld, struct ld_input *li)
{
	struct ld_state *ls;
	struct ld_file *lf;
	struct ld_archive_member *lam;

	assert(li->li_elf == NULL);
	ls = &ld->ld_state;
	if (li->li_file != ls->ls_file) {
		if (ls->ls_file != NULL)
			ld_file_unload(ld, ls->ls_file);
		ld_file_load(ld, li->li_file);
	}
	lf = li->li_file;
	if (lf->lf_ar != NULL) {
		assert(li->li_lam != NULL);
		lam = li->li_lam;
		if (elf_rand(lf->lf_elf, lam->lam_off) != lam->lam_off)
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
	struct ld_symbol *lsb, *tmp;
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

	/* Allocate one more pseudo section to hold common symbols */
	li->li_shnum++;

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

		if (ndx >= li->li_shnum - 1)
			ld_fatal(ld, "%s: section index of '%s' section is"
			    " invalid", li->li_name, name);

		is = &li->li_is[ndx];
		if ((is->is_name = strdup(name)) == NULL)
			ld_fatal_std(ld, "%s: calloc", li->li_name);
		is->is_off = sh.sh_offset;
		is->is_size = sh.sh_size;
		is->is_entsize = sh.sh_entsize;
		is->is_addr = sh.sh_addr;
		is->is_align = sh.sh_addralign;
		is->is_type = sh.sh_type;
		is->is_flags = sh.sh_flags;
		is->is_link = sh.sh_link;
		is->is_info = sh.sh_info;
		is->is_index = elf_ndxscn(scn);
		is->is_input = li;
		is->is_orphan = 1;

		/*
		 * Check for informational sections which should not
		 * be included in the output object, process them
		 * and mark them as discarded if need.
		 */

		if (strcmp(is->is_name, ".note.GNU-stack") == 0) {
			ld->ld_gen_gnustack = 1;
			if (!ld->ld_stack_exec_set)
				ld->ld_stack_exec |= is->is_size;
			is->is_discard = 1;
		}
	}
	elferr = elf_errno();
	if (elferr != 0)
		ld_fatal(ld, "%s: elf_nextscn failed: %s", li->li_name,
		    elf_errmsg(elferr));

	/*
	 * Create a pseudo section named COMMON to keep track of common symbols.
	 * Go through the common symbols hash table, if there are common symbols
	 * belonging to this input object, increase the size of the pseudo COMMON
	 * section accordingly.
	 */

	is = &li->li_is[li->li_shnum - 1];
	if ((is->is_name = strdup("COMMON")) == NULL)
		ld_fatal_std(ld, "%s: calloc", li->li_name);
	is->is_off = 0;
	is->is_size = 0;
	is->is_entsize = 1;
	is->is_align = 1;
	is->is_type = SHT_NOBITS;
	is->is_flags = SHF_ALLOC | SHF_WRITE;
	is->is_link = 0;
	is->is_info = 0;
	is->is_index = SHN_COMMON;
	is->is_input = li;
	is->is_orphan = 1;
	HASH_ITER(hh, ld->ld_symtab_common, lsb, tmp) {
		if (lsb->lsb_input != li)
			continue;
#if 0
		printf("add common symbol %s to %s\n", lsb->lsb_name,
		    li->li_name);
#endif
		if (lsb->lsb_size > is->is_align)
			is->is_align = lsb->lsb_size;
		lsb->lsb_value = is->is_size;
		is->is_size += lsb->lsb_size;
	}

	ld_input_unload(ld, li);
}

static off_t
_offset_sort(struct ld_archive_member *a, struct ld_archive_member *b)
{

	return (a->lam_off - b->lam_off);
}
