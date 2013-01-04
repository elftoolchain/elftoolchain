/*-
 * Copyright (c) 2013 Joseph Koshy
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

/*
 * include(`elfts.m4')
 */


#include <errno.h>
#include <libelftc.h>
#include <stdlib.h>
#include <string.h>

#include "tet_api.h"

/*
 * A list of test strings.
 *
 * For the curious, these are titles of stories by Cordwainer Smith.
 */

static const char *test_strings[] = {
	"Mark Elf",
	"Scanners Live in Vain",
	"The Dead Lady of Clown Town",
	"The Lady Who Sailed the \"The Soul\"",
	"No, No, Not Rogov!",
	NULL
};

const int nteststrings = sizeof(test_strings) / sizeof(test_strings[0]);

/*
 * Verify that strings are inserted at the expected offsets, and that
 * the returned value is equivalent to the original string.
 */
void
tcInsertReturnValues(void)
{
	int	result;
	const char **s, *str;
	unsigned int expectedindex, hashindex;
	Elftc_String_Table *table;

	result = TET_UNRESOLVED;

	TP_ANNOUNCE("Verify return values from lookup().");

	if ((table = elftc_string_table_create(0)) == NULL) {
		TP_UNRESOLVED("elftc_string_table_create() failed: %s",
		    strerror(errno));
		goto done;
	}

	expectedindex = 1;
	/* Insert test strings. */
	for (s = test_strings; *s != NULL; s++) {
		str = elftc_string_table_insert(table, *s, &hashindex);
		if (str == NULL || strcmp(str, *s)) {
			TP_FAIL("return value is incorrect (%p) \"%s\".", str,
				str ? str : "");
			goto done;
		}

		if (hashindex != expectedindex) {
			TP_FAIL("incorrect hash index: expected %d, actual %d",
				expectedindex, hashindex);
			goto done;
		}

		expectedindex += strlen(*s) + 1;
	}

	result = TET_PASS;

done:
	if (table)
		(void) elftc_string_table_destroy(table);
	tet_result(result);
}

/*
 * Verify that multiple insertions of the same string yield the same
 * return values and offsets.
 */

void
tcInsertDuplicate(void)
{
	int	n, result;
	const char **s, **savedptrs, *str;
	unsigned int hindex, *hashrecord;
	Elftc_String_Table *table;

	result = TET_UNRESOLVED;

	TP_ANNOUNCE("Verify return from multiple insertions.");

	hashrecord = NULL;
	savedptrs = NULL;

	if ((table = elftc_string_table_create(0)) == NULL) {
		TP_UNRESOLVED("elftc_string_table_create() failed: %s",
		    strerror(errno));
		goto done;
	}

	if ((hashrecord = malloc(nteststrings*sizeof(*hashrecord))) == NULL) {
		TP_UNRESOLVED("memory allocation failed.");
		goto done;
	}

	if ((savedptrs = malloc(nteststrings*sizeof(char *))) == NULL) {
		TP_UNRESOLVED("memory allocation failed.");
		goto done;
	}

	/* Insert test strings. */
	for (n = 0, s = test_strings; *s != NULL; s++, n++)
		savedptrs[n] = elftc_string_table_insert(table, *s,
		    &hashrecord[n]);

	/* Re-insert, and verify the returned pointers and offsets. */
	for (n = 0, s = test_strings; *s != NULL; s++, n++) {
		str = elftc_string_table_insert(table, *s, &hindex);

		if (str == NULL || str != savedptrs[n]) {
			TP_FAIL("Lookup of \"%s\" returned (%p) \"%s\" "
			    "& (%p) \"%s\".", *s, str, str, savedptrs[n],
			    savedptrs[n]);
			goto done;
		}

		if (hindex != hashrecord[n]) {
			TP_FAIL("incorrect hash index: expected %d, actual %d",
				hashrecord[n], hindex);
			goto done;
		}
	}

	result = TET_PASS;

done:
	if (table)
		(void) elftc_string_table_destroy(table);
	free(hashrecord);
	free(savedptrs);

	tet_result(result);
}

/*
 * Verify that the lookup() API returns the expected
 * values.
 */

