/*-
 * Copyright (c) 2010 Joseph Koshy
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
 * $Id$
 */

/*
 *
 * These definitions are based on the public specification of the ELF
 * format defined in the October 2009 draft of System V ABI.
 *
 * See: http://www.sco.com/developers/gabi/latest/ch4.intro.html
 *
 */

#ifndef _ELFDEFINITIONS_H_
#define _ELFDEFINITIONS_H_

#include <stdint.h>

/*
 * Types of capabilities.
 */

#define	_ELF_DEFINE_CAPABILITIES()				\
_ELF_DEFINE_CA(CA_SUNW_NULL,	0,	"ignored")		\
_ELF_DEFINE_CA(CA_SUNW_HW_1,	1,	"hardware capability")	\
_ELF_DEFINE_CA(CA_SUNW_SW_1,	2,	"software capability")

#undef	_ELF_DEFINE_CA
#define	_ELF_DEFINE_CA(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_CAPABILITIES() };

/*
 * Flags used with dynamic linking entries.
 */

#define	_ELF_DEFINE_DYN_FLAGS()					\
_ELF_DEFINE_DF(DF_ORIGIN,           0x1,			\
	"object being loaded may refer to $ORIGIN")		\
_ELF_DEFINE_DF(DF_SYMBOLIC,         0x2,			\
	"search library for references before executable")	\
_ELF_DEFINE_DF(DF_TEXTREL,          0x4,			\
	"relocation entries may modify text segment")		\
_ELF_DEFINE_DF(DF_BIND_NOW,         0x8,			\
	"process relocation entries at load time")		\
_ELF_DEFINE_DF(DF_STATIC_TLS,       0x10,			\
	"uses static thread-local storage")
#undef	_ELF_DEFINE_DF
#define	_ELF_DEFINE_DF(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_DYN_FLAGS() };


/*
 * Dynamic linking entry types.
 */

#define	_ELF_DEFINE_DYN_TYPES()						\
_ELF_DEFINE_DT(DT_NULL,             0, "end of array")			\
_ELF_DEFINE_DT(DT_NEEDED,           1, "names a needed library")	\
_ELF_DEFINE_DT(DT_PLTRELSZ,         2,					\
	"size in bytes of associated relocation entries")		\
_ELF_DEFINE_DT(DT_PLTGOT,           3,					\
	"address associated with the procedure linkage table")		\
_ELF_DEFINE_DT(DT_HASH,             4,					\
	"address of the symbol hash table")				\
_ELF_DEFINE_DT(DT_STRTAB,           5,					\
	"address of the string table")					\
_ELF_DEFINE_DT(DT_SYMTAB,           6,					\
	"address of the symbol table")					\
_ELF_DEFINE_DT(DT_RELA,             7,					\
	"address of the relocation table")				\
_ELF_DEFINE_DT(DT_RELASZ,           8, "size of the DT_RELA table")	\
_ELF_DEFINE_DT(DT_RELAENT,          9, "size of each DT_RELA entry")	\
_ELF_DEFINE_DT(DT_STRSZ,            10, "size of the string table")	\
_ELF_DEFINE_DT(DT_SYMENT,           11,					\
	"size of a symbol table entry")					\
_ELF_DEFINE_DT(DT_INIT,             12,					\
	"address of the initialization function")			\
_ELF_DEFINE_DT(DT_FINI,             13,					\
	"address of the finalization function")				\
_ELF_DEFINE_DT(DT_SONAME,           14, "names the shared object")	\
_ELF_DEFINE_DT(DT_RPATH,            15,					\
	"runtime library search path")					\
_ELF_DEFINE_DT(DT_SYMBOLIC,         16,					\
	"alter symbol resolution algorithm")				\
_ELF_DEFINE_DT(DT_REL,              17,					\
	"address of the DT_REL table")					\
_ELF_DEFINE_DT(DT_RELSZ,            18, "size of the DT_REL table")	\
_ELF_DEFINE_DT(DT_RELENT,           19, "size of each DT_REL entry")	\
_ELF_DEFINE_DT(DT_PLTREL,           20,					\
	"type of relocation entry in the procedure linkage table")	\
_ELF_DEFINE_DT(DT_DEBUG,            21, "used for debugging")		\
_ELF_DEFINE_DT(DT_TEXTREL,          22,					\
	"text segment may be written to during relocation")		\
_ELF_DEFINE_DT(DT_JMPREL,           23,					\
	"address of relocation entries associated with the procedure linkage table") \
_ELF_DEFINE_DT(DT_BIND_NOW,         24,					\
	"bind symbols at loading time")					\
_ELF_DEFINE_DT(DT_INIT_ARRAY,       25,					\
	"pointers to initialization functions")				\
_ELF_DEFINE_DT(DT_FINI_ARRAY,       26,					\
	"pointers to termination functions")				\
_ELF_DEFINE_DT(DT_INIT_ARRAYSZ,     27, "size of the DT_INIT_ARRAY")	\
_ELF_DEFINE_DT(DT_FINI_ARRAYSZ,     28, "size of the DT_FINI_ARRAY")	\
_ELF_DEFINE_DT(DT_RUNPATH,          29,					\
	"index of library search path string")				\
_ELF_DEFINE_DT(DT_FLAGS,            30,					\
	"flags specific to the object being loaded")			\
_ELF_DEFINE_DT(DT_ENCODING,         32, "standard semantics")		\
_ELF_DEFINE_DT(DT_PREINIT_ARRAY,    32,					\
	"pointers to pre-initialization functions")			\
_ELF_DEFINE_DT(DT_PREINIT_ARRAYSZ,  33,					\
	"size of pre-initialization array")				\
_ELF_DEFINE_DT(DT_LOOS,             0x6000000D,				\
    "start of OS-specific types")					\
_ELF_DEFINE_DT(DT_HIOS,             0x6ffff000,				\
    "end of OS-specific types")						\
_ELF_DEFINE_DT(DT_LOPROC,           0x70000000,				\
	"start of processor-specific types")				\
_ELF_DEFINE_DT(DT_HIPROC,           0x7fffffff,				\
	"end of processor-specific types")

#undef	_ELF_DEFINE_DT
#define	_ELF_DEFINE_DT(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_DYN_TYPES() };

/*
 * Offsets in the `ei_ident[]` field of an ELF executable header.
 */
#define	_ELF_DEFINE_EI_OFFSETS()			\
_ELF_DEFINE_EI(EI_MAG0,     0, "magic number")		\
_ELF_DEFINE_EI(EI_MAG1,     1, "magic number")		\
_ELF_DEFINE_EI(EI_MAG2,     2, "magic number")		\
_ELF_DEFINE_EI(EI_MAG3,     3, "magic number")		\
_ELF_DEFINE_EI(EI_CLASS,    4, "file class")		\
_ELF_DEFINE_EI(EI_DATA,     5, "data encoding")		\
_ELF_DEFINE_EI(EI_VERSION,  6, "file version")		\
_ELF_DEFINE_EI(EI_OSABI,    7, "OS ABI kind")		\
_ELF_DEFINE_EI(EI_ABIVERSION, 8, "OS ABI version")	\
_ELF_DEFINE_EI(EI_PAD,	    9, "padding start")		\
_ELF_DEFINE_EI(EI_NIDENT,  16, "total size")

#undef	_ELF_DEFINE_EI
#define	_ELF_DEFINE_EI(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_EI_OFFSETS() };

/*
 * The ELF class of an object.
 */
