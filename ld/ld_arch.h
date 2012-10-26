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

#define	MAX_ARCH_NAME_LEN	64
#define	MAX_TARGET_NAME_LEN	128

struct ld_input_section;
struct ld_reloc_entry;

struct ld_arch {
	char name[MAX_ARCH_NAME_LEN + 1];
	char *script;
	uint64_t (*get_max_page_size)(struct ld *);
	uint64_t (*get_common_page_size)(struct ld *);
	void (*process_reloc)(struct ld *, struct ld_input_section *,
	    struct ld_reloc_entry *, uint64_t, uint8_t *);
	void (*create_pltgot)(struct ld *);
	void (*finalize_pltgot)(struct ld *);
	UT_hash_handle hh;		/* hash handle */
	struct ld_arch *alias;
};

void	ld_arch_init(struct ld *);
int	ld_arch_equal(struct ld_arch *, struct ld_arch *);
struct ld_arch *ld_arch_find(struct ld *, char *);
struct ld_arch *ld_arch_guess_arch_name(struct ld *, int);
void	ld_arch_set(struct ld *, char *);
void	ld_arch_set_from_target(struct ld *);
void	ld_arch_verify(struct ld *, const char *, int);