void
tcLookupReturn(void)
{
	int	result;
	const char **s, *str;
	unsigned int expectedindex, hashindex;
	Elftc_String_Table *table;

	result = TET_UNRESOLVED;

	TP_ANNOUNCE("Verify lookup() after insertion.");

	if ((table = elftc_string_table_create(0)) == NULL) {
		TP_UNRESOLVED("elftc_string_table_create() failed: %s",
		    strerror(errno));
		goto done;
	}

	/* Insert test strings. */
	for (s = test_strings; *s != NULL; s++)
		str = elftc_string_table_insert(table, *s, NULL);

	expectedindex = 1;
	for (s = test_strings; *s != NULL; s++) {
		str = elftc_string_table_lookup(table, *s, &hashindex);

		if (str == NULL || strcmp(str, *s)) {
			TP_UNRESOLVED("Lookup of \"%s\" returned \"%s\".", *s,
			    str);
			goto done;
		}

		if (hashindex != expectedindex) {
			TP_FAIL("incorrect hash index: expected %d, actual %d",
				expectedindex, hashindex);
			goto done;
		}

		expectedindex += strlen(*s) + 1;
	}

	result = TET_PASS;

done:
	if (table)
		(void) elftc_string_table_destroy(table);
	tet_result(result);
}

/*
 * Verify that multiple lookups return the same pointer
 * and string offset.
 */

void
tcLookupDuplicate(void)
{
	int	n, result;
	const char **s, *str1, *str2;
	unsigned int hindex1, hindex2, *hashrecord;
	Elftc_String_Table *table;

	result = TET_UNRESOLVED;

	TP_ANNOUNCE("Check multiple invocations of lookup.");

	hashrecord = NULL;

	if ((table = elftc_string_table_create(0)) == NULL) {
		TP_UNRESOLVED("elftc_string_table_create() failed: %s",
		    strerror(errno));
		goto done;
	}

	if ((hashrecord = malloc(nteststrings*sizeof(*hashrecord))) == NULL) {
		TP_UNRESOLVED("memory allocation failed.");
		goto done;
	}

	/* Insert test strings. */
	for (n = 0, s = test_strings; *s != NULL; s++, n++)
		(void) elftc_string_table_insert(table, *s, &hashrecord[n]);

	for (n = 0, s = test_strings; *s != NULL; s++, n++) {
		str1 = elftc_string_table_lookup(table, *s, &hindex1);
		str2 = elftc_string_table_lookup(table, *s, &hindex2);

		if (str1 == NULL || str2 == NULL || str1 != str2 ||
		    strcmp(str1, *s)) {
			TP_FAIL("Lookup of \"%s\" returned \"%s\" & \"%s\".",
			    *s, str1, str2);
			goto done;
		}

		if (hindex1 != hindex2 || hindex1 != hashrecord[n]) {
			TP_FAIL("incorrect hash index: expected %d, actual %d & %d",
				hashrecord[n], hindex1, hindex2);
			goto done;
		}
	}

	result = TET_PASS;

done:
	if (table)
		(void) elftc_string_table_destroy(table);
	free(hashrecord);

	tet_result(result);
}

/*
 * Verify that a deleted string cannot be subsequently looked up.
 */

void
tcDeletionCheck(void)
{
	const char **s, *str;
	int hindex, n, result, status;
	Elftc_String_Table *table;

	result = TET_UNRESOLVED;

	TP_ANNOUNCE("Verify deletion of strings.");

	if ((table = elftc_string_table_create(0)) == NULL) {
		TP_UNRESOLVED("elftc_string_table_create() failed: %s",
		    strerror(errno));
		goto done;
	}

	/* Insert test strings. */
	for (n = 0, s = test_strings; *s != NULL; s++, n++)
		(void) elftc_string_table_insert(table, *s, NULL);

	/* Delete strings, and look them up. */
	for (n = 0, s = test_strings; *s != NULL; s++, n++) {
		status = elftc_string_table_remove(table, *s);
		if (status == 0) {
			TP_FAIL("Deletion of \"%s\" failed.", *s);
			goto done;
		}

		hindex = 0;
		str = elftc_string_table_lookup(table, *s, &hindex);
		if (str != NULL || hindex != 0) {
			TP_FAIL("Lookup of \"%s\" succeeded unexpectedly.");
			goto done;
		}
	}

	result = TET_PASS;

done:
	if (table)
		(void) elftc_string_table_destroy(table);

	tet_result(result);

}

/*
 * Verify that a deleted string is re-inserted at the old index.
 */

/*
 * Verify that the 2nd deletion of the string fails.
 */

/*
 * Verify that deletion of an unknown string fails.
 */
