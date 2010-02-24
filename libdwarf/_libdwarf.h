/*-
 * Copyright (c) 2009, 2010 Kai Wang
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
 *
 * $FreeBSD$
 */

#ifndef	__LIBDWARF_H_
#define	__LIBDWARF_H_

#include <sys/param.h>
#include <sys/queue.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gelf.h>
#include "dwarf.h"
#include "libdwarf.h"

#include "_elftc.h"

#define DWARF_DIE_HASH_SIZE		8191

struct _libdwarf_globals {
	Dwarf_Handler	errhand;
	Dwarf_Ptr	errarg;
};

extern struct _libdwarf_globals _libdwarf;

#define	_DWARF_SET_ERROR(_e, _err, _elf_err)				\
	do {								\
		Dwarf_Error _de;					\
									\
		_de.err_error = _err;					\
		_de.elf_error = _elf_err;				\
		_de.err_func  = __func__;				\
		_de.err_line  = __LINE__;				\
		_de.err_msg[0] = '\0';					\
									\
		if (_e)							\
			*_e = _de;					\
									\
		if (_libdwarf.errhand)					\
			_libdwarf.errhand(_de, _libdwarf.errarg);	\
									\
		if (_e == NULL && _libdwarf.errhand == NULL)		\
			abort();					\
	} while (0)

#define	DWARF_SET_ERROR(_e, _err)	_DWARF_SET_ERROR(_e, _err, 0)
#define	DWARF_SET_ELF_ERROR(_e)						\
	_DWARF_SET_ERROR(_e, DWARF_E_ELF, elf_errno())

struct _Dwarf_AttrDef {
	uint64_t	ad_attrib;		/* DW_AT_XXX */
	uint64_t	ad_form;		/* DW_FORM_XXX */
	uint64_t	ad_offset;		/* Offset in abbrev section. */
	STAILQ_ENTRY(_Dwarf_AttrDef) ad_next;	/* Next attribute define. */
};

struct _Dwarf_Attribute {
	Dwarf_Die		at_die;		/* Ptr to containing DIE. */
	Dwarf_Die		at_refdie;	/* Ptr to reference DIE. */
	uint64_t		at_offset;	/* Offset in info section. */
	uint64_t		at_attrib;	/* DW_AT_XXX */
	uint64_t		at_form;	/* DW_FORM_XXX */
	int			at_indirect;	/* Has indirect form. */
	union {
		uint64_t	u64;
		int64_t		s64;
		char		*s;   		/* String. */
		uint8_t		*u8p;		/* Block. */
	} u[2];					/* Value. */
	Dwarf_Locdesc		*at_ld;		/* at value is locdesc. */
	uint64_t		at_relsym;	/* Relocation symbol index. */
	STAILQ_ENTRY(_Dwarf_Attribute) at_next;	/* Next attribute. */
};

struct _Dwarf_Abbrev {
	uint64_t	ab_entry;	/* Abbrev entry. */
	uint64_t	ab_tag;		/* Tag: DW_TAG_ */
	uint8_t		ab_children;	/* DW_CHILDREN_no or DW_CHILDREN_yes */
	uint64_t	ab_offset;	/* Offset in abbrev section. */
	uint64_t	ab_length;	/* Length of this abbrev entry. */
	uint64_t	ab_atnum;	/* Number of attribute defines. */
	STAILQ_HEAD(, _Dwarf_AttrDef) ab_attrdef; /* List of attribute defs. */
	STAILQ_ENTRY(_Dwarf_Abbrev) ab_next; /* Next abbrev. */
};

