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
#include <gelf.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include "elfcopy.h"

ELFTC_VCSID("$Id$");

static char hex_digit(uint8_t n);
static void ihex_write(int ofd, int type, uint64_t addr, uint64_t num,
    const void *buf, size_t sz);
static void ihex_write_00(int ofd, uint64_t addr, const void *buf, size_t sz);
static void ihex_write_01(int ofd);
static void ihex_write_04(int ofd, uint16_t addr);
static void ihex_write_05(int ofd, uint64_t e_entry);
static void srec_write(int ofd, char type, uint64_t addr, const void *buf,
    size_t sz);
static void srec_write_S0(int ofd, const char *ofn);
static void srec_write_Sd(int ofd, char dr, uint64_t addr, const void *buf,
    size_t sz);
static void srec_write_Se(int ofd, uint64_t e_entry, int forceS3);
static void write_num(char *line, int *len, uint64_t num, size_t sz,
    int *checksum);

#define	_LINE_BUFSZ	1024

/*
 * Convert ELF object to S-Record.
 */
void
create_srec(struct elfcopy *ecp, int ifd, int ofd, const char *ofn)
{
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Ehdr eh;
	GElf_Shdr sh;
	uint64_t max_addr;
	int elferr;
	char dr;

	if ((e = elf_begin(ifd, ELF_C_READ, NULL)) == NULL)
		errx(EX_DATAERR, "elf_begin() failed: %s",
		    elf_errmsg(-1));

	/*
	 * Find maximum address size in the first iteration.
	 */
	if (ecp->flags & SREC_FORCES3)
		dr = '3';
	else {
		max_addr = 0;
		scn = NULL;
		while ((scn = elf_nextscn(e, scn)) != NULL) {
			if (gelf_getshdr(scn, &sh) == NULL) {
				warnx("gelf_getshdr failed: %s",
				    elf_errmsg(-1));
				(void) elf_errno();
				continue;
			}
			if ((sh.sh_flags & SHF_ALLOC) == 0 ||
			    sh.sh_type == SHT_NOBITS ||
			    sh.sh_size == 0)
				continue;
			if ((uint64_t) sh.sh_addr > max_addr)
				max_addr = sh.sh_addr;
		}
		elferr = elf_errno();
		if (elferr != 0)
			warnx("elf_nextscn failed: %s", elf_errmsg(elferr));

		if (max_addr <= 0xFFFF)
			dr = '1';
		else if (max_addr <= 0xFFFFFF)
			dr = '2';
		else
			dr = '3';
	}

	/* Generate S0 record which contains the output filename. */
	srec_write_S0(ofd, ofn);

	/* Generate S{1,2,3} data records for section data. */
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
		if (sh.sh_addr > 0xFFFFFFFF) {
			warnx("address space too big for S-Record file");
			continue;
		}
		(void) elf_errno();
		if ((d = elf_getdata(scn, NULL)) == NULL) {
			elferr = elf_errno();
			if (elferr != 0)
				warnx("elf_getdata failed: %s", elf_errmsg(-1));
			continue;
		}
		if (d->d_buf == NULL || d->d_size == 0)
			continue;
		srec_write_Sd(ofd, dr, sh.sh_addr, d->d_buf, d->d_size);
	}
	elferr = elf_errno();
	if (elferr != 0)
		warnx("elf_nextscn failed: %s", elf_errmsg(elferr));

	/* Generate S{7,8,9} end of block recrod. */
	if (gelf_getehdr(e, &eh) == NULL)
		errx(EX_SOFTWARE, "gelf_getehdr() failed: %s",
		    elf_errmsg(-1));
	srec_write_Se(ofd, eh.e_entry, ecp->flags & SREC_FORCES3);
}

void
create_ihex(int ifd, int ofd)
{
	Elf *e;
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Ehdr eh;
	GElf_Shdr sh;
	int elferr;
	uint16_t addr_hi, old_addr_hi;

	if ((e = elf_begin(ifd, ELF_C_READ, NULL)) == NULL)
		errx(EX_DATAERR, "elf_begin() failed: %s",
		    elf_errmsg(-1));

	old_addr_hi = 0;
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
		if (sh.sh_addr > 0xFFFFFFFF) {
			warnx("address space too big for Intel Hex file");
			continue;
		}
		(void) elf_errno();
		if ((d = elf_getdata(scn, NULL)) == NULL) {
			elferr = elf_errno();
			if (elferr != 0)
				warnx("elf_getdata failed: %s", elf_errmsg(-1));
			continue;
		}
		if (d->d_buf == NULL || d->d_size == 0)
			continue;
		addr_hi = (sh.sh_addr >> 16) & 0xFFFF;
		if (addr_hi > 0 && addr_hi != old_addr_hi) {
			/* Write 04 record if addr_hi is new. */
			old_addr_hi = addr_hi;
			ihex_write_04(ofd, addr_hi);
		}
		ihex_write_00(ofd, sh.sh_addr, d->d_buf, d->d_size);
	}
	elferr = elf_errno();
	if (elferr != 0)
		warnx("elf_nextscn failed: %s", elf_errmsg(elferr));

	if (gelf_getehdr(e, &eh) == NULL)
		errx(EX_SOFTWARE, "gelf_getehdr() failed: %s",
		    elf_errmsg(-1));
	ihex_write_05(ofd, eh.e_entry);
	ihex_write_01(ofd);
}

