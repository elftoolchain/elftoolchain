/*-
 * Copyright (c) 2013 Kai Wang
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

#include "ld.h"
#include "ld_ehframe.h"
#include "ld_input.h"
#include "ld_output.h"
#include "ld_reloc.h"
#include "ld_utils.h"

ELFTC_VCSID("$Id$");

struct ld_ehframe_cie {
	uint64_t cie_off;	/* offset in section */
	uint64_t cie_off_orig;	/* orignial offset (before optimze) */
	uint64_t cie_size;	/* CIE size (include length field) */
	uint8_t *cie_content;	/* CIE content */
	struct ld_ehframe_cie *cie_dup; /* duplicate entry */
	STAILQ_ENTRY(ld_ehframe_cie) cie_next;
};

STAILQ_HEAD(ld_ehframe_cie_head, ld_ehframe_cie);

static void _process_ehframe_section(struct ld *ld, struct ld_output *lo,
    struct ld_input_section *is);

void
ld_ehframe_scan(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_output_section *os;
	struct ld_output_element *oe;
	struct ld_input_section *is;
	struct ld_input_section_head *islist;
	uint64_t ehframe_off;
	char ehframe_name[] = ".eh_frame";

	lo = ld->ld_output;
	assert(lo != NULL);

	/*
	 * Search for .eh_frame output section. Nothing needs to be done
	 * if .eh_frame section not exist or is empty.
	 */
	HASH_FIND_STR(lo->lo_ostbl, ehframe_name, os);
	if (os == NULL || os->os_empty)
		return;

	if ((ld->ld_cie = malloc(sizeof(*ld->ld_cie))) == NULL)
		ld_fatal_std(ld, "malloc");
	STAILQ_INIT(ld->ld_cie);

	/*
	 * Remove duplicate CIE from each input .eh_frame section.
	 */
	ehframe_off = 0;
	STAILQ_FOREACH(oe, &os->os_e, oe_next) {
		/*
		 * XXX We currently do not support .eh_frame section which
		 * contains elements other than OET_INPUT_SECTION_LIST.
		 */
		if (oe->oe_type != OET_INPUT_SECTION_LIST)
			continue;

		islist = oe->oe_islist;
		STAILQ_FOREACH(is, islist, is_next) {
			/*
			 * Process each input .eh_frame section and search
			 * for duplicate CIE's. The input section relative
			 * offset in the output section is resync'ed since
			 * the input section might be shrinked.
			 */
			is->is_reloff = ehframe_off;
			_process_ehframe_section(ld, lo, is);
			ehframe_off += is->is_size;
		}
	}
}

static void
_process_ehframe_section(struct ld *ld, struct ld_output *lo,
    struct ld_input_section *is)
{
	struct ld_input *li;
	struct ld_ehframe_cie *cie, *_cie;
	struct ld_ehframe_cie_head cie_h;
	struct ld_reloc_entry *lre;
	uint64_t length, es, off, off_orig, remain, shrink;
	uint32_t cie_id, cie_pointer, length_size;
	uint8_t *p, *et;

	li = is->is_input;

	STAILQ_INIT(&cie_h);

	/*
	 * .eh_frame section content should already be preloaded
	 * in is->is_ibuf.
	 */
	assert(is->is_ibuf != NULL && is->is_size > 0);

	shrink = 0;
	p = is->is_ibuf;
	off = off_orig = 0;
	remain = is->is_size;
	while (remain > 0) {

		et = p;
		off = et - (uint8_t *) is->is_ibuf;

		/* Read CIE/FDE length field. */
		READ_32(p, length);
		p += 4;
		es = length + 4;
		if (length == 0xffffffff) {
			READ_64(p, length);
			p += 8;
			es += 8;
			length_size = 8;
		} else
			length_size = 4;

		/* Check for terminator */
		if (length == 0)
			break;

		/* Read CIE ID/Pointer field. */
		READ_32(p, cie_id);
		p += 4;
	
		if (cie_id == 0) {

			/* This is a Common Information Entry (CIE). */
			if ((cie = malloc(sizeof(*cie))) == NULL)
				ld_fatal_std(ld, "malloc");
			cie->cie_off = off;
			cie->cie_off_orig = off_orig;
			cie->cie_size = es;
			cie->cie_content = et;
			cie->cie_dup = NULL;
			STAILQ_INSERT_TAIL(&cie_h, cie, cie_next);

			/*
			 * This is a Common Information Entry (CIE). Search
			 * in the CIE list see if we can found a duplicate
			 * entry.
			 */
			STAILQ_FOREACH(_cie, ld->ld_cie, cie_next) {
				if (memcmp(et, _cie->cie_content, es) == 0) {
					cie->cie_dup = _cie;
					break;
				}
			}
			if (_cie != NULL) {
				/*
				 * We found a duplicate entry. It should be
				 * removed and the subsequent FDE's should
				 * point to the previously stored CIE.
				 */
				memmove(et, et + es, remain - es);
				shrink += es;
				p = et;
			} else {
				/*
				 * This is a new CIE entry which should be
				 * kept.
				 */
				p = et + es;
			}

		} else {

			/*
			 * This is a Frame Description Entry (FDE). First
			 * Search for the associated CIE.
			 */
			STAILQ_FOREACH(cie, &cie_h, cie_next) {
				if (cie->cie_off_orig ==
				    off_orig + length_size - cie_id)
					break;
			}

			/*
			 * If we can not found the associated CIE, this FDE
			 * is invalid and we ignore it.
			 */
			if (cie == NULL) {
				ld_warn(ld, "%s(%s): malformed FDE",
				    li->li_name, is->is_name);
				p = et + es;
				goto next_entry;
			}

			/* Calculate the new CIE pointer value. */
			if (cie->cie_dup != NULL)
				cie_pointer = off + length_size +
				    is->is_reloff - cie->cie_dup->cie_off;
			else
				cie_pointer = off + length_size - cie->cie_off;

			/* Rewrite CIE pointer value. */
			if (cie_id != cie_pointer) {
				p -= 4;
				WRITE_32(p, cie_pointer);
			}

			p = et + es;
		}

	next_entry:
		off_orig += es;
		remain -= es;
	}

	/*
	 * Update the relocation entry offsets since we shrinked the
	 * section content.
	 */
	if (shrink > 0 && is->is_ris != NULL && is->is_ris->is_reloc != NULL) {
		STAILQ_FOREACH(lre, is->is_ris->is_reloc, lre_next) {
			STAILQ_FOREACH(cie, &cie_h, cie_next) {
				if (cie->cie_off_orig > lre->lre_offset)
					break;
				if (cie->cie_dup == NULL)
					continue;
				lre->lre_offset -= cie->cie_size;
			}
		}
	}

	/* Insert newly found non-duplicate CIE's to the global CIE list. */
	STAILQ_FOREACH_SAFE(cie, &cie_h, cie_next, _cie) {
		STAILQ_REMOVE(&cie_h, cie, ld_ehframe_cie, cie_next);
		if (cie->cie_dup == NULL) {
			cie->cie_off += is->is_reloff;
			STAILQ_INSERT_TAIL(ld->ld_cie, cie, cie_next);
		}
	}

	/* Update the size of input .eh_frame section */
	is->is_size -= shrink;
}
