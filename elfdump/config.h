/*-
 * Copyright (c) 2008 Joseph Koshy
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

#ifdef __FreeBSD__

#include <sys/endian.h>

#define	ELFDUMP_VCSID(ID)	__FBSDID(ID)

#endif  /* __FreeBSD__ */

#ifdef __NetBSD__

#include <sys/endian.h>

#define	ELFDUMP_VCSID(ID)	__RCSID(ID)

#define	roundup2	roundup

#if	ARCH_ELFSIZE == 32
#define	Elf_Note		Elf32_Nhdr
#else
#define	Elf_Note		Elf64_Nhdr
#endif

#endif	/* __NetBSD__ */

/*
 * GNU & Linux compatibility.
 *
 * `__linux__' is defined in an environment runs the Linux kernel and glibc.
 * `__GNU__' is defined in an environment runs a GNU kernel (Hurd) and glibc.
 * `__GLIBC__' is defined for an environment that runs glibc over a non-GNU
 *     kernel such as GNU/kFreeBSD.
 */

#if defined(__linux__) || defined(__GNU__) || defined(__GLIBC__)

#if defined(__linux__)

/*
 * We include <asm/elf.h> in order to access the symbols `ELF_ARCH',
 * `ELF_DATA' and `ELF_CLASS'.  However, a few symbols in this
 * file will collide with those in <elf.h> so these need to be
 * explicitly #undef'ed.
 */

#undef R_386_NUM
#undef R_X86_64_NUM

#include <asm/elf.h>

#endif	/* defined(__linux__) */

#define	ELFDUMP_VCSID(ID)

#if	ELF_CLASS == ELFCLASS32
#define	Elf_Note		Elf32_Nhdr
#elif   ELF_CLASS == ELFCLASS64
#define	Elf_Note		Elf64_Nhdr
#else
#error  ELF_CLASS needs to be one of ELFCLASS32 or ELFCLASS64
#endif

#define	roundup2	roundup

static __inline uint32_t
be32dec(const void *pp)
{
	unsigned char const *p = (unsigned char const *)pp;

	return ((p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
}

static __inline uint32_t
le32dec(const void *pp)
{
	unsigned char const *p = (unsigned char const *)pp;

	return ((p[3] << 24) | (p[2] << 16) | (p[1] << 8) | p[0]);
}

/*
 * Copyright (c) 1998 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

/*
 * Appends src to string dst of size siz (unlike strncat, siz is the
 * full size of dst, not space left).  At most siz-1 characters
 * will be copied.  Always NUL terminates (unless siz <= strlen(dst)).
 * Returns strlen(src) + MIN(siz, strlen(initial dst)).
 * If retval >= siz, truncation occurred.
 */

static size_t
strlcat(char * __restrict dst, const char * __restrict src, size_t siz)
{
	char *d = dst;
	const char *s = src;
	size_t n = siz;
	size_t dlen;

	/* Find the end of dst and adjust bytes left but don't go past end */
	while (n-- != 0 && *d != '\0')
		d++;
	dlen = d - dst;
	n = siz - dlen;

	if (n == 0)
		return(dlen + strlen(s));
	while (*s != '\0') {
		if (n != 1) {
			*d++ = *s;
			n--;
		}
		s++;
	}
	*d = '\0';

	return(dlen + (s - src));	/* count does not include NUL */
}

/*
 * Supply symbols missing in at least some Linux based systems.
 */

#ifndef	SHT_SUNW_verdef
#define	SHT_SUNW_verdef		0x6FFFFFFD
#endif

#ifndef	SHT_SUNW_verneed
#define	SHT_SUNW_verneed	0x6FFFFFFE
#endif

#ifndef	SHT_SUNW_versym
#define	SHT_SUNW_versym		0x6FFFFFFF
#endif

#endif /* defined(__linux__) || defined(__GNU__) || defined(__GLIBC__) */