#define	_ELF_DEFINE_ELFCLASS()				\
_ELF_DEFINE_EC(ELFCLASSNONE, 0, "Unknown ELF class")	\
_ELF_DEFINE_EC(ELFCLASS32,   1, "32 bit objects")	\
_ELF_DEFINE_EC(ELFCLASS64,   2, "64 bit objects")

#undef	_ELF_DEFINE_EC
#define	_ELF_DEFINE_EC(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_ELFCLASS() };

/*
 * Endianness of data in an ELF object.
 */

#define	_ELF_DEFINE_ELF_DATA_ENDIANNESS()			\
_ELF_DEFINE_ED(ELFDATANONE, 0, "Unknown data endianness")	\
_ELF_DEFINE_ED(ELFDATA2LSB, 1, "little endian")			\
_ELF_DEFINE_ED(ELFDATA2MSB, 2, "big endian")

#undef	_ELF_DEFINE_ED
#define	_ELF_DEFINE_ED(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_ELF_DATA_ENDIANNESS() };

/*
 * Values of the magic numbers used in identification array.
 */
#define	_ELF_DEFINE_ELF_MAGIC()			\
_ELF_DEFINE_EMAG(ELFMAG0, 0x7FU)		\
_ELF_DEFINE_EMAG(ELFMAG1, 'E')			\
_ELF_DEFINE_EMAG(ELFMAG2, 'L')			\
_ELF_DEFINE_EMAG(ELFMAG3, 'F')

#undef	_ELF_DEFINE_EMAG
#define	_ELF_DEFINE_EMAG(N, V)		N = V ,
enum { _ELF_DEFINE_ELF_MAGIC() };

/*
 * ELF OS ABI field.
 */
#define	_ELF_DEFINE_ELF_OSABI()						\
_ELF_DEFINE_EABI(ELFOSABI_NONE,       0,				\
	"No extensions or unspecified")					\
_ELF_DEFINE_EABI(ELFOSABI_HPUX,       1, "Hewlett-Packard HP-UX")	\
_ELF_DEFINE_EABI(ELFOSABI_NETBSD,     2, "NetBSD")			\
_ELF_DEFINE_EABI(ELFOSABI_LINUX,      3, "Linux")			\
_ELF_DEFINE_EABI(ELFOSABI_SOLARIS,    6, "Sun Solaris")			\
_ELF_DEFINE_EABI(ELFOSABI_AIX,        7, "AIX")				\
_ELF_DEFINE_EABI(ELFOSABI_IRIX,       8, "IRIX")			\
_ELF_DEFINE_EABI(ELFOSABI_FREEBSD,    9, "FreeBSD")			\
_ELF_DEFINE_EABI(ELFOSABI_TRU64,      10, "Compaq TRU64 UNIX")		\
_ELF_DEFINE_EABI(ELFOSABI_MODESTO,    11, "Novell Modesto")		\
_ELF_DEFINE_EABI(ELFOSABI_OPENBSD,    12, "Open BSD")			\
_ELF_DEFINE_EABI(ELFOSABI_OPENVMS,    13, "Open VMS")			\
_ELF_DEFINE_EABI(ELFOSABI_NSK,        14,				\
	"Hewlett-Packard Non-Stop Kernel")				\
_ELF_DEFINE_EABI(ELFOSABI_AROS,       15, "Amiga Research OS")		\
_ELF_DEFINE_EABI(ELFOSABI_FENIXOS,    16,				\
	"The FenixOS highly scalable multi-core OS")

#undef	_ELF_DEFINE_EABI
#define	_ELF_DEFINE_EABI(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_ELF_OSABI() };

/*
 * ELF Machine types: (EM_*).
 */
#define	_ELF_DEFINE_ELF_MACHINES()					\
_ELF_DEFINE_EM(EM_NONE,             0, "No machine")			\
_ELF_DEFINE_EM(EM_M32,              1, "AT&T WE 32100")			\
_ELF_DEFINE_EM(EM_SPARC,            2, "SPARC")				\
_ELF_DEFINE_EM(EM_386,              3, "Intel 80386")			\
_ELF_DEFINE_EM(EM_68K,              4, "Motorola 68000")		\
_ELF_DEFINE_EM(EM_88K,              5, "Motorola 88000")		\
_ELF_DEFINE_EM(EM_860,              7, "Intel 80860")			\
_ELF_DEFINE_EM(EM_MIPS,             8, "MIPS I Architecture")		\
_ELF_DEFINE_EM(EM_S370,             9, "IBM System/370 Processor")	\
_ELF_DEFINE_EM(EM_MIPS_RS3_LE,      10, "MIPS RS3000 Little-endian")	\
_ELF_DEFINE_EM(EM_PARISC,           15, "Hewlett-Packard PA-RISC")	\
_ELF_DEFINE_EM(EM_VPP500,           17, "Fujitsu VPP500")		\
_ELF_DEFINE_EM(EM_SPARC32PLUS,      18,					\
	"Enhanced instruction set SPARC")				\
_ELF_DEFINE_EM(EM_960,              19, "Intel 80960")			\
_ELF_DEFINE_EM(EM_PPC,              20, "PowerPC")			\
_ELF_DEFINE_EM(EM_PPC64,            21, "64-bit PowerPC")		\
_ELF_DEFINE_EM(EM_S390,             22, "IBM System/390 Processor")	\
_ELF_DEFINE_EM(EM_SPU,              23, "IBM SPU/SPC")			\
_ELF_DEFINE_EM(EM_V800,             36, "NEC V800")			\
_ELF_DEFINE_EM(EM_FR20,             37, "Fujitsu FR20")			\
_ELF_DEFINE_EM(EM_RH32,             38, "TRW RH-32")			\
_ELF_DEFINE_EM(EM_RCE,              39, "Motorola RCE")			\
_ELF_DEFINE_EM(EM_ARM,              40, "Advanced RISC Machines ARM")	\
_ELF_DEFINE_EM(EM_ALPHA,            41, "Digital Alpha")		\
_ELF_DEFINE_EM(EM_SH,               42, "Hitachi SH")			\
_ELF_DEFINE_EM(EM_SPARCV9,          43, "SPARC Version 9")		\
_ELF_DEFINE_EM(EM_TRICORE,          44,					\
	"Siemens TriCore embedded processor")				\
_ELF_DEFINE_EM(EM_ARC,              45,					\
	"Argonaut RISC Core, Argonaut Technologies Inc.")		\
_ELF_DEFINE_EM(EM_H8_300,           46, "Hitachi H8/300")		\
_ELF_DEFINE_EM(EM_H8_300H,          47, "Hitachi H8/300H")		\
_ELF_DEFINE_EM(EM_H8S,              48, "Hitachi H8S")			\
_ELF_DEFINE_EM(EM_H8_500,           49, "Hitachi H8/500")		\
_ELF_DEFINE_EM(EM_IA_64,            50,					\
	"Intel IA-64 processor architecture")				\
_ELF_DEFINE_EM(EM_MIPS_X,           51, "Stanford MIPS-X")		\
_ELF_DEFINE_EM(EM_COLDFIRE,         52, "Motorola ColdFire")		\
_ELF_DEFINE_EM(EM_68HC12,           53, "Motorola M68HC12")		\
_ELF_DEFINE_EM(EM_MMA,              54,					\
	"Fujitsu MMA Multimedia Accelerator")				\
_ELF_DEFINE_EM(EM_PCP,              55, "Siemens PCP")			\
_ELF_DEFINE_EM(EM_NCPU,             56,					\
	"Sony nCPU embedded RISC processor")				\
