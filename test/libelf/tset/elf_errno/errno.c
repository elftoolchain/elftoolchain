/*-
 * Copyright (c) 2006 Joseph Koshy
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

#include <libelf.h>

#include "tet_api.h"

/*
 * Test the `elf_errno' entry point.
 *
 * Specific errors expected from other elf_* APIs are tested in the test
 * cases for those APIs.
 *
 * The tests here only check the behaviour of the elf_errno() API.
 */

/*
 * Assertion: The initial value of the libraries error number is zero.
 */

void
tp_init(void)
{
	int err;

	tet_infoline("assertion: initial error must be zero");
	err = elf_errno();
	tet_result(err == 0 ? TET_PASS : TET_FAIL);
}

/*
 * Assertion: an elf_errno() call resets the stored error number.
 */

void
tp_reset(void)
{
	int err;

	tet_infoline("assertion: error must be reset by elf_errno()");

	(void) elf_errno();	/* discard stored error */
	err = elf_errno();
	tet_result(err == 0 ? TET_PASS : TET_FAIL);
}

#if	defined(LIBELF_TEST_HOOKS)

/*
 * Verify that a non-zero error code is retrieved correctly.
 */

void
tp_set(void)
{
	tet_infoline("assertion: the error values must be retrieved "
	    "correctly");
	_libelf_set_error(1);	/* any value > 0 && < max */
	tet_result(elf_errno() == 1 ? TET_PASS : TET_FAIL);
}
#endif	/* LIBELF_TEST_HOOKS */
