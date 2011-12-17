/*-
 * Copyright (c) 2011 Kai Wang
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
#include "ld_script.h"
#include "ld_layout.h"
#include "ld_options.h"

ELFTC_VCSID("$Id$");

/*
 * Support routines for output section layout.
 */

static void _calc_offset(struct ld *ld);
static void _create_input_objects(struct ld *ld);
static struct ld_output_element *_create_output_element(struct ld *ld,
    struct ld_output_element_head *head, enum ld_output_element_type type,
    void *entry);
static void _layout_orphan_section(struct ld *ld, struct ld_input *li);
static void _layout_output_section(struct ld *ld, struct ld_input *li,
    struct ld_script_sections_output *ldso);
static void _layout_sections(struct ld *ld, struct ld_script_sections *ldss);
static void _load_input_sections(struct ld *ld, struct ld_input *li);
static off_t _offset_sort(struct ld_archive_member *a,
    struct ld_archive_member *b);
static int _wildcard_match(struct ld_wildcard *lw, const char *string);
static int _wildcard_list_match(struct ld_script_list *list,
    const char *string);

void
ld_layout_sections(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_script *lds;
	struct ld_script_cmd *ldc;
	struct ld_state *ls;
	int sections_cmd_exist;

	ls = &ld->ld_state;
	lo = ld->ld_output;
	lds = ld->ld_scp;

	sections_cmd_exist = 0;
	STAILQ_FOREACH(ldc, &lds->lds_c, ldc_next) {
		switch (ldc->ldc_type) {
		case LSC_ASSERT:
			_create_output_element(ld, &lo->lo_oelist, OET_ASSERT,
			    ldc->ldc_cmd);
			break;
		case LSC_ASSIGN:
			_create_output_element(ld, &lo->lo_oelist, OET_ASSIGN,
			    ldc->ldc_cmd);
		case LSC_ENTRY:
			/* TODO */
			break;
		case LSC_SECTIONS:
			if (sections_cmd_exist)
				ld_fatal(ld, "found multiple SECTIONS commands"
				    " in the linker script");
			sections_cmd_exist = 1;
			_layout_sections(ld, ldc->ldc_cmd);
			break;
		default:
			break;
		}
	}
	if (!sections_cmd_exist)
		_layout_sections(ld, NULL);

	_calc_offset(ld);
}

off_t
ld_layout_calc_header_size(struct ld *ld)
{
	struct ld_script_phdr *ldsp;
	off_t header_size;
	unsigned ec;
	size_t num_phdrs;

	header_size = 0;

	ec = elftc_bfd_target_class(ld->ld_otgt);
	if (ec == ELFCLASS32)
		header_size += sizeof(Elf32_Ehdr);
	else
		header_size += sizeof(Elf64_Ehdr);

	if (!STAILQ_EMPTY(&ld->ld_scp->lds_p)) {
		num_phdrs = 0;
		STAILQ_FOREACH(ldsp, &ld->ld_scp->lds_p, ldsp_next)
			num_phdrs++;
	} else {
		/*
		 * TODO: depending on different output file type, ld(1)
		 * generate different number of segements.
		 */
		num_phdrs = 4;
	}

	if (ec == ELFCLASS32)
		header_size += num_phdrs * sizeof(Elf32_Phdr);
	else
		header_size += num_phdrs * sizeof(Elf64_Phdr);

	return (header_size);
}

static off_t
_offset_sort(struct ld_archive_member *a, struct ld_archive_member *b)
{

	return (a->lam_off - b->lam_off);
}

static void
_create_input_objects(struct ld *ld)
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