_ELF_DEFINE_EM(EM_NDR1,             57, "Denso NDR1 microprocessor")	\
_ELF_DEFINE_EM(EM_STARCORE,         58, "Motorola Star*Core processor")	\
_ELF_DEFINE_EM(EM_ME16,             59, "Toyota ME16 processor")	\
_ELF_DEFINE_EM(EM_ST100,            60,					\
	"STMicroelectronics ST100 processor")				\
_ELF_DEFINE_EM(EM_TINYJ,            61,					\
	"Advanced Logic Corp. TinyJ embedded processor family")		\
_ELF_DEFINE_EM(EM_X86_64,           62, "AMD x86-64 architecture")	\
_ELF_DEFINE_EM(EM_PDSP,             63, "Sony DSP Processor")		\
_ELF_DEFINE_EM(EM_PDP10,            64,					\
	"Digital Equipment Corp. PDP-10")				\
_ELF_DEFINE_EM(EM_PDP11,            65,					\
	"Digital Equipment Corp. PDP-11")				\
_ELF_DEFINE_EM(EM_FX66,             66, "Siemens FX66 microcontroller")	\
_ELF_DEFINE_EM(EM_ST9PLUS,          67,					\
	"STMicroelectronics ST9+ 8/16 bit microcontroller")		\
_ELF_DEFINE_EM(EM_ST7,              68,					\
	"STMicroelectronics ST7 8-bit microcontroller")			\
_ELF_DEFINE_EM(EM_68HC16,           69,					\
	"Motorola MC68HC16 Microcontroller")				\
_ELF_DEFINE_EM(EM_68HC11,           70,					\
	"Motorola MC68HC11 Microcontroller")				\
_ELF_DEFINE_EM(EM_68HC08,           71,					\
	"Motorola MC68HC08 Microcontroller")				\
_ELF_DEFINE_EM(EM_68HC05,           72,					\
	"Motorola MC68HC05 Microcontroller")				\
_ELF_DEFINE_EM(EM_SVX,              73, "Silicon Graphics SVx")		\
_ELF_DEFINE_EM(EM_ST19,             74,					\
	"STMicroelectronics ST19 8-bit microcontroller")		\
_ELF_DEFINE_EM(EM_VAX,              75, "Digital VAX")			\
_ELF_DEFINE_EM(EM_CRIS,             76,					\
	"Axis Communications 32-bit embedded processor")		\
_ELF_DEFINE_EM(EM_JAVELIN,          77,					\
	"Infineon Technologies 32-bit embedded processor")		\
_ELF_DEFINE_EM(EM_FIREPATH,         78,					\
	"Element 14 64-bit DSP Processor")				\
_ELF_DEFINE_EM(EM_ZSP,              79,					\
	"LSI Logic 16-bit DSP Processor")				\
_ELF_DEFINE_EM(EM_MMIX,             80,					\
	"Donald Knuth's educational 64-bit processor")			\
_ELF_DEFINE_EM(EM_HUANY,            81,					\
	"Harvard University machine-independent object files")		\
_ELF_DEFINE_EM(EM_PRISM,            82, "SiTera Prism")			\
_ELF_DEFINE_EM(EM_AVR,              83,					\
	"Atmel AVR 8-bit microcontroller")				\
_ELF_DEFINE_EM(EM_FR30,             84, "Fujitsu FR30")			\
_ELF_DEFINE_EM(EM_D10V,             85, "Mitsubishi D10V")		\
_ELF_DEFINE_EM(EM_D30V,             86, "Mitsubishi D30V")		\
_ELF_DEFINE_EM(EM_V850,             87, "NEC v850")			\
_ELF_DEFINE_EM(EM_M32R,             88, "Mitsubishi M32R")		\
_ELF_DEFINE_EM(EM_MN10300,          89, "Matsushita MN10300")		\
_ELF_DEFINE_EM(EM_MN10200,          90, "Matsushita MN10200")		\
_ELF_DEFINE_EM(EM_PJ,               91, "picoJava")			\
_ELF_DEFINE_EM(EM_OPENRISC,         92,					\
	"OpenRISC 32-bit embedded processor")				\
_ELF_DEFINE_EM(EM_ARC_COMPACT,      93,					\
	"ARC International ARCompact processor")			\
_ELF_DEFINE_EM(EM_XTENSA,           94,					\
	"Tensilica Xtensa Architecture")				\
_ELF_DEFINE_EM(EM_VIDEOCORE,        95,					\
	"Alphamosaic VideoCore processor")				\
_ELF_DEFINE_EM(EM_TMM_GPP,          96,					\
	"Thompson Multimedia General Purpose Processor")		\
_ELF_DEFINE_EM(EM_NS32K,            97,					\
	"National Semiconductor 32000 series")				\
_ELF_DEFINE_EM(EM_TPC,              98, "Tenor Network TPC processor")	\
_ELF_DEFINE_EM(EM_SNP1K,            99, "Trebia SNP 1000 processor")	\
_ELF_DEFINE_EM(EM_ST200,            100,				\
	"STMicroelectronics (www.st.com) ST200 microcontroller")	\
_ELF_DEFINE_EM(EM_IP2K,             101,				\
	"Ubicom IP2xxx microcontroller family")				\
_ELF_DEFINE_EM(EM_MAX,              102, "MAX Processor")		\
_ELF_DEFINE_EM(EM_CR,               103,				\
	"National Semiconductor CompactRISC microprocessor")		\
_ELF_DEFINE_EM(EM_F2MC16,           104, "Fujitsu F2MC16")		\
_ELF_DEFINE_EM(EM_MSP430,           105,				\
	"Texas Instruments embedded microcontroller msp430")		\
_ELF_DEFINE_EM(EM_BLACKFIN,         106,				\
	"Analog Devices Blackfin (DSP) processor")			\
_ELF_DEFINE_EM(EM_SE_C33,           107,				\
	"S1C33 Family of Seiko Epson processors")			\
_ELF_DEFINE_EM(EM_SEP,              108,				\
	"Sharp embedded microprocessor")				\
_ELF_DEFINE_EM(EM_ARCA,             109, "Arca RISC Microprocessor")	\
_ELF_DEFINE_EM(EM_UNICORE,          110,				\
	"Microprocessor series from PKU-Unity Ltd. and MPRC of Peking University") \
_ELF_DEFINE_EM(EM_EXCESS,           111,				\
	"eXcess: 16/32/64-bit configurable embedded CPU")		\
_ELF_DEFINE_EM(EM_DXP,              112,				\
	"Icera Semiconductor Inc. Deep Execution Processor")		\
_ELF_DEFINE_EM(EM_ALTERA_NIOS2,     113,				\
	"Altera Nios II soft-core processor")				\
_ELF_DEFINE_EM(EM_CRX,              114,				\
	"National Semiconductor CompactRISC CRX microprocessor")	\
_ELF_DEFINE_EM(EM_XGATE,            115,				\
	"Motorola XGATE embedded processor")				\
_ELF_DEFINE_EM(EM_C166,             116,				\
	"Infineon C16x/XC16x processor")				\
_ELF_DEFINE_EM(EM_M16C,             117,				\
	"Renesas M16C series microprocessors")				\
_ELF_DEFINE_EM(EM_DSPIC30F,         118,				\
	"Microchip Technology dsPIC30F Digital Signal Controller")	\
_ELF_DEFINE_EM(EM_CE,               119,				\
	"Freescale Communication Engine RISC core")			\
