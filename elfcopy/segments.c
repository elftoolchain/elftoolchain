/*-
 * Copyright (c) 2007,2008 Kai Wang
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
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(__FBSDID)
__FBSDID("$FreeBSD$");
#elif defined(__RCSID)
__RCSID("$Id$");
#endif

#include <sys/queue.h>
#include <err.h>
#include <gelf.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>

#include "elfcopy.h"

static void	insert_to_inseg_list(struct segment *seg, struct section *sec);

/*
 * elfcopy's segment handling is relatively simpler and less powerful than
 * libbfd. Program headers are modified or copied from input to output objects,
 * but never re-generated. As a result, if the input object has incorrect
 * program headers, the output object's program headers will remain incorrect
 * or become even worse.
 */

/*
 * Check whether a section is "loadable". If so, add it to the
 * corresponding segment list(s) and return 1.
 */
int
add_to_inseg_list(struct elfcopy *ecp, struct section *s)
{
	struct segment	*seg;
	int		 loadable;

	if (ecp->ophnum == 0)
		return (0);

	/*
	 * Segment is a different view of an ELF object. One segment can
	 * contain one or more sections, and one section can be included
	 * in one or more segments, or not included in any segment at all.
	 * We call those sections which can be found in one or more segments
	 * "loadable" sections, and call the rest "unloadable" sections.
	 * We keep track of "loadable" sections in their containing
	 * segment(s)' v_sec queue. These information are later used to
	 * recalculate the extents of segments, when sections are removed,
	 * for example.
	 */
	loadable = 0;
	STAILQ_FOREACH(seg, &ecp->v_seg, seg_list) {
		if (s->off < seg->off)
			continue;
		if (s->off + s->sz > seg->off + seg->fsz &&
		    s->type != SHT_NOBITS)
			continue;
		if (s->off + s->sz > seg->off + seg->msz)
			continue;

		insert_to_inseg_list(seg, s);
		loadable = 1;
	}

	return (loadable);
}

static void
insert_to_inseg_list(struct segment *seg, struct section *sec)
{
	struct section *s;

	TAILQ_FOREACH(s, &seg->v_sec, in_seg) {
		if (sec->off < s->off) {
			TAILQ_INSERT_BEFORE(s, sec, in_seg);
			return;
		}
	}

	TAILQ_INSERT_TAIL(&seg->v_sec, sec, in_seg);
}

void
setup_phdr(struct elfcopy *ecp)
{
	struct segment	*seg;
	GElf_Phdr	 iphdr;
	size_t		 iphnum;
	int		 i;

	if (elf_getphnum(ecp->ein, &iphnum) == 0)
		errx(EX_DATAERR, "elf_getphnum failed: %s",
		    elf_errmsg(-1));

	ecp->ophnum = ecp->iphnum = iphnum;
	if (iphnum == 0)
		return;

	/* If --only-keep-debug is specified, discard all program headers. */
	if (ecp->strip == STRIP_NONDEBUG) {
		ecp->ophnum = 0;
		return;
	}
		
	for (i = 0; (size_t)i < iphnum; i++) {
		if (gelf_getphdr(ecp->ein, i, &iphdr) != &iphdr)
			errx(EX_SOFTWARE, "gelf_getphdr failed: %s",
			    elf_errmsg(-1));
		if ((seg = calloc(1, sizeof(*seg))) == NULL)
			err(EX_SOFTWARE, "calloc failed");
		seg->off	= iphdr.p_offset;
		seg->fsz	= iphdr.p_filesz;
		seg->msz	= iphdr.p_memsz;
		seg->type	= iphdr.p_type;
		TAILQ_INIT(&seg->v_sec);
		STAILQ_INSERT_TAIL(&ecp->v_seg, seg, seg_list);
	}
}

void
copy_phdr(struct elfcopy *ecp)
{
	struct segment	*seg;
	struct section	*s;
	GElf_Phdr	 iphdr, ophdr;
	size_t		 t;
	int		 i;

	STAILQ_FOREACH(seg, &ecp->v_seg, seg_list) {
		/* Do not touch phdr itself. */
		if (seg->type == PT_PHDR)
			continue;

		if (!TAILQ_EMPTY(&seg->v_sec)) {
			s = TAILQ_LAST(&seg->v_sec, sec_head);
			t = s->off + s->sz - seg->off;
			/*
			 * We simply assume fsz and msz become the same if
			 * sections are removed at the end of a segment.
			 * This is usually true, since .bss section is usually
			 * positioned at the end.
			 */
			if (seg->msz != t)
				seg->fsz = seg->msz = t;
		} else
			seg->fsz = seg->msz = 0;
	}

	/*
	 * Allocate space for program headers, note that libelf keep
	 * track of the number in internal variable, and a call to
	 * elf_update is needed to update e_phnum of ehdr.
	 */
	if (gelf_newphdr(ecp->eout, ecp->ophnum) == NULL)
		errx(EX_SOFTWARE, "gelf_newphdr() failed: %s", elf_errmsg(-1));

	/*
	 * This elf_update() call is to update the e_phnum field in
	 * ehdr. It's necessary because later we will call gelf_getphdr(),
	 * which does sanity check by comparing ndx argument with e_phnum.
	 */
	if (elf_update(ecp->eout, ELF_C_NULL) < 0)
		errx(EX_SOFTWARE, "elf_update() failed: %s", elf_errmsg(-1));

	/*
	 * iphnum == ophnum, since we don't remove program headers even if
	 * they no longer contain sections.
	 */
	i = 0;
	STAILQ_FOREACH(seg, &ecp->v_seg, seg_list) {
		if (i >= ecp->iphnum)
			break;
		if (gelf_getphdr(ecp->ein, i, &iphdr) != &iphdr)
			errx(EX_SOFTWARE, "gelf_getphdr failed: %s",
			    elf_errmsg(-1));
		if (gelf_getphdr(ecp->eout, i, &ophdr) != &ophdr)
			errx(EX_SOFTWARE, "gelf_getphdr failed: %s",
			    elf_errmsg(-1));

		ophdr.p_type = iphdr.p_type;
		ophdr.p_vaddr = iphdr.p_vaddr;
		ophdr.p_paddr = iphdr.p_paddr;
		ophdr.p_flags = iphdr.p_flags;
		ophdr.p_align = iphdr.p_align;
		ophdr.p_offset = iphdr.p_offset;
		ophdr.p_filesz = seg->fsz;
		ophdr.p_memsz = seg->msz;
		if (!gelf_update_phdr(ecp->eout, i, &ophdr))
			err(EX_SOFTWARE, "gelf_update_phdr failed :%s",
			    elf_errmsg(-1));

		i++;
	}
}