static void
_load_input_sections(struct ld *ld, struct ld_input *li)
{
	struct ld_input_section *is;
	struct ld_file *lf;
	Elf *e;
	Elf_Scn *scn;
	const char *name;
	GElf_Shdr sh;
	size_t shstrndx, ndx;
	int elferr;

	lf = li->li_file;
	if (lf->lf_ar != NULL) {
		assert(li->li_moff != 0);
		if (elf_rand(lf->lf_elf, li->li_moff) != li->li_moff)
			ld_fatal(ld, "%s: elf_rand: %s", lf->lf_name,
			    elf_errmsg(-1));
		if ((e = elf_begin(-1, ELF_C_READ, lf->lf_elf)) == NULL)
			ld_fatal(ld, "%s: elf_begin: %s", lf->lf_name,
			    elf_errmsg(-1));
	} else
		e = lf->lf_elf;

	if (elf_getshdrnum(e, &li->li_shnum) < 0)
		ld_fatal(ld, "%s: elf_getshdrnum: %s", lf->lf_name,
		    elf_errmsg(-1));

	assert(li->li_is == NULL);
	if ((li->li_is = calloc(li->li_shnum, sizeof(*is))) == NULL)
		ld_fatal_std(ld, "%s: calloc: %s", lf->lf_name);

	if (elf_getshdrstrndx(e, &shstrndx) < 0)
		ld_fatal(ld, "%s: elf_getshdrstrndx: %s", lf->lf_name,
		    elf_errmsg(-1));

	(void) elf_errno();
	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) != &sh)
			ld_fatal(ld, "%s: gelf_getshdr: %s", lf->lf_name,
			    elf_errmsg(-1));

		if ((name = elf_strptr(e, shstrndx, sh.sh_name)) == NULL)
			ld_fatal(ld, "%s: elf_strptr: %s", lf->lf_name,
			    elf_errmsg(-1));

		if ((ndx = elf_ndxscn(scn)) == SHN_UNDEF)
			ld_fatal(ld, "%s: elf_ndxscn: %s", lf->lf_name,
			    elf_errmsg(-1));

		if (ndx >= li->li_shnum)
			ld_fatal(ld, "%s: section index of '%s' section is"
			    " invalid", lf->lf_name, name);

		is = &li->li_is[ndx];
		if ((is->is_name = strdup(name)) == NULL)
			ld_fatal_std(ld, "%s: calloc", lf->lf_name);
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
		ld_fatal(ld, "%s: elf_nextscn failed: %s", lf->lf_name,
		    elf_errmsg(elferr));

	if (lf->lf_ar != NULL)
		(void) elf_end(e);
}

static struct ld_output_element *
_create_output_element(struct ld *ld, struct ld_output_element_head *head,
    enum ld_output_element_type type, void *entry)
{
	struct ld_output_element *oe;

	if ((oe = calloc(1, sizeof(*oe))) == NULL)
		ld_fatal_std(ld, "calloc");
	oe->oe_type = type;
	oe->oe_entry = entry;
	STAILQ_INSERT_TAIL(head, oe, oe_next);

	return (oe);
}

static void
_layout_sections(struct ld *ld, struct ld_script_sections *ldss)
{
	struct ld_file *lf;
	struct ld_input *li;
	struct ld_output *lo;
	struct ld_script_cmd *ldc;

	_create_input_objects(ld);

	lo = ld->ld_output;
	lf = NULL;
	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		if (lf == NULL || li->li_file != lf) {
			if (lf != NULL)
				ld_file_unload(ld, lf);
			ld_file_load(ld, li->li_file);
			lf = li->li_file;
		}
		_load_input_sections(ld, li);
		STAILQ_FOREACH(ldc, &ldss->ldss_c, ldc_next) {
			switch (ldc->ldc_type) {
			case LSC_ASSERT:
				_create_output_element(ld, &lo->lo_oelist,
				    OET_ASSIGN, ldc->ldc_cmd);
			case LSC_ASSIGN:
				_create_output_element(ld, &lo->lo_oelist,
				    OET_ASSIGN, ldc->ldc_cmd);
				break;
			case LSC_ENTRY:
				/* TODO */
				break;
			case LSC_SECTIONS_OUTPUT:
				_layout_output_section(ld, li, ldc->ldc_cmd);
			case LSC_SECTIONS_OVERLAY:
				/* TODO */
				break;
			default:
				break;
			}
		}
		_layout_orphan_section(ld, li);
	}
	if (lf != NULL)
		ld_file_unload(ld, lf);
}

