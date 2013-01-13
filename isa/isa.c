/*-
 * Copyright (c) 2012,2013 Joseph Koshy
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

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <libelftc.h>

#include "_elftc.h"

ELFTC_VCSID("$Id$");

#define	ISA_MAX_LONG_OPTION_LENGTH	64

static struct option isa_long_options[] = {
	{ "arch",  required_argument, NULL, 'a' },
	{ "cpu", required_argument, NULL, 'c' },
	{ "dry-run", no_argument, NULL, 'n' },
	{ "input", required_argument, NULL, 'i' },
	{ "list-instructions", no_argument, NULL, 'L' },
	{ "ntests", required_argument, NULL, 'N' },
	{ "output", required_argument, NULL, 'o' },
	{ "prefix", required_argument, NULL, 'p' },
	{ "quiet", no_argument, NULL, 'q' },
	{ "random-seed", required_argument, NULL, 'R' },
	{ "spec", required_argument, NULL, 's' },
	{ "test", no_argument, NULL, 'T' },
	{ "verbose", no_argument, NULL, 'v' },
	{ "version", no_argument, NULL, 'V' },
	{ NULL, 0, NULL, 0 }
};

static const char *isa_usage_message = "\
usage: %s [options] [command] [specfiles]...\n\
    Process an instruction set specification.\n\
\n\
Supported values for 'command' are:\n\
    decode	Build an instruction stream decoder.\n\
    encode	Build an instruction stream encoder.\n\
    query	(default) Retrieve information about an instruction set.\n\
\n\
Supported global options are:\n\
    -a ARCH | --arch ARCH    Process instruction specifications for ARCH.\n\
    -c CPU  | --cpu CPU      Process instruction specifications for CPU.\n\
    -n      | --dry-run      Exit after checking inputs for errors.\n\
    -s FILE | --spec FILE    Read instruction specifications from FILE.\n\
    -q      | --quiet        Suppress warning messages.\n\
    -v      | --verbose      Be verbose.\n\
    -V      | --version      Display a version identifier and exit.\n\
\n\
Supported options for command 'decode' are:\n\
    -i FILE | --input FILE   Read source to be expanded from FILE.\n\
    -o FILE | --output FILE  Write generated output to FILE.\n\
\n\
Supported options for command 'encode' are:\n\
    -o FILE | --output FILE  Write generated output to FILE.\n\
    -p STR | --prefix STR    Use STR as a prefix for generated symbols.\n\
\n\
Supported options for command 'query' are:\n\
    -L | --list-instructions Generate a list of all known instructions.\n\
    -N NUM | --ntests NUM    Specify the number of test sequences generated.\n\
    -R N   | --random-seed N Use N as the random number generator seed.\n\
    -T     | --test          Generate test sequences.\n\
";

void
isa_usage(void)
{
	(void) fprintf(stderr, isa_usage_message, ELFTC_GETPROGNAME());
	exit(1);
}

void
isa_unimplemented(int option, int option_index, struct option *options_table)
{
	char msgbuf[ISA_MAX_LONG_OPTION_LENGTH];

	if (option_index >= 0)
		(void) snprintf(msgbuf, sizeof(msgbuf), "\"--%s\"",
		    options_table[option_index].name);
	else
		(void) snprintf(msgbuf, sizeof(msgbuf), "'-%c'",
		    option);
	errx(1, "ERROR: option %s is unimplemented.", msgbuf);
}

int
main(int argc, char **argv)
{
	int option, option_index;

	for (option_index = -1;
	     (option = getopt_long(argc, argv, "a:c:i:no:p:qs:vLN:R:TV",
		 isa_long_options, &option_index)) != -1;
	     option_index = -1) {
		switch (option) {
		case 'V':
			(void) printf("%s (%s)\n", ELFTC_GETPROGNAME(),
			    elftc_version());
			exit(0);
			break;

		case 'C': case 'D': case 'E':
		case 'L': case 'M': case 'm':
		case 'o': case 'p': case 'R':
		case 'Z':
			isa_unimplemented(option, option_index,
			    isa_long_options);
			break;
		default:
			isa_usage();
			break;
		}
	}
}

