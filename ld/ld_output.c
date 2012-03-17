/*-
 * Copyright (c) 2011,2012 Kai Wang
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
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
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
#include "ld_input.h"
#include "ld_output.h"
#include "ld_layout.h"
#include "ld_reloc.h"
#include "ld_script.h"
#include "ld_strtab.h"
#include "ld_symbols.h"

ELFTC_VCSID("$Id$");

static void _add_input_section_data(struct ld *ld, Elf_Scn *scn,
    struct ld_input_section *is);
static void _add_to_shstrtab(struct ld *ld, const char *name);
static Elf_Scn *_create_elf_scn(struct ld *ld, struct ld_output *lo,
    struct ld_output_section *os);
static void _create_elf_section(struct ld *ld, struct ld_output_section *os);
static void _create_elf_sections(struct ld *ld);
static void _create_phdr(struct ld *ld);
static void _create_shstrtab(struct ld *ld);
static uint64_t _find_entry_point(struct ld *ld);
static uint64_t _insert_shdr(struct ld *ld);

void
ld_output_init(struct ld *ld)
{
	struct ld_output *lo;

	if ((lo = calloc(1, sizeof(*lo))) == NULL)
		ld_fatal_std(ld, "calloc");

	STAILQ_INIT(&lo->lo_oelist);
	STAILQ_INIT(&lo->lo_oslist);
	ld->ld_output = lo;
}

void
ld_output_determine_arch(struct ld *ld)
{
	char *end, target[MAX_TARGET_NAME_LEN + 1];
	size_t len;

	if (ld->ld_otgt != NULL) {
		ld->ld_arch = ld_arch_get_arch_from_target(ld,
		    ld->ld_otgt_name);
		if (ld->ld_arch == NULL)
			ld_fatal(ld, "target %s is not supported",
			    ld->ld_otgt_name);
	} else {
		if ((end = strrchr(ld->ld_progname, '-')) != NULL &&
		    end != ld->ld_progname) {
			len = end - ld->ld_progname + 1;
			if (len > MAX_TARGET_NAME_LEN)
				return;
			strncpy(target, ld->ld_progname, len);
			target[len] = '\0';
			ld->ld_arch = ld_arch_get_arch_from_target(ld, target);
		}
	}
}

void
ld_output_verify_arch(struct ld *ld, struct ld_input *li)
{

	/*
	 * TODO: Guess arch if the output arch is not yet determined.
	 * Otherwise, verify the arch of the input object match the arch
	 * of the output file.
	 */

	(void) ld;
	(void) li;
}

void
ld_output_format(struct ld *ld, char *def, char *be, char *le)
{

	ld->ld_otgt_name = def;
	if ((ld->ld_otgt = elftc_bfd_find_target(def)) == NULL)
		ld_fatal(ld, "invalid BFD format %s", def);

	ld->ld_otgt_be_name = be;
	if ((ld->ld_otgt_be = elftc_bfd_find_target(be)) == NULL)
		ld_fatal(ld, "invalid BFD format %s", be);

	ld->ld_otgt_le_name = le;
	if ((ld->ld_otgt_le = elftc_bfd_find_target(le)) == NULL)
		ld_fatal(ld, "invalid BFD format %s", le);
}

struct ld_output_element *
ld_output_create_element(struct ld *ld, struct ld_output_element_head *head,
    enum ld_output_element_type type, void *entry,
    struct ld_output_element *after)
{
	struct ld_output_element *oe;

	if ((oe = calloc(1, sizeof(*oe))) == NULL)
		ld_fatal_std(ld, "calloc");

	oe->oe_type = type;
	oe->oe_entry = entry;

	if (after == NULL)
		STAILQ_INSERT_TAIL(head, oe, oe_next);
	else
		STAILQ_INSERT_AFTER(head, after, oe, oe_next);

	return (oe);
}

struct ld_output_section *
ld_output_alloc_section(struct ld *ld, const char *name,
    struct ld_output_section *after)
{
	struct ld_output *lo;
	struct ld_output_section *os;

	lo = ld->ld_output;

	if ((os = calloc(1, sizeof(*os))) == NULL)
		ld_fatal_std(ld, "calloc");

