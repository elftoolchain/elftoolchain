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
#define	_ELF_DEFINE_EI(N, V, DESCR) N = V ,
enum { _ELF_DEFINE_EI_OFFSETS() };

/*
 * The ELF class of an object.
 */
#define	_ELF_DEFINE_ELFCLASS()				\
_ELF_DEFINE_EC(ELFCLASSNONE, 0, "Unknown ELF class")	\
_ELF_DEFINE_EC(ELFCLASS32,   1, "32 bit objects")	\
_ELF_DEFINE_EC(ELFCLASS64,   2, "64 bit objects")

#undef	_ELF_DEFINE_EC
#define	_ELF_DEFINE_EC(N, V, DESCR) N = V ,
enum { _ELF_DEFINE_ELFCLASS() };

/*
 * Endianness of data in an ELF object.
 */

#define	_ELF_DEFINE_ELF_DATA_ENDIANNESS()			\
_ELF_DEFINE_ED(ELFDATANONE, 0, "Unknown data endianness")	\
_ELF_DEFINE_ED(ELFDATA2LSB, 1, "little endian")			\
_ELF_DEFINE_ED(ELFDATA2MSB, 2, "big endian")

#undef	_ELF_DEFINE_ED
#define	_ELF_DEFINE_ED(N, V, DESCR) N = V ,
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
#define	_ELF_DEFINE_EMAG(N, V) N = V ,
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
#define	_ELF_DEFINE_EABI(N, V, DESCR) N = V ,
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
#define	_ELF_DEFINE_EM(N, V, DESCR) N = V ,
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
#define	_ELF_DEFINE_ET(N, V, DESCR) N = V ,
enum {	_ELF_DEFINE_ELF_TYPES() };

/* ELF file format version numbers. */
#define	EV_NONE		0
#define	EV_CURRENT	1

#define _ELF_DEFINE_ELF_SHNS()						\
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
#define	_ELF_DEFINE_SHN(N, V, DESCR) N = V ,
enum { _ELF_DEFINE_ELF_SHNS() };


/**
 ** ELF Types.
 **/

typedef uint32_t	Elf32_Addr;	/* Program address. */
typedef uint16_t	Elf32_Half;	/* Unsigned medium integer. */
typedef uint32_t	Elf32_Off;	/* File offset. */
typedef int32_t		Elf32_Sword;	/* Signed integer. */
typedef uint32_t	Elf32_Word;	/* Unsigned integer. */

typedef uint64_t	Elf64_Addr;	/* Program address. */
typedef uint16_t	Elf64_Half;	/* Unsigned medium integer. */
typedef uint64_t	Elf64_Off;	/* File offset. */
typedef int32_t		Elf64_Sword;	/* Signed integer. */
typedef uint32_t	Elf64_Word;	/* Unsigned integer. */
typedef uint64_t	Elf64_Xword;	/* Unsigned long integer. */
typedef int64_t		Elf64_Sxword;	/* Signed long integer. */


typedef struct {
	Elf32_Sword	d_tag;
	union {
		Elf32_Word	d_val;
		Elf32_Addr	d_ptr;
	} d_un;
} Elf32_Dyn;

typedef struct {
	Elf64_Sxword	d_tag;
	union {
		Elf64_Xword	d_val;
		Elf64_Addr	d_ptr;
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
	Elf32_Half      e_shstrndx;  /* Index of the section name string table. */
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
	Elf64_Half      e_shstrndx;  /* Index of the section name string table. */
} Elf64_Ehdr;


/*
 * Program Header Table (PHDR) entries.
 */

/* 32 bit PHDR entry. */
typedef struct {
	Elf32_Word	p_type;
	Elf32_Off	p_offset;
	Elf32_Addr	p_vaddr;
	Elf32_Addr	p_paddr;
	Elf32_Word	p_filesz;
	Elf32_Word	p_memsz;
	Elf32_Word	p_flags;
	Elf32_Word	p_align;
} Elf32_Phdr;

/* 64 bit PHDR entry. */
typedef struct {
	Elf64_Word	p_type;
	Elf64_Word	p_flags;
	Elf64_Off	p_offset;
	Elf64_Addr	p_vaddr;
	Elf64_Addr	p_paddr;
	Elf64_Xword	p_filesz;
	Elf64_Xword	p_memsz;
	Elf64_Xword	p_align;
} Elf64_Phdr;

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


typedef struct {
	Elf32_Word	st_name;
	Elf32_Addr	st_value;
	Elf32_Word	st_size;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf32_Half	st_shndx;
} Elf32_Sym;

typedef struct {
	Elf64_Word	st_name;
	unsigned char	st_info;
	unsigned char	st_other;
	Elf64_Half	st_shndx;
	Elf64_Addr	st_value;
	Elf64_Xword	st_size;
} Elf64_Sym;


typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
} Elf32_Rel;

typedef struct {
	Elf32_Addr	r_offset;
	Elf32_Word	r_info;
	Elf32_Sword	r_addend;
} Elf32_Rela;

typedef struct {
	Elf64_Addr	r_offset;
	Elf64_Xword	r_info;
} Elf64_Rel;

typedef struct {
	Elf64_Addr	r_offset;
	Elf64_Xword	r_info;
	Elf64_Sxword	r_addend;
} Elf64_Rela;


#define ELF32_R_SYM(I)		((I) >> 8)
#define ELF32_R_TYPE(I)		((unsigned char) (I))
#define ELF32_R_INFO(S,T)	(((S) << 8) + (unsigned char) (T))

#define ELF64_R_SYM(I)		((I) >> 32)
#define ELF64_R_TYPE(I)		((I) & 0xFFFFFFFFUL)
#define ELF64_R_INFO(S,T)	(((S) << 32) + ((T) & 0xFFFFFFFFUL))

#define ELF32_ST_BIND(I)	((I) >> 4)
#define ELF32_ST_TYPE(I)	((I) & 0xFU)
#define ELF32_ST_INFO(B,T)	(((B) << 4) + ((T) & 0xF))

#define ELF64_ST_BIND(I)	((I) >> 4)
#define ELF64_ST_TYPE(I)	((I) & 0xFU)
#define ELF64_ST_INFO(B,T)	(((B) << 4) + ((T) & 0xF))

#define ELF32_ST_VISIBILITY(O)	((O) & 0x3)
#define ELF64_ST_VISIBILITY(O)	((O) & 0x3)

#endif	/* _ELFDEFINITIONS_H_ */