_ELF_DEFINE_EM(EM_M32C,             120,				\
	"Renesas M32C series microprocessors")				\
_ELF_DEFINE_EM(EM_TSK3000,          131, "Altium TSK3000 core")		\
_ELF_DEFINE_EM(EM_RS08,             132,				\
	"Freescale RS08 embedded processor")				\
_ELF_DEFINE_EM(EM_ECOG2,            134,				\
	"Cyan Technology eCOG2 microprocessor")				\
_ELF_DEFINE_EM(EM_SCORE7,           135,				\
	"Sunplus S+core7 RISC processor")				\
_ELF_DEFINE_EM(EM_DSP24,            136,				\
	"New Japan Radio (NJR) 24-bit DSP Processor")			\
_ELF_DEFINE_EM(EM_VIDEOCORE3,       137,				\
	"Broadcom VideoCore III processor")				\
_ELF_DEFINE_EM(EM_LATTICEMICO32,    138,				\
	"RISC processor for Lattice FPGA architecture")			\
_ELF_DEFINE_EM(EM_SE_C17,           139, "Seiko Epson C17 family")	\
_ELF_DEFINE_EM(EM_TI_C6000,         140,				\
	"The Texas Instruments TMS320C6000 DSP family")			\
_ELF_DEFINE_EM(EM_TI_C2000,         141,				\
	"The Texas Instruments TMS320C2000 DSP family")			\
_ELF_DEFINE_EM(EM_TI_C5500,         142,				\
	"The Texas Instruments TMS320C55x DSP family")			\
_ELF_DEFINE_EM(EM_MMDSP_PLUS,       160,				\
	"STMicroelectronics 64bit VLIW Data Signal Processor")		\
_ELF_DEFINE_EM(EM_CYPRESS_M8C,      161, "Cypress M8C microprocessor")	\
_ELF_DEFINE_EM(EM_R32C,             162,				\
	"Renesas R32C series microprocessors")				\
_ELF_DEFINE_EM(EM_TRIMEDIA,         163,				\
	"NXP Semiconductors TriMedia architecture family")		\
_ELF_DEFINE_EM(EM_QDSP6,            164, "QUALCOMM DSP6 Processor")	\
_ELF_DEFINE_EM(EM_8051,             165, "Intel 8051 and variants")	\
_ELF_DEFINE_EM(EM_STXP7X,           166,				\
	"STMicroelectronics STxP7x family of configurable and extensible RISC processors") \
_ELF_DEFINE_EM(EM_NDS32,            167,				\
	"Andes Technology compact code size embedded RISC processor family") \
_ELF_DEFINE_EM(EM_ECOG1,            168,				\
	"Cyan Technology eCOG1X family")				\
_ELF_DEFINE_EM(EM_ECOG1X,           168,				\
	"Cyan Technology eCOG1X family")				\
_ELF_DEFINE_EM(EM_MAXQ30,           169,				\
	"Dallas Semiconductor MAXQ30 Core Micro-controllers")		\
_ELF_DEFINE_EM(EM_XIMO16,           170,				\
	"New Japan Radio (NJR) 16-bit DSP Processor")			\
_ELF_DEFINE_EM(EM_MANIK,            171,				\
	"M2000 Reconfigurable RISC Microprocessor")			\
_ELF_DEFINE_EM(EM_CRAYNV2,          172,				\
	"Cray Inc. NV2 vector architecture")				\
_ELF_DEFINE_EM(EM_RX,               173, "Renesas RX family")		\
_ELF_DEFINE_EM(EM_METAG,            174,				\
	"Imagination Technologies META processor architecture")		\
_ELF_DEFINE_EM(EM_MCST_ELBRUS,      175,				\
	"MCST Elbrus general purpose hardware architecture")		\
_ELF_DEFINE_EM(EM_ECOG16,           176,				\
	"Cyan Technology eCOG16 family")				\
_ELF_DEFINE_EM(EM_CR16,             177,				\
	"National Semiconductor CompactRISC CR16 16-bit microprocessor") \
_ELF_DEFINE_EM(EM_ETPU,             178,				\
	"Freescale Extended Time Processing Unit")			\
_ELF_DEFINE_EM(EM_SLE9X,            179,				\
	"Infineon Technologies SLE9X core")				\
_ELF_DEFINE_EM(EM_AVR32,            185,				\
	"Atmel Corporation 32-bit microprocessor family")		\
_ELF_DEFINE_EM(EM_STM8,             186,				\
	"STMicroeletronics STM8 8-bit microcontroller")			\
_ELF_DEFINE_EM(EM_TILE64,           187,				\
	"Tilera TILE64 multicore architecture family")			\
_ELF_DEFINE_EM(EM_TILEPRO,          188,				\
	"Tilera TILEPro multicore architecture family")			\
_ELF_DEFINE_EM(EM_MICROBLAZE,       189,				\
	"Xilinx MicroBlaze 32-bit RISC soft processor core")		\
_ELF_DEFINE_EM(EM_CUDA,             190, "NVIDIA CUDA architecture")

#undef	_ELF_DEFINE_EM
#define	_ELF_DEFINE_EM(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_ELF_MACHINES() };


/*
 * ELF file types: (ET_*).
 */
#define	_ELF_DEFINE_ELF_TYPES()						\
_ELF_DEFINE_ET(ET_NONE,   0,	    "No file type")			\
_ELF_DEFINE_ET(ET_REL,    1, 	    "Relocatable object")		\
_ELF_DEFINE_ET(ET_EXEC,   2, 	    "Executable")			\
_ELF_DEFINE_ET(ET_DYN,    3, 	    "Shared object")			\
_ELF_DEFINE_ET(ET_CORE,   4, 	    "Core file")			\
_ELF_DEFINE_ET(ET_LOOS,   0xFE00U,  "Begin OS-specific range")		\
_ELF_DEFINE_ET(ET_HIOS,   0xFEFFU,  "End OS-specific range")		\
_ELF_DEFINE_ET(ET_LOPROC, 0xFF00U,  "Begin processor-specific range")	\
_ELF_DEFINE_ET(ET_HIPROC, 0xFFFFU,  "End processor-specific range")

#undef	_ELF_DEFINE_ET
#define	_ELF_DEFINE_ET(N, V, DESCR)	N = V ,
enum {	_ELF_DEFINE_ELF_TYPES() };

/* ELF file format version numbers. */
#define	EV_NONE		0
#define	EV_CURRENT	1

/*
 * Flags for section groups.
 */
#define	GRP_COMDAT 	0x1		/* COMDAT semantics */
#define	GRP_MASKOS 	0x0ff00000	/* OS-specific flags */
#define	GRP_MASKPROC 	0xf0000000	/* processor-specific flags */

/*
 * Flags used by program header table entries.
 */

#define	_ELF_DEFINE_PHDR_FLAGS()					\
_ELF_DEFINE_PF(PF_X,                0x1, "Execute")			\
_ELF_DEFINE_PF(PF_W,                0x2, "Write")			\
_ELF_DEFINE_PF(PF_R,                0x4, "Read")			\
_ELF_DEFINE_PF(PF_MASKOS,           0x0ff00000, "OS-specific flags")	\
_ELF_DEFINE_PF(PF_MASKPROC,         0xf0000000, "Processor-specific flags")

#undef	_ELF_DEFINE_PF
#define	_ELF_DEFINE_PF(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_PHDR_FLAGS() };

/*
 * Types of program header table entries.
 */

