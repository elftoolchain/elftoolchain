/*-
 * Copyright (c) 2010,2011 Kai Wang
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
#include "uthash.h"
#include "_elftc.h"

struct ld_file;
struct ld_path;
struct ld_symbol;

#define	LD_MAX_NESTED_GROUP	16

struct ld_state {
	/*
	 * State variables for command line options parsing stage.
	 */
	Elftc_Bfd_Target *ls_itgt;	/* input bfd target set by -b */
	unsigned ls_static;		/* use static library */
	unsigned ls_whole_archive;	/* include whole archive */
	unsigned ls_as_needed;		/* DT_NEEDED */
	unsigned ls_group_level;	/* archive group level */
	unsigned ls_extracted[LD_MAX_NESTED_GROUP + 1];
					/* extracted from archive group */
	unsigned ls_search_dir;		/* search library directories */
	STAILQ_HEAD(, ld_path) ls_lplist; /* search path list */
};

struct ld {
	const char *ld_progname;	/* ld(1) program name */
	char *ld_output;		/* output file name */
	char *ld_entry;			/* entry point set by -e */
	char *ld_scp_entry;		/* entry point set by linker script */
	struct ld_script *ld_scp;	/* linker script */
	struct ld_state ld_ls;		/* linker state */
	struct ld_symbol *ld_symtab_def;/* defined symbols */
	struct ld_symbol *ld_symtab_undef; /* undefined symbols */
	unsigned ld_common_alloc;	/* always alloc space for common sym */
	unsigned ld_common_no_alloc;	/* never alloc space for common sym */
	TAILQ_HEAD(ld_file_head, ld_file) ld_lflist; /* input file list */
};

void	ld_err(struct ld *, const char *, ...);
void	ld_fatal(struct ld *, const char *, ...);
void	ld_fatal_std(struct ld *, const char *, ...);
void	ld_layout_sections(struct ld *);
void	ld_options_parse(struct ld*, int, char **);
void	ld_script_parse(const char *);
void	ld_script_parse_internal(void);
void	ld_warn(struct ld *, const char *, ...);
