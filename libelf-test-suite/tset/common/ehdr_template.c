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

/*
 * Boilerplate for testing the *_getehdr and *_newehdr APIs.
 *
 * This template is to be used as follows:
 *
 * #define	TS_EHDRFUNC		_getehdr	(or _newehdr)
 * #define	TS_EHDRSZ		32	(or 64)
 * #include "getehdr_template.c"
 */

#include <sys/cdefs.h>

/* Variations of __CONCAT and __STRING which expand their arguments. */
#define __XCONCAT(x,y)	__CONCAT(x,y)
#ifndef __XSTRING
#define __XSTRING(x)	__STRING(x)
#endif

#define	TS_ICFUNC	__XCONCAT(elf,__XCONCAT(TS_EHDRSZ,TS_EHDRFUNC))
#define	TS_EHDR		__XCONCAT(Elf,__XCONCAT(TS_EHDRSZ,_Ehdr))
#define	TS_ICNAME	__XSTRING(TS_ICFUNC)
#define	TS_ELFCLASS	__XCONCAT(ELFCLASS,TS_EHDRSZ)

IC_REQUIRES_VERSION_INIT();

/*
 * Checks for the contents of an Ehdr structure. The values here must
 * match that in the "ehdr.yaml" file in the test case directory.
 */

#define	CHECK_SIGFIELD(E,I,V,R)	do {					\
		if ((E)->e_ident[EI_##I] != (V)) {			\
			tet_printf("fail: " #I " value 0x%x != "	\
			    "expected 0x%x.", (E)->e_ident[EI_##I],	\
			    (V));					\
			(R) = TET_FAIL;					\
		}							\
	} while (0)

#define	CHECK_SIG(E,ED,EC,EV,EABI,EABIVER,R)		do {		\
		if ((E)->e_ident[EI_MAG0] != ELFMAG0 ||			\
		    (E)->e_ident[EI_MAG1] != ELFMAG1 ||			\
		    (E)->e_ident[EI_MAG2] != ELFMAG2 ||			\
		    (E)->e_ident[EI_MAG3] != ELFMAG3) {			\
			tet_printf("fail: incorrect ELF signature "	\
			    "(%x %x %x %x).", (E)->e_ident[EI_MAG0],	\
			    (E)->e_ident[EI_MAG1], (E)->e_ident[EI_MAG2],\
			    (E)->e_ident[EI_MAG3]);			\
			(R) = TET_FAIL;					\
		}							\
		CHECK_SIGFIELD(E,CLASS,	EC, R);				\
		CHECK_SIGFIELD(E,DATA,	ED, R);				\
		CHECK_SIGFIELD(E,VERSION, EV, R);			\
		CHECK_SIGFIELD(E,OSABI,	EABI, R);			\
		CHECK_SIGFIELD(E,ABIVERSION, EABIVER, R);		\
	} while (0)


#define	CHECK_FIELD(E,FIELD,VALUE,R)	do {				\
		if ((E)->e_##FIELD != (VALUE)) {			\
			tet_printf("fail: field \"%s\" actual 0x%jx "	\
			    "!= expected 0x%jx.", #FIELD, 		\
			    (uintmax_t) (E)->e_##FIELD,			\
			    (uintmax_t) (VALUE));			\
			(R) = TET_FAIL;					\
		}							\
	} while (0)

#define	CHECK_EHDR(E,ED,EC,R)	do {		\
		CHECK_SIG(E,ED,EC,EV_CURRENT,ELFOSABI_FREEBSD,1,R);	\
		CHECK_FIELD(E,type,	ET_REL, R);	\
		CHECK_FIELD(E,machine,	0x42, R);	\
		CHECK_FIELD(E,version,	EV_CURRENT, R);	\
		CHECK_FIELD(E,entry,	0xF0F0F0F0, R);	\
		CHECK_FIELD(E,phoff,	0x0E0E0E0E, R);	\
		CHECK_FIELD(E,shoff,	0xD0D0D0D0, R);	\
		CHECK_FIELD(E,flags,	64+8+2+1, R);	\
		CHECK_FIELD(E,ehsize,	0x0A0A, R);	\
		CHECK_FIELD(E,phentsize,0xB0B0, R);	\
		CHECK_FIELD(E,phnum,	0x0C0C, R);	\
		CHECK_FIELD(E,shentsize,0xD0D0, R);	\
		CHECK_FIELD(E,shnum,	0x0E0E, R);	\
		CHECK_FIELD(E,shstrndx,	0xF0F0, R);	\
	} while (0)

