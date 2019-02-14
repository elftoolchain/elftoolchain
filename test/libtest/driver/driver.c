/*-
 * Copyright (c) 2018, Joseph Koshy
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * The implementation of the test driver.
 */

#include <stdbool.h>
#include <sysexits.h>
#include <stdlib.h>

#include "driver.h"

/* Split a colon-separated path value into its constituent paths. */
void
test_run_add_search_path(struct test_run *tr, const char *search_path)
{
	/*Stub.*/
	return;
}

/* Set the run time directory for tests. */
void
test_run_set_runtime_directory(struct test_run *tr, const char *tmpdir)
{

}

struct test_run *
test_run_allocate(void)
{
	struct test_run *tr;
	const char *search_path;

	tr = calloc(sizeof(struct test_run), 1);
	tr->tr_action = TR_ACTION_EXECUTE;
	tr->tr_style = TR_STYLE_LIBTEST;
	STAILQ_INIT(&tr->tr_test_cases);
	STAILQ_INIT(&tr->tr_search_path);

	if ((search_path = getenv(TEST_ENVIRONMENT_SEARCH_PATH)) != NULL)
	    test_run_add_search_path(tr, search_path);

	return (tr);
}

void
test_run_release(struct test_run *tr)
{
	struct test_search_path_entry *path_entry;
	struct test_case_selector *selector;
	struct test_function_selector *function_entry;

	/* Free the search path list. */
	while (!STAILQ_EMPTY(&tr->tr_search_path)) {
		path_entry = STAILQ_FIRST(&tr->tr_search_path);
		STAILQ_REMOVE_HEAD(&tr->tr_test_cases, tcr_next);
		free(path_entry);
	}

	/* Free the test selector list. */
	while (!STAILQ_EMPTY(&tr->tr_test_cases)) {
		selector = STAILQ_FIRST(&tr->tr_test_cases);
		STAILQ_REMOVE_HEAD(&tr->tr_test_cases, tcr_next);
		while (!STAILQ_EMPTY(&selector->tcr_functions)) {
			function_entry = STAILQ_FIRST(&selector->tcr_functions);
			STAILQ_REMOVE_HEAD(&selector->tcr_functions, tf_next);
			free(function_entry);
		}
		free(selector);
	}
}
