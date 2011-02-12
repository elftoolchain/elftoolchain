/*-
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
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

#include "_libdwarf.h"

void
dwarf_dealloc(Dwarf_Debug dbg, Dwarf_Ptr p, Dwarf_Unsigned alloc_type)
{
	/*
	 * This libdwarf implementation does not use the SGI/libdwarf
	 * style of memory allocation. It does not copy things to return
	 * to the client, so the client does not need to remember to
	 * free them.
	 */

	(void) dbg; (void) p; (void) alloc_type;
}

void
dwarf_srclines_dealloc(Dwarf_Debug dbg, Dwarf_Line *linebuf,
	Dwarf_Signed count)
{
	/*
	 * In this libdwarf implementation, line information remains
	 * associated with the DIE for a compilation unit for the
	 * lifetime of the DIE.  The client does not need to free
	 * the memory returned by `dwarf_srclines()`.
	 */ 

	(void) dbg; (void) linebuf; (void) count;
}

void
dwarf_ranges_dealloc(Dwarf_Debug dbg, Dwarf_Ranges *ranges,
    Dwarf_Signed range_count)
{
	/*
	 * In this libdwarf implementation, ranges information is
	 * kept by a STAILQ inside Dwarf_Debug object. The client
	 * does not need to free the memory returned by
	 * `dwarf_get_ranges()` or `dwarf_get_ranges_a()`.
	 */

	(void) dbg; (void) ranges; (void) range_count;
}
