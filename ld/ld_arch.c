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
 */

#include "ld.h"
#include "ld_arch.h"
#include "amd64.h"

ELFTC_VCSID("$Id$");

void
ld_arch_init(struct ld *ld)
{

	amd64_register(ld);
}

struct ld_arch *
ld_arch_guess_arch_name(struct ld *ld, int mach)
{
	const char *arch;

	/* TODO: we should also consider elf class and endianess. */

	switch (mach) {
	case EM_386:
		arch = "i386";
		break;
	case EM_ARM:
		arch = "arm";
		break;
	case EM_MIPS:
	case EM_MIPS_RS3_LE:
		arch = "mips";
		break;
	case EM_PPC:
	case EM_PPC64:
		arch = "ppc";
		break;
	case EM_SPARC:
	case EM_SPARCV9:
		arch = "sparc";
		break;
	case EM_X86_64:
		arch = "amd64";
		break;
	default:
		return (NULL);
	}

	return (ld_arch_find(ld ,arch));
}

struct ld_arch *
ld_arch_get_arch_from_target(struct ld *ld, const char *target)
{
	struct ld_arch *la;
	char *begin, *end, name[MAX_ARCH_NAME_LEN + 1];
	size_t len;

	if ((begin = strchr(target, '-')) == NULL)
		return (NULL);
	if ((end = strrchr(target, '-')) == NULL)
		return (NULL);

	if (begin == end)
		la = ld_arch_find(ld, begin);
	else {
		len = end - begin + 1;
		if (len > MAX_ARCH_NAME_LEN)
			return (NULL);
		strncpy(name, begin, len);
		name[len] = '\0';
		la = ld_arch_find(ld, name);
	}

	return (la);
}

struct ld_arch *
ld_arch_find(struct ld *ld, const char *str)
{
	struct ld_arch *la;
	char arch[MAX_ARCH_NAME_LEN + 1];

	strncpy(arch, str, sizeof(arch) - 1);
	arch[sizeof(arch) - 1] = '\0';
	HASH_FIND_STR(ld->ld_arch, arch, la);

	return (la);
}
