/*-
 * Copyright (c) 2011 Joseph Koshy
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

#include <libelf.h>
#include <gelf.h>

#include "elfts.h"
#include "tet_api.h"

include(`elfts.m4')

IC_REQUIRES_VERSION_INIT();

/*
 * Find an ELF section with the given name.
 */
static Elf_Scn *
findscn(Elf *e, const char *name)
{
	size_t shstrndx;
	const char *scn_name;
	Elf_Scn *scn;
	GElf_Shdr shdr;

	/* Locate the string table. */
	if (elf_getshdrstrndx(e, &shstrndx) != 0)
		return (NULL);

	/* Find a section with a matching name. */
	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &shdr) == NULL)
			return (NULL);
		if ((scn_name = elf_strptr(e, shstrndx,
		    (size_t) shdr.sh_name)) == NULL)
			return (NULL);
		if (strcmp(scn_name, name) == 0)
			return (scn);
	}

	return (NULL);
}

define(`ZEROSECTION',".zerosection")
undefine(`FN')
define(`FN',`
void
tcZeroSection$1$2(void)
{
	Elf *e;
	int error, fd, result;
	Elf_Scn *scn;
	Elf_Data *ed;

	e = NULL;
	fd = -1;
	result = TET_UNRESOLVED;

	TP_ANNOUNCE("a data descriptor for a zero sized section is correctly retrieved");

	_TS_OPEN_FILE(e, "zerosection.$1$2", ELF_C_READ, fd, goto done;);

	if ((scn = findscn(e, ZEROSECTION)) == NULL) {
		TP_UNRESOLVED("Cannot find section \""ZEROSECTION"\"");
		goto done;
	}

	ed = NULL;
	if ((ed = elf_getdata(scn, ed)) == NULL) {
		error = elf_errno();
		TP_FAIL("elf_getdata failed %d \"%s\"", error,
		    elf_errmsg(error));
		goto done;
	}

	if (ed->d_size != 0 || ed->d_buf != NULL) {
		TP_FAIL("Illegal values returned: size %d buf %p",
		    (int) ed->d_size, (void *) ed->d_buf);
		goto done;
	}

	result = TET_PASS;

done:
	if (e)
		elf_end(e);
	if (fd != -1)
		(void) close(fd);
	tet_result(result);
}
')

FN(lsb,32)
FN(lsb,64)
FN(msb,32)
FN(msb,64)
