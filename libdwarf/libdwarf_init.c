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

#include <stdlib.h>
#include <string.h>
#include "_libdwarf.h"

static const char *debug_snames[DWARF_DEBUG_SNAMES] = {
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
	".symtab",
	".strtab"
};

static int
apply_relocations(Dwarf_Debug dbg, Elf_Data *reld, int secindx)
{
	Elf_Data *d;
	GElf_Rela rela;
	int indx = 0;
	int ret = DWARF_E_NONE;
	uint64_t offset;

	/* Point to the data to be relocated: */
	d = dbg->dbg_s[secindx].s_data;

	/* Enter a loop to process each relocation addend: */
	while (gelf_getrela(reld, indx++, &rela) != NULL) {
		GElf_Sym sym;
		Elf64_Xword symindx = ELF64_R_SYM(rela.r_info);

		if (gelf_getsym(dbg->dbg_s[DWARF_symtab].s_data, symindx, &sym) == NULL) {
			printf("Couldn't find symbol index %lu for relocation\n",(u_long) symindx);
			continue;
		}

		offset = rela.r_offset;

		dbg->write(&d, &offset, rela.r_addend, dbg->dbg_pointer_size);
	}

	return ret;
}

static int
relocate(Dwarf_Debug dbg, Dwarf_Error *error)
{
	Elf_Scn *scn = NULL;
	GElf_Shdr shdr;
	int i;
	int ret = DWARF_E_NONE;

	/* Look for sections which relocate the debug sections. */
	while ((scn = elf_nextscn(dbg->dbg_elf, scn)) != NULL) {
		if (gelf_getshdr(scn, &shdr) == NULL) {
			DWARF_SET_ELF_ERROR(error, elf_errno());
			return DWARF_E_ELF;
		}

		if (shdr.sh_type != SHT_RELA || shdr.sh_size == 0)
			continue;

		for (i = 0; i < DWARF_DEBUG_SNAMES; i++) {
			if (dbg->dbg_s[i].s_shnum == shdr.sh_info &&
			    dbg->dbg_s[DWARF_symtab].s_shnum == shdr.sh_link) {
				Elf_Data *rd;

				/* Get the relocation data. */
				if ((rd = elf_getdata(scn, NULL)) == NULL) {
					DWARF_SET_ELF_ERROR(error, elf_errno());
					return DWARF_E_ELF;
				}

				/* Apply the relocations. */
				apply_relocations(dbg, rd, i);
				break;
			}
		}
	}

	return ret;
}

