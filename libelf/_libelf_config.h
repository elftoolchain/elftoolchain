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

#include <sys/limits.h>
#include <machine/elf.h>

#if __FreeBSD_version >= 330000
#define	LIBELF_CONFIG_STRL_FUNCTIONS	1
#endif

#if __FreeBSD_version >= 600102
#define	LIBELF_CONFIG_ADDR	1
#define	LIBELF_CONFIG_BYTE	1
#define	LIBELF_CONFIG_DYN	1
#define	LIBELF_CONFIG_EHDR	1
#define	LIBELF_CONFIG_HALF	1
#define	LIBELF_CONFIG_NOTE	1
#define	LIBELF_CONFIG_OFF	1
#define	LIBELF_CONFIG_PHDR	1
#define	LIBELF_CONFIG_REL	1
#define	LIBELF_CONFIG_RELA	1
#define	LIBELF_CONFIG_SHDR	1
#define	LIBELF_CONFIG_SWORD	1
#define	LIBELF_CONFIG_SYM	1
#define	LIBELF_CONFIG_WORD	1
#endif

#if __FreeBSD_version >= 700009
#define	LIBELF_CONFIG_VDEF	1
#define	LIBELF_CONFIG_VNEED	1
#define	LIBELF_CONFIG_XWORD	1
#endif

#if __FreeBSD_version >= 700025
#define	LIBELF_CONFIG_CAP	1
#define	LIBELF_CONFIG_LWORD	1
#define	LIBELF_CONFIG_MOVE	1
#define	LIBELF_CONFIG_MOVEP	1
#define	LIBELF_CONFIG_SYMINFO	1
#endif

#if __FreeBSD_version >= 800062
#define	LIBELF_CONFIG_GNUHASH	1
#endif

#define	LIBELF_VCSID(ID)	__FBSDID(ID)

#define	LIBELF_ARCH		ELF_ARCH
#define	LIBELF_BYTEORDER	ELF_TARG_DATA
#define	LIBELF_CLASS		ELF_TARG_CLASS

#endif  /* __FreeBSD__ */


#ifdef __NetBSD__

#include <sys/exec_elf.h>
#include <machine/elf_machdep.h>

#define	LIBELF_CONFIG_STRL_FUNCTIONS	1

#define	LIBELF_CONFIG_ADDR	1
#define	LIBELF_CONFIG_BYTE	1
#define	LIBELF_CONFIG_DYN	1
#define	LIBELF_CONFIG_EHDR	1
#define	LIBELF_CONFIG_HALF	1
#define	LIBELF_CONFIG_NOTE	1
#define	LIBELF_CONFIG_OFF	1
#define	LIBELF_CONFIG_PHDR	1
#define	LIBELF_CONFIG_REL	1
#define	LIBELF_CONFIG_RELA	1
#define	LIBELF_CONFIG_SHDR	1
#define	LIBELF_CONFIG_SWORD	1
#define	LIBELF_CONFIG_SYM	1
#define	LIBELF_CONFIG_WORD	1

#define	LIBELF_CONFIG_SXWORD	1
#define	LIBELF_CONFIG_XWORD	1

#define	LIBELF_CONFIG_MOVEP	1

#if	__NetBSD_Version > 400000003	/* NetBSD 4.0 */
#define	LIBELF_CONFIG_CAP	1
#define	LIBELF_CONFIG_MOVE	1
#define	LIBELF_CONFIG_SYMINFO	1
#define	LIBELF_CONFIG_LWORD	1
#define	LIBELF_CONFIG_VDEF	1
#define	LIBELF_CONFIG_VNEED	1
#endif	/* __NetBSD_Version > 400000003 */

#define	LIBELF_VCSID(ID)	__RCSID(ID)

#ifndef STAILQ_FOREACH_SAFE
#define STAILQ_FOREACH_SAFE(var, head, field, tvar)            \
       for ((var) = STAILQ_FIRST((head));                      \
            (var) && ((tvar) = STAILQ_NEXT((var), field), 1);  \
            (var) = (tvar))
#endif

