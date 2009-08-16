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

#include <string.h>
#include "_libdwarf.h"

int
dwarf_elf_init(Elf *elf, int mode, Dwarf_Handler errhand, Dwarf_Ptr errarg,
    Dwarf_Debug *ret_dbg, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	int ret;

	_libdwarf.errhand = errhand;
	_libdwarf.errarg = errarg;

	if (elf == NULL || ret_dbg == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if ((dbg = calloc(sizeof(struct _Dwarf_Debug), 1)) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DW_DLV_ERROR);
	}

	dbg->dbg_elf		= elf;
	dbg->dbg_elf_close 	= 0;
	dbg->dbg_mode		= mode;
	STAILQ_INIT(&dbg->dbg_cu);

	*ret_dbg = dbg;

	/* Read the ELF sections. */
	ret = elf_read(dbg, error);

	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	return (DW_DLV_OK);
}

int
dwarf_init(int fd, int mode, Dwarf_Handler errhand, Dwarf_Ptr errarg,
    Dwarf_Debug *ret_dbg, Dwarf_Error *error)
{
	Dwarf_Error lerror;
	Elf *elf;
	Elf_Cmd	c;
	int ret;

	_libdwarf.errhand = errhand;
	_libdwarf.errarg = errarg;

	if (fd < 0 || ret_dbg == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	/* Translate the DWARF mode to ELF mode. */
	switch (mode) {
	default:
	case DW_DLC_READ:
		c = ELF_C_READ;
		break;
	}

	if (elf_version(EV_CURRENT) == EV_NONE) {
		DWARF_SET_ELF_ERROR(error, elf_errno());
		return (DW_DLV_ERROR);
	}

	if ((elf = elf_begin(fd, c, NULL)) == NULL) {
		DWARF_SET_ELF_ERROR(error, elf_errno());
		return (DW_DLV_ERROR);
	}

	ret = dwarf_elf_init(elf, mode, errhand, errarg, ret_dbg, error);

	if (*ret_dbg != NULL)
		/* Remember to close the ELF file. */
		(*ret_dbg)->dbg_elf_close = 1;

	if (ret != DWARF_E_NONE) {
		if (*ret_dbg != NULL) {
			dwarf_finish(*ret_dbg, &lerror);
			*ret_dbg = NULL;
		} else
			elf_end(elf);

		return (DW_DLV_ERROR);
	}

	return (DW_DLV_OK);
}
