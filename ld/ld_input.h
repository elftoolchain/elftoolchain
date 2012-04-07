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
 *
 * $Id$
 */

struct ld_input_section {
	char *is_name;			/* section name */
	struct ld_input *is_input;	/* containing input object */
	struct ld_output_section *is_output; /* containing output section */
	uint64_t is_off;		/* section file offset */
	uint64_t is_reloff;		/* relative offset in output section */
	uint64_t is_size;		/* section file size */
	uint64_t is_entsize;		/* seciton entry size */
	uint64_t is_align;		/* section align */
	uint64_t is_type;		/* section type */
	uint64_t is_flags;		/* section flags */
	uint64_t is_link;		/* section link */
	uint64_t is_info;		/* section info */
	uint64_t is_index;		/* section index */
	unsigned is_orphan;		/* orphan section */
	unsigned is_discard;		/* dicard section */
	void *is_data;			/* output section data descriptor */
	struct ld_reloc_entry_head *is_reloc; /* reloc list */
	STAILQ_ENTRY(ld_input_section) is_next; /* next section */
};

STAILQ_HEAD(ld_input_section_head, ld_input_section);

struct ld_input {
	char *li_name;			/* input object name */
	Elf *li_elf;			/* input object ELF descriptor */
	struct ld_file *li_file;	/* containing file */
	size_t li_shnum;		/* num of sections in ELF object */
	struct ld_input_section *li_is;	/* input section list */
	off_t li_moff;			/* archive member offset */
	struct ld_symbol_head *li_local; /* local symbol list */
	struct ld_symbol_head *li_nonlocal; /* non-local symbol list */
	STAILQ_ENTRY(ld_input) li_next;	/* next input object */
};

struct ld_input *ld_input_alloc(struct ld *, struct ld_file *, const char *);
void	*ld_input_get_section_rawdata(struct ld *, struct ld_input_section *);
void	ld_input_init_sections(struct ld *, struct ld_input *);
void	ld_input_link_objects(struct ld *);
void	ld_input_load(struct ld *, struct ld_input *);
void	ld_input_unload(struct ld *, struct ld_input *);