	if ((os->os_name = strdup(name)) == NULL)
		ld_fatal_std(ld, "strdup");

	os->os_align = 1;
	os->os_empty = 1;

	STAILQ_INIT(&os->os_e);

	HASH_ADD_KEYPTR(hh, lo->lo_ostbl, os->os_name, strlen(os->os_name), os);

	if (after == NULL) {
		STAILQ_INSERT_TAIL(&lo->lo_oslist, os, os_next);
		os->os_pe = ld_output_create_element(ld, &lo->lo_oelist,
		    OET_OUTPUT_SECTION, os, NULL);
	} else {
		STAILQ_INSERT_AFTER(&lo->lo_oslist, after, os, os_next);
		os->os_pe = ld_output_create_element(ld, &lo->lo_oelist,
		    OET_OUTPUT_SECTION, os, after->os_pe);
	}

	return (os);
}

static Elf_Scn *
_create_elf_scn(struct ld *ld, struct ld_output *lo,
    struct ld_output_section *os)
{
	Elf_Scn *scn;

	assert(lo->lo_elf != NULL);

	if ((scn = elf_newscn(lo->lo_elf)) == NULL)
		ld_fatal(ld, "elf_newscn failed: %s", elf_errmsg(-1));

	if (os != NULL)
		os->os_scn = scn;

	return (scn);
}

static void
_create_elf_section(struct ld *ld, struct ld_output_section *os)
{
	struct ld_output *lo;
	struct ld_output_element *oe;
	struct ld_input_section *is;
	struct ld_input_section_head *islist;
	Elf_Data *d;
	Elf_Scn *scn;
	GElf_Shdr sh;

	lo = ld->ld_output;
	assert(lo->lo_elf != NULL);

	/* Create section data. */
	scn = NULL;
	STAILQ_FOREACH(oe, &os->os_e, oe_next) {
		switch (oe->oe_type) {
		case OET_DATA:
			if (scn == NULL)
				scn = _create_elf_scn(ld, lo, os);
			/* TODO */
			break;
		case OET_INPUT_SECTION_LIST:
			islist = oe->oe_entry;
			STAILQ_FOREACH(is, islist, is_next) {
				if (scn == NULL)
					scn = _create_elf_scn(ld, lo, os);
				if (os->os_type != SHT_NOBITS)
					_add_input_section_data(ld, scn, is);
			}
			break;
		case OET_KEYWORD:
			if (scn == NULL)
				scn = _create_elf_scn(ld, lo, os);
			/* TODO */
			break;
		default:
			break;
		}
	}

	if (scn == NULL)
		return;

	if (os->os_type == SHT_NOBITS) {
		if ((d = elf_newdata(scn)) == NULL)
			ld_fatal(ld, "elf_newdata failed: %s", elf_errmsg(-1));

		d->d_align = os->os_align;
		d->d_off = 0;
		d->d_type = ELF_T_BYTE;
		d->d_size = os->os_size;
		d->d_version = EV_CURRENT;
		d->d_buf = NULL;
	}

	if (gelf_getshdr(scn, &sh) == NULL)
		ld_fatal(ld, "gelf_getshdr failed: %s", elf_errmsg(-1));

	sh.sh_flags = os->os_flags;
	sh.sh_addr = os->os_addr;
	sh.sh_addralign = os->os_align;
	sh.sh_offset = os->os_off;
	sh.sh_size = os->os_size;
	sh.sh_type = os->os_type;

	_add_to_shstrtab(ld, os->os_name);

	if (!gelf_update_shdr(scn, &sh))
		ld_fatal(ld, "gelf_update_shdr failed: %s", elf_errmsg(-1));
}

static void
_add_input_section_data(struct ld *ld, Elf_Scn *scn,
    struct ld_input_section *is)
{
	Elf_Data *d;

	if (is->is_type == SHT_NOBITS || is->is_size == 0)
		return;

	if ((d = elf_newdata(scn)) == NULL)
		ld_fatal(ld, "elf_newdata failed: %s", elf_errmsg(-1));

	d->d_align = is->is_align;
	d->d_off = is->is_reloff;
	d->d_type = ELF_T_BYTE;
	d->d_size = is->is_size;
	d->d_version = EV_CURRENT;
	d->d_buf = ld_input_get_section_rawdata(ld, is);
	ld_reloc_process_input_section(ld, is, d->d_buf);
}