static void
srec_write_S0(int ofd, const char *ofn)
{

	srec_write(ofd, '0', 0, ofn, strlen(ofn));
}

static void
srec_write_Sd(int ofd, char dr, uint64_t addr, const void *buf, size_t sz)
{
	const uint8_t *p, *pe;

	p = buf;
	pe = p + sz;
	while (pe - p >= 16) {
		srec_write(ofd, dr, addr, p, 16);
		addr += 16;
		p += 16;
	}
	if (pe - p > 0)
		srec_write(ofd, dr, addr, p, pe - p);
}

static void
srec_write_Se(int ofd, uint64_t e_entry, int forceS3)
{
	char er;

	if (e_entry > 0xFFFFFFFF) {
		warnx("address space too big for S-Record file");
		return;
	}

	if (forceS3)
		er = '7';
	else {
		if (e_entry <= 0xFFFF)
			er = '9';
		else if (e_entry <= 0xFFFFFF)
			er = '8';
		else
			er = '7';
	}

	srec_write(ofd, er, e_entry, NULL, 0);
}

static void
srec_write(int ofd, char type, uint64_t addr, const void *buf, size_t sz)
{
	char line[_LINE_BUFSZ];
	const uint8_t *p, *pe;
	int len, addr_sz, checksum;

	if (sz > 16)
		errx(EX_SOFTWARE, "Internal: srec_write() sz too big");
	if (type == '0' || type == '1' || type == '5' || type == '9')
		addr_sz = 2;
	else if (type == '2' || type == '8')
		addr_sz = 3;
	else
		addr_sz = 4;

	checksum = 0;
	line[0] = 'S';
	line[1] = type;
	len = 2;
	write_num(line, &len, addr_sz + sz + 1, 1, &checksum);
	write_num(line, &len, addr, addr_sz, &checksum);
	for (p = buf, pe = p + sz; p < pe; p++)
		write_num(line, &len, *p, 1, &checksum);
	write_num(line, &len, ~checksum & 0xFF, 1, NULL);
	line[len++] = '\r';
	line[len++] = '\n';
	if (write(ofd, line, len) != (ssize_t) len)
		err(EX_DATAERR, "write failed");
}

static void
ihex_write_00(int ofd, uint64_t addr, const void *buf, size_t sz)
{
	uint16_t addr_hi, old_addr_hi;
	const uint8_t *p, *pe;

	old_addr_hi = (addr >> 16) & 0xFFFF;
	p = buf;
	pe = p + sz;
	while (pe - p >= 16) {
		ihex_write(ofd, 0, addr, 0, p, 16);
		addr += 16;
		p += 16;
		addr_hi = (addr >> 16) & 0xFFFF;
		if (addr_hi != old_addr_hi) {
			old_addr_hi = addr_hi;
			ihex_write_04(ofd, addr_hi);
		}
	}
	if (pe - p > 0)
		ihex_write(ofd, 0, addr, 0, p, pe - p);
}

static void
ihex_write_01(int ofd)
{

	ihex_write(ofd, 1, 0, 0, NULL, 0);
}

static void
ihex_write_04(int ofd, uint16_t addr)
{

	ihex_write(ofd, 4, 0, addr, NULL, 2);
}

static void
ihex_write_05(int ofd, uint64_t e_entry)
{

	if (e_entry > 0xFFFFFFFF) {
		warnx("address space too big for Intel Hex file");
		return;
	}

	ihex_write(ofd, 5, 0, e_entry, NULL, 4);
}

static void
ihex_write(int ofd, int type, uint64_t addr, uint64_t num, const void *buf,
    size_t sz)
{
	char line[_LINE_BUFSZ];
	const uint8_t *p, *pe;
	int len, checksum;

	if (sz > 16)
		errx(EX_SOFTWARE, "Internal: ihex_write() sz too big");
	checksum = 0;
	line[0] = ':';
	len = 1;
	write_num(line, &len, sz, 1, &checksum);
	write_num(line, &len, addr, 2, &checksum);
	write_num(line, &len, type, 1, &checksum);
	if (sz > 0) {
		if (buf != NULL) {
			for (p = buf, pe = p + sz; p < pe; p++)
				write_num(line, &len, *p, 1, &checksum);
		} else
			write_num(line, &len, num, sz, &checksum);
	}
	write_num(line, &len, (~checksum + 1) & 0xFF, 1, NULL);
	line[len++] = '\r';
	line[len++] = '\n';
	if (write(ofd, line, len) != (ssize_t) len)
		err(EX_DATAERR, "write failed");
}

static void
write_num(char *line, int *len, uint64_t num, size_t sz, int *checksum)
{
	uint8_t b;

	for (; sz > 0; sz--) {
		b = (num >> ((sz - 1) * 8)) & 0xFF;
		line[*len] = hex_digit((b >> 4) & 0xF);
		line[*len + 1] = hex_digit(b & 0xF);
		*len += 2;
		if (checksum != NULL)
			*checksum = (*checksum + b) & 0xFF;
	}
}

static char
hex_digit(uint8_t n)
{

	return ((n < 10) ? '0' + n : 'A' + (n - 10));
}
