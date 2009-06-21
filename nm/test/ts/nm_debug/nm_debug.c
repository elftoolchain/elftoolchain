/*-
 * Copyright (c) 2008 Hyogeol Lee <hyogeollee@gmail.com>
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
 * THIS SOFTWARE IS PROVIDED BY THE AUTHORS ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include <tet_api.h>

static int	exec_cmd(const char *, const char *);
static void	startup();
static void	test_debug();
static bool	test_nm_out(const char *, const char *);

struct tet_testlist tet_testlist[] = {
	{ test_debug, 1},
	{ NULL, 0}
};

#define	NM_CMD		NM " %s " TESTFILE " > test.out"
#define DIFF_CMD	"diff test.out " TC_DIR "/" TESTFILE "%s.txt > /dev/null"

void (*tet_startup)() = startup;
void (*tet_cleanup)() = NULL;

static int
exec_cmd(const char *cmd, const char *op)
{
	char *this_cmd;
	int rtn;
	size_t cmd_len;

	if (cmd == NULL || op == NULL)
		return (-1);

	cmd_len = strlen(cmd) + strlen(op);

	if ((this_cmd = malloc(sizeof(char) * cmd_len)) == NULL) {
		tet_infoline("cannot allocate memory");

		return (-1);
	}

	snprintf(this_cmd, cmd_len, cmd, op);

	rtn = system(this_cmd);

	free(this_cmd);

	return (rtn);
}

static void
startup()
{

	if (system("cp " TC_DIR "/" TESTFILE " .") < 0) {
		tet_infoline("cannot cp object");
		
		exit(EX_SOFTWARE);
	}
}

static void
test_debug()
{
	bool rtn = true;

	tet_infoline("OPTION -a, --debug-syms");

	rtn |= test_nm_out("-a", "-a");
	rtn |= test_nm_out("--debug-syms", "-a");

	tet_result(rtn == true ? TET_PASS : TET_FAIL);
}

static bool
test_nm_out(const char *op, const char *d_op)
{
	int rtn;

	if (op == NULL) {
		tet_result(TET_FAIL);

		return (false);
	}

	if ((rtn = exec_cmd(NM_CMD, op)) < 0) {
		tet_infoline("system function failed");

		return (false);
	} else if (rtn == 127) {
		tet_infoline("execution shell failed");

		return (false);
	}

	if ((rtn = exec_cmd(DIFF_CMD, d_op)) < 0)
		tet_infoline("system function failed");
	else {
		switch (rtn) {
		case 127:
			tet_infoline("execution shell failed");

			break;
		case 2:
			tet_infoline("diff has trouble");

			break;
		case 1:
			tet_infoline("output is different");

			break;
		case 0:
			return (true);
		}
	}

	return (false);
}
