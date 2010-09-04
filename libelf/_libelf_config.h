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

#define	LIBELF_VCSID(ID)	__FBSDID(ID)

#define	LIBELF_ARCH		ELF_ARCH
#define	LIBELF_BYTEORDER	ELF_TARG_DATA
#define	LIBELF_CLASS		ELF_TARG_CLASS

#endif  /* __FreeBSD__ */


#ifdef __NetBSD__

#include <sys/exec_elf.h>
#include <machine/elf_machdep.h>

#define	LIBELF_CONFIG_STRL_FUNCTIONS	1

#define	LIBELF_VCSID(ID)	__RCSID(ID)

#define	roundup2	roundup

#if	!defined(ARCH_ELFSIZE)
#error	ARCH_ELFSIZE is not defined.
#endif

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

#define	LIBELF_CLASS		ELFTC_CLASS
#define	LIBELF_ARCH		ELFTC_ARCH
#define	LIBELF_BYTEORDER	ELFTC_BYTEORDER

#endif	/* defined(__linux__) */

#define	LIBELF_VCSID(ID)

#if	LIBELF_CLASS == ELFCLASS32
#define	Elf_Note		Elf32_Nhdr
#elif   LIBELF_CLASS == ELFCLASS64
#define	Elf_Note		Elf64_Nhdr
#else
#error  LIBELF_CLASS needs to be one of ELFCLASS32 or ELFCLASS64
#endif

#define	roundup2	roundup

#endif /* defined(__linux__) || defined(__GNU__) || defined(__GLIBC__) */