void
tcNull_tpGet(void)
{
	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: " TS_ICNAME "(NULL) fails with "
	    "ELF_E_ARGUMENT.");

	if (TS_ICFUNC(NULL) != NULL ||
	    elf_errno() != ELF_E_ARGUMENT)
		tet_result(TET_FAIL);
	else
		tet_result(TET_PASS);
}

static char data[] = "This isn't an ELF file.";

void
tcData_tpElf(void)
{
	Elf *e;

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: " TS_ICNAME "(E) for non-ELF (E) fails with "
	    "ELF_E_ARGUMENT.");

	TS_OPEN_MEMORY(e, data);

	if (TS_ICFUNC(e) != NULL ||
	    elf_errno() != ELF_E_ARGUMENT)
		tet_result(TET_FAIL);
	else
		tet_result(TET_PASS);

	(void) elf_end(e);
}


/*
 * A malformed (too short) ELF header.
 */

static char badelftemplate[EI_NIDENT+1] = {
	[EI_MAG0] = '\177',
	[EI_MAG1] = 'E',
	[EI_MAG2] = 'L',
	[EI_MAG3] = 'F',
	[EI_CLASS] = ELFCLASS64,
	[EI_DATA]  = ELFDATA2MSB,
	[EI_NIDENT] = '@'
};

void
tcBadElfVersion_tpElf(void)
{
	int err;
	Elf *e;
	TS_EHDR *eh;
	char badelf[sizeof(badelftemplate)];

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: " TS_ICNAME "() with an unsupported version "
	    "fails with ELF_E_VERSION.");

	(void) memcpy(badelf, badelftemplate, sizeof(badelf));

	badelf[EI_VERSION] = EV_NONE;
	badelf[EI_CLASS] = TS_ELFCLASS;

	TS_OPEN_MEMORY(e, badelf);

	if ((eh = TS_ICFUNC(e)) != NULL ||
	    (err = elf_errno()) != ELF_E_VERSION) {
		tet_printf("fail: error=%d eh=%p.", err, (void *) eh);
		tet_result(TET_FAIL);
	} else
		tet_result(TET_PASS);

	(void) elf_end(e);
}

void
tcBadElf_tpElf(void)
{
	int err;
	Elf *e;
	TS_EHDR *eh;
	char badelf[sizeof(badelftemplate)];

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: " TS_ICNAME "() on a malformed ELF file "
	    "fails with ELF_E_HEADER.");

	(void) memcpy(badelf, badelftemplate, sizeof(badelf));
	badelf[EI_VERSION] = EV_CURRENT;
	badelf[EI_CLASS]   = TS_ELFCLASS;

	TS_OPEN_MEMORY(e, badelf);

	if ((eh = TS_ICFUNC(e)) != NULL ||
	    (err = elf_errno()) != ELF_E_HEADER) {
		tet_printf("fail: error=%d eh=%p.", err, (void *) eh);
		tet_result(TET_FAIL);
	} else
		tet_result(TET_PASS);

	(void) elf_end(e);
}

void
tcElf_tpElfValid(void)
{
	int fd;
	Elf *e;
	TS_EHDR *eh;

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: " TS_ICNAME "(E) on valid EHDR returns"
	    " non-NULL.");

	TS_OPEN_FILE(e,"ehdr.msb" __XSTRING(TS_EHDRSZ),ELF_C_READ,fd);

	if ((eh = TS_ICFUNC(e)) == NULL)
		tet_result(TET_FAIL);
	else
		tet_result(TET_PASS);

	(void) elf_end(e);
	(void) close(fd);
}

