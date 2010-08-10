/*-
 * Copyright (c) 2006 Joseph Koshy
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

#include <sys/types.h>
#include <sys/stat.h>

#include <errno.h>
#include <fcntl.h>
#include <libelf.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include "elfts.h"

#include "tet_api.h"

include(`elfts.m4')

/*
 * Tests for the `elf_update' API.
 */

IC_REQUIRES_VERSION_INIT();

static char rawdata[] = "This is not an ELF file.";

/*
 * A NULL Elf argument returns ELF_E_ARGUMENT.
 */

void
tcArgs_tpNull(void)
{
	int error, result;
	off_t offset;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("elf_update(NULL,*) fails with ELF_E_ARGUMENT.");

	result = TET_PASS;
	if ((offset = elf_update(NULL, 0)) != -1 ||
	    (error = elf_errno()) != ELF_E_ARGUMENT)
		TP_FAIL("offset=%jd, error=\"%s\".", (intmax_t) offset,
		    elf_errmsg(error));

	tet_result(result);
}

/*
 * Illegal values for argument `cmd' are rejected.
 */

void
tcArgs_tpBadCmd(void)
{
	Elf *e;
	Elf_Cmd c;
	int error, result;
	off_t offset;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("illegal cmd values are rejected with ELF_E_ARGUMENT.");

	TS_OPEN_MEMORY(e, rawdata);

	result = TET_PASS;
	for (c = ELF_C_NULL-1; result == TET_PASS && c < ELF_C_NUM; c++) {
		if (c == ELF_C_WRITE || c == ELF_C_NULL) /* legal values */
			continue;
		if ((offset = elf_update(e, c)) != -1 ||
		    (error = elf_errno()) != ELF_E_ARGUMENT)
			TP_FAIL("cmd=%d, offset=%jd, error=\"%s\".", c,
			    (intmax_t) offset, elf_errmsg(error));
	}

	(void) elf_end(e);
	tet_result(result);
}

/*
 * Non-ELF descriptors are rejected by elf_update().
 */
undefine(`FN')
define(`FN',`
void
tcArgs_tpNonElf$1(void)
{
	Elf *e;
	int error, fd, result;
	off_t offset;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("elf_update(non-elf,ELF_C_$1) returns ELF_E_ARGUMENT.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	_TS_WRITE_FILE(TS_NEWFILE,rawdata,sizeof(rawdata),goto done;);
	_TS_OPEN_FILE(e, TS_NEWFILE, ELF_C_READ, fd, goto done;);

	result = TET_PASS;

	if ((offset = elf_update(e, ELF_C_$1)) != -1 ||
	    (error = elf_errno()) != ELF_E_ARGUMENT)
		TP_FAIL("ELF_C_$1 offset=%jd error=\"%s\".",
		    (intmax_t) offset, elf_errmsg(error));

 done:
	if (e)
		(void) elf_end(e);
	if (fd != -1)
		(void) close(fd);
	(void) unlink(TS_NEWFILE);

	tet_result(result);
}')

FN(`NULL')
FN(`WRITE')

/*
 * In-memory (i.e., non-writeable) ELF objects are rejected for
 * ELF_C_WRITE with error ELF_E_MODE.
 */

undefine(`FN')
define(`FN',`
void
tcMemElfWrite_tp$1$2(void)
{
	Elf *e;
	off_t offset;
	int error, result;
	char elf[sizeof(Elf64_Ehdr)]; /* larger of the Ehdr variants */

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: ELF_C_WRITE with in-memory objects "
	    "returns ELF_E_MODE.");

	result = TET_UNRESOLVED;
	e = NULL;

	_TS_READ_FILE("newehdr.$2$1", elf, sizeof(elf), goto done;);

	TS_OPEN_MEMORY(e, elf);

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_WRITE)) != -1 ||
	    (error = elf_errno()) != ELF_E_MODE)
		TP_FAIL("offset=%jd error=%d \"%s\".", (intmax_t) offset,
		    error, elf_errmsg(error));

 done:
	(void) elf_end(e);
	tet_result(result);
}')

