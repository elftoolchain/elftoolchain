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

struct ld_symbol {
	char *lsb_name;			/* symbol name */
	char *lsb_ver;			/* symbol version */
	uint64_t lsb_size;		/* symbol size */
	uint64_t lsb_value;		/* symbol value */
	uint16_t lsb_shndx;		/* symbol index */
	struct ld_script_variable *lsb_var; /* associated ldscript variable */
	unsigned char lsb_bind;		/* symbol binding */
	unsigned char lsb_type;		/* symbol type */
	unsigned char lsb_other;	/* symbol visibility */
	unsigned char lsb_provide;	/* provide symbol */
	unsigned char lsb_provide_refed; /* provide symbol is referenced */
	struct ld_symbol *lsb_ref;	/* symbol reference */
	struct ld_input *lsb_input;	/* containing input object */
	UT_hash_handle hh;		/* hash handle */
	STAILQ_ENTRY(ld_symbol) lsb_next; /* next symbol */
};

STAILQ_HEAD(ld_symbol_head, ld_symbol);

struct ld_symbol_table {
	void *sy_buf;
	size_t sy_cap;
	size_t sy_size;
	size_t sy_first_nonlocal;
};

void	ld_symbols_add_extern(struct ld *, char *);
void	ld_symbols_add_variable(struct ld *, struct ld_script_variable *,
    unsigned, unsigned);
void	ld_symbols_build_symtab(struct ld *);
void	ld_symbols_cleanup(struct ld *);
int	ld_symbols_get_value(struct ld *, char *, uint64_t *);
int	ld_symbols_get_value_local(struct ld_input *, char *, uint64_t *);
void	ld_symbols_resolve(struct ld *);
void	ld_symbols_update(struct ld *);
