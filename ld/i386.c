/*-
 * Copyright (c) 2012 Kai Wang
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
#include "ld_arch.h"
#include "ld_input.h"
#include "ld_output.h"
#include "ld_reloc.h"
#include "ld_utils.h"
#include "i386.h"

ELFTC_VCSID("$Id$");

static uint64_t _get_max_page_size(struct ld *ld);
static uint64_t _get_common_page_size(struct ld *ld);
static void _process_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_reloc_entry *lre, uint64_t s, uint8_t *buf);

static uint64_t
_get_max_page_size(struct ld *ld)
{

	(void) ld;
	return (0x1000);
}

static uint64_t
_get_common_page_size(struct ld *ld)
{

	(void) ld;
	return (0x1000);
}

static void
_process_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_reloc_entry *lre, uint64_t s, uint8_t *buf)
{
	struct ld_output *lo;
	uint32_t p;
	int32_t a;

	lo = ld->ld_output;
	assert(lo != NULL);

	p = lre->lre_offset + is->is_output->os_addr + is->is_reloff;
	READ_32(buf + lre->lre_offset, a);

	switch (lre->lre_type) {
	case R_386_NONE:
		break;
	case R_386_32:
		WRITE_32(buf + lre->lre_offset, (uint32_t)s + a);
		break;
	case R_386_PC32:
		WRITE_32(buf + lre->lre_offset, (uint32_t)s + a - p);
		break;
	default:
		ld_fatal(ld, "Relocation %d not supported", lre->lre_type);
		break;
	}
}

void
i386_register(struct ld *ld)
{
	struct ld_arch *i386_arch;

	if ((i386_arch = calloc(1, sizeof(*i386_arch))) == NULL)
		ld_fatal_std(ld, "calloc");
	
	snprintf(i386_arch->name, sizeof(i386_arch->name), "%s", "i386");

	i386_arch->script = i386_script;
	i386_arch->get_max_page_size = _get_max_page_size;
	i386_arch->get_common_page_size = _get_common_page_size;
	i386_arch->process_reloc = _process_reloc;

	HASH_ADD_STR(ld->ld_arch_list, name, i386_arch);
}