struct _Dwarf_Die {
	Dwarf_Die	die_parent;	/* Parent DIE. */
	Dwarf_Die	die_child;	/* First child DIE. */
	Dwarf_Die	die_left;	/* Left sibling DIE. */
	Dwarf_Die	die_right;	/* Right sibling DIE. */
	uint64_t	die_offset;	/* DIE offset in section. */
	uint64_t	die_abnum;	/* Abbrev number. */
	Dwarf_Abbrev	die_ab;		/* Abbrev pointer. */
	Dwarf_Tag	die_tag;	/* DW_TAG_ */
	Dwarf_Debug	die_dbg;	/* Dwarf_Debug pointer. */
	Dwarf_CU	die_cu;		/* Compilation unit pointer. */
	char		*die_name;	/* Ptr to the name string. */
	Dwarf_Attribute	*die_attrarray;	/* Array of attributes. */
	STAILQ_HEAD(, _Dwarf_Attribute)	die_attr; /* List of attributes. */
	STAILQ_ENTRY(_Dwarf_Die) die_next; /* Next die in list. */
	STAILQ_ENTRY(_Dwarf_Die) die_hash; /* Next die in hash table. */
	STAILQ_ENTRY(_Dwarf_Die) die_pro_next; /* Next die in pro-die list. */
};

struct _Dwarf_Loclist {
	Dwarf_Locdesc 	**ll_ldlist;	/* Array of Locdesc pointer. */
	int 		ll_ldlen;	/* Number of Locdesc. */
	Dwarf_Unsigned	ll_offset;	/* Offset in .debug_loc section. */
	Dwarf_Unsigned	ll_length;	/* Length (in bytes) of the loclist. */
	TAILQ_ENTRY(_Dwarf_Loclist) ll_next; /* Next loclist in list. */
};

struct _Dwarf_P_Expr_Entry {
	Dwarf_Loc	ee_loc;		/* Location expression. */
	Dwarf_Unsigned	ee_sym;		/* Optional related reloc sym index. */
	STAILQ_ENTRY(_Dwarf_P_Expr_Entry) ee_next; /* Next entry in list. */
};

struct _Dwarf_P_Expr {
	Dwarf_Debug	pe_dbg;		/* Dwarf_Debug pointer. */
	uint8_t		*pe_block;	/* Expression block data. */
	int		pe_invalid;	/* Block data is up-to-date or not. */
	Dwarf_Unsigned	pe_length;	/* Length of the block. */
	STAILQ_HEAD(, _Dwarf_P_Expr_Entry) pe_eelist; /* List of entries. */
	STAILQ_ENTRY(_Dwarf_P_Expr) pe_next; /* Next expr in list. */
};

struct _Dwarf_Line {
	Dwarf_LineInfo	ln_li;		/* Ptr to line info. */
	Dwarf_Addr	ln_addr;	/* Line address. */
	Dwarf_Unsigned	ln_symndx;	/* Symbol index for relocation. */
	Dwarf_Unsigned	ln_fileno;	/* File number. */
	Dwarf_Unsigned	ln_lineno;	/* Line number. */
	Dwarf_Signed	ln_column;	/* Column number. */
	Dwarf_Bool	ln_bblock;	/* Basic block flag. */
	Dwarf_Bool	ln_stmt;	/* Begin statement flag. */
	Dwarf_Bool	ln_endseq;	/* End sequence flag. */
	STAILQ_ENTRY(_Dwarf_Line) ln_next; /* Next line in list. */
};

struct _Dwarf_LineFile {
	char		*lf_fname;	/* Filename. */
	char		*lf_fullpath;	/* Full pathname of the file. */
	Dwarf_Unsigned	lf_dirndx;	/* Dir index. */
	Dwarf_Unsigned	lf_mtime;	/* Modification time. */
	Dwarf_Unsigned	lf_size;	/* File size. */
	STAILQ_ENTRY(_Dwarf_LineFile) lf_next; /* Next file in list. */
};

