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

enum ld_output_element_type {
	OET_ASSERT,
	OET_ASSIGN,
	OET_DATA,
	OET_INPUT_SECTION_LIST,
	OET_KEYWORD,
	OET_OUTPUT_SECTION,
	OET_OVERLAY,
};

struct ld_output_element {
	enum ld_output_element_type oe_type; /* output element type */
	uint64_t oe_off;		/* output element offset */
	void *oe_entry;			/* output element */
	unsigned char oe_insec;		/* element inside SECTIONS */
	STAILQ_ENTRY(ld_output_element) oe_next; /* next element */
};

STAILQ_HEAD(ld_output_element_head, ld_output_element);

struct ld_output_section {
	char *os_name;			/* output section name */
	uint64_t os_addr;		/* output section vma */
	uint64_t os_off;		/* output section offset */
	uint64_t os_size;		/* output section size */
	uint64_t os_align;		/* output section alignment */
	uint64_t os_flags;		/* output section flags */
	struct ld_output_element_head os_e; /* output section elements */
	STAILQ_ENTRY(ld_output_section) os_next; /* next output section */
	UT_hash_handle hh;		/* hash handle */
};

STAILQ_HEAD(ld_output_section_head, ld_output_section);

struct ld_output {
	int lo_fd;			 /* output file descriptor */
	int lo_ec;			 /* output object elf class */
	int lo_endian;			 /* outout object endianess */
	int lo_osabi;			 /* output object osabi */
	struct ld_output_element_head lo_oelist; /* output element list */
	struct ld_output_section_head lo_oslist; /* output section list */
	struct ld_output_section *lo_ostbl; /* output section hash table */
};

struct ld_output_section *ld_output_alloc_section(struct ld *, const char *,
    struct ld_output_section *);
struct ld_output_element *ld_output_create_element(struct ld *,
    struct ld_output_element_head *, enum ld_output_element_type, void *);
void	ld_output_determine_arch(struct ld *);
void	ld_output_format(struct ld *, char *, char *, char *);
void	ld_output_init(struct ld *);
void	ld_output_verify_arch(struct ld *, struct ld_input *);
void	ld_output_write(struct ld *);