#define	_ELF_DEFINE_PHDR_TYPES()				\
_ELF_DEFINE_PT(PT_NULL,             0, "ignored entry")		\
_ELF_DEFINE_PT(PT_LOAD,             1, "loadable segment")	\
_ELF_DEFINE_PT(PT_DYNAMIC,          2,				\
	"contains dynamic linking information")			\
_ELF_DEFINE_PT(PT_INTERP,           3, "names an interpreter")	\
_ELF_DEFINE_PT(PT_NOTE,             4, "auxiliary information")	\
_ELF_DEFINE_PT(PT_SHLIB,            5, "reserved")		\
_ELF_DEFINE_PT(PT_PHDR,             6,				\
	"describes the program header itself")			\
_ELF_DEFINE_PT(PT_TLS,              7, "thread local storage")	\
_ELF_DEFINE_PT(PT_LOOS,             0x60000000,			\
    "start of OS-specific range")				\
_ELF_DEFINE_PT(PT_HIOS,             0x6fffffff,			\
    "end of OS-specific range")					\
_ELF_DEFINE_PT(PT_LOPROC,           0x70000000,			\
	"start of processor-specific range")			\
_ELF_DEFINE_PT(PT_HIPROC,           0x7fffffff,			\
	"end of processor-specific range")

#undef	_ELF_DEFINE_PT
#define	_ELF_DEFINE_PT(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_PHDR_TYPES() };


/*
 * Section flags.
 */

#define	_ELF_DEFINE_SECTION_FLAGS()					\
_ELF_DEFINE_SHF(SHF_WRITE,           0x1,				\
	"writable during program execution")				\
_ELF_DEFINE_SHF(SHF_ALLOC,           0x2,				\
	"occupies memory during program execution")			\
_ELF_DEFINE_SHF(SHF_EXECINSTR,       0x4, "executable instructions")	\
_ELF_DEFINE_SHF(SHF_MERGE,           0x10,				\
	"may be merged to prevent duplication")				\
_ELF_DEFINE_SHF(SHF_STRINGS,         0x20,				\
	"NUL-terminated character strings")				\
_ELF_DEFINE_SHF(SHF_INFO_LINK,       0x40,				\
	"the sh_info field holds a link")				\
_ELF_DEFINE_SHF(SHF_LINK_ORDER,      0x80,				\
	"special ordering requirements during linking")			\
_ELF_DEFINE_SHF(SHF_OS_NONCONFORMING, 0x100,				\
	"requires OS-specific processing during linking")		\
_ELF_DEFINE_SHF(SHF_GROUP,           0x200,				\
	"member of a section group")					\
_ELF_DEFINE_SHF(SHF_TLS,             0x400,				\
	"holds thread-local storage")					\
_ELF_DEFINE_SHF(SHF_MASKOS,          0x0ff00000,			\
	"bits reserved for OS-specific semantics")			\
_ELF_DEFINE_SHF(SHF_MASKPROC,        0xf0000000,			\
	"bits reserved for processor-specific semantics")

#undef	_ELF_DEFINE_SHF
#define	_ELF_DEFINE_SHF(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SECTION_FLAGS() };

/*
 * Special section indices.
 */
#define _ELF_DEFINE_SECTION_INDICES()					\
_ELF_DEFINE_SHN(SHN_UNDEF, 	0, 	 "undefined section")		\
_ELF_DEFINE_SHN(SHN_LORESERVE, 	0xFF00U, "start of reserved area")	\
_ELF_DEFINE_SHN(SHN_LOPROC, 	0XFF00U,				\
	"start of processor-specific range")				\
_ELF_DEFINE_SHN(SHN_HIPROC, 	0xFF1FU,				\
	"end of processor-specific range")				\
_ELF_DEFINE_SHN(SHN_LOOS, 	0xFF20U,				\
	"start of OS-specific range")					\
_ELF_DEFINE_SHN(SHN_HIOS, 	0xFF3FU,				\
	"end of OS-specific range")					\
_ELF_DEFINE_SHN(SHN_ABS, 	0xFFF1U, "absolute references")		\
_ELF_DEFINE_SHN(SHN_COMMON, 	0xFFF2U, "references to COMMON areas")	\
_ELF_DEFINE_SHN(SHN_XINDEX, 	0xFFFFU, "extended index")		\
_ELF_DEFINE_SHN(SHN_HIRESERVE, 	0xFFFFU, "end of reserved area")

#undef	_ELF_DEFINE_SHN
#define	_ELF_DEFINE_SHN(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SECTION_INDICES() };

/*
 * Section types.
 */

#define	_ELF_DEFINE_SECTION_TYPES()					\
_ELF_DEFINE_SHT(SHT_NULL,            0, "inactive header")		\
_ELF_DEFINE_SHT(SHT_PROGBITS,        1, "program defined information")	\
_ELF_DEFINE_SHT(SHT_SYMTAB,          2, "symbol table")			\
_ELF_DEFINE_SHT(SHT_STRTAB,          3, "string table")			\
_ELF_DEFINE_SHT(SHT_RELA,            4,					\
	"relocation entries with addends")				\
_ELF_DEFINE_SHT(SHT_HASH,            5, "symbol hash table")		\
_ELF_DEFINE_SHT(SHT_DYNAMIC,         6,					\
	"information for dynamic linking")				\
_ELF_DEFINE_SHT(SHT_NOTE,            7, "additional notes")		\
_ELF_DEFINE_SHT(SHT_NOBITS,          8, "section occupying no space")	\
_ELF_DEFINE_SHT(SHT_REL,             9,					\
	"relocation entries without addends")				\
_ELF_DEFINE_SHT(SHT_SHLIB,           10, "reserved")			\
_ELF_DEFINE_SHT(SHT_DYNSYM,          11, "symbol table")		\
_ELF_DEFINE_SHT(SHT_INIT_ARRAY,      14,				\
	"pointers to initialization functions")				\
_ELF_DEFINE_SHT(SHT_FINI_ARRAY,      15,				\
	"pointers to termination functions")				\
_ELF_DEFINE_SHT(SHT_PREINIT_ARRAY,   16,				\
	"pointers to functions called before initialization")		\
_ELF_DEFINE_SHT(SHT_GROUP,           17, "defines a section group")	\
_ELF_DEFINE_SHT(SHT_SYMTAB_SHNDX,    18,				\
	"used for extended section numbering")				\
_ELF_DEFINE_SHT(SHT_LOOS,            0x60000000,			\
	"start of OS-specific range")					\
_ELF_DEFINE_SHT(SHT_HIOS,            0x6fffffff,			\
	"end of OS-specific range")					\
_ELF_DEFINE_SHT(SHT_LOPROC,          0x70000000,			\
	"start of processor-specific range")				\
_ELF_DEFINE_SHT(SHT_HIPROC,          0x7fffffff,			\
	"end of processor-specific range")				\
_ELF_DEFINE_SHT(SHT_LOUSER,          0x80000000,			\
	"start of application-specific range")				\
_ELF_DEFINE_SHT(SHT_HIUSER,          0xffffffff,			\
	"end of application-specific range")

#undef	_ELF_DEFINE_SHT
#define	_ELF_DEFINE_SHT(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SECTION_TYPES() };

/*
 * Symbol binding information.
 */

#define	_ELF_DEFINE_SYMBOL_BINDING()					\
_ELF_DEFINE_STB(STB_LOCAL,           0,					\
	"not visible outside defining object file")			\
_ELF_DEFINE_STB(STB_GLOBAL,          1,					\
	"visible across all object files being combined")		\
