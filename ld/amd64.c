/*-
 * Copyright (c) 2012 Kai Wang
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
#include "ld_arch.h"
#include "ld_dynamic.h"
#include "ld_input.h"
#include "ld_layout.h"
#include "ld_output.h"
#include "ld_reloc.h"
#include "ld_symbols.h"
#include "ld_utils.h"
#include "amd64.h"

ELFTC_VCSID("$Id$");

static void _create_plt_reloc(struct ld *ld, struct ld_symbol *lsb,
    uint64_t offset);
static void _create_got_reloc(struct ld *ld, struct ld_symbol *lsb,
    uint64_t type, uint64_t offset);
static void _create_copy_reloc(struct ld *ld, struct ld_symbol *lsb);
static void _create_dynamic_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_symbol *lsb, uint64_t type, uint64_t offset, int64_t addend);
static void _scan_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_reloc_entry *lre);
static struct ld_input_section *_find_and_create_got_section(struct ld *ld,
    int create);
static struct ld_input_section *_find_and_create_gotplt_section(struct ld *ld,
    int create);
static struct ld_input_section *_find_and_create_plt_section(struct ld *ld,
    int create);
static void _finalize_got_and_plt(struct ld *ld);
static uint64_t _get_max_page_size(struct ld *ld);
static uint64_t _get_common_page_size(struct ld *ld);
static void _process_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_reloc_entry *lre, struct ld_symbol *lsb, uint8_t *buf);
static const char *_reloc2str(uint64_t r);
static void _reserve_got_entry(struct ld *ld, struct ld_symbol *lsb);
static void _reserve_gotplt_entry(struct ld *ld, struct ld_symbol *lsb);
static void _reserve_plt_entry(struct ld *ld, struct ld_symbol *lsb);
static int _is_absolute_reloc(uint64_t r);
static void _warn_pic(struct ld *ld, struct ld_reloc_entry *lre);

static uint64_t
_get_max_page_size(struct ld *ld)
{

	(void) ld;
	return (0x200000);
}

static uint64_t
_get_common_page_size(struct ld *ld)
{

	(void) ld;
	return (0x1000);
}

static const char *
_reloc2str(uint64_t r)
{
	static char s[32];

	switch (r) {
		case 0: return "R_X86_64_NONE";
		case 1: return "R_X86_64_64";
		case 2: return "R_X86_64_PC32";
		case 3: return "R_X86_64_GOT32";
		case 4: return "R_X86_64_PLT32";
		case 5: return "R_X86_64_COPY";
		case 6: return "R_X86_64_GLOB_DAT";
		case 7: return "R_X86_64_JMP_SLOT";
		case 8: return "R_X86_64_RELATIVE";
		case 9: return "R_X86_64_GOTPCREL";
		case 10: return "R_X86_64_32";
		case 11: return "R_X86_64_32S";
		case 12: return "R_X86_64_16";
		case 13: return "R_X86_64_PC16";
		case 14: return "R_X86_64_8";
		case 15: return "R_X86_64_PC8";
		case 16: return "R_X86_64_DTPMOD64";
		case 17: return "R_X86_64_DTPOFF64";
		case 18: return "R_X86_64_TPOFF64";
		case 19: return "R_X86_64_TLSGD";
		case 20: return "R_X86_64_TLSLD";
		case 21: return "R_X86_64_DTPOFF32";
		case 22: return "R_X86_64_GOTTPOFF";
		case 23: return "R_X86_64_TPOFF32";
	default:
		snprintf(s, sizeof(s), "<unkown: %ju>", r);
		return (s);
	}
}

static int
_is_absolute_reloc(uint64_t r)
{

	if (r == R_X86_64_64 || r == R_X86_64_32 || r == R_X86_64_32S ||
	    r == R_X86_64_16 || r == R_X86_64_8)
		return (1);

	return (0);
}

static void
_warn_pic(struct ld *ld, struct ld_reloc_entry *lre)
{
	struct ld_symbol *lsb;

	lsb = lre->lre_sym;

	if (lsb->lsb_bind != STB_LOCAL)
		ld_warn(ld, "relocation %s against `%s' can not be used"
		    " by runtime linker; recompile with -fPIC",
		    _reloc2str(lre->lre_type), lsb->lsb_name);
	else
		ld_warn(ld, "relocation %s can not be used by runtime linker;"
		    " recompile with -fPIC", _reloc2str(lre->lre_type));
}

static struct ld_input_section *
_find_and_create_got_section(struct ld *ld, int create)
{
	struct ld_input_section *is;

	/* Check if the GOT section is already created. */
	is = ld_input_find_internal_section(ld, ".got");
	if (is != NULL)
		return (is);

	if (create) {
		is = ld_input_add_internal_section(ld, ".got");
		is->is_entsize = 8;
		is->is_align = 8;
		is->is_type = SHT_PROGBITS;
		is->is_flags = SHF_ALLOC | SHF_WRITE;
	}

	return (is);
}

