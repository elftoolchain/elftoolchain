/*-
 * Copyright (c) 2009 Kai Wang
 * All rights reserved.
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

	_libdwarf.errhand = errhand;
	_libdwarf.errarg = errarg;

	if (elf == NULL || ret_dbg == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (mode != DW_DLC_READ) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (_dwarf_alloc(&dbg, mode, error) != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	if (_dwarf_elf_init(dbg, elf, 0, error) != DWARF_E_NONE) {
		free(dbg);
		return (DW_DLV_ERROR);
	}

	if (_dwarf_init(dbg, 0, error) != DWARF_E_NONE) {
		_dwarf_elf_deinit(dbg);
		free(dbg);
		return (DW_DLV_ERROR);
	}

	*ret_dbg = dbg;

	return (DW_DLV_OK);
}

int
dwarf_init(int fd, int mode, Dwarf_Handler errhand, Dwarf_Ptr errarg,
    Dwarf_Debug *ret_dbg, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	Elf *elf;

	_libdwarf.errhand = errhand;
	_libdwarf.errarg = errarg;

	if (fd < 0 || ret_dbg == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (mode != DW_DLC_READ) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (elf_version(EV_CURRENT) == EV_NONE) {
		DWARF_SET_ELF_ERROR(error);
		return (DW_DLV_ERROR);
	}

	if ((elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		DWARF_SET_ELF_ERROR(error);
		return (DW_DLV_ERROR);
	}

	if (_dwarf_alloc(&dbg, mode, error) != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	if (_dwarf_elf_init(dbg, elf, 1, error) != DWARF_E_NONE) {
		free(dbg);
		return (DW_DLV_ERROR);
	}

	if (_dwarf_init(dbg, 0, error) != DWARF_E_NONE) {
		_dwarf_elf_deinit(dbg);
		free(dbg);
		return (DW_DLV_ERROR);
	}

	*ret_dbg = dbg;

	return (DW_DLV_OK);
}

int
dwarf_object_init(Dwarf_Obj_Access_Interface *iface, Dwarf_Handler errhand,
    Dwarf_Ptr errarg, Dwarf_Debug *ret_dbg, Dwarf_Error *error)
{
	Dwarf_Debug dbg;

	_libdwarf.errhand = errhand;
	_libdwarf.errarg = errarg;

	if (iface == NULL || ret_dbg == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (_dwarf_alloc(&dbg, DW_DLC_READ, error) != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	dbg->dbg_iface = iface;

	if (_dwarf_init(dbg, 0, error) != DWARF_E_NONE) {
		free(dbg);
		return (DW_DLV_ERROR);
	}

	*ret_dbg = dbg;

	return (DW_DLV_OK);
}
