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
#include "ld_layout.h"
#include "ld_script.h"

ELFTC_VCSID("$Id$");

/*
 * Support routines for output section layout.
 */

static void _layout_output_section(struct ld *ld, struct ld_file *lf,
    struct ld_script_sections_output *ldso);
static void _layout_sections(struct ld *ld, struct ld_script_sections *ldss);
static off_t _calc_header_size(struct ld *ld);

void
ld_layout_sections(struct ld *ld)
{
	struct ld_script *lds;
	struct ld_script_cmd *ldc;
	off_t header_size;
	int sections_cmd_exist;

	lds = ld->ld_scp;

	sections_cmd_exist = 0;
	STAILQ_FOREACH(ldc, &lds->lds_c, ldc_next) {
		switch (ldc->ldc_type) {
		case LSC_ASSERT:
			break;
		case LSC_ENTRY:
			break;
		case LSC_SECTIONS:
			if (sections_cmd_exist)
				ld_fatal(ld, "found multiple SECTIONS commands"
				    " in the linker script");
			_layout_sections(ld, ldc->ldc_cmd);
			break;
		default:
			break;
		}
	}
	if (!sections_cmd_exist)
		_layout_sections(ld, NULL);

	header_size = _calc_header_size(ld);
}

static off_t
_calc_header_size(struct ld *ld)
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
	struct ld_file *lf;
	struct ld_script_cmd *ldc;

	TAILQ_FOREACH(lf, &ld->ld_lflist, lf_next) {
		ld_file_load(ld, lf);
		STAILQ_FOREACH(ldc, &ldss->ldss_c, ldc_next) {
			switch (ldc->ldc_type) {
			case LSC_ENTRY:
				break;
			case LSC_ASSIGN:
				break;
			case LSC_SECTIONS_OUTPUT:
				_layout_output_section(ld, lf, ldc->ldc_cmd);
			case LSC_SECTIONS_OVERLAY:
				break;
			default:
				break;
			}
		}
		ld_file_unload(ld, lf);
	}
}

static void
_layout_output_section(struct ld *ld, struct ld_file *lf,
    struct ld_script_sections_output *ldso)
{

	(void) ld;
	(void) lf;
	(void) ldso;
}
