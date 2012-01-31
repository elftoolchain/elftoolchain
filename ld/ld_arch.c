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
	char arch[MAX_ARCH_NAME_LEN + 1];

	/* TODO: we should also consider elf class and endianess. */

	switch (mach) {
	case EM_386:
		snprintf(arch, sizeof(arch), "%s", "i386");
		break;
	case EM_ARM:
		snprintf(arch, sizeof(arch), "%s", "arm");
		break;
	case EM_MIPS:
	case EM_MIPS_RS3_LE:
		snprintf(arch, sizeof(arch), "%s", "mips");
		break;
	case EM_PPC:
	case EM_PPC64:
		snprintf(arch, sizeof(arch), "%s", "ppc");
		break;
	case EM_SPARC:
	case EM_SPARCV9:
		snprintf(arch, sizeof(arch), "%s", "sparc");
		break;
	case EM_X86_64:
		snprintf(arch, sizeof(arch), "%s", "amd64");
		break;
	default:
		return (NULL);
	}

	return (ld_arch_find(ld, arch));
}

struct ld_arch *
ld_arch_get_arch_from_target(struct ld *ld, char *target)
{
	struct ld_arch *la;
	char *begin, *end, name[MAX_TARGET_NAME_LEN + 1];

	if ((begin = strchr(target, '-')) == NULL) {
		la = ld_arch_find(ld, target);
		return (la);
	}
	la = ld_arch_find(ld, begin + 1);
	if (la != NULL)
		return (la);

	strncpy(name, begin + 1, sizeof(name) - 1);
	name[sizeof(name) - 1] = '\0';
	while ((end = strrchr(name, '-')) != NULL) {
		*end = '\0';
		la = ld_arch_find(ld, name);
		if (la != NULL)
			return (la);
	}

	return (NULL);
}

struct ld_arch *
ld_arch_find(struct ld *ld, char *arch)
{
	struct ld_arch *la;

	HASH_FIND_STR(ld->ld_arch, arch, la);

	return (la);
}