_ELF_DEFINE_STB(STB_WEAK,            2,					\
	"visible across all object files but with low precedence")	\
_ELF_DEFINE_STB(STB_LOOS,            10, "start of OS-specific range")	\
_ELF_DEFINE_STB(STB_HIOS,            12, "end of OS-specific range")	\
_ELF_DEFINE_STB(STB_LOPROC,          13,				\
	"start of processor-specific range")				\
_ELF_DEFINE_STB(STB_HIPROC,          15,				\
	"end of processor-specific range")

#undef	_ELF_DEFINE_STB
#define	_ELF_DEFINE_STB(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SYMBOL_BINDING() };

/*
 * Symbol types
 */

#define	_ELF_DEFINE_SYMBOL_TYPES()					\
_ELF_DEFINE_STT(STT_NOTYPE,          0, "unspecified type")		\
_ELF_DEFINE_STT(STT_OBJECT,          1, "data object")			\
_ELF_DEFINE_STT(STT_FUNC,            2, "executable code")		\
_ELF_DEFINE_STT(STT_SECTION,         3, "section")			\
_ELF_DEFINE_STT(STT_FILE,            4, "source file")			\
_ELF_DEFINE_STT(STT_COMMON,          5, "uninitialized common block")	\
_ELF_DEFINE_STT(STT_TLS,             6, "thread local storage")		\
_ELF_DEFINE_STT(STT_LOOS,            10, "start of OS-specific types")	\
_ELF_DEFINE_STT(STT_HIOS,            12, "end of OS-specific types")	\
_ELF_DEFINE_STT(STT_LOPROC,          13,				\
	"start of processor-specific types")				\
_ELF_DEFINE_STT(STT_HIPROC,          15,				\
	"end of processor-specific types")

#undef	_ELF_DEFINE_STT
#define	_ELF_DEFINE_STT(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SYMBOL_TYPES() };

/*
 * Symbol binding.
 */

#define	_ELF_DEFINE_SYMBOL_BINDING()			\
_ELF_DEFINE_SYB(SYMINFO_BT_SELF,	0xFFFFU,	\
	"bound to self")				\
_ELF_DEFINE_SYB(SYMINFO_BT_PARENT,	0xFFFEU,	\
	"bound to parent")				\
_ELF_DEFINE_SYB(SYMINFO_BT_NONE,	0xFFFDU,	\
	"no special binding")

#undef	_ELF_DEFINE_SYB
#define	_ELF_DEFINE_SYB(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SYMBOL_BINDING() };

/*
 * Symbol visibility.
 */

#define	_ELF_DEFINE_SYMBOL_VISIBILITY()		\
_ELF_DEFINE_STV(STV_DEFAULT,         0,		\
	"as specified by symbol type")		\
_ELF_DEFINE_STV(STV_INTERNAL,        1,		\
	"as defined by processor semantics")	\
_ELF_DEFINE_STV(STV_HIDDEN,          2,		\
	"hidden from other components")		\
_ELF_DEFINE_STV(STV_PROTECTED,       3,		\
	"local references are not preemptable")

#undef	_ELF_DEFINE_STV
#define	_ELF_DEFINE_STV(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SYMBOL_VISIBILITY() };

/*
 * Symbol flags.
 */
#define	_ELF_DEFINE_SYMBOL_FLAGS()		\
_ELF_DEFINE_SYF(SYMINFO_FLG_DIRECT,	0x01,	\
	"directly assocated reference")		\
_ELF_DEFINE_SYF(SYMINFO_FLG_COPY,	0x04,	\
	"definition by copy-relocation")	\
_ELF_DEFINE_SYF(SYMINFO_FLG_LAZYLOAD,	0x08,	\
	"object should be lazily loaded")	\
_ELF_DEFINE_SYF(SYMINFO_FLG_DIRECTBIND,	0x10,	\
	"reference should be directly bound")	\
_ELF_DEFINE_SYF(SYMINFO_FLG_NOEXTDIRECT, 0x20,	\
	"external references not allowed to bind to definition")

#undef	_ELF_DEFINE_SYF
#define	_ELF_DEFINE_SYF(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_SYMBOL_FLAGS() };

/*
 * Version dependencies.
 */
#define	_ELF_DEFINE_VERSIONING_DEPENDENCIES()			\
_ELF_DEFINE_VERD(VER_NDX_LOCAL,		0,	"local scope")	\
_ELF_DEFINE_VERD(VER_NDX_GLOBAL,	1,	"global scope")
#undef	_ELF_DEFINE_VERD
#define	_ELF_DEFINE_VERD(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_VERSIONING_DEPENDENCIES() };

/*
 * Version flags.
 */
#define	_ELF_DEFINE_VERSIONING_FLAGS()				\
_ELF_DEFINE_VERF(VER_FLG_BASE,		0x1,	"file version") \
_ELF_DEFINE_VERF(VER_FLG_WEAK,		0x2,	"weak version")
#undef	_ELF_DEFINE_VERF
#define	_ELF_DEFINE_VERF(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_VERSIONING_FLAGS() };

/*
 * Version needs
 */
#define	_ELF_DEFINE_VERSIONING_NEEDS()					\
_ELF_DEFINE_VRN(VER_NEED_NONE,		0,	"invalid version")	\
_ELF_DEFINE_VRN(VER_NEED_CURRENT,	1,	"current version")
#undef	_ELF_DEFINE_VRN
#define	_ELF_DEFINE_VRN(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_VERSIONING_NEEDS() };

/*
 * Version numbers.
 */
#define	_ELF_DEFINE_VERSIONING_NUMBERS()				\
_ELF_DEFINE_VRNU(VER_DEF_NONE,		0,	"invalid version")	\
_ELF_DEFINE_VRNU(VER_DEF_CURRENT,	1, 	"current version")
#undef	_ELF_DEFINE_VRNU
#define	_ELF_DEFINE_VRNU(N, V, DESCR)	N = V ,
enum { _ELF_DEFINE_VERSIONING_NUMBERS() };

/**
 ** ELF Types.
 **/

typedef uint32_t	Elf32_Addr;	/* Program address. */
typedef uint16_t	Elf32_Half;	/* Unsigned medium integer. */
typedef uint32_t	Elf32_Off;	/* File offset. */
typedef int32_t		Elf32_Sword;	/* Signed integer. */
typedef uint32_t	Elf32_Word;	/* Unsigned integer. */
typedef uint64_t	Elf32_Lword;	/* Unsigned long integer. */

typedef uint64_t	Elf64_Addr;	/* Program address. */
typedef uint16_t	Elf64_Half;	/* Unsigned medium integer. */
typedef uint64_t	Elf64_Off;	/* File offset. */
typedef int32_t		Elf64_Sword;	/* Signed integer. */
typedef uint32_t	Elf64_Word;	/* Unsigned integer. */
typedef uint64_t	Elf64_Lword;	/* Unsigned long integer. */
typedef uint64_t	Elf64_Xword;	/* Unsigned long integer. */
typedef int64_t		Elf64_Sxword;	/* Signed long integer. */


/*
 * Capability descriptors.
 */

/* 32-bit capability descriptor. */
typedef struct {
	Elf32_Word	c_tag;	     /* Type of entry. */
	union {
		Elf32_Word	c_val; /* Integer value. */
		Elf32_Addr	c_ptr; /* Pointer value. */
	} c_un;
} Elf32_Cap;

