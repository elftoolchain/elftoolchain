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
 * Test the `elf_errmsg' entry point.
 */

/*
 * Assertion: the function returns NULL if the argument is zero and
 * there is no pending error in the library.
 */
void
tp_zero_noerror(void)
{

	tet_infoline("assertion: returns NULL with zero & no "
	    "current error");

	(void) elf_errno();	/* discard current error number */

	if (elf_errmsg(0) == NULL)
		tet_result(TET_PASS);
	else
		tet_result(TET_FAIL);
}

/*
 * An error value of -1 should return non-NULL
 */

void
tp_minusone_noerror(void)
{
	const char *msg;

	tet_infoline("assertion: Returns non-null for arg -1 & no "
	    "current error");

	(void) elf_errno();	/* discard stored error */

	msg = elf_errmsg(-1);
	if (msg == NULL)
		tet_result(TET_FAIL);

	tet_result(TET_PASS);
}

#if	defined(LIBELF_TEST_HOOKS)

/*
 * Assertion: a value of -1 returns the default no-error message.
 */
void
tp_minusone_noerror_message(void)
{
	const char *msg, *defaultmsg;

	tet_infoline("assertion: returns the default no-error message "
	    "for error == -1");

	if ((defaultmsg = _libelf_get_no_error_message()) == NULL) {
		tet_infoline("unresolved: cannot determine no-error message");
		tet_result(TET_UNRESOLVED);
		return;
	}

	(void) elf_errno();	/* discard any stored error */
	msg = elf_errmsg(-1);	/* get message */
	tet_result(msg == defaultmsg ? TET_PASS : TET_FAIL);
}

/*
 * Assertion: if a zero is passed in and there is a pending error, a non-null
 * value is returned.
 */

void
tp_zero_with_error(void)
{
	tet_infoline("assertion: return non-null with a pending error and "
	    "arg zero");

	_libelf_set_error(1);	/* any value > 0 is ok */
	tet_result(elf_errmsg(0) == NULL ? TET_FAIL : TET_PASS);
}

/*
 * Assertion: all error codes upto the max supported error return a valid
 * error message pointer.
 */

void
tp_nonzero_valid(void)
{
	int i, maxerr;

	tet_infoline("assertion: all valid error codes return a non-null "
	    "pointer");

	maxerr = _libelf_get_max_error();

	if (maxerr <= 0) {
		tet_infoline("unresolved: could not determine max error");
		tet_result(TET_UNRESOLVED);
		return;
	}

	for (i = 1; i < maxerr; i++)
		if (elf_errmsg(i) == NULL) {
			tet_printf("fail: error %d returned a NULL pointer", i);
			tet_result(TET_FAIL);
			return;
		}

	tet_result(TET_PASS);
}

/*
 * Assertion: error codes out of the legal range of errors return a non-NULL
 * pointer.
 */

void
tp_invalid_error(void)
{
	int maxerr;
	const char *illegal_msg;

	maxerr = _libelf_get_max_error();

	if (maxerr <= 0) {
		tet_infoline("unresolved: could not determine max error");
		tet_result(TET_UNRESOLVED);
		return;
	}

	if ((illegal_msg = _libelf_get_unknown_error_message()) == NULL) {
		tet_infoline("unresolved: could not determine the unknown "
		    "error message");
		tet_result(TET_UNRESOLVED);
		return;
	}

	tet_infoline("assertion: test illegal values for the error number");
	if (elf_errmsg(-2) != illegal_msg)	/* less than -1 */
		tet_result(TET_FAIL);
	else if (elf_errmsg(maxerr) != illegal_msg)	/* == max error */
		tet_result(TET_FAIL);
	else if (elf_errmsg(maxerr+1) != illegal_msg)	/* > max error */
		tet_result(TET_FAIL);
	else
		tet_result(TET_PASS);
}

#endif	/* LIBELF_TEST_HOOKS */
