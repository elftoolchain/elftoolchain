/*-
 * Copyright (c) 2010 Kai Wang
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

#include <sys/cdefs.h>
#include <sys/param.h>
#include <err.h>
#include <gelf.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>

#include "elfcopy.h"

ELFTC_VCSID("$Id$");

/*
 * Convert ELF object to `binary'.
 */
void
create_binary(int ifd, int ofd)
{
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Shdr sh;
	off_t base, off;
	int elferr;

	if ((e = elf_begin(ifd, ELF_C_READ, NULL)) == NULL)
		errx(EX_DATAERR, "elf_begin() failed: %s",
		    elf_errmsg(-1));

	base = 0;
	if (lseek(ofd, base, SEEK_SET) < 0)
		err(EX_DATAERR, "lseek failed");

	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) == NULL) {
			warnx("gelf_getshdr failed: %s", elf_errmsg(-1));
			(void) elf_errno();
			continue;
		}
		if ((sh.sh_flags & SHF_ALLOC) == 0)
			continue;
		if (sh.sh_size == 0)
			continue;
		(void) elf_errno();
		if ((d = elf_getdata(scn, NULL)) == NULL) {
			elferr = elf_errno();
			if (elferr != 0)
				warnx("elf_getdata failed: %s", elf_errmsg(-1));
			continue;
		}
		if (d->d_buf == NULL || d->d_size == 0)
			continue;

		/*
		 * Use the offset of the first SHF_ALLOC section as base
		 * offset.
		 */
		if (base == 0)
			base = sh.sh_offset;

		/* lseek to section offset relative to `base'. */
		off = sh.sh_offset - base;
		if (lseek(ofd, off, SEEK_SET) < 0)
			err(EX_DATAERR, "lseek failed");

		/* Write out section contents. */
		if (write(ofd, d->d_buf, d->d_size) != (ssize_t) d->d_size)
			err(EX_DATAERR, "write failed");
	}
	elferr = elf_errno();
	if (elferr != 0)
		warnx("elf_nextscn failed: %s", elf_errmsg(elferr));
}
