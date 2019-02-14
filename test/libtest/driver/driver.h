/*-
 * Copyright (c) 2018,2019 Joseph Koshy
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

#ifndef	_LIBTEST_DRIVER_H_
#define	_LIBTEST_DRIVER_H_

#include <sys/queue.h>

#include <stdbool.h>

#include "_elftc.h"

#include "test.h"

#define	TEST_ENVIRONMENT_SEARCH_PATH	"TEST_PATH"

/*
 * Run time data strucrures.
 */

/* The completion status for a test run */
enum test_run_status {
	/*
	 * All test cases were successfully invoked, and all their contained
	 * test purposes passed.
	 */
	TR_STATUS_PASS = 0,

	/*
	 * All test cases were successfully invoked but at least one test
	 * function reported a failure.
	 */
	TR_STATUS_FAIL = 1,

	/*
	 * At least one test case reported an error during its set up or tear
	 * down phase.
	 */
	TR_STATUS_ERROR = 2
};

/*
 * The 'style' of the run determines the manner in which the test
 * executable reports test status.
 */
enum test_run_style {
	/* Libtest semantics. */
	TR_STYLE_LIBTEST,

	/*
	 * Be compatible with the Test Anything Protocol
	 * (http://testanything.org/).
	 */
	TR_STYLE_TAP,

	/* Be compatible with NetBSD ATF(9). */
	TR_STYLE_ATF
};

/*
 * Structures used for selecting tests.
 */
struct test_function_selector {
	const struct test_function_descriptor *tf_descriptor;

	STAILQ_ENTRY(test_function_selector) tf_next;
	int	tf_is_selected;
};

STAILQ_HEAD(test_function_selector_list, test_function_selector);

struct test_case_selector {
	const struct test_case_descriptor	*tcr_descriptor;
	STAILQ_ENTRY(test_case_selector)	tcr_next;
	int	tcr_has_selected_tests;
	struct test_function_selector_list	tcr_functions;
};

/*
 * The action being requested of the test driver.
 */
enum test_run_action {
	TR_ACTION_EXECUTE,	/* Execute the selected tests. */
	TR_ACTION_LIST,		/* Only list tests. */
};

STAILQ_HEAD(test_case_selector_list, test_case_selector);

/*
 * Runtime directories to look up data files.
 */
struct test_search_path_entry {
	char *tp_directory;
	STAILQ_ENTRY(test_search_path_entry)	tp_next;
};

STAILQ_HEAD(test_search_path_list, test_search_path_entry);

/*
 * Parameters for the run.
 */
struct test_run {
	/* What the test run should do. */
	enum test_run_action	tr_action;

	/* The desired behavior of the test harness. */
	enum test_run_style	tr_style;

	/* An optional name assigned by the user for this test run. */
	const char	*tr_name;

	/*
	 * Directories to use when resolving non-absolute data file
	 * names.
	 */
	struct test_search_path_list tr_search_path;

	/*
	 * The absolute path to the directory under which the test is
	 * to be run.
	 *
	 * Each test case will be invoked in some subdirectory of this
	 * directory.
	 */
	char	*tr_runtime_base_directory;

	/*
	 * The test timeout in seconds.
	 *
	 * A value of zero indicates that the test driver should wait
	 * indefinitely for tests.
	 */
	long	tr_max_seconds_per_test;

	/*
	 * If not NULL, An absolute pathname to an archive that will hold
	 * the artefacts created by a test run.
	 */
	char	*tr_artefact_archive;

	/* The desired verbosity level. */
	int	tr_verbosity;

	/* All tests that were selected for this run. */
	struct	test_case_selector_list	tr_test_cases;
};

#ifdef	__cplusplus
extern "C" {
#endif
struct test_run	*test_run_allocate(void);
void		test_run_release(struct test_run *);
void		test_run_add_search_path(struct test_run *,
    const char *search_path);
void		test_run_set_runtime_directory(struct test_run *,
    const char *tmpdir);
#ifdef	__cplusplus
}
#endif

#endif	/* _LIBTEST_DRIVER_H_ */
