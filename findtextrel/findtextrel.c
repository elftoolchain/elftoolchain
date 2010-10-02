/*-
 * Copyright (c) 2010 Kai Wang
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

#include <sys/cdefs.h>
#include <sys/param.h>
#include <err.h>
#include <dwarf.h>
#include <fcntl.h>
#include <gelf.h>
#include <getopt.h>
#include <libdwarf.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "_elftc.h"

ELFTC_VCSID("$Id$");

static struct option longopts[] = {
	{"help", no_argument, NULL, 'H'},
	{"version", no_argument, NULL, 'V'},
	{NULL, 0, NULL, 0}
};

static void
usage(void)
{

	fprintf(stderr, "usage: %s [files ...]\n", ELFTC_GETPROGNAME());
	exit(1);
}

static void
version(void)
{

	fprintf(stderr, "%s 1.0\n", ELFTC_GETPROGNAME());
	exit(0);
}

static const char *
find_symbol(const char *fn, Elf *e, Elf_Data *d, GElf_Shdr *sh, uintmax_t off)
{
	GElf_Sym sym;
	const char *name;
	int i, len;

	len = d->d_size / sh->sh_entsize;
	for (i = 0; i < len; i++) {
		if (gelf_getsym(d, i, &sym) != &sym) {
			warnx("%s: gelf_getsym() failed: %s", fn,
			    elf_errmsg(-1));
			continue;
		}
		if (GELF_ST_TYPE(sym.st_info) != STT_FUNC)
			continue;
		if (off >= sym.st_value && off < sym.st_value + sym.st_size) {
			name = elf_strptr(e, sh->sh_link, sym.st_name);
			if (name == NULL)
				warnx("%s: elf_strptr() failed: %s", fn,
				    elf_errmsg(-1));
			return (name);
		}
	}

	return (NULL);
}

static void
report_textrel(const char *fn, Elf *e, Dwarf_Debug dbg, uintmax_t off,
    int *textrel)
{
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Shdr sh;
	const char *name;
	int elferr;

	if (!*textrel) {
		printf("%s: ELF object contains text relocation records:\n",
		    fn);
		*textrel = 1;
	}

	if (dbg == NULL) {
		printf("%s:   off: %#jx", fn, off);
		scn = NULL;
		while ((scn = elf_nextscn(e, scn)) != NULL) {
			if (gelf_getshdr(scn, &sh) == NULL) {
				warnx("%s: gelf_getshdr() failed: %s", fn,
				    elf_errmsg(-1));
				continue;
			}
			if (sh.sh_type != SHT_DYNSYM &&
			    sh.sh_type != SHT_SYMTAB)
				continue;
			(void) elf_errno();
			if ((d = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0)
					warnx("%s: elf_getdata() failed: %s",
					    fn, elf_errmsg(-1));
				continue;
			}
			if (d->d_size <= 0)
				continue;
			if ((name = find_symbol(fn, e, d, &sh, off)) != NULL) {
				printf(", func: %s", name);
				break;
			}
		}
		elferr = elf_errno();
		if (elferr != 0)
			warnx("%s: elf_nextscn() failed: %s", fn,
			    elf_errmsg(elferr));
	}

	putchar('\n');
}

static void
examine_reloc(const char *fn, Elf *e, Elf_Data *d, GElf_Shdr *sh, GElf_Phdr *ph,
    int phnum, Dwarf_Debug dbg, int *textrel)
{
	GElf_Rel rel;
	GElf_Rela rela;
	int i, j, len;

	len = d->d_size / sh->sh_entsize;
	for (i = 0; i < len; i++) {
		if (sh->sh_type == SHT_REL) {
			if (gelf_getrel(d, i, &rel) != &rel) {
				warnx("%s: gelf_getrel() failed: %s", fn,
				    elf_errmsg(-1));
				continue;
			}
		} else {
			if (gelf_getrela(d, i, &rela) != &rela) {
				warnx("%s: gelf_getrela() failed: %s", fn,
				    elf_errmsg(-1));
				continue;
			}
		}
		for (j = 0; j < phnum; j++) {
			if (sh->sh_type == SHT_REL) {
				if (rel.r_offset >= ph[j].p_offset &&
				    rel.r_offset < ph[j].p_offset +
				    ph[j].p_filesz)
					report_textrel(fn, e, dbg,
					    (uintmax_t) rel.r_offset, textrel);
			} else {
				if (rela.r_offset >= ph[j].p_offset &&
				    rela.r_offset < ph[j].p_offset +
				    ph[j].p_filesz)
					report_textrel(fn, e, dbg,
					    (uintmax_t) rela.r_offset, textrel);
			}
		}
	}
}

static void
find_textrel(const char *fn)
{
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Ehdr eh;
	GElf_Phdr *ph;
	GElf_Shdr sh;
	Dwarf_Debug dbg;
	Dwarf_Error de;
	int elferr, fd, i, phnum, textrel;

	e = NULL;
	ph = NULL;
	dbg = NULL;

	if ((fd = open(fn, O_RDONLY)) < 0) {
		warn("%s", fn);
		return;
	}

	if ((e = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		warnx("%s: elf_begin() failed: %s", fn, elf_errmsg(-1));
		goto exit;
	}

	if (gelf_getehdr(e, &eh) != &eh) {
		warnx("%s: gelf_getehdr() failed: %s", fn, elf_errmsg(-1));
		goto exit;
	}

	if (eh.e_type != ET_DYN) {
		printf("%s: ELF object is not a DSO/PIE\n", fn);
		goto exit;
	}

	/*
	 * Search program header for executable segments.
	 */

	if (eh.e_phnum == 0) {
		printf("%s: ELF object does not contain progream headers\n",
		    fn);
		goto exit;
	}
	if ((ph = calloc(eh.e_phnum, sizeof(GElf_Phdr))) == NULL)
		err(1, "calloc");
	phnum = 0;
	for (i = 0; (unsigned) i < eh.e_phnum; i++) {
		if (gelf_getphdr(e, i, &ph[phnum]) != &ph[phnum]) {
			warnx("%s: gelf_getphdr() failed: %s", fn,
			    elf_errmsg(-1));
			continue;
		}
		if (ph[phnum].p_flags & PF_X)
			phnum++;
	}
	if (phnum == 0) {
		printf("%s: ELF object does not contain any executable "
		    "segment\n", fn);
		goto exit;
	}

	/* Check if debugging information is available. */
	if (dwarf_elf_init(e, DW_DLC_READ, NULL, NULL, &dbg, &de))
		dbg = NULL;
	dbg = NULL;

	/*
	 * Search relocation records for possible text relocations.
	 */
	textrel = 0;
	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) == NULL) {
			warnx("%s: gelf_getshdr() failed: %s", fn,
			    elf_errmsg(-1));
			continue;
		}
		if (sh.sh_type == SHT_REL || sh.sh_type == SHT_RELA) {
			(void) elf_errno();
			if ((d = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0)
					warnx("%s: elf_getdata() failed: %s",
					    fn, elf_errmsg(-1));
				continue;
			}
			if (d->d_size <= 0)
				continue;
			examine_reloc(fn, e, d, &sh, ph, phnum, dbg, &textrel);
		}
	}
	elferr = elf_errno();
	if (elferr != 0)
		warnx("%s: elf_nextscn() failed: %s", fn, elf_errmsg(elferr));

	if (!textrel)
		printf("%s: ELF object does not contain any text relocation\n",
		    fn);

exit:
	if (dbg)
		dwarf_finish(dbg, &de);
	if (ph)
		free(ph);
	if (e)
		(void) elf_end(e);
	close(fd);
}

int
main(int argc, char **argv)
{
	int i, opt;

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(1, "elf_version(): %s", elf_errmsg(-1));

	while ((opt = getopt_long(argc, argv, "HV", longopts, NULL)) != -1) {
		switch (opt) {
		case 'H':
			usage();
		case 'V':
			version();
		default:
			usage();
		}
	}

	argv += optind;
	argc -= optind;

	if (argc > 0)
		for (i = 0; i < argc; i++)
			find_textrel(argv[i]);
	else
		find_textrel("a.out");

	exit(0);
}
