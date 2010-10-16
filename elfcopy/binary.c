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
#include <sys/mman.h>
#include <sys/stat.h>
#include <err.h>
#include <gelf.h>
#include <stdio.h>
#include <sysexits.h>
#include <unistd.h>

#include "elfcopy.h"

ELFTC_VCSID("$Id$");

/*
 * Convert ELF object to `binary'. Sections with SHF_ALLOC flag set
 * are copied to the result binary. The relative offsets for each section
 * are retained, so the result binary file might contain "holes".
 */
void
create_binary(int ifd, int ofd)
{
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Shdr sh;
	off_t base, off;
	int elferr;

	if ((e = elf_begin(ifd, ELF_C_READ, NULL)) == NULL)
		errx(EX_DATAERR, "elf_begin() failed: %s",
		    elf_errmsg(-1));

	base = 0;
	if (lseek(ofd, base, SEEK_SET) < 0)
		err(EX_DATAERR, "lseek failed");

	/*
	 * Find base offset in the first iteration.
	 */
	base = -1;
	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) == NULL) {
			warnx("gelf_getshdr failed: %s", elf_errmsg(-1));
			(void) elf_errno();
			continue;
		}
		if ((sh.sh_flags & SHF_ALLOC) == 0 ||
		    sh.sh_type == SHT_NOBITS ||
		    sh.sh_size == 0)
			continue;
		if (base == -1 || (off_t) sh.sh_offset < base)
			base = sh.sh_offset;
	}	
	elferr = elf_errno();
	if (elferr != 0)
		warnx("elf_nextscn failed: %s", elf_errmsg(elferr));

	if (base == -1)
		return;

	/*
	 * Write out sections in the second iteration.
	 */
	scn = NULL;
	while ((scn = elf_nextscn(e, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) == NULL) {
			warnx("gelf_getshdr failed: %s", elf_errmsg(-1));
			(void) elf_errno();
			continue;
		}
		if ((sh.sh_flags & SHF_ALLOC) == 0 ||
		    sh.sh_type == SHT_NOBITS ||
		    sh.sh_size == 0)
			continue;
		(void) elf_errno();
		if ((d = elf_getdata(scn, NULL)) == NULL) {
			elferr = elf_errno();
			if (elferr != 0)
				warnx("elf_getdata failed: %s", elf_errmsg(-1));
			continue;
		}
		if (d->d_buf == NULL || d->d_size == 0)
			continue;

		/* lseek to section offset relative to `base'. */
		off = sh.sh_offset - base;
		if (lseek(ofd, off, SEEK_SET) < 0)
			err(EX_DATAERR, "lseek failed");

		/* Write out section contents. */
		if (write(ofd, d->d_buf, d->d_size) != (ssize_t) d->d_size)
			err(EX_DATAERR, "write failed");
	}
	elferr = elf_errno();
	if (elferr != 0)
		warnx("elf_nextscn failed: %s", elf_errmsg(elferr));
}

/*
 * Convert `binary' to ELF object. The input `binary' is converted to
 * a relocatable (.o) file, a few symbols will also be created to make
 * it easier to access the binary data in other compilation units.
 */