struct _Dwarf_LineInfo {
	Dwarf_Unsigned	li_length;	/* Length of line info data. */
	Dwarf_Half	li_version;	/* Version of line info. */
	Dwarf_Unsigned	li_hdrlen;	/* Length of line info header. */
	Dwarf_Small	li_minlen;	/* Minimum instrutction length. */
	Dwarf_Small	li_defstmt;	/* Default value of is_stmt. */
	int8_t		li_lbase;    	/* Line base for special opcode. */
	Dwarf_Small	li_lrange;    	/* Line range for special opcode. */
	Dwarf_Small	li_opbase;	/* Fisrt std opcode number. */
	Dwarf_Small	*li_oplen;	/* Array of std opcode len. */
	char		**li_incdirs;	/* Array of include dirs. */
	Dwarf_Unsigned	li_inclen;	/* Length of inc dir array. */
	char		**li_lfnarray;	/* Array of file names. */
	Dwarf_Unsigned	li_lflen;	/* Length of filename array. */
	STAILQ_HEAD(, _Dwarf_LineFile) li_lflist; /* List of files. */
	Dwarf_Line	*li_lnarray;	/* Array of lines. */
	Dwarf_Unsigned	li_lnlen;	/* Length of the line array. */
	STAILQ_HEAD(, _Dwarf_Line) li_lnlist; /* List of lines. */
};

struct _Dwarf_NamePair {
	Dwarf_NameTbl	np_nt;		/* Ptr to containing name table. */
	Dwarf_Unsigned	np_offset;	/* Offset in CU. */
	char		*np_name;	/* Object/Type name. */
	STAILQ_ENTRY(_Dwarf_NamePair) np_next; /* Next pair in the list. */
};

struct _Dwarf_NameTbl {
	Dwarf_Unsigned	nt_length;	/* Name lookup table length. */
	Dwarf_Half	nt_version;	/* Name lookup table version. */
	Dwarf_CU	nt_cu;		/* Ptr to Ref. CU. */
	Dwarf_Unsigned	nt_cu_offset;	/* Ref. CU offset in .debug_info */
	Dwarf_Unsigned	nt_cu_length;	/* Ref. CU length. */
	STAILQ_HEAD(, _Dwarf_NamePair) nt_nplist; /* List of offset+name pairs. */
	STAILQ_ENTRY(_Dwarf_NameTbl) nt_next; /* Next name table in the list. */
};

struct _Dwarf_NameSec {
	STAILQ_HEAD(, _Dwarf_NameTbl) ns_ntlist; /* List of name tables. */
	Dwarf_NamePair	*ns_array;	/* Array of pairs of all tables. */
	Dwarf_Unsigned	ns_len;		/* Length of the pair array. */
};

struct _Dwarf_Fde {
	Dwarf_Debug	fde_dbg;	/* Ptr to containing dbg. */
	Dwarf_Cie	fde_cie;	/* Ptr to associated CIE. */
	Dwarf_FrameSec	fde_fs;		/* Ptr to containing .debug_frame. */
	Dwarf_Ptr	fde_addr;	/* Ptr to start of the FDE. */
	Dwarf_Unsigned	fde_offset;	/* Offset of the FDE. */
	Dwarf_Unsigned	fde_length;	/* Length of the FDE. */
	Dwarf_Unsigned	fde_cieoff;	/* Offset of associated CIE. */
	Dwarf_Unsigned	fde_initloc;	/* Initial location. */
	Dwarf_Unsigned	fde_adrange;	/* Address range. */
	Dwarf_Unsigned	fde_auglen;	/* Augmentation length. */
	uint8_t		*fde_augdata;	/* Augmentation data. */
	Dwarf_Ptr	fde_inst;	/* Instructions. */
	Dwarf_Unsigned	fde_instlen;	/* Length of instructions. */
	Dwarf_Unsigned	fde_symndx;	/* Symbol index for relocation. */
	Dwarf_Unsigned	fde_esymndx;	/* End symbol index for relocation. */
	Dwarf_Addr	fde_eoff;	/* Offset from the end symbol. */
	STAILQ_ENTRY(_Dwarf_Fde) fde_next; /* Next FDE in list. */
};