/* 64-bit capability descriptor. */
typedef struct {
	Elf64_Xword	c_tag;	     /* Type of entry. */
	union {
		Elf64_Xword	c_val; /* Integer value. */
		Elf64_Addr	c_ptr; /* Pointer value. */
	} c_un;
} Elf64_Cap;

/*
 * Dynamic section entries.
 */

/* 32-bit entry. */
typedef struct {
	Elf32_Sword	d_tag;	     /* Type of entry. */
	union {
		Elf32_Word	d_val; /* Integer value. */
		Elf32_Addr	d_ptr; /* Pointer value. */
	} d_un;
} Elf32_Dyn;

/* 64-bit entry. */
typedef struct {
	Elf64_Sxword	d_tag;	     /* Type of entry. */
	union {
		Elf64_Xword	d_val; /* Integer value. */
		Elf64_Addr	d_ptr; /* Pointer value; */
	} d_un;
} Elf64_Dyn;


/*
 * The executable header (EHDR).
 */

/* 32 bit EHDR. */
typedef struct {
	unsigned char   e_ident[EI_NIDENT]; /* ELF identification. */
	Elf32_Half      e_type;	     /* Object file type (ET_*). */
	Elf32_Half      e_machine;   /* Machine type (EM_*). */
	Elf32_Word      e_version;   /* File format version (EV_*). */
	Elf32_Addr      e_entry;     /* Start address. */
	Elf32_Off       e_phoff;     /* File offset to the PHDR table. */
	Elf32_Off       e_shoff;     /* File offset to the SHDRheader. */
	Elf32_Word      e_flags;     /* Flags (EF_*). */
	Elf32_Half      e_ehsize;    /* Elf header size in bytes. */
	Elf32_Half      e_phentsize; /* PHDR table entry size in bytes. */
	Elf32_Half      e_phnum;     /* Number of PHDR entries. */
	Elf32_Half      e_shentsize; /* SHDR table entry size in bytes. */
	Elf32_Half      e_shnum;     /* Number of SHDR entries. */
	Elf32_Half      e_shstrndx;  /* Index of section name string table. */
} Elf32_Ehdr;


/* 64 bit EHDR. */
typedef struct {
	unsigned char   e_ident[EI_NIDENT]; /* ELF identification. */
	Elf64_Half      e_type;	     /* Object file type (ET_*). */
	Elf64_Half      e_machine;   /* Machine type (EM_*). */
	Elf64_Word      e_version;   /* File format version (EV_*). */
	Elf64_Addr      e_entry;     /* Start address. */
	Elf64_Off       e_phoff;     /* File offset to the PHDR table. */
	Elf64_Off       e_shoff;     /* File offset to the SHDRheader. */
	Elf64_Word      e_flags;     /* Flags (EF_*). */
	Elf64_Half      e_ehsize;    /* Elf header size in bytes. */
	Elf64_Half      e_phentsize; /* PHDR table entry size in bytes. */
	Elf64_Half      e_phnum;     /* Number of PHDR entries. */
	Elf64_Half      e_shentsize; /* SHDR table entry size in bytes. */
	Elf64_Half      e_shnum;     /* Number of SHDR entries. */
	Elf64_Half      e_shstrndx;  /* Index of section name string table. */
} Elf64_Ehdr;


/*
 * Note descriptors.
 */

/* 32-bit note header. */
typedef struct {
	Elf32_Word	n_namesz;	/* Length of note's name. */
	Elf32_Word	n_descsz;	/* Length of note's value. */
	Elf32_Word	n_type;		/* Type of note. */
} Elf32_Nhdr;

/* 64-bit note header. */
typedef struct {
	Elf64_Word	n_namesz;	/* Length of note's name */
	Elf64_Word	n_descsz;	/* Length of note's value. */
	Elf64_Word	n_type;		/* Type of note. */
} Elf64_Nhdr;


/*
 * Program Header Table (PHDR) entries.
 */

/* 32 bit PHDR entry. */
typedef struct {
	Elf32_Word	p_type;	     /* Type of segment. */
	Elf32_Off	p_offset;    /* File offset to segment. */
	Elf32_Addr	p_vaddr;     /* Virtual address in memory. */
	Elf32_Addr	p_paddr;     /* Physical address (if relevant). */
	Elf32_Word	p_filesz;    /* Size of segment in file. */
	Elf32_Word	p_memsz;     /* Size of segment in memory. */
	Elf32_Word	p_flags;     /* Segment flags. */
	Elf32_Word	p_align;     /* Alignment constraints. */
} Elf32_Phdr;

/* 64 bit PHDR entry. */
typedef struct {
	Elf64_Word	p_type;	     /* Type of segment. */
	Elf64_Word	p_flags;     /* File offset to segment. */
	Elf64_Off	p_offset;    /* Virtual address in memory. */
	Elf64_Addr	p_vaddr;     /* Physical address (if relevant). */
	Elf64_Addr	p_paddr;     /* Size of segment in file. */
	Elf64_Xword	p_filesz;    /* Size of segment in memory. */
	Elf64_Xword	p_memsz;     /* Segment flags. */
	Elf64_Xword	p_align;     /* Alignment constraints. */
} Elf64_Phdr;


/*
 * Move entries, for describing data in COMMON blocks in a compact
 * manner.
 */

/* 32-bit move entry. */
typedef struct {
	Elf32_Lword	m_value;     /* Initialization value. */
	Elf32_Word 	m_info;	     /* Encoded size and index. */
	Elf32_Word	m_poffset;   /* Offset relative to symbol. */
	Elf32_Half	m_repeat;    /* Repeat count. */
	Elf32_Half	m_stride;    /* Number of units to skip. */
} Elf32_Move;

/* 64-bit move entry. */
typedef struct {
	Elf64_Lword	m_value;     /* Initialization value. */
	Elf64_Xword 	m_info;	     /* Encoded size and index. */
	Elf64_Xword	m_poffset;   /* Offset relative to symbol. */
	Elf64_Half	m_repeat;    /* Repeat count. */
	Elf64_Half	m_stride;    /* Number of units to skip. */
} Elf64_Move;

#define ELF32_M_SYM(I)		((I) >> 8)
#define ELF32_M_SIZE(I)		((unsigned char) (I))
#define ELF32_M_INFO(M, S)	(((M) << 8) + (unsigned char) (S))

#define ELF64_M_SYM(I)		((I) >> 8)
#define ELF64_M_SIZE(I)		((unsigned char) (I))
#define ELF64_M_INFO(M, S)	(((M) << 8) + (unsigned char) (S))

/*
 * Section Header Table (SHDR) entries.
 */

/* 32 bit SHDR */
typedef struct {
	Elf32_Word	sh_name;     /* index of section name */
	Elf32_Word	sh_type;     /* section type */
	Elf32_Word	sh_flags;    /* section flags */
	Elf32_Addr	sh_addr;     /* in-memory address of section */
	Elf32_Off	sh_offset;   /* file offset of section */
	Elf32_Word	sh_size;     /* section size in bytes */
	Elf32_Word	sh_link;     /* section header table link */
	Elf32_Word	sh_info;     /* extra information */
	Elf32_Word	sh_addralign; /* alignment constraint */
	Elf32_Word	sh_entsize;   /* size for fixed-size entries */
} Elf32_Shdr;

