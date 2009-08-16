/*-
 * Copyright (c) 2009 Kai Wang
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

#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "_libdwarf.h"

static int
lineno_add_file(Dwarf_LineInfo li, uint8_t **p, const char *compdir,
    Dwarf_Error *error)
{
	Dwarf_LineFile lf;
	uint8_t *src;
	int slen;

	src = *p;

	if ((lf = malloc(sizeof(struct _Dwarf_LineFile))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	lf->lf_fname = (char *) src;

	/* Make full pathname if need. */
	if (compdir != NULL && *lf->lf_fname != '/') {
		slen = strlen(compdir) + strlen(lf->lf_fname) + 2;
		if ((lf->lf_fullpath = malloc(slen)) == NULL) {
			free(lf);
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			return (DWARF_E_MEMORY);
		}
		snprintf(lf->lf_fullpath, slen, "%s/%s", compdir, lf->lf_fname);
	}
	src += strlen(lf->lf_fname) + 1;
	lf->lf_dirndx = decode_uleb128(&src);
	lf->lf_mtime = decode_uleb128(&src);
	lf->lf_size = decode_uleb128(&src);
	STAILQ_INSERT_TAIL(&li->li_lflist, lf, lf_next);
	li->li_lflen++;

	*p = src;

	return (DWARF_E_NONE);
}

static int
lineno_run_program(Dwarf_CU cu, Dwarf_LineInfo li, uint8_t *p, uint8_t *pe,
    const char *compdir, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	Dwarf_Line ln, tln;
	uint64_t address, file, line, column, isa, opsize;
	int is_stmt, basic_block, end_sequence;
	int prologue_end, epilogue_begin;
	int ret;

#define	RESET_REGISTERS						\
	do {							\
		address	       = 0;				\
		file	       = 1;				\
		line	       = 1;				\
		column	       = 0;				\
		is_stmt	       = li->li_defstmt;		\
		basic_block    = 0;				\
		end_sequence   = 0;				\
		prologue_end   = 0;				\
		epilogue_begin = 0;				\
	} while(0)

#define	APPEND_ROW						\
	do {							\
		ln = malloc(sizeof(struct _Dwarf_Line));	\
		if (ln == NULL) {				\
			ret = DWARF_E_MEMORY;			\
			DWARF_SET_ERROR(error, ret);		\
			goto prog_fail;				\
		}						\
		ln->ln_li     = li;				\
		ln->ln_addr   = address;			\
		ln->ln_fileno = file;				\
		ln->ln_lineno = line;				\
		ln->ln_column = column;				\
		ln->ln_bblock = basic_block;			\
		ln->ln_stmt   = is_stmt;			\
		ln->ln_endseq = end_sequence;			\
		STAILQ_INSERT_TAIL(&li->li_lnlist, ln, ln_next);\
		li->li_lnlen++;					\
	} while(0)

#define	LINE(x) (li->li_lbase + (((x) - li->li_opbase) % li->li_lrange))
#define	ADDRESS(x) ((((x) - li->li_opbase) / li->li_lrange) * li->li_minlen)

	dbg = cu->cu_dbg;

	/*
	 * Set registers to their default values.
	 */
	RESET_REGISTERS;

	/*
	 * Start line number program.
	 */
	while (p < pe) {
		if (*p == 0) {

			/*
			 * Extended Opcodes.
			 */

			p++;
			opsize = decode_uleb128(&p);
			switch (*p) {
			case DW_LNE_end_sequence:
				p++;
				end_sequence = 1;
				APPEND_ROW;
				RESET_REGISTERS;
				break;
			case DW_LNE_set_address:
				p++;
				address = dbg->decode(&p, cu->cu_pointer_size);
				break;
			case DW_LNE_define_file:
				p++;
				ret = lineno_add_file(li, &p, compdir, error);
				if (ret != DWARF_E_NONE)
					goto prog_fail;
				break;
			default:
				/* Unrecognized extened opcodes. */
				p += opsize;
			}

		} else if (*p > 0 && *p < li->li_opbase) {

			/*
			 * Standard Opcodes.
			 */

			switch (*p++) {
			case DW_LNS_copy:
				APPEND_ROW;
				basic_block = 0;
				prologue_end = 0;
				epilogue_begin = 0;
				break;
			case DW_LNS_advance_pc:
				address += decode_uleb128(&p) * li->li_minlen;
				break;
			case DW_LNS_advance_line:
				line += decode_sleb128(&p);
				break;
			case DW_LNS_set_file:
				file = decode_uleb128(&p);
				break;
			case DW_LNS_set_column:
				column = decode_uleb128(&p);
				break;
			case DW_LNS_negate_stmt:
				is_stmt = !is_stmt;
				break;
			case DW_LNS_set_basic_block:
				basic_block = 1;
				break;
			case DW_LNS_const_add_pc:
				address += ADDRESS(255);
				break;
			case DW_LNS_fixed_advance_pc:
				address += dbg->decode(&p, 2);
				break;
			case DW_LNS_set_prologue_end:
				prologue_end = 1;
				break;
			case DW_LNS_set_epilogue_begin:
				epilogue_begin = 1;
				break;
			case DW_LNS_set_isa:
				isa = decode_uleb128(&p);
				break;
			default:
				/* Unrecognized extened opcodes. What to do? */
				break;
			}

		} else {

			/*
			 * Special Opcodes.
			 */

			line += LINE(*p);
			address += ADDRESS(*p);
			APPEND_ROW;
			basic_block = 0;
			prologue_end = 0;
			epilogue_begin = 0;
			p++;
		}
	}

	return (DWARF_E_NONE);

prog_fail:

	STAILQ_FOREACH_SAFE(ln, &li->li_lnlist, ln_next, tln) {
		STAILQ_REMOVE(&li->li_lnlist, ln, _Dwarf_Line, ln_next);
		free(ln);
	}

	return (ret);

#undef	RESET_REGISTERS
#undef	APPEND_ROW
#undef	LINE
#undef	ADDRESS
}