struct _Dwarf_Cie {
	Dwarf_Unsigned	cie_index;	/* Index of the CIE. */
	Dwarf_Unsigned	cie_offset;	/* Offset of the CIE. */
	Dwarf_Unsigned	cie_length;	/* Length of the CIE. */
	Dwarf_Half	cie_version;	/* CIE version. */
	uint8_t		*cie_augment;	/* CIE augmentation (UTF-8). */
	Dwarf_Unsigned	cie_ehdata;	/* Optional EH Data. */
	Dwarf_Unsigned	cie_caf;	/* Code alignment factor. */
	Dwarf_Signed	cie_daf;	/* Data alignment factor. */
	Dwarf_Unsigned	cie_ra;		/* Return address register. */
	Dwarf_Unsigned	cie_auglen;	/* Augmentation length. */
	uint8_t		*cie_augdata;	/* Augmentation data; */
	Dwarf_Ptr	cie_initinst;	/* Initial instructions. */
	Dwarf_Unsigned	cie_instlen;	/* Length of init instructions. */
	STAILQ_ENTRY(_Dwarf_Cie) cie_next;  /* Next CIE in list. */
};

struct _Dwarf_FrameSec {
	STAILQ_HEAD(, _Dwarf_Cie) fs_cielist; /* List of CIE. */
	STAILQ_HEAD(, _Dwarf_Fde) fs_fdelist; /* List of FDE. */
	Dwarf_Cie	*fs_ciearray;	/* Array of CIE. */
	Dwarf_Unsigned	fs_cielen;	/* Length of CIE array. */
	Dwarf_Fde	*fs_fdearray;	/* Array of FDE.*/
	Dwarf_Unsigned	fs_fdelen;	/* Length of FDE array. */
};

struct _Dwarf_Arange {
	Dwarf_ArangeSet	ar_as;		/* Ptr to the set it belongs to. */
	Dwarf_Unsigned	ar_address;	/* Start PC. */
	Dwarf_Unsigned	ar_range;	/* PC range. */
	Dwarf_Unsigned	ar_symndx;	/* First symbol index for reloc. */
	Dwarf_Unsigned	ar_esymndx;	/* Second symbol index for reloc. */
	Dwarf_Addr	ar_eoff;	/* Offset from second symbol. */
	STAILQ_ENTRY(_Dwarf_Arange) ar_next; /* Next arange in list. */
};

struct _Dwarf_ArangeSet {
	Dwarf_Unsigned	as_length;	/* Length of the arange set. */
	Dwarf_Half	as_version;	/* Version of the arange set. */
	Dwarf_Unsigned	as_cu_offset;	/* Offset of associated CU. */
	Dwarf_CU	as_cu;		/* Ptr to associated CU. */
	Dwarf_Small	as_addrsz;	/* Target address size. */
	Dwarf_Small	as_segsz;	/* Target segment size. */
	STAILQ_HEAD (, _Dwarf_Arange) as_arlist; /* List of ae entries. */
	STAILQ_ENTRY(_Dwarf_ArangeSet) as_next; /* Next set in list. */
};

struct _Dwarf_MacroSet {
	Dwarf_Macro_Details *ms_mdlist; /* Array of macinfo entries. */
	Dwarf_Unsigned	ms_cnt;		/* Length of the array. */
	STAILQ_ENTRY(_Dwarf_MacroSet) ms_next; /* Next set in list. */
};

struct _Dwarf_Rangelist {
	Dwarf_CU	rl_cu;		/* Ptr to associated CU. */
	Dwarf_Unsigned	rl_offset;	/* Offset of the rangelist. */
	Dwarf_Ranges	*rl_rgarray;	/* Array of ranges. */
	Dwarf_Unsigned	rl_rglen;	/* Length of the ranges array. */
	STAILQ_ENTRY(_Dwarf_Rangelist) rl_next; /* Next rangelist in list. */
};

