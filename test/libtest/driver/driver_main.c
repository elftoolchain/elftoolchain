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

/* Selection list entry. */
struct selection_option {
	STAILQ_ENTRY(selection_option)	s_next;
	const char	*s_option;
};

/* All selection options specified. */
STAILQ_HEAD(selection_option_list, selection_option);

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

/* Add the selected tests to the test run. */
static void
select_tests(struct test_run *tr,
    const struct selection_option_list *selections)
{
	/*Stub*/
	(void) tr;
	(void) selections;
}

static void
free_selections(struct selection_option_list *selections)
{
	struct selection_option *selection;

	while (!STAILQ_EMPTY(selections)) {
		selection = STAILQ_FIRST(selections);
		STAILQ_REMOVE_HEAD(selections, s_next);
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
	/*Stub*/
	(void) tr;
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
		case 'T':	/* Max execution time for a test case. */
			/* TODO convert optarg to int. */
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
				errx(EX_USAGE, "option -%c: unrecognized run "
				    "style \"%s\"", option, optarg);
			tr->tr_style = run_style;
			break;
		case 't':	/* Test selection option. */
			/*TODO check selection syntax. */
			selector =
			    calloc(1, sizeof(struct selection_option));
			selector->s_option = optarg;
			STAILQ_INSERT_TAIL(&selections, selector, s_next);
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

	/* Choose the tests and test cases to act upon. */
	select_tests(tr, &selections);

	/* Free up selections. */
	free_selections(&selections);

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
