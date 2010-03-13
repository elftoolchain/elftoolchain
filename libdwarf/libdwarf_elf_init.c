/*-
 * Copyright (c) 2009 Kai Wang
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

static const char *debug_name[] = {
	".debug_abbrev",
	".debug_aranges",
	".debug_frame",
	".debug_info",
	".debug_line",
	".debug_pubnames",
	".eh_frame",
	".debug_macinfo",
	".debug_str",
	".debug_loc",
	".debug_pubtypes",
	".debug_ranges",
	".debug_static_func",
	".debug_static_vars",
	".debug_types",
	".debug_weaknames",
	NULL
};

static void
_dwarf_elf_apply_reloc(Elf_Data *d, Elf_Data *rel_data, Elf_Data *symtab_data,
    int pointer_size, int endian)
{
	GElf_Rela rela;
	GElf_Sym sym;
	size_t symndx;
	uint64_t offset;
	int j;

	j = 0;
	while (gelf_getrela(rel_data, j++, &rela) != NULL) {
		symndx = GELF_R_SYM(rela.r_info);

		if (gelf_getsym(symtab_data, symndx, &sym) == NULL)
			continue;

		offset = rela.r_offset;

		if (endian == ELFDATA2MSB)
			_dwarf_write_msb(d->d_buf, &offset, rela.r_addend,
			    pointer_size);
		else
			_dwarf_write_lsb(d->d_buf, &offset, rela.r_addend,
			    pointer_size);
	}
}

static int
_dwarf_elf_relocate(Elf *elf, Elf_Data *d, size_t shndx, size_t symtab,
    Elf_Data *symtab_data, Dwarf_Error *error)
{
	GElf_Ehdr eh;
	GElf_Shdr sh;
	Elf_Scn *scn;
	Elf_Data *rel;
	int elferr, pointer_size;

	if (symtab == 0 || symtab_data == NULL)
		return (DWARF_E_NONE);

	if (gelf_getclass(elf) == ELFCLASS32)
		pointer_size = 4;
	else
		pointer_size = 8;

	if (gelf_getehdr(elf, &eh) == NULL) {
		DWARF_SET_ELF_ERROR(error);
		return (DWARF_E_ELF);
	}

	scn = NULL;
	(void) elf_errno();
	while ((scn = elf_nextscn(elf, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) == NULL) {
			DWARF_SET_ELF_ERROR(error);
			return (DWARF_E_ELF);
		}

		if (sh.sh_type != SHT_RELA || sh.sh_size == 0)
			continue;

		if (sh.sh_info == shndx && sh.sh_link == symtab) {
			if ((rel = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0) {
					_DWARF_SET_ERROR(error, DWARF_E_ELF,
					    elferr);
					return (DWARF_E_ELF);
				} else
					return (DWARF_E_NONE);
			}

			_dwarf_elf_apply_reloc(d, rel, symtab_data,
			    pointer_size, eh.e_ident[EI_DATA]);

			return (DWARF_E_NONE);
		}
	}
	elferr = elf_errno();
	if (elferr != 0) {
		DWARF_SET_ELF_ERROR(error);
		return (DWARF_E_ELF);
	}

	return (DWARF_E_NONE);
}

int
_dwarf_elf_init(Dwarf_Debug dbg, Elf *elf, Dwarf_Error *error)
{
	Dwarf_Obj_Access_Interface *iface;
	Dwarf_Elf_Object *e;
	const char *name;
	GElf_Shdr sh;
	Elf_Scn *scn;
	Elf_Data *symtab_data;
	size_t symtab_ndx;
	int elferr, i, j, n, ret;

	ret = DWARF_E_NONE;

	if ((iface = calloc(1, sizeof(*iface))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	if ((e = calloc(1, sizeof(*e))) == NULL) {
		free(iface);
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	e->eo_elf = elf;
	e->eo_methods.get_section_info = _dwarf_elf_get_section_info;
	e->eo_methods.get_byte_order = _dwarf_elf_get_byte_order;
	e->eo_methods.get_length_size = _dwarf_elf_get_length_size;
	e->eo_methods.get_pointer_size = _dwarf_elf_get_pointer_size;
	e->eo_methods.get_section_count = _dwarf_elf_get_section_count;
	e->eo_methods.load_section = _dwarf_elf_load_section;

	iface->object = e;
	iface->methods = &e->eo_methods;

	dbg->dbg_iface = iface;

	if (gelf_getehdr(elf, &e->eo_ehdr) == NULL) {
		DWARF_SET_ELF_ERROR(error);
		ret = DWARF_E_ELF;
		goto fail_cleanup;
	}

	if (!elf_getshstrndx(elf, &e->eo_strndx)) {
		DWARF_SET_ELF_ERROR(error);
		ret = DWARF_E_ELF;
		goto fail_cleanup;
	}

	n = 0;
	symtab_ndx = 0;
	symtab_data = NULL;
	scn = NULL;
	(void) elf_errno();
	while ((scn = elf_nextscn(elf, scn)) != NULL) {
		if (gelf_getshdr(scn, &sh) == NULL) {
			DWARF_SET_ELF_ERROR(error);
			ret = DWARF_E_ELF;
			goto fail_cleanup;
		}

		if ((name = elf_strptr(elf, e->eo_strndx, sh.sh_name)) ==
		    NULL) {
			DWARF_SET_ELF_ERROR(error);
			ret = DWARF_E_ELF;
			goto fail_cleanup;
		}

		if (!strcmp(name, ".symtab")) {
			symtab_ndx = elf_ndxscn(scn);
			if ((symtab_data = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0) {
					_DWARF_SET_ERROR(error, DWARF_E_ELF,
					    elferr);
					ret = DWARF_E_ELF;
					goto fail_cleanup;
				}
			}
			continue;
		}

		for (i = 0; debug_name[i] != NULL; i++) {
			if (!strcmp(name, debug_name[i]))
				n++;
		}
	}
	elferr = elf_errno();
	if (elferr != 0) {
		DWARF_SET_ELF_ERROR(error);
		return (DWARF_E_ELF);
	}

	e->eo_seccnt = n;

	if ((e->eo_data = calloc(n, sizeof(Elf_Data *))) == NULL ||
	    (e->eo_shdr = calloc(n, sizeof(GElf_Shdr))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		ret = DWARF_E_MEMORY;
		goto fail_cleanup;
	}

	scn = NULL;
	j = 0;
	while ((scn = elf_nextscn(elf, scn)) != NULL && j < n) {
		if (gelf_getshdr(scn, &sh) == NULL) {
			DWARF_SET_ELF_ERROR(error);
			ret = DWARF_E_ELF;
			goto fail_cleanup;
		}

		memcpy(&e->eo_shdr[j], &sh, sizeof(sh));

		if ((name = elf_strptr(elf, e->eo_strndx, sh.sh_name)) ==
		    NULL) {
			DWARF_SET_ELF_ERROR(error);
			ret = DWARF_E_ELF;
			goto fail_cleanup;
		}

		for (i = 0; debug_name[i] != NULL; i++) {
			if (strcmp(name, debug_name[i]))
				continue;

			(void) elf_errno();
			if ((e->eo_data[j] = elf_getdata(scn, NULL)) == NULL) {
				elferr = elf_errno();
				if (elferr != 0) {
					_DWARF_SET_ERROR(error, DWARF_E_ELF,
					    elferr);
					ret = DWARF_E_ELF;
					goto fail_cleanup;
				}
			}

			if (_dwarf_elf_relocate(elf,  e->eo_data[j],
			    elf_ndxscn(scn), symtab_ndx, symtab_data, error) !=
			    DWARF_E_NONE)
				goto fail_cleanup;

			j++;
		}
	}

	assert(j == n);

	return (DWARF_E_NONE);

fail_cleanup:

	_dwarf_elf_deinit(dbg);

	return (ret);
}

void
_dwarf_elf_deinit(Dwarf_Debug dbg)
{
	Dwarf_Obj_Access_Interface *iface;
	Dwarf_Elf_Object *e;

	iface = dbg->dbg_iface;
	assert(iface != NULL);

	e = iface->object;
	assert(e != NULL);

	if (e->eo_data)
		free(e->eo_data);
	if (e->eo_shdr)
		free(e->eo_shdr);

	free(e);
	free(iface);

	dbg->dbg_iface = NULL;
}
