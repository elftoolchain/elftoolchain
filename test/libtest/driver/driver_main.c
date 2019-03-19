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
 * This file defines a "main()" that invokes (or lists) the tests that were
 * linked into the current executable.
 */

#include <sys/param.h>
#include <sys/queue.h>
#include <sys/stat.h>

#include <assert.h>
#include <err.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sysexits.h>
#include <time.h>
#include <unistd.h>

#include "_elftc.h"

#include "test.h"
#include "test_case.h"

#include "driver.h"

#if defined(ELFTC_VCSID)
ELFTC_VCSID("$Id$");
#endif

enum selection_scope {
	SCOPE_TEST_CASE = 0,	/* c:STRING */
	SCOPE_TEST_FUNCTION,	/* f:STRING */
	SCOPE_TAG,		/* t:STRING */
};

/* Selection list entry. */
struct selection_option {
	STAILQ_ENTRY(selection_option)	so_next;

	/* The text to use for matching. */
	const char	*so_text;

	/*
	 * Whether matched test and test cases should be selected
	 * (if false) or deselected (if true).
	 */
	bool		so_select_tests;

	/* The kind of information to match. */
	enum selection_scope	so_selection_scope;
};

/* All selection options specified. */
STAILQ_HEAD(selection_option_list, selection_option);

static struct selection_option *
parse_selection_option(const char *option)
{
	int scope_char;
	bool select_tests;
	enum selection_scope scope;
	struct selection_option *so;

	scope_char = '\0';
	select_tests = true;
	scope = SCOPE_TEST_CASE;

	/* Deselection patterns start with a '-'. */
	if (*option == '-') {
		select_tests = false;
		option++;
	}

	/*
	 * If a scope was not specified, the selection scope defaults
	 * to SCOPE_TEST_CASE.
	 */
	if (strchr(option, ':') == NULL)
		scope_char = 'c';
	else {
		scope_char = *option++;
		if (*option != ':')
			return (NULL);
		option++;	/* Skip over the ':'. */
	}

	if (*option == '\0')
		return (NULL);

	switch (scope_char) {
	case 'c':
		scope = SCOPE_TEST_CASE;
		break;
	case 'f':
		scope = SCOPE_TEST_FUNCTION;
		break;
	case 't':
		scope = SCOPE_TAG;
		break;
	default:
		return (NULL);
	}

	so = calloc(1, sizeof(*so));
	so->so_text = option;
	so->so_selection_scope = scope;
	so->so_select_tests = select_tests;

	return (so);
}

/* Test execution styles. */
struct style_entry {
	enum test_run_style	se_style;
	const char		*se_name;
};

static const struct style_entry known_styles[] = {
	{ TR_STYLE_LIBTEST, "libtest" },
	{ TR_STYLE_TAP, "tap" },
	{ TR_STYLE_ATF, "atf" }
};

/*
 * Parse a test run style.
 *
 * This function returns true if the run style was recognized, or
 * false otherwise.
 */
static bool
parse_run_style(const char *option, enum test_run_style *run_style)
{
	size_t n;

	for (n = 0; n < sizeof(known_styles) / sizeof(known_styles[0]); n++) {
		if (strcasecmp(option, known_styles[n].se_name) == 0) {
			*run_style = known_styles[n].se_style;
			return (true);
		}
	}

	return (false);
}

/*
 * Return the canonical spelling of a test execution style.
 */
static const char *
to_execution_style_name(enum test_run_style run_style)
{
	size_t n;

	for (n = 0; n < sizeof(known_styles) / sizeof(known_styles[0]); n++) {
		if (known_styles[n].se_style == run_style)
			return (known_styles[n].se_name);
	}

	return (NULL);
}

/*
 * Parse a string value containing a positive integral number.
 */
static bool
parse_execution_time(const char *option, long *execution_time) {
	char *end;
	long value;

	if (option == NULL || *option == '\0')
		return (false);

	value = strtol(option, &end, 10);

	/* Check for parse errors. */
	if (*end != '\0')
		return (false);

	/* Reject negative numbers. */
	if (value < 0)
		return (false);

	/* Check for overflows during parsing. */
	if (value == LONG_MAX && errno == ERANGE)
		return (false);

	*execution_time = value;

	return (true);
}

static void
match_test_cases(struct test_case_selector_list *tl)
{
	/*Stub*/
	(void) tl;
}

static void
match_test_functions(struct test_case_selector_list *tl)
{
	/*Stub*/
	(void) tl;
}

static void
match_tags(struct test_case_selector_list *tl)
{
	/*Stub*/
	(void) tl;
}

/*
 * Add the selected tests to the test run.
 *
 * The memory used by the options list is returned to the system when this
 * function completes.
 */
