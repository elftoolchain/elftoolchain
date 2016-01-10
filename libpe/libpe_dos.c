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

#include <sys/types.h>
#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "_libpe.h"

ELFTC_VCSID("$Id$");

int
libpe_parse_msdos_header(PE *pe, char *hdr)
{
	PE_DosHdr *dh;
	char coff[sizeof(PE_CoffHdr)];
	size_t len;
	uint32_t pe_magic;
	int i;

	if ((pe->pe_stub = malloc(sizeof(PE_DosHdr))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	memcpy(pe->pe_stub, hdr, sizeof(PE_DosHdr));

	if ((dh = malloc(sizeof(*dh))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	pe->pe_doshdr = dh;

	/* Read the conventional MS-DOS EXE header. */
	memcpy(dh->dh_magic, hdr, 2);
	hdr += 2;
	PE_READ16(hdr, dh->dh_lastsize);
	PE_READ16(hdr, dh->dh_nblock);
	PE_READ16(hdr, dh->dh_nreloc);
	PE_READ16(hdr, dh->dh_hdrsize);
	PE_READ16(hdr, dh->dh_minalloc);
	PE_READ16(hdr, dh->dh_maxalloc);
	PE_READ16(hdr, dh->dh_ss);
	PE_READ16(hdr, dh->dh_sp);
	PE_READ16(hdr, dh->dh_checksum);
	PE_READ16(hdr, dh->dh_ip);
	PE_READ16(hdr, dh->dh_cs);
	PE_READ16(hdr, dh->dh_relocpos);
	PE_READ16(hdr, dh->dh_noverlay);

	/* Do not continue if the EXE is not a PE/NE/... (new executable) */
	if (dh->dh_relocpos != 0x40) {
		pe->pe_iflags |= LIBPE_F_UNSUP_DOS_HEADER;
		return (0);
	}

	for (i = 0; i < 4; i++)
		PE_READ16(hdr, dh->dh_reserved1[i]);
	PE_READ16(hdr, dh->dh_oemid);
	PE_READ16(hdr, dh->dh_oeminfo);
	for (i = 0; i < 10; i++)
		PE_READ16(hdr, dh->dh_reserved2[i]);
	PE_READ32(hdr, dh->dh_lfanew);

	/* Check if the e_lfanew pointer is valid. */
	if (dh->dh_lfanew > pe->pe_fsize - 4) {
		pe->pe_iflags |= LIBPE_F_UNSUP_DOS_HEADER;
		return (0);
	}

	if (dh->dh_lfanew < sizeof(PE_DosHdr) &&
	    (pe->pe_iflags & LIBPE_F_SPECIAL_FILE)) {
		pe->pe_iflags |= LIBPE_F_UNSUP_DOS_HEADER;
		return (0);
	}

	if (dh->dh_lfanew > sizeof(PE_DosHdr)) {
		pe->pe_stub_ex = dh->dh_lfanew - sizeof(PE_DosHdr);
		if (pe->pe_iflags & LIBPE_F_SPECIAL_FILE) {
			/* Read in DOS stub now. */
			if (libpe_read_msdos_stub(pe) < 0) {
				pe->pe_iflags |= LIBPE_F_UNSUP_DOS_HEADER;
				return (0);
			}
		}
	}

	if ((pe->pe_iflags & LIBPE_F_SPECIAL_FILE) == 0) {
		/* Jump to the PE header. */
		if (lseek(pe->pe_fd, (off_t) dh->dh_lfanew, SEEK_SET) < 0) {
			pe->pe_iflags |= LIBPE_F_BAD_PE_HEADER;
			return (0);
		}
	}

	if (read(pe->pe_fd, &pe_magic, 4) != 4 ||
	    htole32(pe_magic) != PE_SIGNATURE) {
		pe->pe_iflags |= LIBPE_F_BAD_PE_HEADER;
		return (0);
	}

	if (read(pe->pe_fd, coff, sizeof(coff)) != (ssize_t) sizeof(coff)) {
		pe->pe_iflags |= LIBPE_F_BAD_PE_HEADER;
		return (0);
	}

	return (libpe_parse_coff_header(pe, coff));
}

int
libpe_read_msdos_stub(PE *pe)
{
	void *m;

	assert(pe->pe_stub_ex > 0 &&
	    (pe->pe_iflags & LIBPE_F_LOAD_DOS_STUB) == 0);

	if ((pe->pe_iflags & LIBPE_F_SPECIAL_FILE) == 0) {
		if (lseek(pe->pe_fd, (off_t) sizeof(PE_DosHdr), SEEK_SET) <
		    0) {
			errno = EIO;
			goto fail;
		}
	}

	if ((m = realloc(pe->pe_stub, sizeof(PE_DosHdr) + pe->pe_stub_ex)) ==
	    NULL) {
		errno = ENOMEM;
		goto fail;
	}
	pe->pe_stub = m;

	if (read(pe->pe_fd, pe->pe_stub + sizeof(PE_DosHdr), pe->pe_stub_ex) !=
	    (ssize_t) pe->pe_stub_ex) {
		errno = EIO;
		goto fail;
	}

	pe->pe_iflags |= LIBPE_F_LOAD_DOS_STUB;

	/* Search for the Rich header embedded just before the PE header. */
	(void) libpe_parse_rich_header(pe);

	return (0);

fail:
	pe->pe_stub_ex = 0;

	return (-1);
}
