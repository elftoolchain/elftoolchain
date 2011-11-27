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
 *
 * $Id$
 */

struct ld_wildcard {
	char *lw_name;			/* wildcard */
	unsigned lw_sort;		/* sort mode */
};

struct ld_input_section {
	char *is_name;			/* section name */
	struct ld_file *is_file;	/* containing file */
	uint64_t is_off;		/* section file offset */
	uint64_t is_align;		/* section align */
};

struct ld_output_section_command {
	struct ld_wildcard *osc_ar;	/* archive name */
	struct ld_wildcard *osc_file;	/* file/member name */
	struct ld_wildcard *osc_sec;	/* section name */
	int osc_keep;			/* keep section */
	STAILQ_ENTRY(ld_output_section_command) osc_next;
};

struct ld_output_section {
	char *os_name;			/* section name */
	uint64_t os_vma;		/* section virtual address */
	uint64_t os_lma;		/* section load address */
	uint64_t os_align;		/* section align */
	STAILQ_HEAD(, ld_input_descriptor) os_c; /* command list */
};