struct _Dwarf_CU {
	Dwarf_Debug	cu_dbg;		/* Ptr to containing dbg. */
	uint64_t	cu_offset;	/* Offset to the this CU. */
	uint32_t	cu_length;	/* Length of CU data. */
	uint32_t	cu_header_length; /* Length of the CU header. */
	uint16_t	cu_version;	/* DWARF version. */
	uint64_t	cu_abbrev_offset; /* Offset into .debug_abbrev. */
	uint64_t	cu_abbrev_cnt;	/* Abbrev entry count. */
	uint64_t	cu_lineno_offset; /* Offset into .debug_lineno. */
	uint8_t		cu_pointer_size;/* Number of bytes in pointer. */
	uint64_t	cu_next_offset; /* Offset to the next CU. */
	int		cu_pass2;	/* Two pass DIE traverse. */
	Dwarf_LineInfo	cu_lineinfo;	/* Ptr to Dwarf_LineInfo. */
	STAILQ_HEAD(, _Dwarf_Abbrev) cu_abbrev;	/* List of abbrevs. */
	STAILQ_HEAD(, _Dwarf_Die) cu_die; /* List of dies. */
	STAILQ_HEAD(, _Dwarf_Die) cu_die_hash[DWARF_DIE_HASH_SIZE];
					/* Hash of dies. */
	STAILQ_ENTRY(_Dwarf_CU) cu_next; /* Next compilation unit. */
};

typedef struct _Dwarf_Section {
	const char	*ds_name;	/* Section name. */
	Dwarf_Small	*ds_data;	/* Section data. */
	Dwarf_Unsigned	ds_size;	/* Section size. */
	Dwarf_Unsigned	ds_cap;		/* Section capacity. */
	Dwarf_Unsigned	ds_ndx;		/* ELF section index. */
	STAILQ_ENTRY(_Dwarf_Section) ds_next; /* Next section in the list. */
} Dwarf_Section;

typedef struct {
	Elf		*eo_elf;
	GElf_Ehdr	eo_ehdr;
	GElf_Shdr	*eo_shdr;
	Elf_Data	**eo_data;
	Dwarf_Unsigned	eo_seccnt;
	size_t		eo_strndx;
	Dwarf_Obj_Access_Methods eo_methods;
} Dwarf_Elf_Object;

struct _Dwarf_Debug {
	Dwarf_Obj_Access_Interface *dbg_iface;
	Dwarf_Section	*dbg_section;
	Dwarf_Unsigned	dbg_seccnt;
	int		dbg_mode;	/* Access mode. */
	int		dbg_pointer_size; /* Object address size. */
	int		dbg_offset_size;  /* DWARF offset size. */
	STAILQ_HEAD(, _Dwarf_CU) dbg_cu;/* List of compilation units. */
	Dwarf_CU	dbg_cu_current; /* Ptr to the current CU. */
	TAILQ_HEAD(, _Dwarf_Loclist) dbg_loclist; /* List of location list. */
	Dwarf_NameSec	dbg_globals;	/* Ptr to pubnames lookup section. */
	Dwarf_NameSec	dbg_pubtypes;	/* Ptr to pubtypes lookup section. */
	Dwarf_NameSec	dbg_weaks;	/* Ptr to weaknames lookup section. */
	Dwarf_NameSec	dbg_funcs;	/* Ptr to static funcs lookup sect. */
	Dwarf_NameSec	dbg_vars;	/* Ptr to static vars lookup sect. */
	Dwarf_NameSec	dbg_types;	/* Ptr to types lookup section. */
	Dwarf_FrameSec	dbg_frame;	/* Ptr to .debug_frame section. */
	Dwarf_FrameSec	dbg_eh_frame;	/* Ptr to .eh_frame section. */
	STAILQ_HEAD(, _Dwarf_ArangeSet) dbg_aslist; /* List of arange set. */
	Dwarf_Arange	*dbg_arange_array; /* Array of arange. */
	Dwarf_Unsigned	dbg_arange_cnt;	/* Length of the arange array. */
	char		*dbg_strtab;
	Dwarf_Unsigned	dbg_strtab_cap;
	Dwarf_Unsigned	dbg_strtab_size;
	STAILQ_HEAD(, _Dwarf_MacroSet) dbg_mslist; /* List of macro set. */
	STAILQ_HEAD(, _Dwarf_Rangelist) dbg_rllist; /* List of rangelist. */
	uint64_t	(*read)(uint8_t *, uint64_t *, int);
	void		(*write)(uint8_t *, uint64_t *, uint64_t, int);
	int		(*write_alloc)(uint8_t **, uint64_t *, uint64_t *,
			    uint64_t, int, Dwarf_Error *);
	uint64_t	(*decode)(uint8_t **, int);