static struct ld_input_section *
_find_and_create_gotplt_section(struct ld *ld, int create)
{
	struct ld_input_section *is;

	/* Check if the GOT (for PLT) section is already created. */
	is = ld_input_find_internal_section(ld, ".got.plt");
	if (is != NULL)
		return (is);

	if (create) {
		is = ld_input_add_internal_section(ld, ".got.plt");
		is->is_entsize = 8;
		is->is_align = 8;
		is->is_type = SHT_PROGBITS;
		is->is_flags = SHF_ALLOC | SHF_WRITE;
	
		/* Reserve space for the initial entries. */
		(void) ld_input_reserve_ibuf(is, 3);

		/* Create _GLOBAL_OFFSET_TABLE_ symbol. */
		ld_symbols_add_internal(ld, "_GLOBAL_OFFSET_TABLE_", 0, 0,
		    is->is_index, STB_LOCAL, STT_OBJECT, STV_HIDDEN,
		    STAILQ_FIRST(&ld->ld_lilist), NULL);
	}

	return (is);
}

static struct ld_input_section *
_find_and_create_plt_section(struct ld *ld, int create)
{
	struct ld_input_section *is;

	/* Check if the PLT section is already created. */
	is = ld_input_find_internal_section(ld, ".plt");
	if (is != NULL)
		return (is);

	if (create) {
		is = ld_input_add_internal_section(ld, ".plt");
		is->is_entsize = 16;
		is->is_align = 4;
		is->is_type = SHT_PROGBITS;
		is->is_flags = SHF_ALLOC | SHF_EXECINSTR;

		/* Reserve space for the initial entry. */
		(void) ld_input_reserve_ibuf(is, 1);
	}
	
	return (is);
}

static void
_reserve_got_entry(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_input_section *is;

	is = _find_and_create_got_section(ld, 1);

	/* Check if the entry already has a GOT entry. */
	if (lsb->lsb_got)
		return;

	/* Reserve a GOT entry. */
	lsb->lsb_got_off = ld_input_reserve_ibuf(is, 1);
	lsb->lsb_got = 1;

	/*
	 * If we are building a DSO, create a R_X86_64_GLOB_DAT entry
	 * for this symbol.
	 */
	if (ld->ld_dso)
		_create_got_reloc(ld, lsb, R_X86_64_GLOB_DAT,
		    lsb->lsb_got_off);
}

static void
_reserve_gotplt_entry(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_input_section *is;
	uint64_t off;

	is = _find_and_create_gotplt_section(ld, 1);

	/* Reserve a GOT entry for PLT. */
	off = ld_input_reserve_ibuf(is, 1);

	/*
	 * Record a R_X86_64_JUMP_SLOT entry for this symbol. Note that
	 * we don't need to record the offset (relative to the GOT section)
	 * here, since the PLT relocations will be sorted later and we
	 * will generate GOT section according to the new order.
	 */
	_create_plt_reloc(ld, lsb, 0);
}

static void
_reserve_plt_entry(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_input_section *is;

	is = _find_and_create_plt_section(ld, 1);

	lsb->lsb_plt_off = ld_input_reserve_ibuf(is, 1);
	lsb->lsb_plt = 1;
}

static void
_create_plt_reloc(struct ld *ld, struct ld_symbol *lsb, uint64_t offset)
{

	ld_reloc_create_entry(ld, ".rela.plt", NULL, R_X86_64_JUMP_SLOT,
	    lsb, offset, 0);
}

static void
_create_got_reloc(struct ld *ld, struct ld_symbol *lsb, uint64_t type,
    uint64_t offset)
{
	struct ld_input_section *tis;

	tis = _find_and_create_got_section(ld, 0);
	assert(tis != NULL);

	ld_reloc_create_entry(ld, ".rela.got", tis, type, lsb, offset, 0);
}