void
create_elf_from_binary(struct elfcopy *ecp, int ifd)
{
	struct section *s, *sec, *sec_temp, *shtab;
	struct stat sb;
	GElf_Ehdr oeh;
	GElf_Shdr osh;
	Elf_Data *od;

	/* Reset internal section list. */
	if (!TAILQ_EMPTY(&ecp->v_sec))
		TAILQ_FOREACH_SAFE(sec, &ecp->v_sec, sec_list, sec_temp) {
			TAILQ_REMOVE(&ecp->v_sec, sec, sec_list);
			free(sec);
		}

	if (fstat(ifd, &sb) == -1)
		err(EX_IOERR, "fstat failed");

	/*
	 * TODO: copy the input binary to output binary verbatim if -O is not
	 * specified.
	 */

	/* Create EHDR for output .o file. */
	if (gelf_newehdr(ecp->eout, ecp->oec) == NULL)
		errx(EX_SOFTWARE, "gelf_newehdr failed: %s",
		    elf_errmsg(-1));
	if (gelf_getehdr(ecp->eout, &oeh) == NULL)
		errx(EX_SOFTWARE, "gelf_getehdr() failed: %s",
		    elf_errmsg(-1));

	/* Initialise e_ident fields. */
	oeh.e_ident[EI_CLASS] = ecp->oec;
	oeh.e_ident[EI_DATA] = ecp->oed;
	oeh.e_ident[EI_OSABI] = ELFOSABI_NONE;
	oeh.e_machine = ecp->oem;
	oeh.e_type = ET_REL;
	oeh.e_entry = 0;
	
	ecp->flags |= RELOCATABLE;

	/* Create .shstrtab section */
	init_shstrtab(ecp);
	ecp->shstrtab->off = 0;

	/*
	 * Create .data section which contains the binary data.
	 */

	if ((s = calloc(1, sizeof(*s))) == NULL)
		err(EX_SOFTWARE, "calloc failed");
	s->name = ".data";
	s->is = NULL;
	s->off = gelf_fsize(ecp->eout, ELF_T_EHDR, 1, EV_CURRENT);
	if (s->off == 0)
		errx(EX_SOFTWARE, "gelf_fsize() failed: %s", elf_errmsg(-1));
	s->sz = sb.st_size;
	s->align = 1;
	s->type = SHT_PROGBITS;
	s->vma = 0;
	s->loadable = 1;
	if ((s->buf = mmap(NULL, (size_t) s->sz, PROT_READ, MAP_PRIVATE, ifd,
	    (off_t) 0)) == MAP_FAILED) {
		err(EX_SOFTWARE, "mmap failed");
	}
	insert_to_sec_list(ecp, s, 1);

	if ((s->os = elf_newscn(ecp->eout)) == NULL)
		errx(EX_SOFTWARE, "elf_newscn failed: %s",
		    elf_errmsg(-1));
	if (gelf_getshdr(s->os, &osh) == NULL)
		errx(EX_SOFTWARE, "gelf_getshdr() failed: %s",
		    elf_errmsg(-1));
	osh.sh_type = s->type;
	osh.sh_addr = s->vma;
	osh.sh_offset = s->off;
	osh.sh_size = s->sz;
	osh.sh_link = 0;
	osh.sh_info = 0;
	osh.sh_addralign = s->align;
	osh.sh_entsize = 0;
	osh.sh_flags = SHF_ALLOC | SHF_WRITE;
	add_to_shstrtab(ecp, s->name);
	if (!gelf_update_shdr(s->os, &osh))
		errx(EX_SOFTWARE, "elf_update_shdr failed: %s",
		    elf_errmsg(-1));

	if ((od = elf_newdata(s->os)) == NULL)
		errx(EX_SOFTWARE, "elf_newdata() failed: %s",
		    elf_errmsg(-1));
	od->d_align = 1;
	od->d_off = 0;
	od->d_buf = s->buf;
	od->d_type = ELF_T_BYTE;
	od->d_size = s->sz;
	od->d_version = EV_CURRENT;

	/* Insert .shstrtab after .data section. */
	if ((ecp->shstrtab->os = elf_newscn(ecp->eout)) == NULL)
		errx(EX_SOFTWARE, "elf_newscn failed: %s",
		    elf_errmsg(-1));
	insert_to_sec_list(ecp, ecp->shstrtab, 1);

	/*
	 * TODO: Create symbol table.
	 */

	/*
	 * Write the underlying ehdr. Note that it should be called
	 * before elf_setshstrndx() since it will overwrite e->e_shstrndx.
	 */
	if (gelf_update_ehdr(ecp->eout, &oeh) == 0)
		errx(EX_SOFTWARE, "gelf_update_ehdr() failed: %s",
		    elf_errmsg(-1));

	/* Generate section name string table (.shstrtab). */
	set_shstrtab(ecp);

	/* Update sh_name pointer for each section header entry. */
	update_shdr(ecp);

	/* Renew oeh to get the updated e_shstrndx. */
	if (gelf_getehdr(ecp->eout, &oeh) == NULL)
		errx(EX_SOFTWARE, "gelf_getehdr() failed: %s",
		    elf_errmsg(-1));

	/* Insert section header table */
	shtab = insert_shtab(ecp, 1);
	
	/* Resync section offsets. */
	resync_sections(ecp);

	/* Store SHDR offset in EHDR. */
	oeh.e_shoff = shtab->off;

	/* Update ehdr since we modified e_shoff. */
	if (gelf_update_ehdr(ecp->eout, &oeh) == 0)
		errx(EX_SOFTWARE, "gelf_update_ehdr() failed: %s",
		    elf_errmsg(-1));

	/* Write out the output elf object. */
        if (elf_update(ecp->eout, ELF_C_WRITE) < 0)
                errx(EX_SOFTWARE, "elf_update() failed: %s",
                    elf_errmsg(-1));

}
