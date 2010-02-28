/*-
 * Copyright (c) 2010 Kai Wang
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

Dwarf_Unsigned
_dwarf_get_reloc_type(Dwarf_P_Debug dbg, int is64)
{

	assert(dbg != NULL);

	switch (dbg->dbgp_isa) {
	case DW_DLC_ISA_X86:
		return (R_386_32);
	case DW_DLC_ISA_X86_64:
		return (is64 ? R_X86_64_64 : R_X86_64_32);
	case DW_DLC_ISA_SPARC:
		return (is64 ? R_SPARC_UA64 : R_SPARC_UA32);
	case DW_DLC_ISA_PPC:
		/* TODO */
		return (0);
	case DW_DLC_ISA_ARM:
		/* TODO */
		return (0);
	case DW_DLC_ISA_MIPS:
		/* XXX Should be return (is64 ? R_MIPS_64 : R_MIPS_32); */
		return (R_MIPS_32);
	case DW_DLC_ISA_IA64:
		return (is64 ? R_IA_64_DIR64LSB : R_IA_64_DIR32LSB);
	default:
		assert(0);
	}

	return (0);		/* NOT REACHED */
}

int
_dwarf_reloc_section_init(Dwarf_P_Debug dbg, Dwarf_Rel_Section *drsp,
    Dwarf_P_Section ref, Dwarf_Error *error)
{
	Dwarf_Rel_Section drs;
	char name[128];

	assert(dbg != NULL && drsp != NULL && ref != NULL);

	if ((drs = calloc(1, sizeof(struct _Dwarf_Rel_Section))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	drs->drs_ref = ref;

	/*
	 * Generate the actual reloc ELF section if we are under stream
	 * mode.
	 */
	if ((dbg->dbgp_flags & DW_DLC_SYMBOLIC_RELOCATIONS) == 0) {
		/*
		 * FIXME The logic here is most likely wrong. It should
		 * be the ISA that determines relocation type.
		 */
		if (dbg->dbgp_flags & DW_DLC_SIZE_64)
			drs->drs_addend = 1;
		else
			drs->drs_addend = 0;
		
		snprintf(name, sizeof(name), "%s%s",
		    drs->drs_addend ? ".rela" : ".rel", ref->ds_name);
		if (_dwarf_section_init(dbg, &drs->drs_ds, name, error) !=
		    DWARF_E_NONE) {
			free(drs);
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			return (DWARF_E_MEMORY);
		}
	}

	STAILQ_INIT(&drs->drs_dre);
	STAILQ_INSERT_TAIL(&dbg->dbgp_drslist, drs, drs_next);
	dbg->dbgp_drscnt++;
	*drsp = drs;

	return (DWARF_E_NONE);
}

void
_dwarf_reloc_section_free(Dwarf_P_Debug dbg, Dwarf_Rel_Section *drsp)
{
	Dwarf_Rel_Section drs, tdrs;
	Dwarf_Rel_Entry dre, tdre;

	assert(dbg != NULL && drsp != NULL);

	if (*drsp == NULL)
		return;

	STAILQ_FOREACH_SAFE(drs, &dbg->dbgp_drslist, drs_next, tdrs) {
		if (drs != *drsp)
			continue;
		STAILQ_REMOVE(&dbg->dbgp_drslist, drs, _Dwarf_Rel_Section,
		    drs_next);
		STAILQ_FOREACH_SAFE(dre, &drs->drs_dre, dre_next, tdre) {
			STAILQ_REMOVE(&drs->drs_dre, dre, _Dwarf_Rel_Entry,
			    dre_next);
			free(dre);
		}
		if ((dbg->dbgp_flags & DW_DLC_SYMBOLIC_RELOCATIONS) == 0)
			_dwarf_section_free(dbg, &drs->drs_ds);
		free(drs);
		*drsp = NULL;
		break;
	}
}

int
_dwarf_reloc_entry_add(Dwarf_Rel_Section drs, unsigned char type,
    unsigned char length, Dwarf_Unsigned offset, Dwarf_Unsigned symndx,
    const char *secname, Dwarf_Error *error)
{
	Dwarf_Rel_Entry dre;

	assert(drs != NULL);

	if ((dre = calloc(1, sizeof(struct _Dwarf_Rel_Entry))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	STAILQ_INSERT_TAIL(&drs->drs_dre, dre, dre_next);
	dre->dre_type = type;
	dre->dre_length = length;
	dre->dre_offset = offset;
	dre->dre_symndx = symndx;
	dre->dre_secname = secname;
	drs->drs_drecnt++;

	return (DWARF_E_NONE);
}

int
_dwarf_reloc_elf_create_notify(Dwarf_P_Debug dbg, Dwarf_Rel_Section drs,
    Dwarf_Error *error)
{
	Dwarf_P_Section ds;
	Dwarf_Unsigned unit;
	int ret;

	assert(dbg != NULL && drs != NULL && drs->drs_ds != NULL &&
	    drs->drs_ref != NULL);

	ds = drs->drs_ds;

	/*
	 * Calculate the size (in bytes) of the relocation section and
	 * realloc the section data block to this size.
	 */
	if (dbg->dbgp_flags & DW_DLC_SIZE_64)
		unit = drs->drs_addend ? sizeof(Elf64_Rela) : sizeof(Elf64_Rel);
	else
		unit = drs->drs_addend ? sizeof(Elf32_Rela) : sizeof(Elf32_Rel);
	assert(ds->ds_size == 0);
	ds->ds_cap = drs->drs_drecnt * unit;
	if ((ds->ds_data = realloc(ds->ds_data, ds->ds_cap)) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	/*
	 * Notify the application the creation of this relocation section.
	 * Note that the section link here should point to the .symtab
	 * section, we set it to 0 since we have no way to know .symtab
	 * section index.
	 */
	ret = _dwarf_pro_callback(dbg, ds->ds_name, (int) ds->ds_cap,
	    drs->drs_addend ? SHT_RELA : SHT_REL, 0, 0, drs->drs_ref->ds_ndx,
	    &ds->ds_symndx, NULL);
	if (ret < 0) {
		DWARF_SET_ERROR(error, DWARF_E_USER_CALLBACK);
		return (DWARF_E_USER_CALLBACK);
	}
	ds->ds_ndx = ret;

	return (DWARF_E_NONE);
}
