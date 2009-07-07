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

#define	READELF_VCSID(ID)	__FBSDID(ID)

#endif  /* __FreeBSD__ */

#ifdef __NetBSD__

#include <sys/endian.h>

#define	READELF_VCSID(ID)	__RCSID(ID)

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

#define	READELF_VCSID(ID)

#if	ELF_CLASS == ELFCLASS32
#define	Elf_Note		Elf32_Nhdr
#elif   ELF_CLASS == ELFCLASS64
#define	Elf_Note		Elf64_Nhdr
#else
#error  ELF_CLASS needs to be one of ELFCLASS32 or ELFCLASS64
#endif

#define	roundup2	roundup

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
 * Supply missing EM_XXX definitions
 */
#ifndef	EM_PDSP
#define	EM_PDSP		63
#endif
#ifndef	EM_FX66
#define	EM_FX66		66
#endif
#ifndef	EM_ST9PLUS
#define	EM_ST9PLUS	67
#endif
#ifndef	EM_ST7
#define	EM_ST7		68
#endif
#ifndef	EM_68HC16
#define	EM_68HC16	69
#endif
#ifndef	EM_68HC11
#define	EM_68HC11	70
#endif
#ifndef	EM_68HC08
#define	EM_68HC08	71
#endif
#ifndef	EM_68HC05
#define	EM_68HC05	72
#endif
#ifndef	EM_SVX
#define	EM_SVX		73
#endif
#ifndef	EM_ST19
#define	EM_ST19		74
#endif
#ifndef	EM_VAX
#define	EM_VAX		75
#endif
#ifndef	EM_CRIS
#define	EM_CRIS		76
#endif
#ifndef	EM_JAVELIN
#define	EM_JAVELIN	77
#endif
#ifndef	EM_FIREPATH
#define	EM_FIREPATH	78
#endif
#ifndef	EM_ZSP
#define	EM_ZSP		79
#endif
#ifndef	EM_MMIX
#define	EM_MMIX		80
#endif
#ifndef	EM_HUANY
#define	EM_HUANY	81
#endif
#ifndef	EM_PRISM
#define	EM_PRISM	82
#endif
#ifndef	EM_AVR
#define	EM_AVR		83
#endif
#ifndef	EM_FR30
#define	EM_FR30		84
#endif
#ifndef	EM_D10V
#define	EM_D10V		85
#endif
#ifndef	EM_D30V
#define	EM_D30V		86
#endif
#ifndef	EM_V850
#define	EM_V850		87
#endif
#ifndef	EM_M32R
#define	EM_M32R		88
#endif
#ifndef	EM_MN10300
#define	EM_MN10300	89
#endif
#ifndef	EM_MN10200
#define	EM_MN10200	90
#endif
#ifndef	EM_PJ
#define	EM_PJ		91
#endif
#ifndef	EM_OPENRISC
#define	EM_OPENRISC	92
#endif
#ifndef	EM_ARC_A5
#define	EM_ARC_A5	93
#endif
#ifndef	EM_XTENSA
#define	EM_XTENSA	94
#endif
#ifndef	EM_VIDEOCORE
#define	EM_VIDEOCORE	95
#endif
#ifndef	EM_TMM_GPP
#define	EM_TMM_GPP	96
#endif
#ifndef	EM_NS32K
#define	EM_NS32K	97
#endif
#ifndef	EM_TPC
#define	EM_TPC		98
#endif
#ifndef	EM_SNP1K
#define	EM_SNP1K	99
#endif
#ifndef	EM_ST200
#define	EM_ST200	100
#endif
#ifndef	EM_IP2K
#define	EM_IP2K		101
#endif
#ifndef	EM_MAX
#define	EM_MAX		102
#endif
#ifndef	EM_CR
#define	EM_CR		103
#endif
#ifndef	EM_F2MC16
#define	EM_F2MC16	104
#endif
#ifndef	EM_MSP430
#define	EM_MSP430	105
#endif
#ifndef	EM_BLACKFIN
#define	EM_BLACKFIN	106
#endif
#ifndef	EM_SE_C33
#define	EM_SE_C33	107
#endif
#ifndef	EM_SEP
#define	EM_SEP		108
#endif
#ifndef	EM_ARCA
#define	EM_ARCA		109
#endif
#ifndef	EM_UNICORE
#define	EM_UNICORE	110
#endif

/*
 * GNU hash section.
 */
#ifndef	SHT_GNU_HASH
#define	SHT_GNU_HASH		0x6FFFFFF6U
#endif

#ifndef	DT_GNU_HASH
#define	DT_GNU_HASH	0x6ffffef5
#endif
