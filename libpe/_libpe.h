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
 *
 * $Id$
 */

#ifndef	__LIBPE_H_
#define	__LIBPE_H_

#include <sys/types.h>
#include <sys/queue.h>

#include "libpe.h"

#include "_elftc.h"

typedef struct _PE_SecBuf {
	PE_Buffer sb_pb;
	PE_Scn *sb_ps;
	STAILQ_ENTRY(_PE_SecBuf) sb_next;
} PE_SecBuf;

struct _PE_Scn {
	PE *ps_pe;
	PE_SecHdr ps_sh;
	unsigned int ps_ndx;
	unsigned int ps_flags;
	STAILQ_HEAD(, _PE_SecBuf) ps_b;
	STAILQ_ENTRY(_PE_Scn) ps_next;
};

struct _PE {
	int pe_fd;
	size_t pe_fsize;
	unsigned int pe_flags;
	unsigned int pe_iflags;
	PE_DosHdr *pe_dh;
	char *pe_stub;
	size_t pe_stub_ex;
	PE_RichHdr *pe_rh;
	char *pe_rh_start;
	PE_CoffHdr *pe_ch;
	PE_OptHdr *pe_oh;
	PE_DataDir *pe_dd;
	unsigned int pe_nscn;
	STAILQ_HEAD(, _PE_Scn) pe_scn;
};

/* Library internal flags  */
#define	LIBPE_F_SPECIAL_FILE		0x0001U
#define	LIBPE_F_UNSUP_DOS_HEADER	0x0002U
#define	LIBPE_F_BAD_PE_HEADER		0x0004U
#define	LIBPE_F_LOAD_DOS_STUB		0x0008U
#define	LIBPE_F_BAD_COFF_HEADER		0x0010U
#define	LIBPE_F_BAD_OPT_HEADER		0x0020U
#define	LIBPE_F_BAD_SEC_HEADER		0x0040U

/* Internal section flags */
#define	LIBPE_F_LOAD_SEC		0x100000U

/* Library internal defines */
#define	PE_DOS_MAGIC		0x5a4d
#define	PE_RICH_TEXT		"Rich"
#define	PE_RICH_HIDDEN		0x536e6144 /* DanS */
#define	PE_SIGNATURE		0x4550	   /* PE\0\0 */
#define	PE_COFF_MAX_SECTION	96
#define	PE_SYM_ENTRY_SIZE	18

/* Encode/Decode macros */
#if defined(ELFTC_NEED_BYTEORDER_EXTENSIONS)
static  __inline uint16_t
le16dec(const void *pp)
{
	unsigned char const *p = (unsigned char const *)pp;

	return ((p[1] << 8) | p[0]);
}

static __inline uint32_t
le32dec(const void *pp)
{
	unsigned char const *p = (unsigned char const *)pp;

	return ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

static __inline uint64_t
le64dec(const void *pp)
{
	unsigned char const *p = (unsigned char const *)pp;

	return (((uint64_t)le32dec(p + 4) << 32) | le32dec(p));
}
#endif

#define	PE_READ16(p,v)	do {			\
	(v) = le16dec(p);			\
	p += 2;					\
} while(0)

#define	PE_READ32(p,v)	do {			\
	(v) = le32dec(p);			\
	p += 4;					\
} while(0)

/* Internal function declarations */
PE_SecBuf	*libpe_alloc_buffer(PE_Scn *, size_t);
int		libpe_load_all_sections(PE *);
int		libpe_load_section(PE *, PE_Scn *);
int		libpe_open_object(PE *, PE_Cmd);
int		libpe_parse_msdos_header(PE *, char *);
int		libpe_parse_coff_header(PE *, char *);
int		libpe_parse_rich_header(PE *);
int		libpe_parse_section_headers(PE *);
int		libpe_read_msdos_stub(PE *);
void		libpe_sort_sections(PE *);

#endif	/* !__LIBPE_H_ */
