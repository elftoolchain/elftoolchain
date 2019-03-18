/*-
 * Copyright (c) 2019 Joseph Koshy
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

#include <sys/types.h>
#include <sys/stat.h>

#include <ar.h>
#include <libelf.h>
#include <unistd.h>

#include "elfts.h"
#include "tet_api.h"

IC_REQUIRES_VERSION_INIT();

include(`elfts.m4')

/*
 * The following definition should match that in `./Makefile'.
 */
define(`TP_ARFILE',`"a.ar"')

/*
 * The use of an offset less than SARMAG should fail.
 */
void
tcSeekBelowSarmag(void)
{
	Elf *ar;
	off_t offset;
	int error, fd, result;

	fd = -1;
	ar = NULL;
	result = TET_UNRESOLVED;

	TP_CHECK_INITIALIZATION();
	TP_ANNOUNCE("elf_rand() fails for an offset less than SARMAG");

	TS_OPEN_FILE(ar, TP_ARFILE, ELF_C_READ, fd);

	result = TET_PASS;

	if ((offset = elf_rand(ar, 1)) != 0) {
		TP_FAIL("elf_rand() succeeded with offset=%lld",
		    (unsigned long long) offset);
	} else if ((error = elf_errno()) != ELF_E_ARGUMENT) {
		TP_FAIL("unexpected error=%d \"%s\"", error,
		    elf_errmsg(error));
	}

	(void) elf_end(ar);
	(void) close(fd);

	tet_result(result);
}

/*
 * The use of an offset greater than the largest valid file offset
 * should fail.
 */
void
tcSeekMoreThanFileSize(void)
{
	Elf *ar;
	off_t offset;
	struct stat sb;
	int error, fd, result;

	result = TET_UNRESOLVED;
	ar = NULL;
	fd = -1;

	TP_CHECK_INITIALIZATION();
	TP_ANNOUNCE("elf_rand() fails with a too-large offset");

	TS_OPEN_FILE(ar, TP_ARFILE, ELF_C_READ, fd);

	/* Get the file size of the archive. */
	if (fstat(fd, &sb) < 0) {
		TP_UNRESOLVED("cannot determine the size of \"%s\"",
		    TP_ARFILE);
		goto done;
	}

	result = TET_PASS;

	if ((offset = elf_rand(ar, sb.st_size)) != 0) {
		TP_FAIL("elf_rand() succeeded with offset=%lld",
		    (unsigned long long) offset);
	} else if ((error = elf_errno()) != ELF_E_ARGUMENT) {
		TP_FAIL("unexpected error=%d \"%s\"", error,
		    elf_errmsg(error));
	}

done:
	if (ar)
		(void) elf_end(ar);
	if (fd != -1)
		(void) close(fd);

	tet_result(result);
}