static void
_create_copy_reloc(struct ld *ld, struct ld_symbol *lsb)
{
	struct ld_input_section *tis;

	ld_dynamic_reserve_dynbss_entry(ld, lsb);

	tis = ld_input_find_internal_section(ld, ".dynbss");
	assert(tis != NULL);
	
	ld_reloc_create_entry(ld, ".rela.bss", tis, R_X86_64_COPY, lsb,
	    lsb->lsb_value, 0);
}

static void
_create_dynamic_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_symbol *lsb, uint64_t type, uint64_t offset, int64_t addend)
{

	if (lsb->lsb_bind == STB_LOCAL) {
		if (is->is_flags & SHF_WRITE)
			ld_reloc_create_entry(ld, ".rela.data.rel.local",
			    is, type, lsb, offset, addend);
		else
			ld_reloc_create_entry(ld, ".rela.data.rel.ro.local",
			    is, type, lsb, offset, addend);
	} else {
		if (is->is_flags & SHF_WRITE)
			ld_reloc_create_entry(ld, ".rela.data.rel",
			    is, type, lsb, offset, addend);
		else
			ld_reloc_create_entry(ld, ".rela.data.rel.ro",
			    is, type, lsb, offset, addend);
	}
}

static void
_finalize_reloc(struct ld *ld, struct ld_input_section *tis,
    struct ld_reloc_entry *lre)
{
	struct ld_symbol *lsb;

	(void) ld;
	(void) tis;

	lsb = ld_symbols_ref(lre->lre_sym);

	switch (lre->lre_type) {
	case R_X86_64_RELATIVE:
		/*
		 * Update the addend stored in the original relocation
		 * to point to the new location, by adding the updated
		 * symbol value.
		 */
		lre->lre_addend += lsb->lsb_value;
		break;

	default:
		break;
	}
}