static void
select_tests(struct test_run *tr,
    struct selection_option_list *selections)
{
	int i, j;
	struct selection_option *selection;
	const struct test_case_descriptor *tcd;
	struct test_case_selector *tcs;
	struct test_function_selector *tfs;
	struct test_case_selector_list *tcsl;
	bool default_selection_state;

	default_selection_state = STAILQ_EMPTY(selections);
	tcsl = NULL;

	/*
	 * Set up runtime descriptors.
	 */
	for (i = 0; i < test_case_count; i++) {
		if ((tcs = calloc(1, sizeof(*tcs))) == NULL)
			err(EX_OSERR, "cannot allocate a test-case selector");
		STAILQ_INSERT_TAIL(&tr->tr_test_cases, tcs, tcs_next);
		STAILQ_INIT(&tcs->tcs_functions);

		tcd = &test_cases[i];

		tcs->tcs_descriptor = tcd;
		tcs->tcs_has_selected_tests = default_selection_state;

		for (j = 0; j < tcd->tc_count; j++) {
			if ((tfs = calloc(1, sizeof(*tfs))) == NULL)
				err(EX_OSERR, "cannot allocate a test "
				    "function selector");
			STAILQ_INSERT_TAIL(&tcs->tcs_functions, tfs, tfs_next);

			tfs->tfs_descriptor = tcd->tc_tests + j;
			tfs->tfs_is_selected = default_selection_state;
		}
	}

	/*
	 * Set or reset the selection state based on the options.
	 */
	STAILQ_FOREACH(selection, selections, so_next) {
		tcsl = &tr->tr_test_cases;
		switch (selection->so_selection_scope) {
		case SCOPE_TEST_CASE:
			match_test_cases(tcsl);
			break;
		case SCOPE_TEST_FUNCTION:
			match_test_functions(tcsl);
			break;
		case SCOPE_TAG:
			match_tags(tcsl);
			break;
		}
	}

	/* Free up the selection list. */
	while (!STAILQ_EMPTY(selections)) {
		selection = STAILQ_FIRST(selections);
		STAILQ_REMOVE_HEAD(selections, so_next);
		free(selection);
	}
}

/*
 * Translate a file name to absolute form.
 *
 * The caller needs to free the returned pointer.
 */
static char *
to_absolute_path(const char *filename)
{
	size_t space_needed;
	char *absolute_path;
	char current_directory[PATH_MAX];

	if (filename == NULL || *filename == '\0')
		return (NULL);
	if (*filename == '/')
		return strdup(filename);

	if (getcwd(current_directory, sizeof(current_directory)) == NULL)
		err(1, "getcwd failed");

	/* Reserve space for the slash separator and the trailing NUL. */
	space_needed = strlen(current_directory) + strlen(filename) + 2;
	if ((absolute_path = malloc(space_needed)) == NULL)
		err(1, "malloc failed");
	if (snprintf(absolute_path, space_needed, "%s/%s", current_directory,
	    filename) != (int) (space_needed - 1))
		err(1, "snprintf failed");
	return (absolute_path);
}

/*
 * Display run parameters.
 */
static void
show_run_header(const struct test_run *tr)
{
	time_t start_time;
	struct test_search_path_entry *path_entry;

	if (tr->tr_verbosity == 0)
		return;

	printf("> test-run-name: %s\n", tr->tr_name);

	printf("> test-execution-style: %s\n",
	    to_execution_style_name(tr->tr_style));

	if (!STAILQ_EMPTY(&tr->tr_search_path)) {
		printf("> test-search-path:\n");
		STAILQ_FOREACH(path_entry, &tr->tr_search_path, tsp_next) {
			printf("+   %s\n", path_entry->tsp_directory);
		}
	}

	printf("> test-run-base-directory: %s\n",
	    tr->tr_runtime_base_directory);

	if (tr->tr_artefact_archive)
		printf("> test-artefact-archive: %s\n",
		    tr->tr_artefact_archive);

	printf("> test-execution-time: ");
	if (tr->tr_max_seconds_per_test == 0)
		printf("(unlimited)\n");
	else
		printf("%lu\n", tr->tr_max_seconds_per_test);

	printf("> test-case-count: %d\n", test_case_count);

	if (tr->tr_action == TEST_RUN_EXECUTE) {
		start_time = time(NULL);
		printf("> test-run-start-time: %s", ctime(&start_time));
	}
}

static void
show_run_trailer(const struct test_run *tr)
{
	time_t end_time;

	if (tr->tr_verbosity == 0)
		return;

	if (tr->tr_action == TEST_RUN_EXECUTE) {
		end_time = time(NULL);
		printf("> test-run-end-time: %s\n",
		    asctime(localtime(&end_time)));
	}
}