static int
init_info(Dwarf_Debug dbg, Dwarf_Error *error)
{
	Dwarf_CU cu;
	Elf_Data *d = NULL;
	Elf_Scn *scn;
	int dwarf_size, i;
	int level = 0;
	int relocated = 0;
	int ret = DWARF_E_NONE;
	uint64_t length;
	uint64_t next_offset;
	uint64_t offset = 0;

	scn = dbg->dbg_s[DWARF_debug_info].s_scn;

	d = dbg->dbg_s[DWARF_debug_info].s_data;

	while (offset < d->d_size) {
		/* Allocate memory for the first compilation unit. */
		if ((cu = calloc(1, sizeof(struct _Dwarf_CU))) == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			return DWARF_E_MEMORY;
		}

		/* Save a pointer to containing dbg. */
		cu->cu_dbg = dbg;

		/* Save the offet to this compilation unit: */
		cu->cu_offset = offset;

		length = dbg->read(&d, &offset, 4);
		if (length == 0xffffffff) {
			length = dbg->read(&d, &offset, 8);
			dwarf_size = 8;
		} else
			dwarf_size = 4;

		/*
		 * Check if there is enough ELF data for this CU.
		 * This assumes that libelf gives us the entire
		 * section in one Elf_Data object.
		 */
		if (length > d->d_size - offset) {
			free(cu);
			DWARF_SET_ERROR(error, DWARF_E_INVALID_CU);
			return (DWARF_E_INVALID_CU);
		}

		/* Relocate the DWARF sections if necessary: */
		if (!relocated) {
			if ((ret = relocate(dbg, error)) != DWARF_E_NONE)
				return ret;
			relocated = 1;
		}

		/* Compute the offset to the next compilation unit: */
		next_offset = offset + length;

		/* Initialise the compilation unit. */
		cu->cu_length 		= length;
		cu->cu_header_length	= (dwarf_size == 4) ? 4 : 12;
		cu->cu_version		= dbg->read(&d, &offset, 2);
		cu->cu_abbrev_offset	= dbg->read(&d, &offset, dwarf_size);
		cu->cu_pointer_size	= dbg->read(&d, &offset, 1);
		cu->cu_next_offset	= next_offset;

		/* Initialise the list of abbrevs. */
		STAILQ_INIT(&cu->cu_abbrev);

		/* Initialise the list of dies. */
		STAILQ_INIT(&cu->cu_die);

		/* Initialise the hash table of dies. */
		for (i = 0; i < DWARF_DIE_HASH_SIZE; i++)
			STAILQ_INIT(&cu->cu_die_hash[i]);

		/* Add the compilation unit to the list. */
		STAILQ_INSERT_TAIL(&dbg->dbg_cu, cu, cu_next);

		if (cu->cu_version != 2 && cu->cu_version != 3) {
			DWARF_SET_ERROR(error, DWARF_E_CU_VERSION);
			ret = DWARF_E_CU_VERSION;
			break;
		}

		/* Parse the .debug_abbrev info for this CU: */
		if ((ret = abbrev_init(dbg, cu, error)) != DWARF_E_NONE)
			break;

		level = 0;

		while (offset < next_offset && offset < d->d_size) {
			Dwarf_Abbrev ab;
			Dwarf_AttrDef ad;
			Dwarf_Die die;
			uint64_t abnum;
			uint64_t die_offset = offset;;

			abnum = read_uleb128(&d, &offset);

			if (abnum == 0) {
				level--;
				continue;
			}

			if ((ab = abbrev_find(cu, abnum)) == NULL) {
				DWARF_SET_ERROR(error, DWARF_E_MISSING_ABBREV);
				return DWARF_E_MISSING_ABBREV;
			}

			if ((ret = die_add(cu, level, die_offset, abnum, ab,
			    &die, error)) != DWARF_E_NONE)
				return ret;

			STAILQ_FOREACH(ad, &ab->ab_attrdef, ad_next) {
				if ((ret = attr_init(dbg, &d, &offset,
				    dwarf_size, cu, die, ad, ad->ad_form, 0,
				    error)) != DWARF_E_NONE)
					return ret;
			}

			if (ab->ab_children == DW_CHILDREN_yes)
				level++;
		}

		offset = next_offset;
	}

	return ret;
}