static void
_finalize_got_and_plt(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_input_section *got_is, *plt_is, *rela_plt_is;
	struct ld_output_section *got_os, *plt_os, *rela_plt_os;
	struct ld_reloc_entry *lre;
	struct ld_symbol *lsb;
	char dynamic_symbol[] = "_DYNAMIC";
	uint8_t *got, *plt;
	uint64_t u64;
	int32_t s32, pltgot, gotpcrel;
	int i, j;

	lo = ld->ld_output;
	assert(lo != NULL);

	/*
	 * Intiailze all .got section entries to zero.
	 */
	got_is = _find_and_create_got_section(ld, 0);
	if (got_is != NULL)
		memset(got_is->is_ibuf, 0, got_is->is_size);

	/*
	 * Find the .plt section. The buffers should have been allocated
	 * at this point.
	 */
	plt_is = _find_and_create_plt_section(ld, 0);
	if (plt_is == NULL)
		return;
	plt_os = plt_is->is_output;
	plt = plt_is->is_ibuf;
	assert(plt != NULL);

	/*
	 * Find the .got.plt and .rela.plt section. If the .plt section
	 * exists, the .got.plt and .rela.plt section should exist too.
	 */
	got_is = _find_and_create_gotplt_section(ld, 0);
	assert(got_is != NULL);
	got_os = got_is->is_output;
	lo->lo_got = got_os;
	got = got_is->is_ibuf;
	assert(got != NULL);
	rela_plt_is = ld_input_find_internal_section(ld, ".rela.plt");
	assert(rela_plt_is != NULL);
	rela_plt_os = rela_plt_is->is_output;
	lo->lo_rel_plt = rela_plt_os;

	/* Point sh_info field of the .rela.plt to .plt section. */
	rela_plt_os->os_info = plt_os;

	/* Fill in the value of symbol _DYNAMIC in the first GOT entry. */
	ld_symbols_get_value(ld, dynamic_symbol, &u64);
	WRITE_64(got, u64);
	got += 8;

	/* Reserve the second and the third entry for the dynamic linker. */
	got += 16;

	/*
	 * Write the initial PLT entry.
	 */

	/* Calculate the relative offset from PLT to GOT. */
	pltgot = got_os->os_addr - plt_os->os_addr;

	/*
	 * Push the second GOT entry to the stack for the dynamic
	 * linker. (PUSH reg/memXX [RIP+disp32]) (6 bytes for push)
	 */
	WRITE_8(plt, 0xff);
	WRITE_8(plt + 1, 0x35);
	s32 = pltgot - 6 + 8;
	WRITE_32(plt + 2, s32);
	plt += 6;
	
	/*
	 * Jump to the address in the third GOT entry (call into
	 * the dynamic linker). (JMP reg/memXX [RIP+disp32])
	 * (6 bytes for jmp)
	 */
	WRITE_8(plt, 0xff);
	WRITE_8(plt + 1, 0x25);
	s32 = pltgot - 12 + 16;
	WRITE_32(plt + 2, s32);
	plt += 6;

	/* Padding: 4-byte nop. (NOP [rAx+disp8]) */
	WRITE_8(plt, 0x0f);
	WRITE_8(plt + 1, 0x1f);
	WRITE_8(plt + 2, 0x40);
	WRITE_8(plt + 3, 0x0);
	plt += 4;

	/*
	 * Walk through the sorted PLT relocations in the output section
	 * and fill in each GOT and PLT entries.
	 */
	i = 3;
	j = 0;
	STAILQ_FOREACH(lre, rela_plt_os->os_reloc, lre_next) {
		lsb = ld_symbols_ref(lre->lre_sym);

		/*
		 * Update the offset for the R_X86_64_JUMP_SLOT relocation
		 * entry, pointing to the corresponding GOT entry.
		 */
		lre->lre_offset = got_os->os_addr + i * 8;

		/*
		 * Set the value of the dynamic symbol to the address of the
		 * PLT slot.
		 */
		lsb->lsb_value = plt_os->os_addr + (i - 2) * 16;

		/*
		 * Calculate the IP-relative offset to the GOT entry for
		 * this function. (6 bytes for jmp)
		 */
		gotpcrel = pltgot + i * 8 - (i - 2) * 16 - 6;

		/*
		 * PLT: Jump to the address in the GOT entry for this
		 * function. (JMP reg/memXX [RIP+disp32])
		 */
		WRITE_8(plt, 0xff);
		WRITE_8(plt + 1, 0x25);
		WRITE_32(plt + 2, gotpcrel);
		plt += 6;

		/*
		 * PLT: Symbol is not resolved, push the relocation index to
		 * the stack. (PUSH imm32)
		 */
		WRITE_8(plt, 0x68);
		WRITE_32(plt + 1, j);
		plt += 5;

		/*
		 * PLT: Jump to the first PLT entry, eventually call the
		 * dynamic linker. (JMP rel32off)
		 */
		WRITE_8(plt, 0xe9);
		s32 = - (i - 1) * 16;
		WRITE_32(plt + 1, s32);
		plt += 5;

		/*
		 * GOT: Write the GOT entry for this function, pointing to
		 * the push op.
		 */
		u64 = plt_os->os_addr + (i - 2) * 16 + 6;
		WRITE_64(got, u64);

		/* Increase relocation entry index. */
		j++;

		/* Move to next GOT entry. */
		got += 8;
		i++;
	}

	assert(got == (uint8_t *) got_is->is_ibuf + got_is->is_size);
	assert(plt == (uint8_t *) plt_is->is_ibuf + plt_is->is_size);
}

