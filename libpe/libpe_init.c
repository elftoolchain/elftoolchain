/*-
 * Copyright (c) 2015 Kai Wang
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

#include <sys/stat.h>
#include <assert.h>
#include <errno.h>

#include "_libpe.h"

ELFTC_VCSID("$Id$");

int
libpe_open_object(PE *pe)
{
	struct stat sb;
	mode_t mode;
	char magic[sizeof(PE_DosHdr)];

	if (fstat(pe->pe_fd, &sb) < 0)
		return (-1);

	mode = sb.st_mode;
	pe->pe_fsize = (size_t) sb.st_size;

	/* Reject unsupported file types. */
	if (!S_ISREG(mode) && !S_ISCHR(mode) && !S_ISFIFO(mode) &&
	    !S_ISSOCK(mode)) {
		errno = EINVAL;
		return (-1);
	}

	/* Read/Write mode is not supported for non-regular file. */
	if (pe->pe_cmd == PE_C_RDWR && !S_ISREG(mode)) {
		errno = EINVAL;
		return (-1);
	}

	/* The minimal file should at least contain a COFF header. */
	if (S_ISREG(mode) && pe->pe_fsize < sizeof(PE_CoffHdr)) {
		errno = ENOENT;
		return (-1);
	}

	/*
	 * Search for MS-DOS header or COFF header.
	 */

	if (read(pe->pe_fd, magic, 2) != 2) {
		errno = EIO;
		return (-1);
	}

	if (magic[0] == 'M' && magic[1] == 'Z') {
		if (read(pe->pe_fd, &magic[2], sizeof(PE_DosHdr) - 2) !=
		    (ssize_t) sizeof(PE_DosHdr) - 2) {
			errno = EIO;
			return (-1);
		}
		return (libpe_parse_msdos_header(pe, magic));
	} else {
		if (read(pe->pe_fd, &magic[2], sizeof(PE_CoffHdr) - 2) !=
		    (ssize_t) sizeof(PE_CoffHdr) - 2) {
			errno = EIO;
			return (-1);
		}
		return (libpe_parse_coff_header(pe, magic));
	}
}
