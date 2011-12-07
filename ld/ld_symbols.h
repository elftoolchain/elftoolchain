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

struct ld_symbol {
	char *lsb_name;			/* symbol name */
	uint64_t lsb_size;		/* symbol size */
	uint64_t lsb_value;		/* symbol value */
	uint16_t lsb_shndx;		/* symbol index */
	struct ld_script_variable *lsb_var; /* associated ldscript variable */
	unsigned char lsb_bind;		/* symbol binding */
	unsigned char lsb_type;		/* symbol type */
	unsigned char lsb_other;	/* symbol visibility */
	unsigned char lsb_provide;	/* provide symbol */
	UT_hash_handle hh;		/* hash handle */
};

void	ld_symbols_add_extern(struct ld *, char *);
void	ld_symbols_add_variable(struct ld *, struct ld_script_variable *,
    unsigned, unsigned);
void	ld_symbols_resolve(struct ld *);