static void
_scan_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_reloc_entry *lre)
{
	struct ld_symbol *lsb;

	lsb = ld_symbols_ref(lre->lre_sym);

	/*
	 * TODO: We do not yet support "Large Models" and relevant
	 * relocation types R_X86_64_GOT64, R_X86_64_GOTPCREL64,
	 * R_X86_64_GOTPC64, R_X86_64_GOTPLT64 and R_X86_64_PLTOFF64.
	 * Refer to AMD64 ELF ABI for details.
	 */

	switch (lre->lre_type) {
	case R_X86_64_NONE:
		break;

	case R_X86_64_64:
	case R_X86_64_32:
	case R_X86_64_32S:
	case R_X86_64_16:
	case R_X86_64_8:

		/*
		 * For a local symbol, if the linker output a PIE or DSO,
		 * we should generate a R_X86_64_RELATIVE reloc for
		 * R_X86_64_64. We don't know how to generate dynamic reloc
		 * for other reloc types since R_X86_64_RELATIVE is 64 bits.
		 * We can not use them directly either because FreeBSD rtld(1)
		 * (and probably glibc) doesn't accept absolute address
		 * reloction other than R_X86_64_64.
		 */
		if (lsb->lsb_bind == STB_LOCAL) {
			if (ld->ld_pie || ld->ld_dso) {
				if (lre->lre_type == R_X86_64_64)
					_create_dynamic_reloc(ld, is, lsb,
					    R_X86_64_RELATIVE, lre->lre_offset,
					    lre->lre_addend);
				else
					_warn_pic(ld, lre);
			}
			break;
		}

		/*
		 * For a global symbol, we probably need to generate PLT entry
		 * and/or a dynamic relocation.
		 *
		 * Note here, normally the compiler will generate a PC-relative
		 * relocation for function calls. However, if the code retrieve
		 * the address of a function and call it indirectly, an absolute
		 * relocation will be generated instead. That's why we should
		 * check if we need to create a PLT entry here.
		 */
		if (ld_reloc_require_plt(ld, lre) && !lsb->lsb_plt) {
			_reserve_gotplt_entry(ld, lsb);
			_reserve_plt_entry(ld, lsb);
		}

		if (ld_reloc_require_copy_reloc(ld, lre) &&
		    !lsb->lsb_copy_reloc)
			_create_copy_reloc(ld, lsb);
		else if (ld_reloc_require_dynamic_reloc(ld, lre)) {
			/* We only support R_X86_64_64. See above */
			if (lre->lre_type != R_X86_64_64) {
				_warn_pic(ld, lre);
				break;
			}
			/*
			 * Check if we can relax R_X86_64_64 to
			 * R_X86_64_RELATIVE instead.
			 */
			if (ld_reloc_relative_relax(ld, lre))
				_create_dynamic_reloc(ld, is, lsb,
				    R_X86_64_RELATIVE, lre->lre_offset,
				    lre->lre_addend);
			else
				_create_dynamic_reloc(ld, is, lsb,
				    R_X86_64_64, lre->lre_offset,
				    lre->lre_addend);
		}

		break;

	case R_X86_64_PLT32:
		/*
		 * In some cases we don't really need to generate a PLT
		 * entry, then a R_X86_64_PLT32 relocation can be relaxed
		 * to a R_X86_64_PC32 relocation.
		 */

		if (lsb->lsb_bind == STB_LOCAL) {
			/* Why use R_X86_64_PLT32 for a local symbol? */
			lre->lre_type = R_X86_64_PC32;
			break;
		}

		/*
		 * If linker outputs an normal executable and the symbol is
		 * defined but is not defined inside a DSO, we can generate
		 * a R_X86_64_PC32 relocation instead.
		 */
		if (ld->ld_exec && lsb->lsb_shndx != SHN_UNDEF &&
		    (lsb->lsb_input == NULL ||
		    lsb->lsb_input->li_type != LIT_DSO)) {
			lre->lre_type = R_X86_64_PC32;
			break;
		}

		/* Create an PLT entry otherwise. */
		if (!lsb->lsb_plt) {
			_reserve_gotplt_entry(ld, lsb);
			_reserve_plt_entry(ld, lsb);
		}
		break;

	case R_X86_64_PC64:
	case R_X86_64_PC32:
	case R_X86_64_PC16:
	case R_X86_64_PC8:

		/*
		 * When these relocations apply to a global symbol, we should
		 * check if we need to generate PLT entry and/or a dynamic
		 * relocation.
		 */
		if (lsb->lsb_bind != STB_LOCAL) {
			if (ld_reloc_require_plt(ld, lre) && !lsb->lsb_plt) {
				_reserve_gotplt_entry(ld, lsb);
				_reserve_plt_entry(ld, lsb);
			}

			if (ld_reloc_require_copy_reloc(ld, lre) &&
			    !lsb->lsb_copy_reloc)
				_create_copy_reloc(ld, lsb);
			else if (ld_reloc_require_dynamic_reloc(ld, lre)) {
				/*
				 * We can not generate dynamic relocation for
				 * these PC-relative relocation since they
				 * are probably not supported by the runtime
				 * linkers.
				 *
				 * Note: FreeBSD rtld(1) does support
				 * R_X86_64_PC32.
				 */
				_warn_pic(ld, lre);
			}
		}
		break;

	case R_X86_64_GOTOFF64:
	case R_X86_64_GOTPC32:
		/*
		 * These relocation types use GOT address as a base address
		 * and instruct the linker to build a GOT.
		 */
		(void) _find_and_create_got_section(ld, 1);
		break;

	case R_X86_64_GOT32:
	case R_X86_64_GOTPCREL:
		/*
		 * These relocation types instruct the linker to build a
		 * GOT and generate a GOT entry.
		 */
		if (!lsb->lsb_got)
			_reserve_got_entry(ld, lsb);
		break;

	case R_X86_64_TLSGD:
	case R_X86_64_TLSLD:
	case R_X86_64_DTPOFF32:
	case R_X86_64_DTPOFF64:
	case R_X86_64_GOTTPOFF:
	case R_X86_64_TPOFF32:
	case R_X86_64_GOTPC32_TLSDESC:
	case R_X86_64_TLSDESC_CALL:
		/* TODO: Handle TLS. */
		break;
	default:
		ld_warn(ld, "can not handle relocation %ju",
		    lre->lre_type);
		break;
	}
}