/* 64 bit SHDR */
typedef struct {
	Elf64_Word	sh_name;     /* index of section name */
	Elf64_Word	sh_type;     /* section type */
	Elf64_Xword	sh_flags;    /* section flags */
	Elf64_Addr	sh_addr;     /* in-memory address of section */
	Elf64_Off	sh_offset;   /* file offset of section */
	Elf64_Xword	sh_size;     /* section size in bytes */
	Elf64_Word	sh_link;     /* section header table link */
	Elf64_Word	sh_info;     /* extra information */
	Elf64_Xword	sh_addralign; /* alignment constraint */
	Elf64_Xword	sh_entsize;  /* size for fixed-size entries */
} Elf64_Shdr;


/*
 * Symbol table entries.
 */

typedef struct {
	Elf32_Word	st_name;     /* index of symbol's name */
	Elf32_Addr	st_value;    /* value for the symbol */
	Elf32_Word	st_size;     /* size of associated data */
	unsigned char	st_info;     /* type and binding attributes */
	unsigned char	st_other;    /* visibility */
	Elf32_Half	st_shndx;    /* index of related section */
} Elf32_Sym;

typedef struct {
	Elf64_Word	st_name;     /* index of symbol's name */
	unsigned char	st_info;     /* value for the symbol */
	unsigned char	st_other;    /* size of associated data */
	Elf64_Half	st_shndx;    /* type and binding attributes */
	Elf64_Addr	st_value;    /* visibility */
	Elf64_Xword	st_size;     /* index of related section */
} Elf64_Sym;

#define ELF32_ST_BIND(I)	((I) >> 4)
#define ELF32_ST_TYPE(I)	((I) & 0xFU)
#define ELF32_ST_INFO(B,T)	(((B) << 4) + ((T) & 0xF))

#define ELF64_ST_BIND(I)	((I) >> 4)
#define ELF64_ST_TYPE(I)	((I) & 0xFU)
#define ELF64_ST_INFO(B,T)	(((B) << 4) + ((T) & 0xF))

#define ELF32_ST_VISIBILITY(O)	((O) & 0x3)
#define ELF64_ST_VISIBILITY(O)	((O) & 0x3)

/*
 * Syminfo descriptors, containing additional symbol information.
 */

/* 32-bit entry. */
typedef struct {
	Elf32_Half	si_boundto;  /* Entry index with additional flags. */
	Elf32_Half	si_flags;    /* Flags. */
} Elf32_Syminfo;

/* 64-bit entry. */
typedef struct {
	Elf64_Half	si_boundto;  /* Entry index with additional flags. */
	Elf64_Half	si_flags;    /* Flags. */
} Elf64_Syminfo;

/*
 * Relocation descriptors.
 */

typedef struct {
	Elf32_Addr	r_offset;    /* location to apply relocation to */
	Elf32_Word	r_info;	     /* type+section for relocation */
} Elf32_Rel;

typedef struct {
	Elf32_Addr	r_offset;    /* location to apply relocation to */
	Elf32_Word	r_info;      /* type+section for relocation */
	Elf32_Sword	r_addend;    /* constant addend */
} Elf32_Rela;

typedef struct {
	Elf64_Addr	r_offset;    /* location to apply relocation to */
	Elf64_Xword	r_info;      /* type+section for relocation */
} Elf64_Rel;

typedef struct {
	Elf64_Addr	r_offset;    /* location to apply relocation to */
	Elf64_Xword	r_info;      /* type+section for relocation */
	Elf64_Sxword	r_addend;    /* constant addend */
} Elf64_Rela;


#define ELF32_R_SYM(I)		((I) >> 8)
#define ELF32_R_TYPE(I)		((unsigned char) (I))
#define ELF32_R_INFO(S,T)	(((S) << 8) + (unsigned char) (T))

#define ELF64_R_SYM(I)		((I) >> 32)
#define ELF64_R_TYPE(I)		((I) & 0xFFFFFFFFUL)
#define ELF64_R_INFO(S,T)	(((S) << 32) + ((T) & 0xFFFFFFFFUL))

/*
 * Symbol versioning structures.
 */

/* 32-bit structures. */
typedef struct
{
	Elf32_Word	vda_name;    /* Index to name. */
	Elf32_Word	vda_next;    /* Offset to next entry. */
} Elf32_Verdaux;

typedef struct
{
	Elf32_Word	vna_hash;    /* Hash value of dependency name. */
	Elf32_Half	vna_flags;   /* Flags. */
	Elf32_Half	vna_other;   /* Unused. */
	Elf32_Word	vna_name;    /* Offset to dependency name. */
	Elf32_Word	vna_next;    /* Offset to next vernaux entry. */
} Elf32_Vernaux;

typedef struct
{
	Elf32_Half	vd_version;  /* Version information. */
	Elf32_Half	vd_flags;    /* Flags. */
	Elf32_Half	vd_ndx;	     /* Index into the versym section. */
	Elf32_Half	vd_cnt;	     /* Number of aux entries. */
	Elf32_Word	vd_hash;     /* Hash value of name. */
	Elf32_Word	vd_aux;	     /* Offset to aux entries. */
	Elf32_Word	vd_next;     /* Offset to next version definition. */
} Elf32_Verdef;

typedef struct
{
	Elf32_Half	vn_version;  /* Version number. */
	Elf32_Half	vn_cnt;	     /* Number of aux entries. */
	Elf32_Word	vn_file;     /* Offset of associated file name. */
	Elf32_Word	vn_aux;	     /* Offset of vernaux array. */
	Elf32_Word	vn_next;     /* Offset of next verneed entry. */
} Elf32_Verneed;

typedef Elf32_Half	Elf32_Versym;

/* 64-bit structures. */

typedef struct {
	Elf64_Word	vda_name;    /* Index to name. */
	Elf64_Word	vda_next;    /* Offset to next entry. */
} Elf64_Verdaux;

typedef struct {
	Elf64_Word	vna_hash;    /* Hash value of dependency name. */
	Elf64_Half	vna_flags;   /* Flags. */
	Elf64_Half	vna_other;   /* Unused. */
	Elf64_Word	vna_name;    /* Offset to dependency name. */
	Elf64_Word	vna_next;    /* Offset to next vernaux entry. */
} Elf64_Vernaux;

typedef struct {
	Elf64_Half	vd_version;  /* Version information. */
	Elf64_Half	vd_flags;    /* Flags. */
	Elf64_Half	vd_ndx;	     /* Index into the versym section. */
	Elf64_Half	vd_cnt;	     /* Number of aux entries. */
	Elf64_Word	vd_hash;     /* Hash value of name. */
	Elf64_Word	vd_aux;	     /* Offset to aux entries. */
	Elf64_Word	vd_next;     /* Offset to next version definition. */
} Elf64_Verdef;

typedef struct {
	Elf64_Half	vn_version;  /* Version number. */
	Elf64_Half	vn_cnt;	     /* Number of aux entries. */
	Elf64_Word	vn_file;     /* Offset of associated file name. */
	Elf64_Word	vn_aux;	     /* Offset of vernaux array. */
	Elf64_Word	vn_next;     /* Offset of next verneed entry. */
} Elf64_Verneed;

typedef Elf64_Half	Elf64_Versym;


/*
 * The header for GNU-style hash sections.
 */

typedef struct {
	u_int32_t	gh_nbuckets;	/* Number of hash buckets. */
	u_int32_t	gh_symndx;	/* First visible symbol in .dynsym. */
	u_int32_t	gh_maskwords;	/* #maskwords used in bloom filter. */
	u_int32_t	gh_shift2;	/* Bloom filter shift count. */
} Elf_GNU_Hash_Header;

#endif	/* _ELFDEFINITIONS_H_ */