static void
_create_elf_sections(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_output_element *oe;

	lo = ld->ld_output;
	assert(lo->lo_elf != NULL);

	STAILQ_FOREACH(oe, &lo->lo_oelist, oe_next) {
		switch (oe->oe_type) {
		case OET_OUTPUT_SECTION:
			_create_elf_section(ld, oe->oe_entry);
			break;
		default:
			break;
		}
	}
}

static uint64_t
_find_entry_point(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_output_section *os;
	char entry_symbol[] = "start";
	uint64_t entry;

	lo = ld->ld_output;

	if (ld->ld_entry != NULL) {
		if (ld_symbols_get_value(ld, ld->ld_entry, &entry) < 0)
			ld_fatal(ld, "symbol %s is undefined", ld->ld_entry);
		return (entry);
	} 

	if (ld->ld_scp->lds_entry_set)
		return (ld->ld_scp->lds_entry_point);

	if (ld_symbols_get_value(ld, entry_symbol, &entry) == 0)
		return (entry);

	STAILQ_FOREACH(os, &lo->lo_oslist, os_next) {
		if (os->os_empty)
			continue;
		if (strcmp(os->os_name, ".text") == 0)
			return (os->os_addr);
	}

	return (0);
}

static void
_create_phdr(struct ld *ld)
{
	struct ld_output *lo;
	struct ld_output_section *os;
	Elf32_Phdr *p32;
	Elf64_Phdr *p64;
	void *phdrs;
	uint64_t addr, off, align, flags, filesz, memsz;
	unsigned w;
	int i, new;

	/* TODO: support segments created by linker script command PHDR. */

#define	_WRITE_PHDR(T,O,A,FSZ,MSZ,FL,AL)		\
	do {						\
		if (lo->lo_ec == ELFCLASS32) {		\
			p32[i].p_type = (T);		\
			p32[i].p_offset = (O);		\
			p32[i].p_vaddr = (A);		\
			p32[i].p_paddr = (A);		\
			p32[i].p_filesz = (FSZ);	\
			p32[i].p_memsz = (MSZ);		\
			p32[i].p_flags = (FL);		\
			p32[i].p_align = (AL);		\
		} else {				\
			p64[i].p_type = (T);		\
			p64[i].p_offset = (O);		\
			p64[i].p_vaddr = (A);		\
			p64[i].p_paddr = (A);		\
			p64[i].p_filesz = (FSZ);	\
			p64[i].p_memsz = (MSZ);		\
			p64[i].p_flags = (FL);		\
			p64[i].p_align = (AL);		\
		}					\
	} while(0)

	lo = ld->ld_output;
	assert(lo->lo_elf != NULL);
	assert(lo->lo_phdr_num != 0);
	assert(ld->ld_arch != NULL);

	if ((phdrs = gelf_newphdr(lo->lo_elf, lo->lo_phdr_num)) == NULL)
		ld_fatal(ld, "gelf_newphdr failed: %s", elf_errmsg(-1));

	p32 = NULL;
	p64 = NULL;
	if (lo->lo_ec == ELFCLASS32)
		p32 = phdrs;
	else
		p64 = phdrs;

	/* Create PT_LOAD segments. */

	align = ld->ld_arch->get_max_page_size(ld);
	new = 1;
	i = -1;
	w = 0;
	filesz = 0;
	memsz = 0;
	flags = PF_R;
	off = 0;

	/* Calculate the start vma of output object. */
	os = STAILQ_FIRST(&lo->lo_oslist);
	addr = os->os_addr - os->os_off;

	STAILQ_FOREACH(os, &lo->lo_oslist, os_next) {
		if (os->os_empty)
			continue;

		if ((os->os_flags & SHF_ALLOC) == 0) {
			new = 1;
			continue;
		}

		if ((os->os_flags & SHF_WRITE) != w || new) {
			new = 0;
			w = os->os_flags & SHF_WRITE;

			if (i >= 0)
				_WRITE_PHDR(PT_LOAD, off, addr, filesz, memsz,
				    flags, align);

			i++;
			if ((unsigned) i >= lo->lo_phdr_num)
				ld_fatal(ld, "not enough room for program"
				    " headers");
			if (i > 0) {
				addr = os->os_addr;
				off = os->os_off;
			}
			flags = PF_R;
			filesz = 0;
			memsz = 0;
		}

		memsz = os->os_addr + os->os_size - addr;
		if (os->os_type != SHT_NOBITS)
			filesz = memsz;

		if (os->os_flags & SHF_WRITE)
			flags |= PF_W;

		if (os->os_flags & SHF_EXECINSTR)
			flags |= PF_X;
	}
	if (i >= 0)
		_WRITE_PHDR(PT_LOAD, off, addr, filesz, memsz, flags, align);

	/* Create PT_NOTE segment. */

	STAILQ_FOREACH(os, &lo->lo_oslist, os_next) {
		if (os->os_type == SHT_NOTE) {
			i++;
			if ((unsigned) i >= lo->lo_phdr_num)
				ld_fatal(ld, "not enough room for program"
				    " headers");
			_WRITE_PHDR(PT_NOTE, os->os_off, os->os_addr,
			    os->os_size, os->os_size, PF_R, os->os_align);
			break;
		}
	}

	assert((unsigned) i + 1 == lo->lo_phdr_num);

#undef	_WRITE_PHDR
}

