/*-
 * Copyright (c) 2010-2012 Kai Wang
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

#include <sys/cdefs.h>
#include <sys/mman.h>
#include <sys/param.h>
#include <sys/queue.h>
#include <sys/stat.h>
#include <ar.h>
#include <assert.h>
#include <dirent.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <fnmatch.h>
#include <gelf.h>
#include <inttypes.h>
#include <libelftc.h>
#include <libgen.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#define oom() ld_fatal(ld, "out of memory")
#include "utarray.h"
#define uthash_fatal(msg) ld_fatal(ld, msg)
#include "uthash.h"
#include "_elftc.h"

struct ld_file;
struct ld_path;
struct ld_symbol;
struct ld_symbol_head;
struct ld_output_data_buffer;
struct ld_wildcard_match;

#define	LD_MAX_NESTED_GROUP	16

struct ld_state {
	Elftc_Bfd_Target *ls_itgt;	/* input bfd target set by -b */
	struct ld_file *ls_file;	/* current open file */
	unsigned ls_static;		/* use static library */
	unsigned ls_whole_archive;	/* include whole archive */
	unsigned ls_as_needed;		/* DT_NEEDED */
	unsigned ls_group_level;	/* archive group level */
	unsigned ls_extracted[LD_MAX_NESTED_GROUP + 1];
					/* extracted from archive group */
	unsigned ls_search_dir;		/* search library directories */
	uint64_t ls_loc_counter;	/* location counter */
	uint64_t ls_offset;		/* current output section file offset */
	STAILQ_HEAD(, ld_path) ls_lplist; /* search path list */
	unsigned ls_arch_conflict;	/* input arch conflict with output */
	unsigned ls_first_elf_object;	/* first ELF object to process */
	unsigned ls_rerun;		/* ld(1) restarted */
	unsigned ls_archive_mb_header;	/* extracted list header printed */
};

struct ld {
	const char *ld_progname;	/* ld(1) program name */
	struct ld_arch *ld_arch;	/* arch-specific callbacks */
	struct ld_arch *ld_arch_list;	/* list of supported archs */
	Elftc_Bfd_Target *ld_otgt;	/* default output format */
	Elftc_Bfd_Target *ld_otgt_be;	/* big-endian output format */
	Elftc_Bfd_Target *ld_otgt_le;	/* little-endian output format */
	char *ld_otgt_name;		/* output format name */
	char *ld_otgt_be_name;		/* big-endian output format name */
	char *ld_otgt_le_name;		/* little-endian output format name */
	struct ld_output *ld_output;	/* output object */
	char *ld_output_file;		/* output file name */
	char *ld_entry;			/* entry point set by -e */
	char *ld_scp_entry;		/* entry point set by linker script */
	char *ld_interp;		/* dynamic linker */
	struct ld_script *ld_scp;	/* linker script */
	struct ld_state ld_state;	/* linker state */
	struct ld_strtab *ld_shstrtab;	/* section name table */
	struct ld_symbol_head *ld_ext_symbols; /* -u/EXTERN symbols */
	struct ld_symbol_head *ld_var_symbols; /* ldscript var symbols */
	struct ld_symbol *ld_symtab_def;/* hash for defined symbols */
	struct ld_symbol *ld_symtab_undef; /* hash for undefined symbols */
	struct ld_symbol *ld_symtab_common; /* hash for common symbols */
	struct ld_symbol *ld_symtab_import; /* hash for import symbols */
	struct ld_symbol *ld_symtab_export; /* hash for export symbols */
	struct ld_symbol_defver *ld_defver; /* default version table */
	struct ld_symbol_table *ld_symtab; /* .symtab symbol table */
	struct ld_strtab *ld_strtab;	/* .strtab string table */
	struct ld_symbol_table *ld_dynsym; /* .dynsym symbol table */
	struct ld_strtab *ld_dynstr;	/* .dynstr string table */
	struct ld_symbol_head *ld_dyn_symbols; /* dynamic symbol list */
	struct ld_wildcard_match *ld_wm; /* wildcard hash table */
	unsigned ld_common_alloc;	/* always alloc space for common sym */
	unsigned ld_common_no_alloc;	/* never alloc space for common sym */
	unsigned ld_emit_reloc;		/* emit relocations */
	unsigned ld_gen_gnustack;	/* generate PT_GNUSTACK */
	unsigned ld_print_linkmap;	/* print link map */
	unsigned ld_stack_exec;		/* stack executable */
	unsigned ld_stack_exec_set;	/* stack executable override */
	STAILQ_HEAD(ld_input_head, ld_input) ld_lilist; /* input object list */
	TAILQ_HEAD(ld_file_head, ld_file) ld_lflist; /* input file list */
};

void	ld_err(struct ld *, const char *, ...);
void	ld_fatal(struct ld *, const char *, ...);
void	ld_fatal_std(struct ld *, const char *, ...);
void	ld_warn(struct ld *, const char *, ...);