void
tcElf_tpElfLSB(void)
{
	int fd, result;
	Elf *e;
	TS_EHDR *eh;

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: " TS_ICNAME "(E) returns the correct LSB ehdr.");

	TS_OPEN_FILE(e,"ehdr.lsb" __XSTRING(TS_EHDRSZ),ELF_C_READ,fd);

	if ((eh = TS_ICFUNC(e)) == NULL) {
		tet_infoline("unresolved: " TS_ICNAME "() failed.");
		tet_result(TET_UNRESOLVED);
		return;
	}

	result = TET_PASS;

	CHECK_EHDR(eh, ELFDATA2LSB, TS_ELFCLASS, result);

	tet_result(result);

	(void) elf_end(e);
	(void) close(fd);
}

void
tcElf_tpElfMSB(void)
{
	int fd, result;
	Elf *e;
	TS_EHDR *eh;

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion:" TS_ICNAME "(E) returns the correct MSB ehdr.");

	TS_OPEN_FILE(e,"ehdr.msb" __XSTRING(TS_EHDRSZ),ELF_C_READ,fd);

	if ((eh = TS_ICFUNC(e)) == NULL) {
		tet_infoline("unresolved: " TS_ICNAME "() failed.");
		tet_result(TET_UNRESOLVED);
		return;
	}

	result = TET_PASS;

	CHECK_EHDR(eh, ELFDATA2MSB, TS_ELFCLASS, result);

	tet_result(result);

	(void) elf_end(e);
	(void) close(fd);
}

void
tcElf_tpElfDup(void)
{
	int fd;
	Elf *e;
	TS_EHDR *eh1, *eh2;

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: successful calls to " TS_ICNAME "() return "
	    "identical pointers.");

	TS_OPEN_FILE(e,"ehdr.msb" __XSTRING(TS_EHDRSZ),ELF_C_READ,fd);

	if ((eh1 = TS_ICFUNC(e)) == NULL ||
	    (eh2 = TS_ICFUNC(e)) == NULL) {
		tet_infoline("unresolved: " TS_ICNAME "() failed.");
		tet_result(TET_UNRESOLVED);
		return;
	}

	tet_result(eh1 == eh2 ? TET_PASS : TET_FAIL);

	(void) elf_end(e);
	(void) close(fd);
}

#if	TS_EHDRSZ == 32
#define	TS_OTHERSIZE	64
#else
#define	TS_OTHERSIZE	32
#endif

void
tcElf_tpElfWrongSize(void)
{
	int error, fd, result;
	Elf *e;
	char *fn;
	TS_EHDR *eh;

	TP_CHECK_INITIALIZATION();

	tet_infoline("assertion: a call to " TS_ICNAME "() and a mismatched "
	    "ELF class fails with ELF_E_CLASS.");

	result = TET_PASS;

	fn = "ehdr.msb" __XSTRING(TS_OTHERSIZE);
	TS_OPEN_FILE(e,fn,ELF_C_READ,fd);
	if ((eh = TS_ICFUNC(e)) != NULL ||
	    (error = elf_errno()) != ELF_E_CLASS) {
		tet_printf("fail: \"%s\" opened (error %d).", fn, error);
		result = TET_FAIL;
	}
	(void) elf_end(e);
	(void) close(fd);

	if (result != TET_PASS) {
		tet_result(result);
		return;
	}

	fn = "ehdr.lsb" __XSTRING(TS_OTHERSIZE);
	TS_OPEN_FILE(e,fn,ELF_C_READ,fd);
	if ((eh = TS_ICFUNC(e)) != NULL ||
	    (error = elf_errno()) != ELF_E_CLASS) {
		tet_printf("fail: \"%s\" opened (error %d).", fn, error);
		result = TET_FAIL;
	}
	(void) elf_end(e);
	(void) close(fd);

	tet_result(result);
}