void
ld_output_create(struct ld *ld)
{
	struct ld_output *lo;
	const char *fn;
	GElf_Ehdr eh;

	if (ld->ld_output_file == NULL)
		fn = "a.out";
	else
		fn = ld->ld_output_file;

	lo = ld->ld_output;

	if ((lo->lo_fd = open(fn, O_WRONLY)) < 0)
		ld_fatal_std(ld, "can not create output file: open %s", fn);

	if ((lo->lo_elf = elf_begin(lo->lo_fd, ELF_C_WRITE, NULL)) == NULL)
		ld_fatal(ld, "elf_begin failed: %s", elf_errmsg(-1));

	elf_flagelf(lo->lo_elf, ELF_C_SET, ELF_F_LAYOUT);

	assert(ld->ld_otgt != NULL);
	lo->lo_ec = elftc_bfd_target_class(ld->ld_otgt);
	lo->lo_endian = elftc_bfd_target_byteorder(ld->ld_otgt);

	if (gelf_newehdr(lo->lo_elf, lo->lo_ec) == NULL)
		ld_fatal(ld, "gelf_newehdr failed: %s", elf_errmsg(-1));

	if (gelf_getehdr(lo->lo_elf, &eh) == NULL)
		ld_fatal(ld, "gelf_getehdr failed: %s", elf_errmsg(-1));

	eh.e_ident[EI_CLASS] = lo->lo_ec;
	eh.e_ident[EI_DATA] = lo->lo_endian;
	eh.e_entry = _find_entry_point(ld);
	eh.e_flags = 0;		/* TODO */
	eh.e_machine = elftc_bfd_target_machine(ld->ld_otgt);
	eh.e_type = ET_EXEC;	/* TODO */
	eh.e_version = EV_CURRENT;

	/* Create program headers. */
	_create_phdr(ld);

	/* Set program header table offset. */
	eh.e_phoff = gelf_fsize(lo->lo_elf, ELF_T_EHDR, 1, EV_CURRENT);
	if (eh.e_phoff == 0)
		ld_fatal(ld, "gelf_fsize failed: %s", elf_errmsg(-1));

	/* Read relocation information from input sections. */
	ld_reloc_read(ld);

	/* Create output ELF sections. */
	_create_elf_sections(ld);

	/* Insert section headers table and point e_shoff to it. */
	eh.e_shoff = _insert_shdr(ld);

	/* Save updated ELF header. */
	if (gelf_update_ehdr(lo->lo_elf, &eh) == 0)
		ld_fatal(ld, "gelf_update_ehdr failed: %s", elf_errmsg(-1));

	/* Generate section name string table section (.shstrtab). */
	_create_shstrtab(ld);

	/* TODO: Add symbol table. */

	/* Finally write out output ELF object. */
	if (elf_update(lo->lo_elf, ELF_C_WRITE) < 0)
		ld_fatal(ld, "elf_update failed: %s", elf_errmsg(-1));
}

