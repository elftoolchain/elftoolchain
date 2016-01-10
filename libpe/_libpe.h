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

#include "libpe.h"

#include "_elftc.h"

struct _PE {
	int pe_fd;
	size_t pe_fsize;
	unsigned int pe_flags;
	unsigned int pe_iflags;
	PE_DosHdr *pe_doshdr;
	char *pe_dstub;
	size_t pe_dstub_len;
};

/* Library internal flags  */
#define	LIBPE_F_SPECIAL_FILE		0x0001U
#define	LIBPE_F_UNSUP_DOS_HEADER	0x0002U
#define	LIBPE_F_BAD_PE_HEADER		0x0004U

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
#endif

#define	PE_READ16(p,v)	do {			\
	(v) = le16dec(p);			\
	p += 2;					\
} while(0)

#define	PE_READ32(p,v)	do {			\
	(v) = le32dec(p);			\
	p += 4;					\
} while(0)

#endif	/* !__LIBPE_H_ */
