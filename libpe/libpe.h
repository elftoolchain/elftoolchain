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

#ifndef	_LIBPE_H_
#define	_LIBPE_H_

#include "pe.h"

/* Library private data structures */
typedef struct _PE PE;
typedef struct _PE_Scn PE_Scn;

/* Section buffers */
typedef struct PE_Buffer {
        unsigned int pb_align;
	off_t pb_off;
	size_t pb_size;
	void *pb_buf;
} PE_Buffer;

/* Commands */
typedef enum {
	PE_C_NULL = 0,
	PE_C_CLR,
	PE_C_FDDONE,
	PE_C_FDREAD,
	PE_C_RDWR,
	PE_C_READ,
	PE_C_SET,
	PE_C_WRITE,
	PE_C_NUM
} PE_Cmd;

/* Flags defined by the API. */

#ifdef __cplusplus
extern "C" {
#endif

PE_CoffHdr	*pe_coff_header(PE *);
PE_DataDir	*pe_data_dir(PE *);
void		pe_finish(PE *);
PE_Buffer	*pe_getbuffer(PE_Scn *, PE_Buffer *);
PE_Scn 		*pe_getscn(PE *, size_t);
PE		*pe_init(int, PE_Cmd);
PE_DosHdr	*pe_msdos_header(PE *);
char		*pe_msdos_stub(PE *, size_t *);
size_t		pe_ndxscn(PE_Scn *);
PE_Scn		*pe_nextscn(PE *, PE_Scn *);
PE_OptHdr	*pe_opt_header(PE *);
PE_RichHdr	*pe_rich_header(PE *);
int		pe_rich_header_validate(PE *);

#ifdef __cplusplus
}
#endif

#endif	/* !_LIBPE_H_ */