static uint64_t
_insert_shdr(struct ld *ld)
{
	struct ld_state *ls;
	struct ld_output *lo;
	struct ld_output_section *os;
	uint64_t shoff;
	int n;

	ls = &ld->ld_state;
	lo = ld->ld_output;

	if (lo->lo_ec == ELFCLASS32)
		shoff = roundup(ls->ls_offset, 4);
	else
		shoff = roundup(ls->ls_offset, 8);

	ls->ls_offset = shoff;

	n = 0;
	STAILQ_FOREACH(os, &lo->lo_oslist, os_next) {
		if (os->os_scn != NULL)
			n++;
	}

	printf("n=%d\n", n);

	ls->ls_offset += gelf_fsize(lo->lo_elf, ELF_T_SHDR, n + 2, EV_CURRENT);

	return (shoff);
}

static void
_add_to_shstrtab(struct ld *ld, const char *name)
{

	if (ld->ld_shstrtab == NULL) {
		ld->ld_shstrtab = ld_strtab_alloc(ld);
		ld_strtab_insert(ld, ld->ld_shstrtab, "");
		ld_strtab_insert(ld, ld->ld_shstrtab, ".symtab");
		ld_strtab_insert(ld, ld->ld_shstrtab, ".strtab");
		ld_strtab_insert(ld, ld->ld_shstrtab, ".shstrtab");
	}

	ld_strtab_insert(ld, ld->ld_shstrtab, name);
}

static void
_create_shstrtab(struct ld *ld)
{
	struct ld_state *ls;
	struct ld_strtab *st;
	struct ld_output *lo;
	struct ld_output_section *os;
	Elf_Scn *scn;
	Elf_Data *d;
	GElf_Shdr sh;

	ls = &ld->ld_state;
	lo = ld->ld_output;
	st = ld->ld_shstrtab;
	assert(st != NULL && st->st_buf != NULL);

	scn = _create_elf_scn(ld, lo, NULL);

	if (!elf_setshstrndx(lo->lo_elf, elf_ndxscn(scn)))
		ld_fatal(ld, "elf_setshstrndx failed: %s", elf_errmsg(-1));

	if (gelf_getshdr(scn, &sh) == NULL)
		ld_fatal(ld, "gelf_getshdr failed: %s", elf_errmsg(-1));

	sh.sh_name = ld_strtab_lookup(st, ".shstrtab");
	sh.sh_flags = 0;
	sh.sh_addr = 0;
	sh.sh_addralign = 1;
	sh.sh_offset = ls->ls_offset;
	sh.sh_size = st->st_size;
	sh.sh_type = SHT_STRTAB;

	if (!gelf_update_shdr(scn, &sh))
		ld_fatal(ld, "gelf_update_shdr failed: %s", elf_errmsg(-1));

	if ((d = elf_newdata(scn)) == NULL)
		ld_fatal(ld, "elf_newdata failed: %s", elf_errmsg(-1));

	d->d_align = 1;
	d->d_off = 0;
	d->d_type = ELF_T_BYTE;
	d->d_size = st->st_size;
	d->d_version = EV_CURRENT;
	d->d_buf = st->st_buf;

	ls->ls_offset += st->st_size;

	/*
	 * Set "sh_name" fields of each section headers to point
	 * to the string table.
	 */

	STAILQ_FOREACH(os, &lo->lo_oslist, os_next) {
		if (os->os_scn == NULL)
			continue;

		if (gelf_getshdr(os->os_scn, &sh) == NULL)
			ld_fatal(ld, "gelf_getshdr failed: %s",
			    elf_errmsg(-1));

		printf("name=%s, offset=%#jx, size=%#jx, type=%#jx\n",
		    os->os_name, (uint64_t) sh.sh_offset,
		    (uint64_t) sh.sh_size, (uint64_t) sh.sh_type);

		sh.sh_name = ld_strtab_lookup(st, os->os_name);

		if (!gelf_update_shdr(os->os_scn, &sh))
			ld_fatal(ld, "gelf_update_shdr failed: %s",
			    elf_errmsg(-1));
	}
}
