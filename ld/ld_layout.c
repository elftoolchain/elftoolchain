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
#include "ld_input.h"
#include "ld_output.h"
#include "ld_layout.h"
#include "ld_options.h"

ELFTC_VCSID("$Id$");

/*
 * Support routines for output section layout.
 */

static void _calc_offset(struct ld *ld);
static void _calc_output_section_offset(struct ld *ld,
    struct ld_output_section *os);
static void _insert_input_to_output(struct ld_output_section *os,
    struct ld_input_section *is, struct ld_input_section_head *islist);
static void _layout_orphan_section(struct ld *ld, struct ld_input *li);
static void _layout_output_section(struct ld *ld, struct ld_input *li,
    struct ld_script_sections_output *ldso);
static void _layout_sections(struct ld *ld, struct ld_script_sections *ldss);
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
			ld_output_create_element(ld, &lo->lo_oelist, OET_ASSERT,
			    ldc->ldc_cmd);
			break;
		case LSC_ASSIGN:
			ld_output_create_element(ld, &lo->lo_oelist, OET_ASSIGN,
			    ldc->ldc_cmd);
			break;
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

static void
_layout_sections(struct ld *ld, struct ld_script_sections *ldss)
{
	struct ld_input *li;
	struct ld_output *lo;
	struct ld_script_cmd *ldc;
	int first;

	ld_input_create_objects(ld);
	ld_output_init(ld);
	ld_output_determine_arch(ld);

	first = 1;
	lo = ld->ld_output;
	STAILQ_FOREACH(li, &ld->ld_lilist, li_next) {
		ld_input_init_sections(ld, li);
		STAILQ_FOREACH(ldc, &ldss->ldss_c, ldc_next) {
			switch (ldc->ldc_type) {
			case LSC_ASSERT:
				if (!first)
					break;
				ld_output_create_element(ld, &lo->lo_oelist,
				    OET_ASSIGN, ldc->ldc_cmd);
			case LSC_ASSIGN:
				if (!first)
					break;
				ld_output_create_element(ld, &lo->lo_oelist,
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
		first = 0;
	}
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
		os = ld_output_alloc_section(ld, ldso->ldso_name, NULL);
		new_section = 1;
	} else
		oe = STAILQ_FIRST(&os->os_e);

	STAILQ_FOREACH(ldc, &ldso->ldso_c, ldc_next) {
		switch (ldc->ldc_type) {
		case LSC_ASSERT:
			if (new_section)
				oe = ld_output_create_element(ld, &os->os_e,
				    OET_ASSERT, ldc->ldc_cmd);
			break;
		case LSC_ASSIGN:
			if (new_section)
				oe = ld_output_create_element(ld, &os->os_e,
				    OET_ASSIGN, ldc->ldc_cmd);
			break;
		case LSC_SECTIONS_OUTPUT_DATA:
			if (new_section)
				oe = ld_output_create_element(ld, &os->os_e,
				    OET_DATA, ldc->ldc_cmd);
			break;
		case LSC_SECTIONS_OUTPUT_INPUT:
			if (new_section) {
				islist = calloc(1, sizeof(*islist));
				if (islist == NULL)
					ld_fatal_std(ld, "calloc");
				STAILQ_INIT(islist);
				oe = ld_output_create_element(ld, &os->os_e,
				    OET_INPUT_SECTION_LIST, islist);
			}
			break;
		case LSC_SECTIONS_OUTPUT_KEYWORD:
			if (new_section)
				ld_output_create_element(ld, &os->os_e,
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
			assert(oe != NULL &&
			    oe->oe_type == OET_INPUT_SECTION_LIST);
			_insert_input_to_output(os, is, oe->oe_entry);
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
			_insert_input_to_output(os, is, oe->oe_entry);
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

		_os = ld_output_alloc_section(ld, is->is_name, os);
		if ((islist = calloc(1, sizeof(*islist))) == NULL)
			ld_fatal_std(ld, "calloc");
		STAILQ_INIT(islist);
		oe = ld_output_create_element(ld, &_os->os_e,
		    OET_INPUT_SECTION_LIST, islist);
		_insert_input_to_output(_os, is, oe->oe_entry);
	}
}

static void
_insert_input_to_output(struct ld_output_section *os,
    struct ld_input_section *is, struct ld_input_section_head *islist)
{

	is->is_orphan = 0;
	os->os_flags |= is->is_flags;
	if (is->is_align > os->os_align) {
		os->os_align = is->is_align;
		printf("os->os_align=%ju\n", os->os_align);
	}
	if (os->os_type == SHT_NULL)
		os->os_type = is->is_type;
	STAILQ_INSERT_TAIL(islist, is, is_next);
}

static void
_calc_offset(struct ld *ld)
{
	struct ld_state *ls;
	struct ld_output *lo;
	struct ld_output_element *oe;

	ls = &ld->ld_state;
	lo = ld->ld_output;
	ls->ls_loc_counter = 0;
	ls->ls_offset = ld_layout_calc_header_size(ld);

	STAILQ_FOREACH(oe, &lo->lo_oelist, oe_next) {
		switch (oe->oe_type) {
		case OET_ASSERT:
			/* TODO */
			break;
		case OET_ASSIGN:
			ld_script_process_assign(ld, oe->oe_entry);
			break;
		case OET_OUTPUT_SECTION:
			_calc_output_section_offset(ld, oe->oe_entry);
			break;
		default:
			break;
		}
	}
}

static void
_calc_output_section_offset(struct ld *ld, struct ld_output_section *os)
{
	struct ld_state *ls;
	struct ld_output_element *oe;
	struct ld_input_section *is;
	struct ld_input_section_head *islist;
	uint64_t addr;

	ls = &ld->ld_state;

	/*
	 * Location counter is an offset relative to the start of the
	 * section, when it's refered inside an output section descriptor.
	 */
	addr = ls->ls_loc_counter;
	ls->ls_loc_counter = 0;

	STAILQ_FOREACH(oe, &os->os_e, oe_next) {
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
			islist = oe->oe_entry;
			STAILQ_FOREACH(is, islist, is_next) {
				is->is_reloff = roundup(ls->ls_loc_counter,
				    is->is_align);
				printf("\t%s(%s): %#jx\n",
				    is->is_input->li_name,
				    is->is_name, ls->ls_loc_counter);
				ls->ls_loc_counter = is->is_reloff +
				    is->is_size;
			}
			break;
		case OET_KEYWORD:
			/* TODO */
			break;
		default:
			break;
		}
	}

	/*
	 * Properly align section vma and offset to the required section
	 * alignment.
	 */
	os->os_addr = roundup(addr, os->os_align);
	os->os_off += ls->ls_offset + (addr - os->os_addr);
	os->os_size = ls->ls_loc_counter;
	printf("layout output section %s: (off:%#jx,size:%#jx) "
	    "vma:%#jx,align:%#jx\n", os->os_name, os->os_off, os->os_size,
	    os->os_addr, os->os_align);
	ls->ls_offset = os->os_off + os->os_size;

	/* Reset location counter to the current VMA. */
	ls->ls_loc_counter = os->os_addr + os->os_size;
}