	Dwarf_Half	dbg_frame_rule_table_size;
	Dwarf_Half	dbg_frame_rule_initial_value;
	Dwarf_Half	dbg_frame_cfa_value;
	Dwarf_Half	dbg_frame_same_value;
	Dwarf_Half	dbg_frame_undefined_value;

	Dwarf_Regtable3	*dbg_internal_reg_table;

	/*
	 * Fields used by libdwarf producer.
	 */

	Dwarf_Unsigned	dbgp_flags;
	Dwarf_Callback_Func dbgp_func;
	Dwarf_Callback_Func_b dbgp_func_b;
	Dwarf_Die	dbgp_root_die;
	STAILQ_HEAD(, _Dwarf_Die) dbgp_dielist;
	STAILQ_HEAD(, _Dwarf_P_Expr) dbgp_pelist;
	Dwarf_LineInfo	dbgp_lineinfo;
	Dwarf_ArangeSet dbgp_as;
	Dwarf_Macro_Details *dbgp_mdlist;
	Dwarf_Unsigned	dbgp_mdcnt;
	STAILQ_HEAD(, _Dwarf_Cie) dbgp_cielist;
	STAILQ_HEAD(, _Dwarf_Fde) dbgp_fdelist;
	Dwarf_Unsigned	dbgp_cielen;
	Dwarf_Unsigned	dbgp_fdelen;
	STAILQ_HEAD(, _Dwarf_Section) dbgp_seclist;
	Dwarf_Unsigned	dbgp_seccnt;
	Dwarf_Section	*dbgp_secpos;
	Dwarf_Section	*dbgp_info;
};

/*
 * Internal function prototypes.
 */

int		_dwarf_abbrev_add(Dwarf_CU, uint64_t, uint64_t, uint8_t,
		    uint64_t, Dwarf_Abbrev *, Dwarf_Error *);
int		_dwarf_abbrev_init(Dwarf_Debug, Dwarf_CU, Dwarf_Error *);
Dwarf_Abbrev	_dwarf_abbrev_find(Dwarf_CU, uint64_t);
int		_dwarf_abbrev_gen(Dwarf_P_Debug, Dwarf_Error *);
int		_dwarf_alloc(Dwarf_Debug *, int, Dwarf_Error *);
void		_dwarf_arange_cleanup(Dwarf_Debug);
int		_dwarf_arange_init(Dwarf_Debug, Dwarf_Section *, Dwarf_Error *);
int		_dwarf_attr_alloc(Dwarf_Die, Dwarf_Attribute *, Dwarf_Error *);
Dwarf_Attribute	_dwarf_attr_find(Dwarf_Die, Dwarf_Half);
int		_dwarf_attr_gen(Dwarf_P_Debug, Dwarf_Section *, Dwarf_CU,
		    Dwarf_Die, int, Dwarf_Error *);
int		_dwarf_attr_init(Dwarf_Debug, Dwarf_Section *, uint64_t *, int,
		    Dwarf_CU, Dwarf_Die, Dwarf_AttrDef, uint64_t, int,
		    Dwarf_Error *);
