/*-
 * Copyright (c) 2011 Kai Wang
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "ld.h"
#include "ld_output.h"
#include "ld_layout.h"

ELFTC_VCSID("$Id$");

void
ld_output_init(struct ld *ld)
{
	struct ld_output *lo;

	if ((lo = calloc(1, sizeof(*lo))) == NULL)
		ld_fatal_std(ld, "calloc");
	STAILQ_INIT(&lo->lo_oelist);
	STAILQ_INIT(&lo->lo_oslist);
	ld->ld_output = lo;
}

void
ld_output_format(struct ld *ld, char *def, char *be, char *le)
{

	if ((ld->ld_otgt = elftc_bfd_find_target(def)) == NULL)
		ld_fatal(ld, "invalid BFD format %s", def);

	if ((ld->ld_otgt_be = elftc_bfd_find_target(be)) == NULL)
		ld_fatal(ld, "invalid BFD format %s", be);

	if ((ld->ld_otgt_le = elftc_bfd_find_target(le)) == NULL)
		ld_fatal(ld, "invalid BFD format %s", le);
}