#ifndef	STAILQ_LAST
#define STAILQ_LAST(head, type, field)                                  \
        (STAILQ_EMPTY((head)) ?                                         \
                NULL :                                                  \
                ((struct type *)(void *)                                \
                ((char *)((head)->stqh_last) - offsetof(struct type, field))))
#endif

#define	roundup2	roundup

#if	ARCH_ELFSIZE == 32
#define	LIBELF_ARCH		ELF32_MACHDEP_ID
#define	LIBELF_BYTEORDER	ELF32_MACHDEP_ENDIANNESS
#define	LIBELF_CLASS		ELFCLASS32
#define	Elf_Note		Elf32_Nhdr
#else
#define	LIBELF_ARCH		ELF64_MACHDEP_ID
#define	LIBELF_BYTEORDER	ELF64_MACHDEP_ENDIANNESS
#define	LIBELF_CLASS		ELFCLASS64
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

#include "native-elf-format.h"

#endif	/* defined(__linux__) */

/*
 * Common configuration for the GNU environment.
 */

#define	LIBELF_CONFIG_ADDR	1
#define	LIBELF_CONFIG_BYTE	1
#define	LIBELF_CONFIG_DYN	1
#define	LIBELF_CONFIG_EHDR	1
#define	LIBELF_CONFIG_HALF	1
#define	LIBELF_CONFIG_MOVEP	1
#define	LIBELF_CONFIG_NOTE	1
#define	LIBELF_CONFIG_OFF	1
#define	LIBELF_CONFIG_PHDR	1
#define	LIBELF_CONFIG_REL	1
#define	LIBELF_CONFIG_RELA	1
#define	LIBELF_CONFIG_SHDR	1
#define	LIBELF_CONFIG_SWORD	1
#define	LIBELF_CONFIG_SXWORD	1
#define	LIBELF_CONFIG_SYM	1
#define	LIBELF_CONFIG_VDEF	1
#define	LIBELF_CONFIG_VNEED	1
#define	LIBELF_CONFIG_WORD	1
#define	LIBELF_CONFIG_XWORD	1

#define	LIBELF_VCSID(ID)

#if	LIBELF_CLASS == ELFCLASS32
#define	Elf_Note		Elf32_Nhdr
#elif   LIBELF_CLASS == ELFCLASS64
#define	Elf_Note		Elf64_Nhdr
#else
#error  LIBELF_CLASS needs to be one of ELFCLASS32 or ELFCLASS64
#endif

#define	roundup2	roundup

/*
 * Supply macros missing from <sys/queue.h>
 */

#ifndef	STAILQ_FOREACH_SAFE
#define STAILQ_FOREACH_SAFE(var, head, field, tvar)            \
       for ((var) = STAILQ_FIRST((head));                      \
            (var) && ((tvar) = STAILQ_NEXT((var), field), 1);  \
            (var) = (tvar))
#endif

#ifndef	STAILQ_LAST
#define STAILQ_LAST(head, type, field)                                  \
        (STAILQ_EMPTY((head)) ?                                         \
                NULL :                                                  \
                ((struct type *)(void *)                                \
                ((char *)((head)->stqh_last) - offsetof(struct type, field))))
#endif


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

/*
 * Symbols that are sometimes missing in system headers.
 */

#ifndef	PN_XNUM
#define	PN_XNUM			0xFFFFU
#endif

#ifndef	SHN_XINDEX
#define	SHN_XINDEX		0xFFFFU
#endif

#ifndef	SHT_GNU_HASH
#define	SHT_GNU_HASH		0x6FFFFFF6U
#endif

#ifndef	LIBELF_CONFIG_GNUHASH
#define	LIBELF_CONFIG_GNUHASH	1

/*
 * The header for GNU-style hash sections.
 */

typedef struct {
	u_int32_t	gh_nbuckets;	/* Number of hash buckets. */
	u_int32_t	gh_symndx;	/* First visible symbol in .dynsym. */
	u_int32_t	gh_maskwords;	/* #maskwords used in bloom filter. */
	u_int32_t	gh_shift2;	/* Bloom filter shift count. */
} Elf_GNU_Hash_Header;
#endif
