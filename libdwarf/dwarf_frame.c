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

#include <assert.h>
#include "_libdwarf.h"

int
dwarf_get_fde_list(Dwarf_Debug dbg, Dwarf_Cie **cie_list,
    Dwarf_Signed *cie_count, Dwarf_Fde **fde_list, Dwarf_Signed *fde_count,
    Dwarf_Error *error)
{

	if (dbg == NULL || cie_list == NULL || cie_count == NULL ||
	    fde_list == NULL || fde_count == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (dbg->dbg_frame == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return (DW_DLV_NO_ENTRY);
	}

	if (dbg->dbg_frame->fs_ciearray == NULL ||
	    dbg->dbg_frame->fs_fdearray == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return (DW_DLV_NO_ENTRY);
	}

	*cie_list = dbg->dbg_frame->fs_ciearray;
	*cie_count = dbg->dbg_frame->fs_cielen;
	*fde_list = dbg->dbg_frame->fs_fdearray;
	*fde_count = dbg->dbg_frame->fs_fdelen;

	return (DW_DLV_OK);
}

int
dwarf_get_fde_list_eh(Dwarf_Debug dbg, Dwarf_Cie **cie_list,
    Dwarf_Signed *cie_count, Dwarf_Fde **fde_list, Dwarf_Signed *fde_count,
    Dwarf_Error *error)
{

	if (dbg == NULL || cie_list == NULL || cie_count == NULL ||
	    fde_list == NULL || fde_count == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	if (dbg->dbg_eh_frame == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return (DW_DLV_NO_ENTRY);
	}

	if (dbg->dbg_eh_frame->fs_ciearray == NULL ||
	    dbg->dbg_eh_frame->fs_fdearray == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return (DW_DLV_NO_ENTRY);
	}

	*cie_list = dbg->dbg_eh_frame->fs_ciearray;
	*cie_count = dbg->dbg_eh_frame->fs_cielen;
	*fde_list = dbg->dbg_eh_frame->fs_fdearray;
	*fde_count = dbg->dbg_eh_frame->fs_fdelen;

	return (DW_DLV_OK);
}

int
dwarf_get_fde_n(Dwarf_Fde *fdelist, Dwarf_Unsigned fde_index,
    Dwarf_Fde *ret_fde, Dwarf_Error *error)
{
	Dwarf_FrameSec fs;

	if (fdelist == NULL || ret_fde == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	fs = fdelist[0]->fde_fs;
	assert(fs != NULL);

	if (fde_index >= fs->fs_fdelen) {
		DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
		return (DW_DLV_NO_ENTRY);
	}

	*ret_fde = fdelist[fde_index];

	return (DW_DLV_OK);
}

int
dwarf_get_fde_at_pc(Dwarf_Fde *fdelist, Dwarf_Addr pc, Dwarf_Fde *ret_fde,
    Dwarf_Addr *lopc, Dwarf_Addr *hipc, Dwarf_Error *error)
{
	Dwarf_FrameSec fs;
	Dwarf_Fde fde;
	int i;

	if (fdelist == NULL || ret_fde == NULL || lopc == NULL ||
	    hipc == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}
	
	fs = fdelist[0]->fde_fs;
	assert(fs != NULL);
	
	for (i = 0; (Dwarf_Unsigned)i < fs->fs_fdelen; i++) {
		fde = fdelist[i];
		if (pc >= fde->fde_initloc && pc < fde->fde_initloc +
		    fde->fde_adrange) {
			*ret_fde = fde;
			*lopc = fde->fde_initloc;
			*hipc = fde->fde_initloc + fde->fde_adrange;
			return (DW_DLV_OK);
		}
	}

	DWARF_SET_ERROR(error, DWARF_E_NO_ENTRY);
	return (DW_DLV_NO_ENTRY);
}

int
dwarf_get_cie_of_fde(Dwarf_Fde fde, Dwarf_Cie *ret_cie, Dwarf_Error *error)
{

	if (fde == NULL || ret_cie == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	*ret_cie = fde->fde_cie;

	return (DW_DLV_OK);
}

int
dwarf_get_fde_range(Dwarf_Fde fde, Dwarf_Addr *low_pc, Dwarf_Unsigned *func_len,
    Dwarf_Ptr *fde_bytes, Dwarf_Unsigned *fde_byte_len, Dwarf_Off *cie_offset,
    Dwarf_Signed *cie_index, Dwarf_Off *fde_offset, Dwarf_Error *error)
{

	if (fde == NULL || low_pc == NULL || func_len == NULL ||
	    fde_bytes == NULL || fde_byte_len == NULL || cie_offset == NULL ||
	    cie_index == NULL || fde_offset == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	*low_pc = fde->fde_initloc;
	*func_len = fde->fde_adrange;
	*fde_bytes = fde->fde_addr;

	/*
	 * XXX should we return a real length, or length excluding
	 * initial length?
	 */
	*fde_byte_len = fde->fde_length;
	*cie_offset = fde->fde_cieoff;
	*cie_index = fde->fde_cie->cie_index;
	*fde_offset = fde->fde_offset;

	return (DW_DLV_OK);
}

int
dwarf_get_cie_info(Dwarf_Cie cie, Dwarf_Unsigned *bytes_in_cie,
    Dwarf_Small *version, char **augmenter, Dwarf_Unsigned *caf,
    Dwarf_Unsigned *daf, Dwarf_Half *ra, Dwarf_Ptr *initinst,
    Dwarf_Unsigned *inst_len, Dwarf_Error *error)
{

	if (cie == NULL || bytes_in_cie == NULL || version == NULL ||
	    augmenter == NULL || caf == NULL || daf == NULL || ra == NULL ||
	    initinst == NULL || inst_len == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	/* XXX probably wrong, see above. */
	*bytes_in_cie = cie->cie_length;
	*version = cie->cie_version;
	*augmenter = cie->cie_augment;
	*caf = cie->cie_caf;
	*daf = cie->cie_daf;
	*ra = cie->cie_ra;
	*initinst = cie->cie_initinst;
	*inst_len = cie->cie_instlen;

	return (DW_DLV_OK);
}

int
dwarf_get_fde_instr_bytes(Dwarf_Fde fde, Dwarf_Ptr *ret_inst,
    Dwarf_Unsigned *ret_len, Dwarf_Error *error)
{

	if (fde == NULL || ret_inst == NULL || ret_len == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	*ret_inst = fde->fde_inst;
	*ret_len = fde->fde_instlen;

	return (DW_DLV_OK);
}

#define	RL	rt->rt3_rules[table_column]
#define	CFA	rt->rt3_cfa_rule

int
dwarf_get_fde_info_for_reg(Dwarf_Fde fde, Dwarf_Half table_column,
    Dwarf_Addr pc_requested, Dwarf_Signed *offset_relevant,
    Dwarf_Signed *register_num, Dwarf_Signed *offset, Dwarf_Addr *row_pc,
    Dwarf_Error *error)
{
	Dwarf_Regtable3 *rt;
	Dwarf_Addr pc;
	int ret;

	if (fde == NULL || offset_relevant == NULL || register_num == NULL ||
	    offset == NULL || row_pc == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	ret = _dwarf_frame_get_internal_table(fde, pc_requested, &rt, &pc,
	    error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	if (table_column == DW_FRAME_CFA_COL) {
		/* Application ask for CFA. */
		*offset_relevant = CFA.dw_offset_relevant;
		*register_num = CFA.dw_regnum;
		*offset = CFA.dw_offset_or_block_len;
	} else {
		/* Application ask for normal registers. */
		*offset_relevant = RL.dw_offset_relevant;
		if (RL.dw_regnum == DW_FRAME_CFA_COL3)
			*register_num = DW_FRAME_CFA_COL;
		else
			*register_num = RL.dw_regnum;
		*offset = RL.dw_offset_or_block_len;
	}

	*row_pc = pc;

	return (DW_DLV_OK);
}

int
dwarf_get_fde_info_for_all_regs(Dwarf_Fde fde, Dwarf_Addr pc_requested,
    Dwarf_Regtable *reg_table, Dwarf_Addr *row_pc, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	Dwarf_Regtable3 *rt;
	Dwarf_Addr pc;
	int i, ret;

	if (fde == NULL || reg_table == NULL || row_pc == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	dbg = fde->fde_dbg;
	assert(dbg != NULL);

	ret = _dwarf_frame_get_internal_table(fde, pc_requested, &rt, &pc,
	    error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	/* Copy the CFA rule to the first column of the reg table. */
	reg_table->rules[0].dw_offset_relevant = CFA.dw_offset_relevant;
	reg_table->rules[0].dw_regnum = CFA.dw_regnum;
	reg_table->rules[0].dw_offset = CFA.dw_offset_or_block_len;

	/* Copy the normal columns. */
	for (i = 1; i < DW_REG_TABLE_SIZE && i < dbg->dbg_frame_rule_table_size;
	     i++) {
		reg_table->rules[i].dw_offset_relevant =
		    rt->rt3_rules[i].dw_offset_relevant;
		if (rt->rt3_rules[i].dw_regnum == DW_FRAME_CFA_COL3)
			reg_table->rules[i].dw_regnum = DW_FRAME_CFA_COL;
		else
			reg_table->rules[i].dw_regnum =
			    rt->rt3_rules[i].dw_regnum;
		reg_table->rules[i].dw_offset =
		    rt->rt3_rules[i].dw_offset_or_block_len;
	}

	*row_pc = pc;

	return (DW_DLV_OK);
}

int
dwarf_get_fde_info_for_reg3(Dwarf_Fde fde, Dwarf_Half table_column,
    Dwarf_Addr pc_requested, Dwarf_Small *value_type,
    Dwarf_Signed *offset_relevant, Dwarf_Signed *register_num,
    Dwarf_Signed *offset_or_block_len, Dwarf_Ptr *block_ptr,
    Dwarf_Addr *row_pc, Dwarf_Error *error)
{
	Dwarf_Regtable3 *rt;
	Dwarf_Addr pc;
	int ret;

	if (fde == NULL || value_type == NULL || offset_relevant == NULL ||
	    register_num == NULL || offset_or_block_len == NULL ||
	    block_ptr == NULL || row_pc == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	ret = _dwarf_frame_get_internal_table(fde, pc_requested, &rt, &pc,
	    error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	*value_type = RL.dw_value_type;
	*offset_relevant = RL.dw_offset_relevant;
	*register_num = RL.dw_regnum;
	*offset_or_block_len = RL.dw_offset_or_block_len;
	*block_ptr = RL.dw_block_ptr;
	*row_pc = pc;

	return (DW_DLV_OK);
}

int
dwarf_get_fde_info_for_cfa_reg3(Dwarf_Fde fde, Dwarf_Addr pc_requested,
    Dwarf_Small *value_type, Dwarf_Signed *offset_relevant,
    Dwarf_Signed *register_num, Dwarf_Signed *offset_or_block_len,
    Dwarf_Ptr *block_ptr, Dwarf_Addr *row_pc, Dwarf_Error *error)
{
	Dwarf_Regtable3 *rt;
	Dwarf_Addr pc;
	int ret;

	if (fde == NULL || value_type == NULL || offset_relevant == NULL ||
	    register_num == NULL || offset_or_block_len == NULL ||
	    block_ptr == NULL || row_pc == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	ret = _dwarf_frame_get_internal_table(fde, pc_requested, &rt, &pc,
	    error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	*value_type = CFA.dw_value_type;
	*offset_relevant = CFA.dw_offset_relevant;
	*register_num = CFA.dw_regnum;
	*offset_or_block_len = CFA.dw_offset_or_block_len;
	*block_ptr = CFA.dw_block_ptr;
	*row_pc = pc;

	return (DW_DLV_OK);
}

#undef	RL
#undef	CFA

int
dwarf_get_fde_info_for_all_regs3(Dwarf_Fde fde, Dwarf_Addr pc_requested,
    Dwarf_Regtable3 *reg_table, Dwarf_Addr *row_pc, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	Dwarf_Regtable3 *rt;
	Dwarf_Addr pc;
	int ret;

	if (fde == NULL || reg_table == NULL || row_pc == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	dbg = fde->fde_dbg;
	assert(dbg != NULL);

	ret = _dwarf_frame_get_internal_table(fde, pc_requested, &rt, &pc,
	    error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	ret = _dwarf_frame_regtable_copy(dbg, &reg_table, rt, error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	*row_pc = pc;

	return (DW_DLV_OK);
}

int
dwarf_expand_frame_instructions(Dwarf_Debug dbg, Dwarf_Ptr instruction,
    Dwarf_Unsigned len, Dwarf_Frame_Op **ret_oplist, Dwarf_Signed *ret_opcnt,
    Dwarf_Error *error)
{
	int ret;

	if (dbg == NULL || instruction == NULL || len == 0 ||
	    ret_oplist == NULL || ret_opcnt == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	ret = _dwarf_frame_get_fop(dbg, instruction, len, ret_oplist, ret_opcnt,
	    error);
	if (ret != DWARF_E_NONE)
		return (DW_DLV_ERROR);

	return (DW_DLV_OK);
}

int
dwarf_frame_instructions_dealloc(Dwarf_Frame_Op *oplist, Dwarf_Signed opcnt,
    Dwarf_Error *error)
{

	if (oplist == NULL || opcnt == 0) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_ERROR);
	}

	_dwarf_frame_free_fop(oplist, opcnt);

	return (DW_DLV_OK);
}

Dwarf_Half
dwarf_set_frame_rule_table_size(Dwarf_Debug dbg, Dwarf_Half value)
{
	Dwarf_Half old_value;

	old_value = dbg->dbg_frame_rule_table_size;
	dbg->dbg_frame_rule_table_size = value;

	return (old_value);
}

Dwarf_Half
dwarf_set_frame_rule_initial_value(Dwarf_Debug dbg, Dwarf_Half value)
{
	Dwarf_Half old_value;

	old_value = dbg->dbg_frame_rule_initial_value;
	dbg->dbg_frame_rule_initial_value = value;

	return (old_value);
}

Dwarf_Half
dwarf_set_frame_cfa_value(Dwarf_Debug dbg, Dwarf_Half value)
{
	Dwarf_Half old_value;

	old_value = dbg->dbg_frame_cfa_value;
	dbg->dbg_frame_cfa_value = value;

	return (old_value);
}

Dwarf_Half
dwarf_set_frame_same_value(Dwarf_Debug dbg, Dwarf_Half value)
{
	Dwarf_Half old_value;

	old_value = dbg->dbg_frame_same_value;
	dbg->dbg_frame_same_value = value;

	return (old_value);
}

Dwarf_Half
dwarf_set_frame_undefined_value(Dwarf_Debug dbg, Dwarf_Half value)
{
	Dwarf_Half old_value;

	old_value = dbg->dbg_frame_undefined_value;
	dbg->dbg_frame_undefined_value = value;

	return (old_value);
}
