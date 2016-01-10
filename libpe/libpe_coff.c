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

#include <errno.h>
#include <stdlib.h>

#include "_libpe.h"

ELFTC_VCSID("$Id$");

int
libpe_parse_coff_header(PE *pe, char *hdr)
{
	char tmp[128];
	PE_CoffHdr *ch;
	PE_OptHdr *oh;
	PE_DataDir *dd;
	unsigned p, r, s;
	int i;

	if ((ch = malloc(sizeof(PE_CoffHdr))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}

	PE_READ16(hdr, ch->ch_machine);
	PE_READ16(hdr, ch->ch_nsec);
	PE_READ32(hdr, ch->ch_timestamp);
	PE_READ32(hdr, ch->ch_symptr);
	PE_READ32(hdr, ch->ch_nsym);
	PE_READ16(hdr, ch->ch_optsize);
	PE_READ16(hdr, ch->ch_char);

	pe->pe_ch = ch;

	/*
	 * The Optional header is omitted for object files.
	 */
	if (ch->ch_optsize == 0)
		return (libpe_parse_section_headers(pe));

	if ((oh = calloc(1, sizeof(PE_OptHdr))) == NULL) {
		errno = ENOMEM;
		return (-1);
	}
	pe->pe_oh = oh;

#define READ_OPT(n)							\
	do {								\
		/*							\
		 * Since the Optional Header size is variable, we must	\
		 * check if the requested read size will overrun the	\
		 * remaining header bytes.				\
		 */							\
		if (p + (n) > ch->ch_optsize) {				\
			/* Consume the "extra" bytes */			\
			r = ch->ch_optsize - p;				\
			if (read(pe->pe_fd, tmp, r) != (ssize_t) r) {	\
				pe->pe_iflags |= LIBPE_F_BAD_SEC_HEADER;\
				return (0);				\
			}						\
			return (libpe_parse_section_headers(pe));	\
		}							\
		if (read(pe->pe_fd, tmp, (n)) != (ssize_t) (n)) {	\
			pe->pe_iflags |= LIBPE_F_BAD_OPT_HEADER;	\
			return (0);					\
		}							\
		p += (n);						\
	} while (0)
#define	READ_OPT8(v) do { READ_OPT(1); (v) = *tmp; } while(0)
#define	READ_OPT16(v) do { READ_OPT(2); (v) = le16dec(tmp); } while(0)
#define	READ_OPT32(v) do { READ_OPT(4); (v) = le32dec(tmp); } while(0)
#define	READ_OPT64(v) do { READ_OPT(8); (v) = le64dec(tmp); } while(0)

	/*
	 * Read in the Optional header. Size of some fields are depending
	 * on the PE format specified by the oh_magic field. (PE32 or PE32+)
	 */

	READ_OPT16(oh->oh_magic);
	READ_OPT8(oh->oh_ldvermajor);
	READ_OPT8(oh->oh_ldverminor);
	READ_OPT32(oh->oh_textsize);
	READ_OPT32(oh->oh_datasize);
	READ_OPT32(oh->oh_bsssize);
	READ_OPT32(oh->oh_entry);
	READ_OPT32(oh->oh_textbase);
	if (oh->oh_magic != PE_FORMAT_32P) {
		READ_OPT32(oh->oh_database);
		READ_OPT32(oh->oh_imgbase);
	} else
		READ_OPT64(oh->oh_imgbase);
	READ_OPT32(oh->oh_secalign);
	READ_OPT32(oh->oh_filealign);
	READ_OPT16(oh->oh_osvermajor);
	READ_OPT16(oh->oh_osverminor);
	READ_OPT16(oh->oh_imgvermajor);
	READ_OPT16(oh->oh_imgverminor);
	READ_OPT16(oh->oh_subvermajor);
	READ_OPT16(oh->oh_subverminor);
	READ_OPT32(oh->oh_win32ver);
	READ_OPT32(oh->oh_imgsize);
	READ_OPT32(oh->oh_hdrsize);
	READ_OPT32(oh->oh_checksum);
	READ_OPT16(oh->oh_subsystem);
	READ_OPT16(oh->oh_dllchar);
	if (oh->oh_magic != PE_FORMAT_32P) {
		READ_OPT32(oh->oh_stacksizer);
		READ_OPT32(oh->oh_stacksizec);
		READ_OPT32(oh->oh_heapsizer);
		READ_OPT32(oh->oh_heapsizec);
	} else {
		READ_OPT64(oh->oh_stacksizer);
		READ_OPT64(oh->oh_stacksizec);
		READ_OPT64(oh->oh_heapsizer);
		READ_OPT64(oh->oh_heapsizec);
	}
	READ_OPT32(oh->oh_ldrflags);
	READ_OPT32(oh->oh_ndatadir);

	/*
	 * Read in the Data Directories.
	 */

	if (oh->oh_ndatadir > 0) {
		if ((dd = calloc(1, sizeof(PE_DataDir))) == NULL) {
			errno = ENOMEM;
			return (-1);
		}
		pe->pe_dd = dd;

		dd->dd_total = oh->oh_ndatadir < PE_DD_MAX ? oh->oh_ndatadir :
			PE_DD_MAX;

		for (i = 0; (uint32_t) i < dd->dd_total; i++) {
			READ_OPT32(dd->dd_e[i].de_addr);
			READ_OPT32(dd->dd_e[i].de_size);
		}
	}

	/* Consume the remaining bytes in the Optional header, if any. */
	if (ch->ch_optsize > p) {
		r = ch->ch_optsize - p;
		for (; r > 0; r -= s) {
			s = r > sizeof(tmp) ? sizeof(tmp) : r;
			if (read(pe->pe_fd, tmp, s) != (ssize_t) s) {
				pe->pe_iflags |= LIBPE_F_BAD_SEC_HEADER;
				return (0);
			}
		}
	}

	return (libpe_parse_section_headers(pe));
}