static void
_process_reloc(struct ld *ld, struct ld_input_section *is,
    struct ld_reloc_entry *lre, struct ld_symbol *lsb, uint8_t *buf)
{
	struct ld_output *lo;
	uint64_t u64, s;
	int64_t s64;
	uint32_t u32;
	int32_t s32;
	uint64_t p;

	lo = ld->ld_output;
	assert(lo != NULL);

	s = lsb->lsb_value;
	p = lre->lre_offset + is->is_output->os_addr + is->is_reloff;

	switch (lre->lre_type) {
	case R_X86_64_NONE:
		break;
	case R_X86_64_64:
		WRITE_64(buf + lre->lre_offset, s + lre->lre_addend);
		break;
	case R_X86_64_PC32:
		s32 = s + lre->lre_addend - p;
		WRITE_32(buf + lre->lre_offset, s32);
		break;
	case R_X86_64_PLT32:
		/* Symbol value has been set to the PLT offset. */
		s32 = s + lre->lre_addend - p;
		WRITE_32(buf + lre->lre_offset, s32);
		break;
	case R_X86_64_GOTPCREL:
		s32 = lsb->lsb_got_off + lre->lre_addend - p;
		break;
	case R_X86_64_32:
		u64 = s + lre->lre_addend;
		u32 = u64 & 0xffffffff;
		if (u64 != u32)
			ld_fatal(ld, "R_X86_64_32 relocation failed");
		WRITE_32(buf + lre->lre_offset, u32);
		break;
	case R_X86_64_32S:
		s64 = s + lre->lre_addend;
		s32 = s64 & 0xffffffff;
		if (s64 != s32)
			ld_fatal(ld, "R_X86_64_32S relocation failed");
		WRITE_32(buf + lre->lre_offset, s32);
		break;
	default:
		ld_warn(ld, "Relocation %s not supported",
		    _reloc2str(lre->lre_type));
		break;
	}
}

void
amd64_register(struct ld *ld)
{
	struct ld_arch *amd64, *amd64_alt;

	if ((amd64 = calloc(1, sizeof(*amd64))) == NULL)
		ld_fatal_std(ld, "calloc");

	snprintf(amd64->name, sizeof(amd64->name), "%s", "amd64");

	amd64->script = amd64_script;
	amd64->interp = "/libexec/ld-elf.so.1";
	amd64->get_max_page_size = _get_max_page_size;
	amd64->get_common_page_size = _get_common_page_size;
	amd64->scan_reloc = _scan_reloc;
	amd64->process_reloc = _process_reloc;
	amd64->is_absolute_reloc = _is_absolute_reloc;
	amd64->finalize_reloc = _finalize_reloc;
	amd64->finalize_got_and_plt = _finalize_got_and_plt;
	amd64->reloc_is_64bit = 1;
	amd64->reloc_is_rela = 1;
	amd64->reloc_entsize = sizeof(Elf64_Rela);

	HASH_ADD_STR(ld->ld_arch_list, name, amd64);

	if ((amd64_alt = calloc(1, sizeof(*amd64_alt))) == NULL)
		ld_fatal_std(ld, "calloc");
	memcpy(amd64_alt, amd64, sizeof(struct ld_arch));
	amd64_alt->alias = amd64;
	snprintf(amd64_alt->name, sizeof(amd64_alt->name), "%s", "x86-64");

	HASH_ADD_STR(ld->ld_arch_list, name, amd64_alt);
}