FN(`32', `lsb')
FN(`32', `msb')
FN(`64', `lsb')
FN(`64', `msb')

/*
 * In-memory ELF objects are updateable with command ELF_C_NULL.
 */

undefine(`FN')
define(`FN',`
void
tcMemElfNull_tp$1$2(void)
{
	Elf *e;
	int result;
	size_t fsz;
	off_t offset;
	char elf[sizeof(Elf64_Ehdr)];

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: ELF_C_NULL updates in-memory objects.");

	result = TET_UNRESOLVED;

	_TS_READ_FILE("newehdr.$2$1", elf, sizeof(elf), goto done;);

	TS_OPEN_MEMORY(e, elf);

	if ((fsz = elf$1_fsize(ELF_T_EHDR, 1, EV_CURRENT)) == 0) {
		TP_UNRESOLVED("elf$2_fsize() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != fsz)
		TP_FAIL("offset=%jd != %d, error=%d \"%s\".",
		    (intmax_t) offset, fsz, elf_errmsg(-1));

 done:
	(void) elf_end(e);
	tet_result(result);
}')

FN(`32', `lsb')
FN(`32', `msb')
FN(`64', `lsb')
FN(`64', `msb')

/*
 * A mismatched class in the Ehdr returns an ELF_E_CLASS error.
 */

undefine(`FN')
define(`FN',`
void
tcClassMismatch_tp$1$2(void)
{
	int error, fd, result;
	off_t offset;
	Elf *e;
	Elf$1_Ehdr *eh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: class-mismatch is detected.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	TS_OPEN_FILE(e, "newehdr.$2$1", ELF_C_READ, fd);

	if ((eh = elf$1_newehdr(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newehdr() failed: %s", elf_errmsg(-1));
		goto done;
	}

	/* change the class */
	eh->e_ident[EI_CLASS] = ELFCLASS`'ifelse($1,32,64,32);

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != -1 ||
	    (error = elf_errno()) != ELF_E_CLASS)
		TP_FAIL("elf_update()->%jd, error=%d (expected %d)",
		     (intmax_t) offset, error, ELF_E_MODE);

 done:
	if (e)
		(void) elf_end(e);
	if (fd != -1)
		(void) close(fd);
	tet_result(result);
}')

FN(`32', `lsb')
FN(`32', `msb')
FN(`64', `lsb')
FN(`64', `msb')

/*
 * Changing the byte order of an ELF file on the fly is not allowed.
 */

undefine(`FN')
define(`FN',`
void
tcByteOrderChange_tp$1$2(void)
{
	int error, fd, result;
	Elf *e;
	off_t offset;
	Elf$1_Ehdr *eh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: byte order changes are rejected.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	TS_OPEN_FILE(e, "newehdr.$2$1", ELF_C_READ, fd);

	if ((eh = elf$1_newehdr(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newehdr() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	eh->e_ident[EI_DATA] = ELFDATA2`'ifelse($2,`lsb',`MSB',`LSB');

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != -1 ||
	    (error = elf_errno()) != ELF_E_HEADER)
		TP_FAIL("elf_update()->%jd, error=%d (expected %d)",
		     (intmax_t) offset, error, ELF_E_HEADER);

 done:
	if (e)
		(void) elf_end(e);
	if (fd != -1)
		(void) close(fd);
	tet_result(result);
}')

FN(`32', `lsb')
FN(`32', `msb')
FN(`64', `lsb')
FN(`64', `msb')

/*
 * An unsupported ELF version is rejected with ELF_E_VERSION.
 */

undefine(`FN')
define(`FN',`
void
tcUnsupportedVersion_tp$1$2(void)
{
	int error, fd, result;
	off_t offset;
	Elf *e;
	Elf$1_Ehdr *eh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: version changes are rejected.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	TS_OPEN_FILE(e, "newehdr.$2$1", ELF_C_READ, fd);

	if ((eh = elf$1_newehdr(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newehdr() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	eh->e_version = EV_CURRENT+1;

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != -1 ||
	    (error = elf_errno()) != ELF_E_VERSION)
		TP_FAIL("elf_update()->%jd, error=%d (expected %d)",
		     (intmax_t) offset, error, ELF_E_VERSION);

 done:
	if (e)
		(void) elf_end(e);
	if (fd != -1)
		(void) close(fd);
	tet_result(result);
}')

FN(`32', `lsb')
FN(`32', `msb')
FN(`64', `lsb')
FN(`64', `msb')

/*
 * Invoking an elf_cntl(ELF_C_FDDONE) causes a subsequent elf_update()
 * to fail with ELF_E_SEQUENCE.
 */
undefine(`FN')
define(`FN',`
void
tcSequence_tpFdDoneWrite$1(void)
{
	int error, fd, result;
	off_t offset;
	Elf *e;
	Elf$1_Ehdr *eh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("*$1: elf_update(ELF_C_WRITE) after an elf_cntl(FDDONE) "
	    "is rejected.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	_TS_OPEN_FILE(e, TS_NEWFILE, ELF_C_WRITE, fd, goto done;);

	if ((eh = elf$1_newehdr(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newehdr() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	if (elf_cntl(e, ELF_C_FDDONE) != 0) {
		TP_UNRESOLVED("elf_cntl() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_WRITE)) != -1 ||
	    (error = elf_errno()) != ELF_E_SEQUENCE)
		TP_FAIL("elf_update()->%jd, error=%d (expected %d)",
		     (intmax_t) offset, error, ELF_E_SEQUENCE);

 done:
	if (e)
		(void) elf_end(e);
	if (fd != -1)
		(void) close(fd);
	tet_result(result);
}')

FN(32)
FN(64)

/*
 * Invoking an elf_cntl(ELF_C_FDDONE) causes a subsequent elf_update(ELF_C_NULL)
 * to succeed.
 */

undefine(`FN')
define(`FN',`
void
tcSequence_tpFdDoneNull$1(void)
{
	int fd, result;
	off_t offset;
	size_t fsz;
	Elf *e;
	Elf$1_Ehdr *eh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("elf_update(ELF_C_NULL) after an elf_cntl(FDDONE) succeeds.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	_TS_OPEN_FILE(e, TS_NEWFILE, ELF_C_WRITE, fd, goto done;);

	if ((eh = elf$1_newehdr(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newehdr() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	if (elf_cntl(e, ELF_C_FDDONE) != 0) {
		TP_UNRESOLVED("elf_cntl() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	if ((fsz = elf$1_fsize(ELF_T_EHDR, 1, EV_CURRENT)) == 0) {
		TP_UNRESOLVED("fsize() failed: %s.", elf_errmsg(-1));
		goto done;
	}

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != fsz)
		TP_FAIL("elf_update()->%jd, (expected %d).", (intmax_t) offset,
		    fsz);

 done:
	if (e)
		(void) elf_end(e);
	if (fd != -1)
		(void) close(fd);
	tet_result(result);
}')

FN(32)
FN(64)

/*
 * Check that elf_update() can create a legal ELF file.
 */

const char strtab[] = {
	'\0',
	'.', 's', 'h', 's', 't', 'r', 't', 'a', 'b', '\0'
};

#define	INIT_PHDR(P)	do {				\
		(P)->p_type   = PT_NULL;		\
		(P)->p_offset = 0x0F0F0F0F;		\
		(P)->p_vaddr  = 0xA0A0A0A0;		\
		(P)->p_filesz = 0x1234;			\
		(P)->p_memsz  = 0x5678;			\
		(P)->p_flags  = PF_X | PF_R;		\
		(P)->p_align  = 64;			\
	} while (0)

#define	INIT_SHDR(S,O)	do {				\
		(S)->sh_name = 1;			\
		(S)->sh_type = SHT_STRTAB;		\
		(S)->sh_flags = 0;			\
		(S)->sh_addr = 0;			\
		(S)->sh_offset = (O);			\
		(S)->sh_size = sizeof(strtab);		\
		(S)->sh_link = 0;			\
		(S)->sh_info = 0;			\
		(S)->sh_addralign = 1;			\
		(S)->sh_entsize = 0;			\
	} while (0)

undefine(`FN')
define(`FN',`
void
tcUpdate_tp$1$2(void)
{
	int fd, result;
	off_t offset;
	size_t esz, fsz, psz, roundup, ssz;
	Elf$1_Shdr *sh;
	Elf$1_Ehdr *eh;
	Elf$1_Phdr *ph;
	Elf_Data *d;
	Elf_Scn *scn;
	Elf *e;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: elf_update() creates a legal ELF file.");

	result = TET_UNRESOLVED;
	fd = -1;
	e = NULL;

	_TS_OPEN_FILE(e, TS_NEWFILE, ELF_C_WRITE, fd, goto done;);

	if ((eh = elf$1_newehdr(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newehdr() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	/* Set the version and endianness */
	eh->e_version = EV_CURRENT;
	eh->e_ident[EI_DATA] = ELFDATA2`'TOUPPER($2);
	eh->e_type = ET_REL;

	if ((esz = elf$1_fsize(ELF_T_EHDR, 1, EV_CURRENT)) == 0 ||
	    (psz = elf$1_fsize(ELF_T_PHDR, 1, EV_CURRENT)) == 0 ||
	    (ssz = elf$1_fsize(ELF_T_SHDR, 2, EV_CURRENT)) == 0) {
		TP_UNRESOLVED("elf$1_fsize() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	if ((ph = elf$1_newphdr(e,1)) == NULL) {
		TP_UNRESOLVED("elf$1_newphdr() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	INIT_PHDR(ph);

	if ((scn = elf_newscn(e)) == NULL) {
		TP_UNRESOLVED("elf_newscn() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	eh->e_shstrndx = elf_ndxscn(scn);

	if ((sh = elf$1_getshdr(scn)) == NULL) {
		TP_UNRESOLVED("elf$1_getshdr() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	if ((d = elf_newdata(scn)) == NULL) {
		TP_UNRESOLVED("elf_newdata() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	d->d_buf  = (char *) strtab;
	d->d_size = sizeof(strtab);
	d->d_off  = (off_t) 0;

	INIT_SHDR(sh, esz+psz);

	fsz = esz + psz + sizeof(strtab);
	roundup = ifelse($1,32,4,8);
	fsz = (fsz + roundup - 1) & ~(roundup - 1);

	fsz += ssz;

	if ((offset = elf_update(e, ELF_C_WRITE)) != fsz) {
		TP_FAIL("ret=%jd != %d [elferror=\"%s\"]",
		    (intmax_t) offset, fsz, elf_errmsg(-1));
		goto done;
	}

	(void) elf_end(e);	e = NULL;
	(void) close(fd);	fd = -1;

	result = elfts_compare_files("u1.$2$1", TS_NEWFILE);

 done:
	if (e)
		(void) elf_end(e);
	if (fd != -1)
		(void) close(fd);
	(void) unlink(TS_NEWFILE);

	tet_result(result);
}')

FN(32,`lsb')
FN(32,`msb')
FN(64,`lsb')
FN(64,`msb')

/*
 * An unsupported section type should be rejected.
 */
undefine(`FN')
define(`FN',`
void
tcSectionType_tp$2$1(void)
{
	int error, fd, result;
	off_t offset;
	Elf *e;
	Elf_Scn *scn;
	Elf$1_Shdr *sh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: unsupported section types are rejected.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	_TS_OPEN_FILE(e, "newehdr.$2$1", ELF_C_READ, fd, goto done;);

	if ((scn = elf_newscn(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newscn() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	if ((sh = elf$1_getshdr(scn)) == NULL) {
		TP_UNRESOLVED("elf$1_getshdr() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	sh->sh_type = SHT_NULL - 1;
	(void) elf_flagshdr(scn, ELF_C_SET, ELF_F_DIRTY);

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != (off_t) -1 ||
	    (error = elf_errno()) != ELF_E_SECTION) {
		TP_FAIL("elf_update()->%jd, error=%d \"%s\".",
		    (intmax_t) offset, error, elf_errmsg(error));
	}

 done:
	if (e)
		(void) elf_end(e);
	if (fd)
		(void) close(fd);
	tet_result(result);
}')

FN(32,`lsb')
FN(32,`msb')
FN(64,`lsb')
FN(64,`msb')

/*
 * A section with data type != section type should be rejected.
 */
undefine(`FN')
define(`FN',`
void
tcSectionDataType_tp$2$1(void)
{
	int error, fd, result;
	off_t offset;
	Elf *e;
	Elf_Data *d;
	Elf_Scn *scn;
	Elf$1_Shdr *sh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: data descriptors with incompatible types "
	    "are rejected.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	_TS_OPEN_FILE(e, "newehdr.$2$1", ELF_C_READ, fd, goto done;);

	if ((scn = elf_newscn(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newscn() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	if ((sh = elf$1_getshdr(scn)) == NULL) {
		TP_UNRESOLVED("elf$1_getshdr() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	sh->sh_type = SHT_STRTAB;
	(void) elf_flagshdr(scn, ELF_C_SET, ELF_F_DIRTY);

	if ((d = elf_newdata(scn)) == NULL) {
		TP_UNRESOLVED("elf_newdata() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	d->d_buf  = (char *) strtab;
	d->d_size = sizeof(strtab);
	d->d_off  = (off_t) 0;
	d->d_type = ELF_T_ADDR; /* Other than ELF_T_BYTE */

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != (off_t) -1 ||
	    (error = elf_errno()) != ELF_E_DATA) {
		TP_FAIL("elf_update()->%jd, error=%d \"%s\".",
		    (intmax_t) offset, error, elf_errmsg(error));
	}

 done:
	if (e)
		(void) elf_end(e);
	if (fd)
		(void) close(fd);
	tet_result(result);
}')

FN(32,`lsb')
FN(32,`msb')
FN(64,`lsb')
FN(64,`msb')

/*
 * An Elf_Data descriptor with incompatible alignment should be rejected.
 */
undefine(`FN')
define(`FN',`
void
tcAlignment_tpData$2$1(void)
{
	int error, fd, result;
	off_t offset;
	Elf *e;
	Elf_Data *d;
	Elf_Scn *scn;
	Elf$1_Shdr *sh;

	TP_CHECK_INITIALIZATION();

	TP_ANNOUNCE("TOUPPER($2)$1: data descriptors with incompatible alignments"
	    " are rejected.");

	result = TET_UNRESOLVED;
	e = NULL;
	fd = -1;

	_TS_OPEN_FILE(e, "newehdr.$2$1", ELF_C_READ, fd, goto done;);

	if ((scn = elf_newscn(e)) == NULL) {
		TP_UNRESOLVED("elf$1_newscn() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	if ((sh = elf$1_getshdr(scn)) == NULL) {
		TP_UNRESOLVED("elf$1_getshdr() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	sh->sh_type = SHT_SYMTAB;
	(void) elf_flagshdr(scn, ELF_C_SET, ELF_F_DIRTY);

	if ((d = elf_newdata(scn)) == NULL) {
		TP_UNRESOLVED("elf_newdata() failed: \"%s\".",
		    elf_errmsg(-1));
		goto done;
	}

	d->d_buf  = (char *) strtab;
	d->d_size = sizeof(strtab);
	d->d_off  = (off_t) 0;
	d->d_type = ELF_T_SYM;
	d->d_align = 1;		/* misaligned for SHT_SYMTAB sections */

	result = TET_PASS;
	if ((offset = elf_update(e, ELF_C_NULL)) != (off_t) -1 ||
	    (error = elf_errno()) != ELF_E_LAYOUT) {
		TP_FAIL("elf_update()->%jd, error=%d \"%s\".",
		    (intmax_t) offset, error, elf_errmsg(error));
	}

 done:
	if (e)
		(void) elf_end(e);
	if (fd)
		(void) close(fd);
	tet_result(result);
}')

FN(32,`lsb')
FN(32,`msb')
FN(64,`lsb')
FN(64,`msb')

/* XXX: check that overlapping sections are rejected */
/* XXX: check that non-overlapping sections are permitted */
