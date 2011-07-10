/*-
 * Copyright (c) 2008 Hyogeol Lee
 * Copyright (c) 2000, 2001 David O'Brien
 * Copyright (c) 1996 Søren Schmidt
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
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
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

#include <sys/cdefs.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <gelf.h>
#include <libelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "_elftc.h"

ELFTC_VCSID("$Id$");

static int elftype(const char *);
static const char *iselftype(int);
static Elf *openelf(int, const char *);
static void printelftypes(void);
static void usage(void);

struct ELFtypes {
	const char *str;
	int value;
};
/* XXX - any more types? */
static struct ELFtypes elftypes[] = {
	{ "86Open",	ELFOSABI_86OPEN },
	{ "AIX",	ELFOSABI_AIX },
	{ "ARM",	ELFOSABI_ARM },
	{ "AROS",	ELFOSABI_AROS },
	{ "FreeBSD",	ELFOSABI_FREEBSD },
	{ "GNU",	ELFOSABI_GNU },
	{ "HP/UX",	ELFOSABI_HPUX},
	{ "Hurd",	ELFOSABI_HURD },
	{ "IRIX",	ELFOSABI_IRIX },
	{ "Linux",	ELFOSABI_GNU },
	{ "Modesto",	ELFOSABI_MODESTO },
	{ "NSK",	ELFOSABI_NSK },
	{ "NetBSD",	ELFOSABI_NETBSD},
	{ "None",	ELFOSABI_NONE},
	{ "OpenBSD",	ELFOSABI_OPENBSD },
	{ "OpenVMS",	ELFOSABI_OPENVMS },
	{ "Standalone",	ELFOSABI_STANDALONE },
	{ "SVR4",	ELFOSABI_NONE },
	{ "Solaris",	ELFOSABI_SOLARIS },
	{ "Tru64",	ELFOSABI_TRU64 }
};

int
main(int argc, char **argv)
{
	GElf_Ehdr ehdr;
	Elf *elf;
	const char *strtype = "FreeBSD";
	int type = ELFOSABI_FREEBSD;
	int retval = 0;
	int ch, change = 0, verbose = 0, force = 0, listed = 0, e_err = 0;

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(1, "elf_version error");

	while ((ch = getopt(argc, argv, "f:lt:v")) != -1)
		switch (ch) {
		case 'f':
			if (change)
				errx(1, "ERROR: the -f option is incompatible "
				    "with the -t option.");
			force = 1;
			type = atoi(optarg);
			if (errno == ERANGE || type < 0 || type > 255) {
				warnx("ERROR: invalid argument to option "
				    "-f: %s", optarg);
				usage();
			}
			break;
		case 'l':
			printelftypes();
			listed = 1;
			break;
		case 'v':
			verbose = 1;
			break;
		case 't':
			if (force)
				errx(1, "the -t option is incompatible with "
				    "the -f option.");
			change = 1;
			strtype = optarg;
			break;
		default:
			usage();
	}
	argc -= optind;
	argv += optind;
	if (!argc) {
		if (listed)
			exit(0);
		else {
			warnx("no file(s) specified");
			usage();
		}
	}

	if (!force && (type = elftype(strtype)) == -1) {
		warnx("ERROR: invalid ELF type '%s'", strtype);
		printelftypes();
		usage();
	}

	while (argc) {
		int fd;

		if ((fd = open(argv[0], change || force ? O_RDWR : O_RDONLY, 0))
		    < 0) {
			warn("error opening file %s", argv[0]);
			retval = 1;
			goto fail;
		}
		if ((elf = openelf(fd, argv[0])) == NULL) {
			retval = 1;
			elf_end(elf);
			goto fail;
		}
		if (gelf_getehdr(elf, &ehdr) == NULL) {
			if ((e_err = elf_errno()) != 0)
				warnx("gelf_getehdr : %s",
				    elf_errmsg(e_err));
			else
				warnx("gelf_getehdr error");
			retval = 1;
			elf_end(elf);
			goto fail;
		}
		if (!change && !force) {
			fprintf(stdout,
			    "File '%s' is of brand '%s' (%u).\n",
			    argv[0], iselftype(ehdr.e_ident[EI_OSABI]),
			    ehdr.e_ident[EI_OSABI]);
			if (!iselftype(type)) {
				warnx("ELF ABI Brand '%u' is unknown",
				      type);
				printelftypes();
			}
		}
		else {
			ehdr.e_ident[EI_OSABI] = type;
			if (gelf_update_ehdr(elf, &ehdr) == 0) {
				if ((e_err = elf_errno()) != 0)
					warnx("gelf_update_ehdr error : %s",
					    elf_errmsg(e_err));
				else
					warnx("gelf_update_ehdr error");
				retval = 1;
				elf_end(elf);
				goto fail;
			}

			if (elf_update(elf, ELF_C_WRITE) == -1) {
				if ((e_err = elf_errno()) != 0)
					warnx("elf_update error : %s",
					    elf_errmsg(e_err));
				else
					warnx("elf_update error");
				retval = 1;
				elf_end(elf);
				goto fail;
			}
		}
fail:

		if (close(fd) == -1) {
			warnx("%s: close error", argv[0]);
			retval = 1;
		}
		argc--;
		argv++;
	}

	return (retval);
}

static void
usage(void)
{
	(void)fprintf(stderr,
	    "usage: brandelf [-lv] [-f ELF_ABI_number] [-t brand] file ...\n");
	exit(1);
}

static const char *
iselftype(int etype)
{
	size_t elfwalk;

	for (elfwalk = 0;
	     elfwalk < sizeof(elftypes)/sizeof(elftypes[0]);
	     elfwalk++)
		if (etype == elftypes[elfwalk].value)
			return (elftypes[elfwalk].str);
	return (0);
}

static int
elftype(const char *elfstrtype)
{
	size_t elfwalk;

	for (elfwalk = 0;
	     elfwalk < sizeof(elftypes)/sizeof(elftypes[0]);
	     elfwalk++)
		if (strcasecmp(elfstrtype, elftypes[elfwalk].str) == 0)
			return (elftypes[elfwalk].value);
	return (-1);
}

static Elf *
openelf(int fd, const char *name)
{
	Elf *elf;
	Elf_Cmd cmd;
	Elf_Kind kind;
	int e_err = 0;

	if (fd < 0)
		return (NULL);

	/* Archive does not support write mode. So try ar type by read cmd. */
	cmd = ELF_C_READ;
begin:
	if ((elf = elf_begin(fd, cmd, (Elf *) NULL)) == NULL) {
		if ((e_err = elf_errno()) != 0)
			warnx("elf_begin error : %s",
			    elf_errmsg(e_err));
		else
			warnx("elf_begin error");

		return (NULL);
	}

	if ((kind = elf_kind(elf)) != ELF_K_ELF) {
		if (kind == ELF_K_AR)
			warnx("file '%s' is an archive.", name);
		else
			warnx("file '%s' is not ELF format", name);

		return (NULL);
	}

	if (cmd == ELF_C_RDWR)
		return (elf);

	elf_end(elf);

	cmd = ELF_C_RDWR;

	goto begin;
}

static void
printelftypes(void)
{
	size_t elfwalk;

	fprintf(stderr, "Known ELF types are: ");
	for (elfwalk = 0;
	     elfwalk < sizeof(elftypes)/sizeof(elftypes[0]);
	     elfwalk++)
		fprintf(stderr, "%s(%u) ", elftypes[elfwalk].str, 
			elftypes[elfwalk].value);
	fprintf(stderr, "\n");
}
