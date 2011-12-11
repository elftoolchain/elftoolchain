/*-
 * Copyright (c) 2010,2011 Kai Wang
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

#include "ld.h"
#include "ld_options.h"
#include "ld_script.h"
#include "ld_file.h"
#include "ld_layout.h"
#include "ld_symbols.h"

ELFTC_VCSID("$Id$");

static struct ld _ld;
struct ld* ld = &_ld;

static void
_ld_init(void)
{

	TAILQ_INIT(&ld->ld_lflist);
	STAILQ_INIT(&ld->ld_lilist);
	STAILQ_INIT(&ld->ld_oslist);
	STAILQ_INIT(&ld->ld_ls.ls_lplist);

	/* Initialise libelf. */
	if (elf_version(EV_CURRENT) == EV_NONE)
		ld_fatal(ld, "ELF library initialization failed: %s",
		    elf_errmsg(-1));

	ld_script_init(ld);
}

int
main(int argc, char **argv)
{

	_ld_init();

	ld->ld_progname = basename(argv[0]);

	ld_script_parse_internal();

	ld_options_parse(ld, argc, argv);

	ld_symbols_resolve(ld);

	ld_layout_sections(ld);

	exit(EXIT_SUCCESS);
}