int
elf_read(Dwarf_Debug dbg, Dwarf_Error *error)
{
	GElf_Shdr shdr;
	Elf_Scn *scn;
	char *sname;
	int i;
	int ret;

	ret = DWARF_E_NONE;

	/* Get a copy of the ELF header. */
	if (gelf_getehdr(dbg->dbg_elf, &dbg->dbg_ehdr) == NULL) {
		DWARF_SET_ELF_ERROR(error, elf_errno());
		return (DWARF_E_ELF);
	}

	/* Check the ELF data format: */
	switch (dbg->dbg_ehdr.e_ident[EI_DATA]) {
	case ELFDATA2MSB:
		dbg->read = read_msb;
		dbg->write = write_msb;
		dbg->decode = decode_msb;
		break;

	case ELFDATA2LSB:
	case ELFDATANONE:
	default:
		dbg->read = read_lsb;
		dbg->write = write_lsb;
		dbg->decode = decode_lsb;
		break;
	}

	/* Set default pointer size. */
	if (gelf_getclass(dbg->dbg_elf) == ELFCLASS32)
		dbg->dbg_pointer_size = 4;
	else
		dbg->dbg_pointer_size = 8;

	/* Get the section index to the string table. */
	if (elf_getshstrndx(dbg->dbg_elf, &dbg->dbg_stnum) == 0) {
		DWARF_SET_ELF_ERROR(error, elf_errno());
		return (DWARF_E_ELF);
	}

	/* Look for the debug sections. */
	scn = NULL;
	while ((scn = elf_nextscn(dbg->dbg_elf, scn)) != NULL) {
		/* Get a copy of the section header: */
		if (gelf_getshdr(scn, &shdr) == NULL) {
			DWARF_SET_ELF_ERROR(error, elf_errno());
			return (DWARF_E_ELF);
		}

		/* Get a pointer to the section name: */
		if ((sname = elf_strptr(dbg->dbg_elf, dbg->dbg_stnum,
		    shdr.sh_name)) == NULL) {
			DWARF_SET_ELF_ERROR(error, elf_errno());
			return (DWARF_E_ELF);
		}

		/*
		 * Look up the section name to check if it's
		 * one we need for DWARF.
		 */
		for (i = 0; i < DWARF_DEBUG_SNAMES; i++) {
			if (strcmp(sname, debug_snames[i]) == 0) {
				dbg->dbg_s[i].s_sname = sname;
				dbg->dbg_s[i].s_shnum = elf_ndxscn(scn);
				dbg->dbg_s[i].s_scn = scn;
				memcpy(&dbg->dbg_s[i].s_shdr, &shdr,
				    sizeof(shdr));
				if ((dbg->dbg_s[i].s_data =
				    elf_getdata(scn, NULL)) == NULL) {
					DWARF_SET_ELF_ERROR(error, elf_errno());
					return (DWARF_E_ELF);
				}
				break;
			}
		}
	}

	/* Check if any of the required sections are missing: */
	if (dbg->dbg_s[DWARF_debug_abbrev].s_scn == NULL ||
	    dbg->dbg_s[DWARF_debug_info].s_scn == NULL) {
		/* Missing debug information. */
		DWARF_SET_ERROR(error, DWARF_E_DEBUG_INFO);
		return (DWARF_E_DEBUG_INFO);
	}

	/* Initialise the loclist. */
	TAILQ_INIT(&dbg->dbg_loclist);

	/* Initialise rangelist list. */
	STAILQ_INIT(&dbg->dbg_rllist);

	/* Initialise the compilation-units. */
	ret = init_info(dbg, error);
	if (ret != DWARF_E_NONE)
		return (ret);

#define	INIT_NAMETBL(NDX, TBL)						\
	do {								\
		if (dbg->dbg_s[DWARF_debug_##NDX].s_scn != NULL) {	\
			ret = nametbl_init(dbg, &dbg->dbg_##TBL,	\
			    dbg->dbg_s[DWARF_debug_##NDX].s_data,	\
			    error);					\
			if (ret != DWARF_E_NONE)			\
				return (ret);				\
		}							\
	} while (0)


	/* Initialise several name lookup sections, if exist. */
	INIT_NAMETBL(pubnames, globals);
	INIT_NAMETBL(pubtypes, pubtypes);
	INIT_NAMETBL(weaknames, weaks);
	INIT_NAMETBL(static_func, funcs);
	INIT_NAMETBL(static_vars, vars);
	INIT_NAMETBL(types, types);

#undef	INIT_NAMETBL

	/* Initialise call frame data. */
	ret = frame_init(dbg, error);
	if (ret != DWARF_E_NONE)
		return (ret);

	/* Initialise address range data. */
	STAILQ_INIT(&dbg->dbg_aslist);
	if (dbg->dbg_s[DWARF_debug_aranges].s_scn != NULL) {
		ret = arange_init(dbg, dbg->dbg_s[DWARF_debug_aranges].s_data,
		    error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}

	/* Initialise macinfo data. */
	STAILQ_INIT(&dbg->dbg_mslist);
	if (dbg->dbg_s[DWARF_debug_macinfo].s_scn != NULL) {
		ret = macinfo_init(dbg, dbg->dbg_s[DWARF_debug_macinfo].s_data,
		    error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}

	return (ret);
}