static int
show_listing(struct test_run *tr)
{
	const struct test_case_selector *tcs;
	const struct test_function_selector *tfs;
	const struct test_case_descriptor *tcd;
	const struct test_function_descriptor *tfd;

	STAILQ_FOREACH(tcs, &tr->tr_test_cases, tcs_next) {
		if (!tcs->tcs_has_selected_tests)
			continue;
		printf("%s", tcs->tcs_descriptor->tc_name);
		if (tr->tr_verbosity > 0) {
			tcd = tcs->tcs_descriptor;
			if (tcd->tc_description)
				printf(" :: %s", tcd->tc_description);
		}
		printf("\n");

		STAILQ_FOREACH(tfs, &tcs->tcs_functions, tfs_next) {
			if (tfs->tfs_is_selected) {
				tfd = tfs->tfs_descriptor;
				printf("  %s", tfd->tf_name);
				if (tr->tr_verbosity > 0 &&
				    tfd->tf_description)
					printf(" :: %s", tfd->tf_description);
				printf("\n");
			}
		}
	}

	return (EXIT_SUCCESS);
}

int
main(int argc, char **argv)
{
	struct test_run *tr;
	int exit_code, option;
	enum test_run_style run_style;
	struct selection_option *selector;
	struct selection_option_list selections =
	    STAILQ_HEAD_INITIALIZER(selections);

	tr = test_driver_allocate_run();

	/* Parse arguments. */
	while ((option = getopt(argc, argv, ":R:T:c:ln:p:s:t:v")) != -1) {
		switch (option) {
		case 'R':	/* Test runtime directory. */
			if (!test_driver_is_directory(optarg))
				errx(EX_USAGE, "option -%c: argument \"%s\" "
				    "does not name a directory.", option,
				    optarg);
			tr->tr_runtime_base_directory = realpath(optarg, NULL);
			if (tr->tr_runtime_base_directory == NULL)
				err(1, "realpath failed for \"%s\"", optarg);
			break;
		case 'T':	/* Max execution time for a test function. */
			if (!parse_execution_time(
			    optarg, &tr->tr_max_seconds_per_test))
				errx(EX_USAGE, "option -%c: argument \"%s\" "
				    "is not a valid execution time value.",
				    option, optarg);
			break;
		case 'c':	/* The archive holding artefacts. */
			tr->tr_artefact_archive = to_absolute_path(optarg);
			break;
		case 'l':	/* List matching tests. */
			tr->tr_action = TEST_RUN_LIST;
			break;
		case 'n':	/* Test run name. */
			if (tr->tr_name)
				free(tr->tr_name);
			tr->tr_name = strdup(optarg);
			break;
		case 'p':	/* Add a search path entry. */
			if (!test_driver_add_search_path(tr, optarg))
				errx(EX_USAGE, "option -%c: argument \"%s\" "
				    "does not name a directory.", option,
				    optarg);
			break;
		case 's':	/* Test execution style. */
			if (!parse_run_style(optarg, &run_style))
				errx(EX_USAGE, "option -%c: argument \"%s\" "
				    "is not a supported test execution style.",
				    option, optarg);
			tr->tr_style = run_style;
			break;
		case 't':	/* Test selection option. */
			if ((selector = parse_selection_option(optarg)) == NULL)
				errx(EX_USAGE, "option -%c: argument \"%s\" "
				    "is not a valid selection pattern.",
				    option, optarg);
			STAILQ_INSERT_TAIL(&selections, selector, so_next);
			break;
		case 'v':
			tr->tr_verbosity++;
			break;
		case ':':
			errx(EX_USAGE,
			    "ERROR: option -%c requires an argument.", optopt);
			break;
		case '?':
			errx(EX_USAGE,
			    "ERROR: unrecognized option -%c", optopt);
			break;
		default:
			errx(EX_USAGE, "ERROR: unspecified error.");
			break;
		}
	}

	/*
	 * Set unset fields of the test run descriptor to their
	 * defaults.
	 */
	if (!test_driver_finish_run_initialization(tr, argv[0]))
		err(EX_OSERR, "cannot initialize test driver");

	/* Choose tests and test cases to act upon. */
	select_tests(tr, &selections);

	assert(STAILQ_EMPTY(&selections));

	show_run_header(tr);

	/* Perform the requested action. */
	switch (tr->tr_action) {
	case TEST_RUN_LIST:
		exit_code = show_listing(tr);
		break;

	case TEST_RUN_EXECUTE:
	default:
		/* Not yet implemented. */
		exit_code = EX_UNAVAILABLE;
	}

	show_run_trailer(tr);

	test_driver_free_run(tr);

	exit(exit_code);
}
