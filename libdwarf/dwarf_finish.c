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
 *
 * $FreeBSD: src/lib/libdwarf/dwarf_finish.c,v 1.1 2008/05/22 02:14:23 jb Exp $
 */

#include <stdlib.h>
#include "_libdwarf.h"

int
dwarf_finish(Dwarf_Debug dbg, Dwarf_Error *error __unused)
{
	Dwarf_Abbrev ab;
	Dwarf_Abbrev tab;
	Dwarf_AttrDef ad;
	Dwarf_AttrDef tad;
	Dwarf_Attribute at;
	Dwarf_Attribute tat;
	Dwarf_CU cu;
	Dwarf_CU tcu;
	Dwarf_Die die;
	Dwarf_Die tdie;
	Dwarf_Loclist ll;
	Dwarf_Loclist tll;
	Dwarf_LineInfo li;
	Dwarf_LineFile lf, tlf;
	Dwarf_Line ln, tln;

	if (dbg == NULL)
		return (DW_DLV_OK);

	/* Free entries in the compilation unit list. */
	STAILQ_FOREACH_SAFE(cu, &dbg->dbg_cu, cu_next, tcu) {
		/* Free entries in the die list */
		STAILQ_FOREACH_SAFE(die, &cu->cu_die, die_next, tdie) {
			/* Free entries in the attribute list */
			STAILQ_FOREACH_SAFE(at, &die->die_attr, at_next, tat) {
				STAILQ_REMOVE(&die->die_attr, at,
				    _Dwarf_Attribute, at_next);
				if (at->at_ld != NULL)
					free(at->at_ld);
				free(at);
			}

			if (die->die_attrarray)
				free(die->die_attrarray);

			STAILQ_REMOVE(&cu->cu_die, die, _Dwarf_Die, die_next);
			free(die);
		}

		/* Free entries in the abbrev list */
		STAILQ_FOREACH_SAFE(ab, &cu->cu_abbrev, ab_next, tab) {
			/* Free entries in the attribute list */
			STAILQ_FOREACH_SAFE(ad, &ab->ab_attrdef, ad_next, tad) {
				STAILQ_REMOVE(&ab->ab_attrdef, ad,
				    _Dwarf_AttrDef, ad_next);
				free(ad);
			}

			STAILQ_REMOVE(&cu->cu_abbrev, ab, _Dwarf_Abbrev,
			    ab_next);
			free(ab);
		}

		/* Free lineinfo. */
		if (cu->cu_lineinfo != NULL) {
			li = cu->cu_lineinfo;
			STAILQ_FOREACH_SAFE(lf, &li->li_lflist, lf_next, tlf) {
				STAILQ_REMOVE(&li->li_lflist, lf,
				    _Dwarf_LineFile, lf_next);
				if (lf->lf_fullpath)
					free(lf->lf_fullpath);
				free(lf);
			}
			STAILQ_FOREACH_SAFE(ln, &li->li_lnlist, ln_next, tln) {
				STAILQ_REMOVE(&li->li_lnlist, ln, _Dwarf_Line,
				    ln_next);
				free(ln);
			}
			if (li->li_oplen)
				free(li->li_oplen);
			if (li->li_incdirs)
				free(li->li_incdirs);
			if (li->li_lnarray)
				free(li->li_lnarray);
			free(li);
		}

		STAILQ_REMOVE(&dbg->dbg_cu, cu, _Dwarf_CU, cu_next);
		free(cu);
	}

	/* Free loclist list. */
	TAILQ_FOREACH_SAFE(ll, &dbg->dbg_loclist, ll_next, tll) {
		TAILQ_REMOVE(&dbg->dbg_loclist, ll, ll_next);
		loclist_cleanup(ll);
	}

	/* Free rangelist. */
	ranges_cleanup(dbg);

	/* Free name lookup tables. */
	if (dbg->dbg_globals)
		nametbl_cleanup(dbg->dbg_globals);
	if (dbg->dbg_pubtypes)
		nametbl_cleanup(dbg->dbg_pubtypes);
	if (dbg->dbg_weaks)
		nametbl_cleanup(dbg->dbg_weaks);
	if (dbg->dbg_funcs)
		nametbl_cleanup(dbg->dbg_funcs);
	if (dbg->dbg_vars)
		nametbl_cleanup(dbg->dbg_vars);
	if (dbg->dbg_types)
		nametbl_cleanup(dbg->dbg_types);

	/* Free call frame data. */
	frame_cleanup(dbg);

	/* Free address range data. */
	arange_cleanup(dbg);

	/* Free macinfo data. */
	macinfo_cleanup(dbg);

	/* Free resources associated with the ELF file. */
	if (dbg->dbg_elf_close)
		elf_end(dbg->dbg_elf);

	free(dbg);

	return (DW_DLV_OK);
}