int
lineno_init(Dwarf_Die die, uint64_t offset, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	Dwarf_CU cu;
	Dwarf_Attribute at;
	Dwarf_LineInfo li;
	Dwarf_LineFile lf, tlf;
	Elf_Data *d;
	const char *compdir;
	uint64_t length, hdroff, endoff;
	uint8_t *p;
	int dwarf_size, i, ret;

	cu = die->die_cu;
	assert(cu != NULL);

	dbg = cu->cu_dbg;
	assert(dbg != NULL);
	if (dbg->dbg_s[DWARF_debug_line].s_scn == NULL)
		return (DWARF_E_NONE);

	/*
	 * Try to find out the dir where the CU was compiled. Later we
	 * will use the dir to create full pathnames, if need.
	 */
	compdir = NULL;
	at = attr_find(die, DW_AT_comp_dir);
	if (at != NULL) {
		switch (at->at_form) {
		case DW_FORM_strp:
			compdir = at->u[1].s;
			break;
		case DW_FORM_string:
			compdir = at->u[0].s;
			break;
		default:
			break;
		}
	}

	d = dbg->dbg_s[DWARF_debug_line].s_data;

	length = dbg->read(&d, &offset, 4);
	if (length == 0xffffffff) {
		dwarf_size = 8;
		length = dbg->read(&d, &offset, 8);
	} else
		dwarf_size = 4;

	if (length > d->d_size - offset) {
		DWARF_SET_ERROR(error, DWARF_E_INVALID_LINE);
		return (DWARF_E_INVALID_LINE);
	}

	if ((li = calloc(1, sizeof(struct _Dwarf_LineInfo))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	/*
	 * Read in line number program header.
	 */
	li->li_length = length;
	endoff = offset + length;
	li->li_version = dbg->read(&d, &offset, 2); /* FIXME: verify version */
	li->li_hdrlen = dbg->read(&d, &offset, dwarf_size);
	hdroff = offset;
	li->li_minlen = dbg->read(&d, &offset, 1);
	li->li_defstmt = dbg->read(&d, &offset, 1);
	li->li_lbase = dbg->read(&d, &offset, 1);
	li->li_lrange = dbg->read(&d, &offset, 1);
	li->li_opbase = dbg->read(&d, &offset, 1);
	STAILQ_INIT(&li->li_lflist);
	STAILQ_INIT(&li->li_lnlist);
		
	if ((int)li->li_hdrlen - 5 < li->li_opbase - 1) {
		ret = DWARF_E_INVALID_LINE;
		DWARF_SET_ERROR(error, ret);
		goto fail_cleanup;
	}

	if ((li->li_oplen = malloc(li->li_opbase)) == NULL) {
		ret = DWARF_E_MEMORY;
		DWARF_SET_ERROR(error, ret);
		goto fail_cleanup;
	}

	/*
	 * Read in std opcode arg length list. Note that the first
	 * element is not used.
	 */
	for (i = 1; i < li->li_opbase; i++)
		li->li_oplen[i] = dbg->read(&d, &offset, 1);

	/*
	 * Check how many strings in the include dir string array.
	 */
	length = 0;
	p = (uint8_t *) d->d_buf + offset;
	while (*p != '\0') {
		while (*p++ != '\0')
			;
		length++;
	}

	/* Sanity check. */
	if (p - (uint8_t *)d->d_buf > (int)d->d_size) {
		ret = DWARF_E_INVALID_LINE;
		DWARF_SET_ERROR(error, ret);
		goto fail_cleanup;
	}

	if ((li->li_incdirs = malloc(length * sizeof(char *))) == NULL) {
		ret = DWARF_E_MEMORY;
		DWARF_SET_ERROR(error, ret);
		goto fail_cleanup;
	}

	/* Fill in include dir array. */
	i = 0;
	p = (uint8_t *) d->d_buf + offset;
	while (*p != '\0') {
		li->li_incdirs[i++] = (char *)p;
		while (*p++ != '\0')
			;
	}

	p++;

	/*
	 * Process file list.
	 */
	while (*p != '\0') {
		ret = lineno_add_file(li, &p, compdir, error);
		if (ret != DWARF_E_NONE)
			goto fail_cleanup;
		if (p - (uint8_t *)d->d_buf > (int)d->d_size) {
			ret = DWARF_E_INVALID_LINE;
			DWARF_SET_ERROR(error, ret);
			goto fail_cleanup;
		}
	}

	p++;

	/* Sanity check. */
	if (p - (uint8_t *)d->d_buf - hdroff != li->li_hdrlen) {
		ret = DWARF_E_INVALID_LINE;
		DWARF_SET_ERROR(error, ret);
		goto fail_cleanup;
	}

	/*
	 * Process line number program.
	 */
	ret = lineno_run_program(cu, li, p, (uint8_t *)d->d_buf + endoff,
	    compdir, error);
	if (ret != DWARF_E_NONE)
		goto fail_cleanup;

	cu->cu_lineinfo = li;

	return (DWARF_E_NONE);

fail_cleanup:

	STAILQ_FOREACH_SAFE(lf, &li->li_lflist, lf_next, tlf) {
		STAILQ_REMOVE(&li->li_lflist, lf, _Dwarf_LineFile, lf_next);
		if (lf->lf_fullpath)
			free(lf->lf_fullpath);
		free(lf);
	}

	if (li->li_oplen)
		free(li->li_oplen);
	if (li->li_incdirs)
		free(li->li_incdirs);
	free(li);

	return (ret);
}
