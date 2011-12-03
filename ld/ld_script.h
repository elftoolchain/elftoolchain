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

enum ld_script_cmd_type {
	LSS_ASSERT,
	LSS_ASSIGN,
	LSS_AS_NEEDED,
	LSS_ENTRY,
	LSS_EXTERN,
	LSS_FCA,
	LSS_HIDDEN_ASSIGN,
	LSS_ICA,
	LSS_INPUT,
	LSS_MEMORY,
	LSS_NOCROSSREFS,
	LSS_OUTPUT,
	LSS_OUTPUT_ARCH,
	LSS_OUTPUT_FORMAT,
	LSS_PHDRS,
	LSS_PROVIDE_ASSIGN,
	LSS_REGION_ALIAS,
	LSS_SEARCH_DIR,
	LSS_SECTIONS,
	LSS_SECTIONS_OUTPUT,
	LSS_SECTIONS_OUTPUT_DATA_BYTE,
	LSS_SECTIONS_OUTPUT_DATA_FILL,
	LSS_SECTIONS_OUTPUT_DATA_LONG,
	LSS_SECTIONS_OUTPUT_DATA_QUAD,
	LSS_SECTIONS_OUTPUT_DATA_SHORT,
	LSS_SECTIONS_OUTPUT_DATA_SQUAD,
	LSS_SECTIONS_OUTPUT_INPUT,
	LSS_SECTIONS_OUTPUT_KEYWORD,
	LSS_SECTIONS_OVERLAY,
	LSS_STARTUP,
	LSS_TARGET,
	LSS_VERSION,
};

struct ld_script_cmd {
	enum ld_script_cmd_type ldc_type; /* ldscript cmd type */
	void *ldc_cmd;			/* ldscript cmd */
	STAILQ_ENTRY(ld_script_cmd) ldc_next; /* next cmd */
};

struct ld_script_list {
	void *ldl_entry;		/* list entry */
	struct ld_script_list *ldl_next; /* next entry */
};

struct ld_script_assert {
	struct ld_exp *lda_exp;		/* expression to assert */
	char *lda_msg;			/* assertion message */
};

struct ld_script_assign {
	char *lda_name;			/* symbol name */
	struct ld_exp *lda_exp;		/* expression */
	unsigned lda_provide;		/* provide */
	unsigned lda_hidden;		/* hidden provide */
};

struct ld_script_nocrossref {
	struct ld_script_list *ldn_l;	/* nocrossref sections */
	STAILQ_ENTRY(ld_script_nocrossref) *ldn_next; /* next nocrossref */
};

struct ld_script_region {
	char *ldsr_name;		/* memory region name */
	unsigned ldsr_attr;		/* memory region attribute */
	struct ld_exp *ldsr_origin;	/* memroy region start address */
	struct ld_exp *ldsr_len;	/* memroy region length */
	STAILQ_ENTRY(ld_script_region) ldsr_next; /* next memory region */
};

struct ld_script_region_alias {
	char *ldra_alias;		/* memory region alias name */
	struct ld_script_region *ldra_region; /* memory region */
	STAILQ_ENTRY(ld_script_region_alias); /* next region alias */
};

struct ld_script_phdr {
	char *ldsp_name;		/* phdr name */
	char *ldsp_type;		/* phdr type */
	unsigned ldsp_filehdr;		/* FILEHDR keyword */
	unsigned ldsp_phdrs;		/* PHDRS keyword */
	struct ld_exp *ldsp_addr;	/* segment address */
	unsigned ldsp_flags;		/* segment flags */
	STAILQ_ENTRY(ld_script_phdr) ldsp_next; /* next phdr */
};

struct ld_script_sections_output_input {
	struct ld_wildcard *ldoi_ar;	/* archive name */
	struct ld_wildcard *ldio_file;	/* file/member name */
	struct ld_script_list *ldio_exclude; /* exclude file list */
	struct ld_script_list *ldio_sec; /* section name list */
	unsigned ldoi_flags;		/* input section flags */
	unsigned ldoi_keep;		/* keep input section */
};

struct ld_script_sections_output {
	char *ldso_name;		/* output section name */
	unsigned ldso_type;		/* output section type */
	uint64_t ldso_vma;		/* output section vma */
	uint64_t ldso_lma;		/* output section lma */
	uint64_t ldso_align;		/* output section align */
	uint64_t ldso_subalign;		/* output sectino subalign */
	unsigned ldso_constraint;	/* output section constraint */
	struct ld_script_region *ldso_region; /* output section region */
	struct ld_script_region *ldso_lma_region; /* output section lma region */
	char *ldso_phdr;		/* output section segment */
	struct ld_exp *ldso_fill;	/* output section fill exp */
	STAILQ_HEAD(, ld_script_cmd) ldso_c; /* output section cmd list */
};

struct ld_script_sections_overlay_section {
	char *ldos_name;		/* overlay section name */
	char *ldos_phdr;		/* overlay section segment */
	struct ld_exp *ldos_fill;	/* overlay section fill exp */
	STAILQ_ENTRY(ld_script_sections_overlay_section) ldos_next;
					/* next section */
};

struct ld_script_sections_overlay {
	uint64_t ldso_vma;		/* overlay vma */
	uint64_t ldso_lma;		/* overlay lma */
	unsigned nocrossref;		/* no corss-ref between sections */
	struct ld_script_region *ldso_region; /* overlay region */
	char *ldso_phdr;		/* overlay segment */
	struct ld_exp *ldso_fill;	/* overlay fill exp */
	STAILQ_HEAD(, ld_script_setions_overlay_section) ldso_s;
					/* overlay section list */
};

struct ld_script_sections {
	STAILQ_HEAD(, ld_script_cmd) ldss_c; /* section cmd list */
};

struct ld_script {
	STAILQ_HEAD(, ld_script_phdr) lds_p; /* phdr table */
	STAILQ_HEAD(, ld_script_region_alias) lds_a; /* region aliases list */
	STAILQ_HEAD(, ld_script_region) lds_r; /* memory region list */
	STAILQ_HEAD(, ld_script_nocrossref) lds_n; /* nocrossref list */
	STAILQ_HEAD(, ld_script_cmd) lds_c; /* other ldscript cmd list */
};

void	ld_script_assert(struct ld *, struct ld_exp *, char *);
void	ld_script_cmd(struct ld *, enum ld_script_cmd_type, void *);
void	ld_script_group(struct ld *, struct ld_script_list *);
void	ld_script_init(struct ld *);
void	ld_script_input(struct ld *, struct ld_script_list *);
struct ld_script_list *ld_script_list(struct ld *, struct ld_script_list *,
    void *);
void	ld_script_list_free(struct ld_script_list *);
struct ld_script_list *ld_script_list_reverse(struct ld_script_list *);
