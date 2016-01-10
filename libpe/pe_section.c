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

#include <errno.h>

#include "_libpe.h"

ELFTC_VCSID("$Id$");

PE_Scn *
pe_getscn(PE *pe, size_t index)
{
	PE_Scn *ps;

	if (pe == NULL || index < 1 || index > 0xFFFFU) {
		errno = EINVAL;
		return (NULL);
	}

	STAILQ_FOREACH(ps, &pe->pe_scn, ps_next) {
		if (ps->ps_ndx == index)
			return (ps);
	}

	errno = ENOENT;

	return (NULL);
}

size_t
pe_ndxscn(PE_Scn *ps)
{

	if (ps == NULL) {
		errno = EINVAL;
		return (0);
	}

	return (ps->ps_ndx);
}

PE_Scn *
pe_nextscn(PE *pe, PE_Scn *ps)
{

	if (pe == NULL) {
		errno = EINVAL;
		return (NULL);
	}

	if (ps == NULL)
		ps = STAILQ_FIRST(&pe->pe_scn);
	else
		ps = STAILQ_NEXT(ps, ps_next);

	while (ps != NULL) {
		if (ps->ps_ndx >= 1 && ps->ps_ndx <= 0xFFFFU)
			return (ps);
		ps = STAILQ_NEXT(ps, ps_next);
	}

	return (NULL);
}
