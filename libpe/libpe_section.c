/*-
 * Copyright (c) 2016 Kai Wang
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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "_libpe.h"

ELFTC_VCSID("$Id$");

static PE_Scn *
alloc_scn(PE *pe)
{
	PE_Scn *ps;

	if ((ps = malloc(sizeof(PE_Scn))) == NULL) {
		errno = ENOMEM;
		return (NULL);
	}
	STAILQ_INIT(&ps->ps_b);
	ps->ps_pe = pe;

	STAILQ_INSERT_TAIL(&pe->pe_scn, ps, ps_next);

	return (ps);
}

int
libpe_parse_section_headers(PE *pe)
{
	char tmp[sizeof(PE_SecHdr)], *hdr;
	PE_Scn *ps;
	PE_SecHdr *sh;
	PE_CoffHdr *ch;
	PE_DataDir *dd;
	int found, i;

	assert(pe->pe_ch != NULL);

	for (i = 0; (uint16_t) i < pe->pe_ch->ch_nsec; i++) {
		if (read(pe->pe_fd, tmp, sizeof(PE_SecHdr)) !=
		    (ssize_t) sizeof(PE_SecHdr)) {
			pe->pe_iflags |= LIBPE_F_BAD_SEC_HEADER;
			return (0);
		}

		if ((ps = alloc_scn(pe)) == NULL)
			return (-1);

		ps->ps_ndx = ++pe->pe_nscn;	/* Setion index is 1-based */

		sh = &ps->ps_sh;

		/*
		 * Note that the section name won't be NUL-terminated if
		 * its length happens to be 8.
		 */
		memcpy(sh->sh_name, tmp, sizeof(sh->sh_name));
		hdr = tmp + 8;
		PE_READ32(hdr, sh->sh_virtsize);
		PE_READ32(hdr, sh->sh_addr);
		PE_READ32(hdr, sh->sh_rawsize);
		PE_READ32(hdr, sh->sh_rawptr);
		PE_READ32(hdr, sh->sh_relocptr);
		PE_READ32(hdr, sh->sh_lineptr);
		PE_READ32(hdr, sh->sh_nreloc);
		PE_READ32(hdr, sh->sh_nline);
		PE_READ32(hdr, sh->sh_char);
	}

	/*
	 * For all the data directories that don't belong to any section,
	 * we create pseudo sections for them to make layout easier.
	 */
	dd = pe->pe_dd;
	if (dd != NULL && dd->dd_total > 0) {
		for (i = 0; (uint32_t) i < pe->pe_dd->dd_total; i++) {
			found = 0;
			STAILQ_FOREACH(ps, &pe->pe_scn, ps_next) {
				sh = &ps->ps_sh;
				if (dd->dd_e[i].de_addr >= sh->sh_rawptr &&
				    dd->dd_e[i].de_addr + dd->dd_e[i].de_size <=
				    sh->sh_rawptr + sh->sh_rawsize) {
					found = 1;
					break;
				}
			}
			if (found)
				continue;

			if ((ps = alloc_scn(pe)) == NULL)
				return (-1);

			ps->ps_ndx = 0xFFFF0000U | i;

			sh = &ps->ps_sh;
			sh->sh_rawptr = dd->dd_e[i].de_addr;
			sh->sh_rawsize = dd->dd_e[i].de_size;
		}
	}

	/*
	 * Also consider the COFF symbol table as a pseudo section.
	 */
	ch = pe->pe_ch;
	if (ch->ch_nsym > 0) {
		if ((ps = alloc_scn(pe)) == NULL)
			return (-1);
		ps->ps_ndx = 0xFFFFFFFFU;
		sh = &ps->ps_sh;
		sh->sh_rawptr = ch->ch_symptr;
		sh->sh_rawsize = ch->ch_nsym * PE_SYM_ENTRY_SIZE;
	}

	/* Sort all sections by their file offsets. */
	libpe_sort_sections(pe);

	/* PE file headers initialization is complete if we reach here. */
	return (0);
}

int
libpe_load_section(PE *pe, PE_Scn *ps)
{
	PE_SecHdr *sh;
	PE_SecBuf *sb;
	size_t sz;
	char tmp[4];

	assert(pe != NULL && ps != NULL);
	assert((ps->ps_flags & LIBPE_F_LOAD_SEC) == 0);

	sh = &ps->ps_sh;

	/* Allocate a PE_SecBuf struct without buffer for empty sections. */
	if (sh->sh_rawsize == 0) {
		(void) libpe_alloc_buffer(ps, 0);
		ps->ps_flags |= LIBPE_F_LOAD_SEC;
		return (0);
	}

	if ((pe->pe_iflags & LIBPE_F_SPECIAL_FILE) == 0) {
		if (lseek(pe->pe_fd, (off_t) sh->sh_rawptr, SEEK_SET) < 0) {
			errno = EIO;
			return (-1);
		}
	}

	if ((sb = libpe_alloc_buffer(ps, sh->sh_rawsize)) == NULL)
		return (-1);

	if (read(pe->pe_fd, sb->sb_pb.pb_buf, sh->sh_rawsize) !=
	    (ssize_t) sh->sh_rawsize) {
		errno = EIO;
		return (-1);
	}

	if (ps->ps_ndx = 0xFFFFFFFFU) {
		/*
		 * Index 0xFFFFFFFF indicates this section is a pseudo
		 * section that contains the COFF symbol table. We should
		 * read in the string table right after it.
		 */
		if (read(pe->pe_fd, tmp, sizeof(tmp)) !=
		    (ssize_t) sizeof(tmp)) {
			errno = EIO;
			return (-1);
		}
		sz = le32dec(tmp);

		/*
		 * The minimum value for the size field is 4, which indicates
		 * there is no string table.
		 */
		if (sz > 4) {
			sz -= 4;
			if ((sb = libpe_alloc_buffer(ps, sz)) == NULL)
				return (-1);
			if (read(pe->pe_fd, sb->sb_pb.pb_buf, sz) !=
			    (ssize_t) sz) {
				errno = EIO;
				return (-1);
			}
		}
	}

	ps->ps_flags |= LIBPE_F_LOAD_SEC;

	return (0);
}

int
libpe_load_all_sections(PE *pe)
{
	PE_Scn *ps;
	PE_SecHdr *sh;
	unsigned r, s;
	off_t off;
	char tmp[256];

	/* Calculate the current offset into the file. */
	off = 0;
	if (pe->pe_dh != NULL)
		off += pe->pe_dh->dh_lfanew + 4;
	if (pe->pe_ch != NULL)
		off += sizeof(PE_CoffHdr) + pe->pe_ch->ch_optsize;

	STAILQ_FOREACH(ps, &pe->pe_scn, ps_next) {
		if (ps->ps_flags |= LIBPE_F_LOAD_SEC)
			continue;
		sh = &ps->ps_sh;

		/*
		 * For special files, we consume the padding in between
		 * and advance to the section offset.
		 */
		if (pe->pe_iflags & LIBPE_F_SPECIAL_FILE) {
			/* Can't go backwards. */
			if (off > sh->sh_rawptr) {
				errno = EIO;
				return (-1);
			}
			if (off < sh->sh_rawptr) {
				r = sh->sh_rawptr - off;
				for (; r > 0; r -= s) {
					s = r > sizeof(tmp) ? sizeof(tmp) : r;
					if (read(pe->pe_fd, tmp, s) !=
					    (ssize_t) s) {
						errno = EIO;
						return (-1);
					}
				}
			}
		}

		/* Load the section content. */
		if (libpe_load_section(pe, ps) < 0)
			return (-1);
	}
}