int		_dwarf_attrdef_add(Dwarf_Abbrev, uint64_t, uint64_t, uint64_t,
		    Dwarf_AttrDef *, Dwarf_Error *);
uint64_t	_dwarf_decode_lsb(uint8_t **, int);
uint64_t	_dwarf_decode_msb(uint8_t **, int);
int64_t		_dwarf_decode_sleb128(uint8_t **);
uint64_t	_dwarf_decode_uleb128(uint8_t **);
void		_dwarf_deinit(Dwarf_Debug);
int		_dwarf_die_add(Dwarf_CU, uint64_t, uint64_t, Dwarf_Abbrev,
		    Dwarf_Die *, Dwarf_Error *);
int		_dwarf_die_alloc(Dwarf_Die *, Dwarf_Error *);
int		_dwarf_die_count_links(Dwarf_P_Die, Dwarf_P_Die,
		    Dwarf_P_Die, Dwarf_P_Die);
Dwarf_Die	_dwarf_die_find(Dwarf_Die, Dwarf_Unsigned);
int		_dwarf_die_gen(Dwarf_P_Debug, Dwarf_CU, Dwarf_Error *);
void		_dwarf_die_link(Dwarf_P_Die, Dwarf_P_Die, Dwarf_P_Die,
		    Dwarf_P_Die, Dwarf_P_Die);
int		_dwarf_die_parse(Dwarf_Debug, Dwarf_Section *, Dwarf_CU, int,
		    uint64_t, uint64_t, Dwarf_Error *);
void		_dwarf_elf_deinit(Dwarf_Debug);
int		_dwarf_elf_init(Dwarf_Debug, Elf *, Dwarf_Error *);
int		_dwarf_elf_load_section(void *, Dwarf_Half, Dwarf_Small **,
		    int *);
Dwarf_Endianness _dwarf_elf_get_byte_order(void *);
Dwarf_Small	_dwarf_elf_get_length_size(void *);
Dwarf_Small	_dwarf_elf_get_pointer_size(void *);
Dwarf_Unsigned	_dwarf_elf_get_section_count(void *);
int		_dwarf_elf_get_section_info(void *, Dwarf_Half,
		    Dwarf_Obj_Access_Section *, int *);
Dwarf_Section	*_dwarf_find_section(Dwarf_Debug, const char *);
int		_dwarf_generate_sections(Dwarf_P_Debug, Dwarf_Error *);
int		_dwarf_info_gen(Dwarf_P_Debug, Dwarf_Error *);
int		_dwarf_info_init(Dwarf_Debug, Dwarf_Section *, Dwarf_Error *);
int		_dwarf_init(Dwarf_Debug, Dwarf_Unsigned, Dwarf_Error *);
Dwarf_Unsigned	_dwarf_pro_callback(Dwarf_P_Debug, const char *, int,
		    Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Unsigned,
		    Dwarf_Unsigned, int *);
uint64_t	_dwarf_read_lsb(uint8_t *, uint64_t *, int);
uint64_t	_dwarf_read_msb(uint8_t *, uint64_t *, int);
int64_t		_dwarf_read_sleb128(uint8_t *, uint64_t *);
uint64_t	_dwarf_read_uleb128(uint8_t *, uint64_t *);
char		*_dwarf_read_string(void *, Dwarf_Unsigned, uint64_t *);
uint8_t		*_dwarf_read_block(void *, uint64_t *, uint64_t);
void		_dwarf_write_lsb(uint8_t *, uint64_t *, uint64_t, int);
int		_dwarf_write_lsb_alloc(uint8_t **, uint64_t *, uint64_t *,
		    uint64_t, int, Dwarf_Error *);
void		_dwarf_write_msb(uint8_t *, uint64_t *, uint64_t, int);
int		_dwarf_write_msb_alloc(uint8_t **, uint64_t *, uint64_t *,
		    uint64_t, int, Dwarf_Error *);