static int
_wildcard_match(struct ld_wildcard *lw, const char *string)
{

	return (fnmatch(lw->lw_name, string, 0) == 0);
}

static int
_wildcard_list_match(struct ld_script_list *list, const char *string)
{
	struct ld_script_list *ldl;

	for (ldl = list; ldl != NULL; ldl = ldl->ldl_next)
		if (_wildcard_match(ldl->ldl_entry, string))
			return (1);

	return (0);
}

static struct ld_output_section *
_alloc_output_section(struct ld *ld, const char *name,
    struct ld_output_section *after)
{
	struct ld_output *lo;
	struct ld_output_section *os;

	lo = ld->ld_output;
	if ((os = calloc(1, sizeof(*os))) == NULL)
		ld_fatal_std(ld, "calloc");
	if ((os->os_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");
	STAILQ_INIT(&os->os_e);
	HASH_ADD_KEYPTR(hh, lo->lo_ostbl, os->os_name, strlen(os->os_name), os);
	if (after == NULL)
		STAILQ_INSERT_TAIL(&lo->lo_oslist, os, os_next);
	else
		STAILQ_INSERT_AFTER(&lo->lo_oslist, os, after, os_next);
	_create_output_element(ld, &lo->lo_oelist, OET_OUTPUT_SECTION, os);

	return (os);
}

static void
_layout_output_section(struct ld *ld, struct ld_input *li,
    struct ld_script_sections_output *ldso)
{
	struct ld_file *lf;
	struct ld_output *lo;
	struct ld_script_cmd *ldc;
	struct ld_script_sections_output_input *ldoi;
	struct ld_input_section *is;
	struct ld_input_section_head *islist;
	struct ld_output_element *oe;
	struct ld_output_section *os;
	int i, new_section;

	lf = li->li_file;
	lo = ld->ld_output;
	new_section = 0;
	oe = NULL;
	HASH_FIND_STR(lo->lo_ostbl, ldso->ldso_name, os);
	if (os == NULL) {
		os = _alloc_output_section(ld, ldso->ldso_name, NULL);
		new_section = 1;
	} else
		oe = STAILQ_FIRST(&os->os_e);

	STAILQ_FOREACH(ldc, &ldso->ldso_c, ldc_next) {
		switch (ldc->ldc_type) {
		case LSC_ASSERT:
			if (new_section)
				oe = _create_output_element(ld, &os->os_e,
				    OET_ASSERT, ldc->ldc_cmd);
			break;
		case LSC_ASSIGN:
			if (new_section)
				oe = _create_output_element(ld, &os->os_e,
				    OET_ASSIGN, ldc->ldc_cmd);
			break;
		case LSC_SECTIONS_OUTPUT_DATA:
			if (new_section)
				oe = _create_output_element(ld, &os->os_e,
				    OET_DATA, ldc->ldc_cmd);
			break;
		case LSC_SECTIONS_OUTPUT_INPUT:
			if (new_section) {
				islist = calloc(1, sizeof(*islist));
				if (islist == NULL)
					ld_fatal_std(ld, "calloc");
				STAILQ_INIT(islist);
				oe = _create_output_element(ld, &os->os_e,
				    OET_INPUT_SECTION_LIST, islist);
			}
			break;
		case LSC_SECTIONS_OUTPUT_KEYWORD:
			if (new_section)
				_create_output_element(ld, &os->os_e,
				    OET_KEYWORD, ldc->ldc_cmd);
			break;
		default:
			ld_fatal(ld, "internal: invalid output section "
			    "command: %d", ldc->ldc_type);
		}
		if (ldc->ldc_type != LSC_SECTIONS_OUTPUT_INPUT)
			goto next_output_cmd;

		ldoi = ldc->ldc_cmd;

		if (ldoi->ldoi_ar != NULL && li->li_moff != 0 &&
		    !_wildcard_match(ldoi->ldoi_ar, lf->lf_name))
			goto next_output_cmd;

		assert(ldoi->ldoi_file != NULL);
		if (!_wildcard_match(ldoi->ldoi_file, li->li_name))
			goto next_output_cmd;

		if (ldoi->ldoi_exclude != NULL &&
		    _wildcard_list_match(ldoi->ldoi_exclude, li->li_name))
			goto next_output_cmd;

		assert(ldoi->ldoi_sec != NULL);
		for (i = 1; (size_t) i < li->li_shnum; i++) {
			is = &li->li_is[i];
			if (!is->is_orphan)
				continue;
			if (!_wildcard_list_match(ldoi->ldoi_sec, is->is_name))
				continue;
			if (strcmp(os->os_name, "/DISCARD/") == 0) {
				is->is_discard = 1;
				continue;
			}
			is->is_orphan = 0;
			os->os_flags |= is->is_flags;
			assert(oe != NULL &&
			    oe->oe_type == OET_INPUT_SECTION_LIST);
			islist = oe->oe_entry;
			STAILQ_INSERT_TAIL(islist, is, is_next);
		}

	next_output_cmd:
		assert(oe != NULL);
		if (!new_section)
			oe = STAILQ_NEXT(oe, oe_next);			
	}
}

static void
_layout_orphan_section(struct ld *ld, struct ld_input *li)
{
	struct ld_input_section *is;
	struct ld_input_section_head *islist;
	struct ld_output *lo;
	struct ld_output_element *oe;
	struct ld_output_section *os, *_os;
	int i;

	/*
	 * Layout the input sections that are not listed in the output
	 * section descriptor in the linker script.
	 */

	lo = ld->ld_output;
	for (i = 1; (size_t) i < li->li_shnum; i++) {
		is = &li->li_is[i];

		if (!is->is_orphan || is->is_discard)
			continue;

		if (strcmp(is->is_name, ".shstrtab") == 0 ||
		    strcmp(is->is_name, ".symtab") == 0 ||
		    strcmp(is->is_name, ".strtab") == 0)
			continue;

		HASH_FIND_STR(lo->lo_ostbl, is->is_name, os);
		if (os != NULL) {
			oe = STAILQ_FIRST(&os->os_e);
			assert(oe != NULL &&
			    oe->oe_type == OET_INPUT_SECTION_LIST);
			islist = oe->oe_entry;
			STAILQ_INSERT_TAIL(islist, is, is_next);
			continue;
		}

		STAILQ_FOREACH(os, &lo->lo_oslist, os_next) {
			if ((os->os_flags & SHF_ALLOC) !=
			    (is->is_flags & SHF_ALLOC))
				continue;
			if (os->os_flags == is->is_flags) {
				_os = STAILQ_NEXT(os, os_next);
				if (_os == NULL ||
				    _os->os_flags != is->is_flags)
					break;
			}
			_os = STAILQ_NEXT(os, os_next);
			if (_os == NULL &&
			    (_os->os_flags & SHF_ALLOC) !=
			    (is->is_flags & SHF_ALLOC))
				break;
		}

		_os = _alloc_output_section(ld, is->is_name, os);
		_os->os_flags = is->is_flags;

		if ((islist = calloc(1, sizeof(*islist))) == NULL)
			ld_fatal_std(ld, "calloc");
		STAILQ_INIT(islist);
		oe = _create_output_element(ld, &_os->os_e,
		    OET_INPUT_SECTION_LIST, islist);
		STAILQ_INSERT_TAIL(islist, is, is_next);
	}
}

static void
_calc_offset(struct ld *ld)
{
	struct ld_state *ls;
	struct ld_output *lo;
	struct ld_output_element *oe;

	ls = &ld->ld_state;
	lo = ld->ld_output;

	STAILQ_FOREACH(oe, &lo->lo_oelist, oe_next) {
		switch (oe->oe_type) {
		case OET_ASSERT:
			/* TODO */
			break;
		case OET_ASSIGN:
			ld_script_process_assign(ld, oe->oe_entry);
			break;
		case OET_DATA:
			/* TODO */
			break;
		case OET_INPUT_SECTION_LIST:
			/* TODO */
			break;
		case OET_KEYWORD:
			/* TODO */
			break;
		case OET_OUTPUT_SECTION:
			/* TODO */
			break;
		default:
			break;
		}
	}

}
