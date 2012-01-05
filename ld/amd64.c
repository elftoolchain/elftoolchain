/*-
 * Copyright (c) 2012 Kai Wang
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

#include "ld.h"
#include "ld_arch.h"
#include "amd64.h"

ELFTC_VCSID("$Id$");

static uint64_t _get_max_page_size(struct ld *ld);
static uint64_t _get_common_page_size(struct ld *ld);

static struct ld_arch amd64 = {
	.arch_name = "amd64",
	.get_max_page_size = _get_max_page_size,
	.get_common_page_size = _get_common_page_size,
};

static uint64_t
_get_max_page_size(struct ld *ld)
{

	(void) ld;
	return (0x100000);
}

static uint64_t
_get_common_page_size(struct ld *ld)
{

	(void) ld;
	return (0x1000);
}

void
amd64_register(struct ld *ld)
{

	HASH_ADD_KEYPTR(hh, ld->ld_arch, amd64.arch_name,
	    strlen(amd64.arch_name), &amd64);
}