void		_dwarf_frame_cleanup(Dwarf_Debug);
void		_dwarf_frame_free_fop(Dwarf_Frame_Op *, Dwarf_Unsigned);
int		_dwarf_frame_get_fop(Dwarf_Debug, uint8_t *, Dwarf_Unsigned,
		    Dwarf_Frame_Op **, Dwarf_Signed *, Dwarf_Error *);
int		_dwarf_frame_get_internal_table(Dwarf_Fde, Dwarf_Addr,
		    Dwarf_Regtable3 **, Dwarf_Addr *, Dwarf_Error *);
int		_dwarf_frame_init(Dwarf_Debug, Dwarf_Error *);
int		_dwarf_frame_regtable_copy(Dwarf_Debug, Dwarf_Regtable3 **,
		    Dwarf_Regtable3 *, Dwarf_Error *);
int		_dwarf_lineno_init(Dwarf_Die, uint64_t, Dwarf_Error *);
int		_dwarf_loc_fill_locdesc(Dwarf_Debug, Dwarf_Locdesc *, uint8_t *,
		    uint64_t, uint8_t, Dwarf_Error *);
int		_dwarf_loc_fill_locexpr(Dwarf_Debug, Dwarf_Locdesc **,
		    uint8_t *, uint64_t, uint8_t, Dwarf_Error *);
int		_dwarf_loc_add(Dwarf_Die, Dwarf_Attribute, Dwarf_Error *);
int		_dwarf_loc_expr_add_atom(Dwarf_Debug, uint8_t *, uint8_t *,
		    Dwarf_Small, Dwarf_Unsigned, Dwarf_Unsigned, int *,
		    Dwarf_Error *);
int		_dwarf_loclist_find(Dwarf_Debug, uint64_t, Dwarf_Loclist *);
int		_dwarf_loclist_add(Dwarf_Debug, Dwarf_CU, uint64_t,
		    Dwarf_Error *);
void		_dwarf_loclist_cleanup(Dwarf_Loclist);
void		_dwarf_macinfo_cleanup(Dwarf_Debug);
int		_dwarf_macinfo_init(Dwarf_Debug, Dwarf_Section *,
		    Dwarf_Error *);
int		_dwarf_nametbl_init(Dwarf_Debug, Dwarf_NameSec *,
		    Dwarf_Section *, Dwarf_Error *);
void		_dwarf_nametbl_cleanup(Dwarf_NameSec);
int		_dwarf_ranges_add(Dwarf_Debug, Dwarf_CU, uint64_t,
		    Dwarf_Error *);
void		_dwarf_ranges_cleanup(Dwarf_Debug);
int		_dwarf_ranges_find(Dwarf_Debug, uint64_t, Dwarf_Rangelist *);
void		_dwarf_section_free(Dwarf_Debug, Dwarf_Section **);
int		_dwarf_section_init(Dwarf_Debug, Dwarf_Section **, const char *,
		    Dwarf_Error *);
int		_dwarf_strtab_add(Dwarf_Debug, char *, uint64_t *,
		    Dwarf_Error *);
void		_dwarf_strtab_cleanup(Dwarf_Debug);
int		_dwarf_strtab_gen(Dwarf_P_Debug, Dwarf_Error *);
char		*_dwarf_strtab_get_table(Dwarf_Debug);
int		_dwarf_strtab_init(Dwarf_Debug, Dwarf_Error *);
int		_dwarf_write_sleb128(uint8_t *, uint8_t *, int64_t);
int		_dwarf_write_sleb128_alloc(uint8_t **, uint64_t *, uint64_t *,
		    int64_t, Dwarf_Error *);
int		_dwarf_write_uleb128(uint8_t *, uint8_t *, uint64_t);
int		_dwarf_write_uleb128_alloc(uint8_t **, uint64_t *, uint64_t *,
		    uint64_t, Dwarf_Error *);

#endif /* !__LIBDWARF_H_ */
