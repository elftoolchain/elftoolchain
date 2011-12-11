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

struct ld_input_section {
	char *is_name;			/* section name */
	struct ld_input *is_input;	/* containing input object */
	uint64_t is_off;		/* section file offset */
	uint64_t is_size;		/* section file size */
	uint64_t is_align;		/* section align */
	uint64_t is_type;		/* section type */
	uint64_t is_flags;		/* section flags */
	unsigned is_orphan;		/* orphan section */
	STAILQ_ENTRY(ld_input_section) is_next; /* next section */
};

STAILQ_HEAD(ld_input_section_head, ld_input_section);

struct ld_input {
	char *li_name;			/* input object name */
	struct ld_file *li_file;	/* containing file */
	size_t li_shnum;		/* num of sections in ELF object */
	struct ld_input_section *li_is;	/* input section list */
	off_t li_moff;			/* archive member offset */
	STAILQ_ENTRY(ld_input) li_next;	/* next input object */
};

enum ld_output_section_part_type {
	OSPT_ASSIGN,
	OSPT_DATA,
	OSPT_INPUT,
	OSPT_KEYWORD,
};

struct ld_output_section_part {
	enum ld_output_section_part_type osp_type; /* section part type */
	union {
		struct ld_script_assign *osp_a; /* symbol assignment */
		struct ld_input_section_head osp_i; /* input section list */
		struct ld_script_sections_output_data *osp_d;
					/* output section data */
		enum ld_script_sections_output_keywords osp_k;
					/* output section keywords */
	} osp_u;
	STAILQ_ENTRY(ld_output_section_part) osp_next; /* next part */
};

STAILQ_HEAD(ld_output_section_part_head, ld_output_section_part);

struct ld_output_section {
	char *os_name;			/* output section name */
	uint64_t os_off;		/* output section offset */
	uint64_t os_size;		/* output section size */
	uint64_t os_align;		/* output section alignment */
	struct ld_output_section_part_head os_p; /* output section parts */
	STAILQ_ENTRY(ld_output_section) os_next; /* next output section */
	UT_hash_handle hh;		/* hash handle */
};
