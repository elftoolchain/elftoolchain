/*-
 * Copyright (c) 2009 Kai Wang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer
 *    in this position and unchanged.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR(S) ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR(S) BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <sys/cdefs.h>
#if defined(__FBSDID)
__FBSDID("$FreeBSD$");
#elif defined(__RCSID)
__RCSID("$Id$");
#endif

#include <sys/param.h>
#include <sys/queue.h>
#include <ctype.h>
#include <err.h>
#include <fcntl.h>
#include <gelf.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#ifndef LIBELF_AR
#include <archive.h>
#include <archive_entry.h>
#endif	/* ! LIBELF_AR */

/*
 * readelf(1) options.
 */
#define	RE_AA	0x00000001
#define	RE_C	0x00000002
#define	RE_DD	0x00000004
#define	RE_D	0x00000008
#define	RE_G	0x00000010
#define	RE_H	0x00000020
#define	RE_II	0x00000040
#define	RE_I	0x00000080
#define	RE_L	0x00000100
#define	RE_NN	0x00000200
#define	RE_N	0x00000400
#define	RE_P	0x00000800
#define	RE_R	0x00001000
#define	RE_SS	0x00002000
#define	RE_S	0x00004000
#define	RE_T	0x00008000
#define	RE_U	0x00010000
#define	RE_VV	0x00020000
#define	RE_WW	0x00040000
#define	RE_W	0x00080000
#define	RE_X	0x00100000

/*
 * readelf(1) run control flags.
 */
#define	DISPLAY_FILENAME	0x0001
#define	SECTIONS_LOADED		0x0002

/*
 * Internal data structure for sections.
 */
struct section {
	const char	*name;		/* section name */
	Elf_Scn		*scn;		/* section scn */
	uint64_t	 off;		/* section offset */
	uint64_t	 sz;		/* section size */
	uint64_t	 entsize;	/* section entsize */
	uint64_t	 align;		/* section alignment */
	uint64_t	 type;		/* section type */
	uint64_t	 flags;		/* section flags */
	uint64_t	 addr;		/* section virtual addr */
	uint32_t	 link;		/* section link ndx */
	uint32_t	 info;		/* section info ndx */
};

struct dumpop {
	size_t		 sn;		/* section index */
#define HEX_DUMP	0x0001
	int		 op;		/* dump op type */
	STAILQ_ENTRY(dumpop) dumpop_list;
};

/*
 * Structure encapsulates the global data for readelf(1).
 */
struct readelf {
	const char	*filename;	/* current processing file. */
	int		 options;	/* command line options. */
	int		 flags;		/* run control flags. */
	Elf		*elf;		/* underlying ELF descriptor. */
	GElf_Ehdr	 ehdr;		/* ELF header. */
	int		 ec;		/* ELF class. */
	size_t		 shnum;		/* #sections. */
	struct section	*sl;		/* list of sections. */
	STAILQ_HEAD(, dumpop) v_dumpop; /* list of dump ops. */
};

struct elf_define {
	const char	*name;
	int		 value;
};

enum options
{
	OPTION_DEBUG_DUMP
};

static struct option longopts[] = {
	{"all", no_argument, NULL, 'a'},
	{"arch-specific", no_argument, NULL, 'A'},
	{"archive-index", no_argument, NULL, 'c'},
	{"debug-dump", optional_argument, NULL, OPTION_DEBUG_DUMP},
	{"dynamic", no_argument, NULL, 'd'},
	{"file-header", no_argument, NULL, 'h'},
	{"full-section-name", no_argument, NULL, 'N'},
	{"headers", no_argument, NULL, 'e'},
	{"help", no_argument, 0, 'H'},
	{"hex-dump", required_argument, NULL, 'x'},
	{"histogram", no_argument, NULL, 'I'},
	{"notes", no_argument, NULL, 'n'},
	{"program-headers", no_argument, NULL, 'l'},
	{"relocs", no_argument, NULL, 'r'},
	{"sections", no_argument, NULL, 'S'},
	{"section-headers", no_argument, NULL, 'S'},
	{"section-groups", no_argument, NULL, 'g'},
	{"section-details", no_argument, NULL, 't'},
	{"segments", no_argument, NULL, 'l'},
	{"string-dump", required_argument, NULL, 'p'},
	{"symbols", no_argument, NULL, 's'},
	{"syms", no_argument, NULL, 's'},
	{"unwind", no_argument, NULL, 'u'},
	{"use-dynamic", no_argument, NULL, 'D'},
	{"version-info", no_argument, 0, 'V'},
	{"version", no_argument, 0, 'v'},
	{"wide", no_argument, 0, 'W'},
	{NULL, 0, NULL, 0}
};

static struct elf_define elf_osabi[] = {
#ifdef ELFOSABI_SYSV
	{"SYSV", ELFOSABI_SYSV},
#endif
#ifdef ELFOSABI_HPUX
	{"HPUS", ELFOSABI_HPUX},
#endif
#ifdef ELFOSABI_NETBSD
	{"NetBSD", ELFOSABI_NETBSD},
#endif
#ifdef ELFOSABI_LINUX
	{"Linux", ELFOSABI_LINUX},
#endif
#ifdef ELFOSABI_HURD
	{"HURD", ELFOSABI_HURD},
#endif
#ifdef ELFOSABI_86OPEN
	{"86OPEN", ELFOSABI_86OPEN},
#endif
#ifdef ELFOSABI_SOLARIS
	{"Solaris", ELFOSABI_SOLARIS},
#endif
#ifdef ELFOSABI_AIX
	{"AIX", ELFOSABI_AIX},
#endif
#ifdef ELFOSABI_IRIX
	{"IRIX", ELFOSABI_IRIX},
#endif
#ifdef ELFOSABI_FREEBSD
	{"FreeBSD", ELFOSABI_FREEBSD},
#endif
#ifdef ELFOSABI_TRU64
	{"TRU64", ELFOSABI_TRU64},
#endif
#ifdef ELFOSABI_MODESTO
	{"MODESTO", ELFOSABI_MODESTO},
#endif
#ifdef ELFOSABI_OPENBSD
	{"OpenBSD", ELFOSABI_OPENBSD},
#endif
#ifdef ELFOSABI_OPENVMS
	{"OpenVMS", ELFOSABI_OPENVMS},
#endif
#ifdef ELFOSABI_NSK
	{"NSK", ELFOSABI_NSK},
#endif
#ifdef ELFOSABI_ARM
	{"ARM", ELFOSABI_ARM},
#endif
#ifdef ELFOSABI_STANDALONE
	{"StandAlone", ELFOSABI_STANDALONE},
#endif
	{NULL, 0}
};

static struct elf_define elf_machine[] = {
#ifdef EM_NONE
	{"Unknown machine", EM_NONE},
#endif
#ifdef EM_M32
	{"AT&T WE32100", EM_M32},
#endif
#ifdef EM_SPARC
	{"Sun SPARC", EM_SPARC},
#endif
#ifdef EM_386
	{"Intel i386", EM_386},
#endif
#ifdef EM_68K
	{"Motorola 68000", EM_68K},
#endif
#ifdef EM_88K
	{"Motorola 88000", EM_88K},
#endif
#ifdef EM_860
	{"Intel i860", EM_860},
#endif
#ifdef EM_MIPS
	{"MIPS R3000 Big-Endian only", EM_MIPS},
#endif
#ifdef EM_S370
	{"IBM System/370", EM_S370},
#endif
#ifdef EM_MIPS_RS3_LE
	{"MIPS R3000 Little-Endian", EM_MIPS_RS3_LE},
#endif
#ifdef EM_PARISC
	{"HP PA-RISC", EM_PARISC},
#endif
#ifdef EM_VPP500
	{"Fujitsu VPP500", EM_VPP500},
#endif
#ifdef EM_SPARC32PLUS
	{"SPARC v8plus", EM_SPARC32PLUS},
#endif
#ifdef EM_960
	{"Intel 80960", EM_960},
#endif
#ifdef EM_PPC
	{"PowerPC 32-bit", EM_PPC},
#endif
#ifdef EM_PPC64
	{"PowerPC 64-bit", EM_PPC64},
#endif
#ifdef EM_S390
	{"IBM System/390", EM_S390},
#endif
#ifdef EM_V800
	{"NEC V800", EM_V800},
#endif
#ifdef EM_FR20
	{"Fujitsu FR20", EM_FR20},
#endif
#ifdef EM_RH32
	{"TRW RH-32", EM_RH32},
#endif
#ifdef EM_RCE
	{"Motorola RCE", EM_RCE},
#endif
#ifdef EM_ARM
	{"ARM", EM_ARM},
#endif
#ifdef EM_SH
	{"Hitachi SH", EM_SH},
#endif
#ifdef EM_SPARCV9
	{"SPARC v9 64-bit", EM_SPARCV9},
#endif
#ifdef EM_TRICORE
	{"Siemens TriCore embedded processor", EM_TRICORE},
#endif
#ifdef EM_ARC
	{"Argonaut RISC Core", EM_ARC},
#endif
#ifdef EM_H8_300
	{"Hitachi H8/300", EM_H8_300},
#endif
#ifdef EM_H8_300H
	{"Hitachi H8/300H", EM_H8_300H},
#endif
#ifdef EM_H8_500
	{"Hitachi H8/500", EM_H8_500},
#endif
#ifdef EM_IA_64
	{"Intel IA-64 Processor", EM_IA_64},
#endif
#ifdef EM_MIPS_X
	{"Stanford MIPS-X", EM_MIPS_X},
#endif
#ifdef EM_COLDFIRE
	{"Motorola ColdFire", EM_COLDFIRE},
#endif
#ifdef EM_68HC12
	{"Motorola M68HC12", EM_68HC12},
#endif
#ifdef EM_MMA
	{"Fujitsu MMA", EM_MMA},
#endif
#ifdef EM_PCP
	{"Siemens PCP", EM_PCP},
#endif
#ifdef EM_NCPU
	{"Sony nCPU", EM_NCPU},
#endif
#ifdef EM_NDR1
	{"Denso NDR1 microprocessor", EM_NDR1},
#endif
#ifdef EM_STARCORE
	{"Motorola Star*Core processor", EM_STARCORE},
#endif
#ifdef EM_ME16
	{"Toyota ME16 processor" ,EM_ME16},
#endif
#ifdef EM_ST100
	{"STMicroelectronics ST100 processor", EM_ST100},
#endif
#ifdef EM_TINYJ
	{"Advanced Logic Corp. TinyJ processor", EM_TINYJ},
#endif
#ifdef EM_X86_64
	{"Advanced Micro Devices x86-64", EM_X86_64},
#endif
#ifdef EM_AMD64
	{"Advanced Micro Devices x86-64", EM_AMD64},
#endif
#ifdef EM_PDSP
	{"Sony DSP Processor", EM_PDSP},
#endif
#ifdef EM_FX66
	{"Siemens FX66 microcontroller", EM_FX66},
#endif
#ifdef EM_ST9PLUS
	{"STMicroelectronics ST9+ 8/16 microcontroller", EM_ST9PLUS},
#endif
#ifdef EM_ST7
	{"STmicroelectronics ST7 8-bit microcontroller", EM_ST7},
#endif
#ifdef EM_68HC16
	{"Motorola MC68HC16 microcontroller", EM_68HC16},
#endif
#ifdef EM_68HC11
	{"Motorola MC68HC11 microcontroller", EM_68HC11},
#endif
#ifdef EM_68HC08
	{"Motorola MC68HC08 microcontroller", EM_68HC08},
#endif
#ifdef EM_68HC05
	{"Motorola MC68HC05 microcontroller", EM_68HC05},
#endif
#ifdef EM_SVX
	{"Silicon Graphics SVx", EM_SVX},
#endif
#ifdef EM_ST19
	{"STMicroelectronics ST19 8-bit mc", EM_ST19},
#endif
#ifdef EM_VAX
	{"Digital VAX", EM_VAX},
#endif
#ifdef EM_CRIS
	{"Axis Communications 32-bit embedded processor", EM_CRIS},
#endif
#ifdef EM_JAVELIN
	{"Infineon Technologies 32-bit embedded processor", EM_JAVELIN},
#endif
#ifdef EM_FIREPATH
	{"Element 14 64-bit DSP Processor", EM_FIREPATH},
#endif
#ifdef EM_ZSP
	{"LSI Logic 16-bit DSP Processor", EM_ZSP},
#endif
#ifdef EM_MMIX
	{"Donald Knuth's educational 64-bit proc", EM_MMIX},
#endif
#ifdef EM_HUANY
	{"Harvard University machine-independent object files", EM_HUANY},
#endif
#ifdef EM_PRISM
	{"SiTera Prism", EM_PRISM},
#endif
#ifdef EM_AVR
	{"Atmel AVR 8-bit microcontroller", EM_AVR},
#endif
#ifdef EM_FR30
	{"Fujitsu FR30", EM_FR30},
#endif
#ifdef EM_D10V
	{"Mitsubishi D10V", EM_D10V},
#endif
#ifdef EM_D30V
	{"Mitsubishi D30V", EM_D30V},
#endif
#ifdef EM_V850
	{"NEC v850", EM_V850},
#endif
#ifdef EM_M32R
	{"Mitsubishi M32R", EM_M32R},
#endif
#ifdef EM_MN10300
	{"Matsushita MN10300", EM_MN10300},
#endif
#ifdef EM_MN10200
	{"Matsushita MN10200", EM_MN10200},
#endif
#ifdef EM_PJ
	{"picoJava", EM_PJ},
#endif
#ifdef EM_OPENRISC
	{"OpenRISC 32-bit embedded processor", EM_OPENRISC},
#endif
#ifdef EM_ARC_A5
	{"ARC Cores Tangent-A5", EM_ARC_A5},
#endif
#ifdef EM_XTENSA
	{"Tensilica Xtensa Architecture", EM_XTENSA},
#endif
#ifdef EM_VIDEOCORE
	{"Alphamosaic VideoCore processor", EM_VIDEOCORE},
#endif
#ifdef EM_TMM_GPP
	{"Thompson Multimedia General Purpose Processor", EM_TMM_GPP},
#endif
#ifdef EM_NS32K
	{"National Semiconductor 32000 series", EM_NS32K},
#endif
#ifdef EM_TPC
	{"Tenor Network TPC processor", EM_TPC},
#endif
#ifdef EM_SNP1K
	{"Trebia SNP 1000 processor", EM_SNP1K},
#endif
#ifdef EM_ST200
	{"STMicroelectronics ST200 microcontroller", EM_ST200},
#endif
#ifdef EM_IP2K
	{"Ubicom IP2xxx microcontroller family", EM_IP2K},
#endif
#ifdef EM_MAX
	{"MAX Processor", EM_MAX},
#endif
#ifdef EM_CR
	{"National Semiconductor CompactRISC microprocessor", EM_CR},
#endif
#ifdef EM_F2MC16
	{"Texas Instruments embedded microcontroller msp430", EM_F2MC16},
#endif
#ifdef EM_BLACKFIN
	{"Analog Devices Blackfin (DSP) processor", EM_BLACKFIN},
#endif
#ifdef EM_SE_C33
	{"S1C33 Family of Seiko Epson processors", EM_SE_C33},
#endif
#ifdef EM_SEP
	{"Sharp embedded microprocessor", EM_SEP},
#endif
#ifdef EM_ARCA
	{"Arca RISC Microprocessor", EM_ARCA},
#endif
#ifdef EM_UNICORE
	{"Microprocessor series from PKU-Unity Ltd", EM_UNICORE},
#endif
#ifdef EM_486
	{"Intel i486", EM_486},
#endif
#ifdef EM_MIPS_RS4_BE
	{"MIPS R4000 Big-Endian", EM_MIPS_RS4_BE},
#endif
#ifdef EM_ALPHA_STD
	{"Digital Alpha", EM_ALPHA_STD},
#endif
#ifdef EM_ALPHA
	{"Digital Alpha", EM_ALPHA},
#endif
	{NULL, 0}
};

static struct elf_define elf_class[] = {
	{"none", ELFCLASSNONE},
	{"ELF32", ELFCLASS32},
	{"ELF64", ELFCLASS64},
	{NULL, 0}
};

static struct elf_define elf_endian[] = {
	{"none", ELFDATANONE},
	{"2's complement, little endian", ELFDATA2LSB},
	{"2's complement, big endian", ELFDATA2MSB},
	{NULL, 0}
};

static struct elf_define elf_type[] = {
	{"NONE (None)", ET_NONE},
	{"REL (Relocatable file)", ET_REL},
	{"EXEC (Executable file)", ET_EXEC},
	{"DYN (Shared object file)", ET_DYN},
	{"CORE (Core file)", ET_CORE},
	{NULL, 0}
};

static struct elf_define elf_ver[] = {
	{"(current)", EV_CURRENT},
	{"(none)", EV_NONE},
	{NULL, 0}
};

static struct elf_define phdr_type[] = {
	{"NULL", PT_NULL},
	{"LOAD", PT_LOAD},
	{"DYNAMIC", PT_DYNAMIC},
	{"INTERP", PT_INTERP},
	{"NOTE", PT_NOTE},
	{"SHLIB", PT_SHLIB},
	{"PHDR", PT_PHDR},
	{"TLS", PT_TLS},
	{"GNU_EH_FRAME", PT_GNU_EH_FRAME},
#ifdef PT_GNU_STACK
	{"GNU_STACK", PT_GNU_STACK},
#endif
#ifdef PT_GNU_RELRO
	{"GNU_RELRO", PT_GNU_RELRO},
#endif
	{NULL, 0}
};

static struct elf_define section_type[] = {
	{"NULL", SHT_NULL},
	{"PROGBITS", SHT_PROGBITS},
	{"SYMTAB", SHT_SYMTAB},
	{"STRTAB", SHT_STRTAB},
	{"RELA", SHT_RELA},
	{"HASH", SHT_HASH},
	{"DYNAMIC", SHT_DYNAMIC},
	{"NOTE", SHT_NOTE},
	{"NOBITS", SHT_NOBITS},
	{"REL", SHT_REL},
	{"SHLIB", SHT_SHLIB},
	{"DYNSYM", SHT_DYNSYM},
	{"INIT_ARRAY", SHT_INIT_ARRAY},
	{"FINI_ARRAY", SHT_FINI_ARRAY},
	{"PREINIT_ARRAY", SHT_PREINIT_ARRAY},
	{"GROUP", SHT_GROUP},
	{"GNU_HASH", SHT_GNU_HASH},
	{"VERDEF", SHT_GNU_verdef},
	{"VERNEED", SHT_GNU_verneed},
	{"VERSYM", SHT_GNU_versym},
	{NULL, 0}
};

static struct elf_define dt_type[] = {
	{"NULL", DT_NULL},
	{"NEEDED", DT_NEEDED},
	{"PLTRELSZ", DT_PLTRELSZ},
	{"PLTGOT", DT_PLTGOT},
	{"HASH", DT_HASH},
	{"STRTAB", DT_STRTAB},
	{"SYMTAB", DT_SYMTAB},
	{"RELA", DT_RELA},
	{"RELASZ", DT_RELASZ},
	{"RELAENT", DT_RELAENT},
	{"STRSZ", DT_STRSZ},
	{"SYMENT", DT_SYMENT},
	{"INIT", DT_INIT},
	{"FINI", DT_FINI},
	{"SONAME", DT_SONAME},
	{"RPATH", DT_RPATH},
	{"SYMBOLIC", DT_SYMBOLIC},
	{"REL", DT_REL},
	{"RELSZ", DT_RELSZ},
	{"RELENT", DT_RELENT},
	{"PLTREL", DT_PLTREL},
	{"DEBUG", DT_DEBUG},
	{"TEXTREL", DT_TEXTREL},
	{"JMPREL", DT_JMPREL},
	{"BIND_NOW", DT_BIND_NOW},
	{"INIT_ARRAY", DT_INIT_ARRAY},
	{"FINI_ARRAY", DT_FINI_ARRAY},
	{"INIT_ARRAYSZ", DT_INIT_ARRAYSZ},
	{"FINI_ARRAYSZ", DT_FINI_ARRAYSZ},
	{"RUNPATH", DT_RUNPATH},
	{"FLAGS", DT_FLAGS},
	{"ENCODING", DT_ENCODING},
	{"PREINIT_ARRAY", DT_PREINIT_ARRAY},
	{"PREINIT_ARRAYSZ", DT_PREINIT_ARRAYSZ},
	{"MAXPOSTAGS", DT_MAXPOSTAGS},
	{"SUNW_AUXILIARY", DT_SUNW_AUXILIARY},
	{"SUNW_RTLDINF", DT_SUNW_RTLDINF},
	{"SUNW_FILTER", DT_SUNW_FILTER},
	{"SUNW_CAP", DT_SUNW_CAP},
	{"CHECKSUM", DT_CHECKSUM},
	{"PLTPADSZ", DT_PLTPADSZ},
	{"MOVEENT", DT_MOVEENT},
	{"MOVESZ", DT_MOVESZ},
	{"FEATURE_1", DT_FEATURE_1},
	{"POSFLAG_1", DT_POSFLAG_1},
	{"SYMINSZ", DT_SYMINSZ},
	{"SYMINENT", DT_SYMINENT},
	{"CONFIG", DT_CONFIG},
	{"DEPAUDIT", DT_DEPAUDIT},
	{"AUDIT", DT_AUDIT},
	{"PLTPAD", DT_PLTPAD},
	{"MOVETAB", DT_MOVETAB},
	{"SYMINFO", DT_SYMINFO},
	{"VERSYM", DT_VERSYM},
	{"RELACOUNT", DT_RELACOUNT},
	{"RELCOUNT", DT_RELCOUNT},
	{"FLAGS_1", DT_FLAGS_1},
	{"VERDEF", DT_VERDEF},
	{"VERDEFNUM", DT_VERDEFNUM},
	{"VERNEED", DT_VERNEED},
	{"VERNEEDNUM", DT_VERNEEDNUM},
	{"DEPRECATED_SPARC_REGISTER", DT_DEPRECATED_SPARC_REGISTER},
	{"AUXILIARY", DT_AUXILIARY},
	{"USED", DT_USED},
	{"FILTER", DT_FILTER},
};

static struct elf_define st_bind[] = {
	{"LOCAL", STB_LOCAL},
	{"GLOBAL", STB_GLOBAL},
	{"WEAK", STB_WEAK},
};

static struct elf_define st_type[] = {
	{"NOTYPE", STT_NOTYPE},
	{"OBJECT", STT_OBJECT},
	{"FUNC", STT_FUNC},
	{"SECTION", STT_SECTION},
	{"FILE", STT_FILE},
	{"COMMON", STT_COMMON},
	{"TLS", STT_TLS},
};

static struct {
	const char *ln;
	char sn;
	int value;
} section_flag[] = {
	{"WRITE", 'W', SHF_WRITE},
	{"ALLOC", 'A', SHF_ALLOC},
	{"EXEC", 'X', SHF_EXECINSTR},
	{"MERGE", 'M', SHF_MERGE},
	{"STRINGS", 'S', SHF_STRINGS},
	{"INFO LINK", 'I', SHF_INFO_LINK},
	{"OS NONCONF", 'O', SHF_OS_NONCONFORMING},
	{"GROUP", 'G', SHF_GROUP},
	{"TLS", 'T', SHF_TLS},
	{NULL, 0, 0}
};

static void		 add_dumpop(struct readelf *re, size_t sn, int op);
static void		 dump_elf(struct readelf *re);
static void		 dump_elf_type(struct readelf *re);
static void		 dump_dyn_val(struct readelf *re, GElf_Dyn *dyn, uint32_t stab);
static void		 dump_dynamic(struct readelf *re);
static void		 dump_hash(struct readelf *re);
static void		 dump_phdr(struct readelf *re);
static void		 dump_symtab(struct readelf *re, int i);
static void		 dump_symtabs(struct readelf *re);
static struct dumpop 	*find_dumpop(struct readelf *re, size_t sn, int op);
static void		 load_sections(struct readelf *re);
static const char	*lookup_define(struct elf_define *define, int value);
static void		 readelf_help(void);
static void		 readelf_usage(void);
static void		 readelf_version(void);

static const char *
lookup_define(struct elf_define *define, int value)
{
	int i;

	for (i = 0; define[i].name != NULL; i++)
		if (define[i].value == value)
			return (define[i].name);

	return (NULL);
}

static void
dump_ehdr(struct readelf *re)
{
	const char	*name;
	size_t		 shnum, shstrndx;
	int		 i;

	printf("ELF Header:\n");

	/* e_ident[]. */
	printf("  Magic:   ");
	for (i = 0; i < EI_NIDENT; i++)
		printf("%.2x ", re->ehdr.e_ident[i]);
	printf("\n");

	/* EI_CLASS. */
	printf("  Class:                             ");
	if ((name = lookup_define(elf_class, re->ehdr.e_ident[EI_CLASS])) !=
	    NULL)
		printf("%s\n", name);
	else
		printf("<unknown: %x>\n", re->ehdr.e_ident[EI_CLASS]);

	/* EI_DATA. */
	printf("  Data:                              ");
	if ((name = lookup_define(elf_endian, re->ehdr.e_ident[EI_DATA])) !=
	    NULL)
		printf("%s\n", name);
	else
		printf("<unknown: %x>\n", re->ehdr.e_ident[EI_DATA]);

	/* EI_VERSION. */
	printf("  Version:                           %d ",
	    re->ehdr.e_ident[EI_VERSION]);
	if ((name = lookup_define(elf_ver, re->ehdr.e_ident[EI_VERSION])) !=
	    NULL)
		printf("%s\n", name);
	else
		printf("(unknown)\n");

	/* EI_OSABI. */
	printf("  OS/ABI:                            ");
	if ((name = lookup_define(elf_osabi, re->ehdr.e_ident[EI_OSABI])) !=
	    NULL)
		printf("%s\n", name);
	else
		printf("<unknown: %x>\n", re->ehdr.e_ident[EI_OSABI]);

	/* EI_ABIVERSION. */
	printf("  ABI Version:                       %d\n",
	    re->ehdr.e_ident[EI_ABIVERSION]);

	/* e_type. */
	printf("  Type:                              ");
	dump_elf_type(re);

	/* e_machine. */
	printf("  Machine:                           ");
	if ((name = lookup_define(elf_machine, re->ehdr.e_machine)) != NULL)
		printf("%s\n", name);
	else
		printf("<unknown: %x>\n", re->ehdr.e_machine);

	/* e_version. */
	printf("  Version:                           0x%x\n",
	    re->ehdr.e_version);

	/* e_entry. */
	printf("  Entry point address:               0x%lx\n",
	    re->ehdr.e_entry);

	/* e_phoff. */
	printf("  Start of program headers:          %ju (bytes into file)\n",
	    re->ehdr.e_phoff);

	/* e_shoff. */
	printf("  Start of section headers:          %ju (bytes into file)\n",
	    re->ehdr.e_shoff);

	/* e_flags. TODO: add machine flags parse. */
	printf("  Flags:                             0x%x\n",
	    re->ehdr.e_flags);

	/* e_ehsize. */
	printf("  Size of this header:               %u (bytes)\n",
	    re->ehdr.e_ehsize);

	/* e_phentsize. */
	printf("  Size of program headers:           %u (bytes)\n",
	    re->ehdr.e_phentsize);

	/* e_phnum. */
	printf("  Number of program headers:         %u\n",
	    re->ehdr.e_phnum);

	/* e_shentsize. */
	printf("  Size of section headers:           %u (bytes)\n",
	    re->ehdr.e_shentsize);

	/* e_shnum. */
	printf("  Number of section headers:         %u", re->ehdr.e_shnum);
	if (re->ehdr.e_shnum == SHN_UNDEF) {
		/* Extended section numbering is in use. */
		if (elf_getshnum(re->elf, &shnum))
			printf(" (%ju)", shnum);
	}
	printf("\n");

	/* e_shstrndx. */
	printf("  Section header string table index: %u", re->ehdr.e_shstrndx);
	if (re->ehdr.e_shstrndx == SHN_XINDEX) {
		/* Extended section numbering is in use. */
		if (elf_getshstrndx(re->elf, &shstrndx))
			printf(" (%ju)", shstrndx);
	}
	printf("\n");
}

static void
dump_elf_type(struct readelf *re)
{
	const char *name;

	if ((name = lookup_define(elf_type, re->ehdr.e_type)) != NULL)
		printf("%s\n", name);
	else {
		if (re->ehdr.e_type >= ET_LOPROC)
			printf("Processor Specific: (%x)\n", re->ehdr.e_type);
		else if (re->ehdr.e_type >= ET_LOOS &&
		    re->ehdr.e_type <= ET_HIOS)
			printf("OS Specific: (%x)\n", re->ehdr.e_type);
		else
			printf("<unknown: %x>\n", re->ehdr.e_type);
	}
}

static void
dump_phdr(struct readelf *re)
{
	const char	*name, *rawfile;
	GElf_Phdr	 phdr;
	size_t		 phnum;
	int		 i, j;

	if ((re->flags & SECTIONS_LOADED) == 0)
		return;
	if (elf_getphnum(re->elf, &phnum) == 0) {
		warnx("elf_getphnum failed: %s", elf_errmsg(-1));
		return;
	}
	if (phnum == 0) {
		printf("\nThere are no program headers in this file.\n");
		return;
	}

	printf("\nElf file type is ");
	dump_elf_type(re);
	printf("Entry point 0x%lx\n", re->ehdr.e_entry);
	printf("There are %ju program headers, starting at offset %ju\n", phnum,
	    re->ehdr.e_phoff);

	/* Dump program headers. */
	printf("\nProgram Headers:\n");
	if (re->ec == ELFCLASS32)
		printf("  %-15s%-9s%-11s%-11s%-8s%-8s%-4s%s\n", "Type",
		    "Offset", "VirtAddr", "PhysAddr", "FileSiz", "MemSiz",
		    "Flg", "Align");
	else if (re->options & RE_WW)
		printf("  %-15s%-9s%-19s%-19s%-9s%-9s%-4s%s\n", "Type",
		    "Offset", "VirtAddr", "PhysAddr", "FileSiz", "MemSiz",
		    "Flg", "Align");
	else {
		printf("  %-15s%-19s%-19s%s\n","Type", "Offset", "VirtAddr",
		    "PhysAddr");
		printf("                 %-19s%-20s%-7s%s\n", "FileSiz",
		    "MemSiz","Flags", "Align");
	}
	for (i = 0; (size_t)i < phnum; i++) {
		if (gelf_getphdr(re->elf, i, &phdr) != &phdr) {
			warnx("gelf_getphdr failed: %s", elf_errmsg(-1));
			continue;
		}
		if ((name = lookup_define(phdr_type, phdr.p_type)) != NULL)
			printf("  %-14.14s ", name);
		else {
			/* TODO: Add arch-specific segment type dump. */
			if (phdr.p_type >= PT_LOPROC &&
			    phdr.p_type <= PT_HIPROC)
				printf("  LOPROC+%-7.6x ", phdr.p_type -
				    PT_LOPROC);
			else if (phdr.p_type >= PT_LOOS &&
			    phdr.p_type <= PT_HIOS)
				printf("  LOOS+%-9.6x ", phdr.p_type -
				    PT_LOOS);
			else
				printf("  <unknown>: %3.3x", phdr.p_type);
		}
		if (re->ec == ELFCLASS32) {
			printf("0x%6.6lx ", phdr.p_offset);
			printf("0x%8.8lx ", phdr.p_vaddr);
			printf("0x%8.8lx ", phdr.p_paddr);
			printf("0x%5.5lx ", phdr.p_filesz);
			printf("0x%5.5lx ", phdr.p_memsz);
			printf("%c%c%c ", phdr.p_flags & PF_R ? 'R' : ' ',
			    phdr.p_flags & PF_W ? 'W' : ' ',
			    phdr.p_flags & PF_X ? 'E' : ' ');
			printf("%#lx\n", phdr.p_align);
		} else if (re->options & RE_WW) {
			printf("0x%6.6lx ", phdr.p_offset);
			printf("0x%16.16lx ", phdr.p_vaddr);
			printf("0x%16.16lx ", phdr.p_paddr);
			printf("0x%6.6lx ", phdr.p_filesz);
			printf("0x%6.6lx ", phdr.p_memsz);
			printf("%c%c%c ", phdr.p_flags & PF_R ? 'R' : ' ',
			    phdr.p_flags & PF_W ? 'W' : ' ',
			    phdr.p_flags & PF_X ? 'E' : ' ');
			printf("%#lx\n", phdr.p_align);
		} else {
			printf("0x%16.16lx ", phdr.p_offset);
			printf("0x%16.16lx ", phdr.p_vaddr);
			printf("0x%16.16lx\n", phdr.p_paddr);
			printf("                 0x%16.16lx ", phdr.p_filesz);
			printf("0x%16.16lx ", phdr.p_memsz);
			printf(" %c%c%c    ", phdr.p_flags & PF_R ? 'R' : ' ',
			    phdr.p_flags & PF_W ? 'W' : ' ',
			    phdr.p_flags & PF_X ? 'E' : ' ');
			printf("%#lx\n", phdr.p_align);
		}
		if (phdr.p_type == PT_INTERP) {
			if ((rawfile = elf_rawfile(re->elf, NULL)) == NULL) {
				warnx("elf_rawfile failed: %s", elf_errmsg(-1));
				continue;
			}
			printf("      [Requesting program interpreter: %s]\n",
				rawfile + phdr.p_offset);
		}
	}

	/* Dump section to segment mapping. */
	printf("\n Section to Segment mapping:\n");
	printf("  Segment Sections...\n");
	for (i = 0; (size_t)i < phnum; i++) {
		if (gelf_getphdr(re->elf, i, &phdr) != &phdr) {
			warnx("gelf_getphdr failed: %s", elf_errmsg(-1));
			continue;
		}
		printf("   %2.2d     ", i);
		/* skip NULL section. */
		for (j = 1; (size_t)j < re->shnum; j++)
			if (re->sl[j].off >= phdr.p_offset &&
			    re->sl[j].off + re->sl[j].sz <=
			    phdr.p_offset + phdr.p_memsz)
				printf("%s ", re->sl[j].name);
		printf("\n");
	}
}

static void
dump_section_flags(struct readelf *re, struct section *s)
{
#define BUF_SZ 256
	char	buf[BUF_SZ];
	int	i, p, nb;

	p = 0;
	nb = re->ec == ELFCLASS32 ? 8 : 16;
	if (re->options & RE_T) {
		snprintf(buf, BUF_SZ, "[%*.*lx]: ", nb, nb, s->flags);
		p += nb + 4;
	}
	for (i = 0; section_flag[i].ln != NULL; i++) {
		if ((s->flags & section_flag[i].value) == 0)
			continue;
		if (re->options & RE_T) {
			snprintf(&buf[p], BUF_SZ - p, "%s, ",
			    section_flag[i].ln);
			p += strlen(section_flag[i].ln) + 2;
		} else
			buf[p++] = section_flag[i].sn;
	}
	if (re->options & RE_T && p > nb + 4)
		p -= 2;
	buf[p] = '\0';
	printf("%3s%c", buf, re->options & RE_T ? '\n' : ' ');
}

static void
dump_section_type(struct readelf *re __unused, uint32_t type)
{
	const char *name;

	if ((name = lookup_define(section_type, type)) != NULL)
		printf("%-15.15s ", name);
	else {
		/* TODO: Add arch-specific types. */
		if (type >= SHT_LOOS && type <= SHT_HIOS)
			printf("LOOS+%-10x ", type - SHT_LOOS);
		else if (type >= SHT_LOUSER && type <= SHT_HIUSER)
			printf("LOUSER+%-8x", type - SHT_LOUSER);
		else
			printf("<unknown>: %-4x", type);
	}
}

static void
dump_shdr(struct readelf *re)
{
	struct section	*s;
	int		 i;

	if ((re->flags & SECTIONS_LOADED) == 0)
		return;
	if (re->shnum == 0) {
		printf("\nThere are no sections in this file.\n");
		return;
	}
	printf("There are %ju section headers, starting at offset 0x%lx:\n",
	    re->shnum, re->ehdr.e_shoff);
	printf("\nSection Headers:\n");
	if (re->ec == ELFCLASS32) {
		if (re->options & RE_T) {
			printf("  %s\n", "[Nr] Name");
			printf("       %-16s%-9s%-7s%-7s%-5s%-3s%-4s%s\n",
			    "Type", "Addr", "Off", "Size", "ES", "Lk", "Inf",
			    "Al");
			printf("       Flags\n");
		} else
			printf("  %-23s%-16s%-9s%-7s%-7s%-3s%-4s%-3s%-4s%s\n",
			    "[Nr] Name", "Type", "Addr", "Off", "Size", "ES",
			    "Flg", "Lk", "Inf", "Al");
	} else if (re->options & RE_WW) {
		if (re->options & RE_T) {
			printf("  %s\n", "[Nr] Name");
			printf("       %-16s%-17s%-7s%-7s%-5s%-3s%-4s%s\n",
			    "Type", "Address", "Off", "Size", "ES", "Lk", "Inf",
			    "Al");
			printf("       Flags\n");
		} else
			printf("  %-23s%-16s%-17s%-7s%-7s%-3s%-4s%-3s%-4s%s\n",
			    "[Nr] Name", "Type", "Address", "Off", "Size", "ES",
			    "Flg", "Lk", "Inf", "Al");
	} else {
		if (re->options & RE_T) {
			printf("  %s\n", "[Nr] Name");
			printf("       %-18s%-17s%-18s%s\n", "Type", "Address",
			    "Offset", "Link");
			printf("       %-18s%-17s%-18s%s\n", "Size", "EntSize",
			    "Info", "Align");
			printf("       Flags\n");
		} else {
			printf("  %-23s%-17s%-18s%s\n", "[Nr] Name", "Type",
			    "Address", "Offset");
			printf("       %-18s%-17s%-7s%-6s%-6s%s\n", "Size",
			    "EntSize", "FLags", "Link", "Info", "Align");
		}
	}
	for (i = 0; (size_t)i < re->shnum; i++) {
		s = &re->sl[i];
		if (re->options & RE_T) {
			printf("  [%2d] %s\n", i, s->name);
			printf("%7s", "");
		} else
			printf("  [%2d] %-17.17s ", i, s->name);
		if (re->ec == ELFCLASS32) {
			if (re->options & RE_T) {
				dump_section_type(re, s->type);
				printf("%8.8lx %6.6lx %6.6lx %2.2lx  ", s->addr,
				    s->off, s->sz, s->entsize);
				printf("%2u %3u %2lu\n", s->link, s->info,
				    s->align);
				printf("%7s", "");
				dump_section_flags(re, s);
			} else {
				dump_section_type(re, s->type);
				printf("%8.8lx %6.6lx %6.6lx %2.2lx ", s->addr,
				    s->off, s->sz, s->entsize);
				dump_section_flags(re, s);
				printf("%2u %3u %2lu\n", s->link, s->info,
				    s->align);
			}
		} else if (re->options & RE_WW) {
			if (re->options & RE_T) {
				dump_section_type(re, s->type);
				printf("%16.16lx %6.6lx %6.6lx %2.2lx ",
				    s->addr, s->off, s->sz, s->entsize);
				printf(" %2u %3u %2lu\n", s->link, s->info,
				    s->align);
				printf("%7s", "");
				dump_section_flags(re, s);
			} else {
				dump_section_type(re, s->type);
				printf("%16.16lx %6.6lx %6.6lx %2.2lx ",
				    s->addr, s->off, s->sz, s->entsize);
				dump_section_flags(re, s);
				printf("%2u %3u %2lu\n", s->link, s->info,
				    s->align);
			}
		} else {
			if (re->options & RE_T) {
				dump_section_type(re, s->type);
				printf(" %16.16lx  %16.16lx  %u\n", s->addr,
				    s->off, s->link);
				printf("       %16.16lx %16.16lx  %-16u  %ju\n",
				    s->sz, s->entsize, s->info, s->align);
				printf("%7s", "");
				dump_section_flags(re, s);
			} else {
				dump_section_type(re, s->type);
				printf(" %16.16lx  %8.8lx\n", s->addr,
				    s->off);
				printf("       %16.16lx  %16.16lx ", s->sz,
				    s->entsize);
				dump_section_flags(re, s);
				printf("     %2u   %3u     %lu\n", s->link,
				    s->info, s->align);
			}
		}
	}
	if ((re->options & RE_T) == 0)
		printf("Key to Flags:\n  W (write), A (alloc),"
		    " X (execute), M (merge), S (strings)\n"
		    "  I (info), L (link order), G (group), x (unknown)\n"
		    "  O (extra OS processing required)"
		    " o (OS specific), p (processor specific)\n");
}

static void
dump_dynamic(struct readelf *re)
{
	GElf_Dyn	 dyn;
	Elf_Data	*d;
	struct section	*s;
	int		 i, j;

	if ((re->flags & SECTIONS_LOADED) == 0 || re->shnum == 0)
		return;

	for (i = 0; (size_t)i < re->shnum; i++) {
		s = &re->sl[i];
		if (s->type != SHT_DYNAMIC)
			continue;
		if ((d = elf_getdata(s->scn, NULL)) == NULL) {
			warnx("elf_getdata failed: %s", elf_errmsg(-1));
			return;
		}
		if (d->d_size <= 0)
			return;
		printf("\nDynamic section at offset 0x%lx", s->off);
		printf(" contains %ju entries:\n", s->sz / s->entsize);
		if (re->ec == ELFCLASS32)
			printf("%5s%12s%28s\n", "Tag", "Type", "Name/Value");
		else
			printf("%5s%20s%28s\n", "Tag", "Type", "Name/Value");
		for (j = 0; (uint64_t)j < s->sz / s->entsize; j++) {
			if (gelf_getdyn(d, j, &dyn) != &dyn) {
				warnx("gelf_getdyn failed: %s", elf_errmsg(-1));
				continue;
			}
			/* Dump dynamic entry type. */
			if (re->ec == ELFCLASS32)
				printf(" 0x%8.8lx", dyn.d_tag);
			else
				printf(" 0x%16.16lx", dyn.d_tag);
			printf(" %-20s", lookup_define(dt_type, dyn.d_tag));
			/* Dump dynamic entry value. */
			dump_dyn_val(re, &dyn, s->link);
		}
	}
}

static void
dump_dyn_val(struct readelf *re, GElf_Dyn *dyn, uint32_t stab)
{
	const char *name;

	/* These entry values are index into the string table. */
	name = NULL;
	if (dyn->d_tag == DT_NEEDED || dyn->d_tag == DT_SONAME ||
	    dyn->d_tag == DT_RPATH || dyn->d_tag == DT_RUNPATH) {
		if (stab == SHN_UNDEF)
			name = "ERROR";
		else if ((name = elf_strptr(re->elf, stab, dyn->d_un.d_val)) ==
		    NULL) {
			(void) elf_errno(); /* clear error */
			name = "ERROR";
		}
	}

	switch(dyn->d_tag) {
	case DT_NULL:
	case DT_PLTGOT:
	case DT_HASH:
	case DT_STRTAB:
	case DT_SYMTAB:
	case DT_RELA:
	case DT_INIT:
	case DT_SYMBOLIC:
	case DT_REL:
	case DT_DEBUG:
	case DT_TEXTREL:
	case DT_JMPREL:
	case DT_FINI:
	case DT_VERDEF:
	case DT_VERNEED:
	case DT_VERSYM:
		printf(" 0x%lx\n", dyn->d_un.d_val);
		break;
	case DT_PLTRELSZ:
	case DT_RELASZ:
	case DT_RELAENT:
	case DT_STRSZ:
	case DT_SYMENT:
	case DT_RELSZ:
	case DT_RELENT:
	case DT_INIT_ARRAYSZ:
	case DT_FINI_ARRAYSZ:
		printf(" %ju (bytes)\n", dyn->d_un.d_val);
		break;
 	case DT_RELACOUNT:
	case DT_RELCOUNT:
	case DT_VERDEFNUM:
	case DT_VERNEEDNUM:
		printf(" %ju\n", dyn->d_un.d_val);
		break;
	case DT_NEEDED:
		printf(" Shared libarary: [%s]\n", name);
		break;
	case DT_SONAME:
		printf(" Library soname: [%s]\n", name);
		break;
	case DT_RPATH:
		printf(" Library rpath: [%s]\n", name);
		break;
	case DT_RUNPATH:
		printf(" Library runpath: [%s]\n", name);
		break;
	case DT_PLTREL:
		printf(" %s\n", lookup_define(dt_type, dyn->d_un.d_val));
		break;
	default:
		printf("\n");
	}
}

static void
dump_st_type(int stt)
{
	const char *name;

	if ((name = lookup_define(st_type, stt)) != NULL)
		printf(" %-7s", name);
	else if (stt >= STT_LOOS && stt <= STT_HIOS)
		printf(" %-7s", "os");
	else if (stt >= STT_LOPROC && stt <= STT_HIPROC)
		printf(" %-7s", "proc");
	else
		printf(" %-7s", "unknown");

}

static void
dump_st_bind(int stb)
{
	const char *name;

	if ((name = lookup_define(st_bind, stb)) != NULL)
		printf(" %-6s", name);
	else if (stb >= STB_LOOS && stb <= STB_HIOS)
		printf(" %-6s", "os");
	else if (stb >= STB_LOPROC && stb <= STB_HIPROC)
		printf(" %-6s", "proc");
	else
		printf(" %-7s", "unknown");
}

static void
dump_st_shndx(int shndx)
{

	if (shndx == SHN_UNDEF)
		printf(" UND");
	else if (shndx == SHN_ABS)
		printf(" ABS");
	else if (shndx == SHN_COMMON)
		printf(" COM");
	else if (shndx >= SHN_LOPROC && shndx <= SHN_HIPROC)
		printf(" PRC");
	else if (shndx >= SHN_LOOS && shndx <= SHN_HIOS)
		printf("  OS");
	else
		printf(" %3d", shndx);
}

static void
dump_symtab(struct readelf *re, int i)
{
	struct section *s;
	Elf_Data *d;
	GElf_Sym sym;
	const char *name;
	int elferr, stab, j;

	s = &re->sl[i];
	stab = s->link;
	(void) elf_errno();
	if ((d = elf_getdata(s->scn, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			warnx("elf_getdata failed: %s", elf_errmsg(elferr));
		return;
	}
	if (d->d_size <= 0)
		return;
	printf("%7s%9s%14s%5s%8s%6s%9s%5s\n", "Num:", "Value", "Size", "Type",
	    "Bind", "Vis", "Ndx", "Name");
	for (j = 0; (uint64_t)j < s->sz / s->entsize; j++) {
		if (gelf_getsym(d, j, &sym) != &sym) {
			warnx("gelf_getsym failed: %s", elf_errmsg(-1));
			continue;
		}
		printf("%6d:", j);
		printf(" %16.16lx", sym.st_value);
		printf(" %5ju", sym.st_size);
		dump_st_type(GELF_ST_TYPE(sym.st_info));
		dump_st_bind(GELF_ST_BIND(sym.st_info));
		printf(" DEFAULT "); /* FIXME */
		dump_st_shndx(sym.st_shndx);
		if ((name = elf_strptr(re->elf, stab, sym.st_name)) != NULL)
			printf(" %s", name);
		printf("\n");
	}

}

static void
dump_symtabs(struct readelf *re)
{
	int i;

	if ((re->flags & SECTIONS_LOADED) == 0 || re->shnum == 0)
		return;

	for (i = 0; (size_t)i < re->shnum; i++) {
		if (re->sl[i].type == SHT_SYMTAB ||
		    re->sl[i].type == SHT_DYNSYM)
			dump_symtab(re, i);
	}
}

static void
dump_hash(struct readelf *re)
{
	Elf_Data	*d;
	struct section	*s;
	uint32_t	*buf;
	uint32_t	 nbucket, nchain;
	uint32_t	*bucket, *chain;
	uint32_t	*bl, *c, maxl, total;
	int		 elferr, i, j;

	/* TODO: Add support for .gnu.hash section. */

	if ((re->flags & SECTIONS_LOADED) == 0 || re->shnum == 0)
		return;

	/* Find .hash section. */
	for (i = 0; (size_t)i < re->shnum; i++) {
		s = &re->sl[i];
		if (s->type == SHT_HASH)
			break;
	}
	if ((size_t)i >= re->shnum)
		return;

	/* Read and parse the content of .hash section. */
	(void) elf_errno();
	if ((d = elf_getdata(s->scn, NULL)) == NULL) {
		elferr = elf_errno();
		if (elferr != 0)
			warnx("elf_getdata failed: %s", elf_errmsg(elferr));
		return;
	}
	if (d->d_size < 2 * sizeof(uint32_t)) {
		warnx(".hash section too small");
		return;
	}
	buf = d->d_buf;
	nbucket = buf[0];
	nchain = buf[1];
	if (nbucket <= 0 || nchain <= 0) {
		warnx("Malformed .hash section");
		return;
	}
	if (d->d_size != (nbucket + nchain + 2) * sizeof(uint32_t)) {
		warnx("Malformed .hash section");
		return;
	}
	bucket = &buf[2];
	chain = &buf[2 + nbucket];

	maxl = 0;
	if ((bl = calloc(nbucket, sizeof(*bl))) == NULL)
		errx(EX_SOFTWARE, "calloc failed");
	for (i = 0; (uint32_t)i < nbucket; i++)
		for (j = bucket[i]; j > 0 && (uint32_t)j < nchain; j = chain[j])
			if (++bl[i] > maxl)
				maxl = bl[i];
	if ((c = calloc(maxl + 1, sizeof(*c))) == NULL)
		errx(EX_SOFTWARE, "calloc failed");
	for (i = 0; (uint32_t)i < nbucket; i++)
		c[bl[i]]++;
	printf("Histogram for bucket list length (total of %u buckets):\n",
	    nbucket);
	printf(" Length\tNumber\t\t%% of total\tCoverage\n");
	total = 0;
	for (i = 0; (uint32_t)i <= maxl; i++) {
		total += c[i] * i;
		printf("%7u\t%-10u\t(%5.1f%%)\t%5.1f%%\n", i, c[i],
		    c[i] * 100.0 / nbucket, total * 100.0 / (nchain - 1));
	}
	free(c);
	free(bl);
}

static void
hex_dump(struct readelf *re)
{
	struct section *s;
	Elf_Data *d;
	uint8_t *buf;
	size_t sz, nbytes;
	uint64_t addr;
	int i, j;

	if ((re->flags & SECTIONS_LOADED) == 0)
		return;

	for (i = 1; (size_t)i < re->shnum; i++) {
		if (find_dumpop(re, (size_t)i, HEX_DUMP) == NULL)
			continue;
		s =& re->sl[i];
		if ((d = elf_getdata(s->scn, NULL)) == NULL) {
			warnx("elf_getdata failed: %s", elf_errmsg(-1));
			continue;
		}
		if (d->d_size <= 0)
			continue;
		buf = d->d_buf;
		sz = d->d_size;
		addr = s->addr;
		printf("\nHex dump of section '%s':\n", s->name);
		while (sz > 0) {
			printf("  0x%8.8lx ", addr);
			nbytes = sz > 16? 16 : sz;
			for (j = 0; j < 16; j++) {
				if ((size_t)j < nbytes)
					printf("%2.2x", buf[j]);
				else
					printf("  ");
				if ((j & 3) == 3)
					printf(" ");
			}
			for (j = 0; (size_t)j < nbytes; j++) {
				if (isprint(buf[j]))
					printf("%c", buf[j]);
				else
					printf(".");
			}
			printf("\n");
			buf += nbytes;
			addr += nbytes;
			sz -= nbytes;
		}
	}
}

static void
load_sections(struct readelf *re)
{
	struct section	*s;
	const char	*name;
	Elf_Scn		*scn;
	GElf_Shdr	 sh;
	size_t		 shstrndx, ndx;
	int		 elferr;
	

	if (re->flags & SECTIONS_LOADED)
		return;

	/* Allocate storage for internal section list. */
	if (!elf_getshnum(re->elf, &re->shnum)) {
		warnx("elf_getshnum failed: %s", elf_errmsg(-1));
		return;
	}
	if (re->sl != NULL)
		free(re->sl);
	if ((re->sl = calloc(re->shnum, sizeof(*re->sl))) == NULL)
		err(EX_SOFTWARE, "calloc failed");

	/* Get the index of .shstrtab section. */
	if (!elf_getshstrndx(re->elf, &shstrndx)) {
		warnx("elf_getshstrndx failed: %s", elf_errmsg(-1));
		return;
	}

	if ((scn = elf_getscn(re->elf, 0)) == NULL) {
		warnx("elf_getscn failed: %s", elf_errmsg(-1));
		return;
	}

	(void) elf_errno();
	do {
		if (gelf_getshdr(scn, &sh) == NULL) {
			warnx("gelf_getshdr failed: %s", elf_errmsg(-1));
			(void) elf_errno();
			continue;
		}
		if ((name = elf_strptr(re->elf, shstrndx, sh.sh_name)) == NULL) {
			(void) elf_errno();
			name = "ERROR";
		}
		if ((ndx = elf_ndxscn(scn)) == SHN_UNDEF) {
			if ((elferr = elf_errno()) != 0)
				warnx("elf_ndxscn failed: %s",
				    elf_errmsg(elferr));
			continue;
		}
		if (ndx >= re->shnum) {
			warnx("section index of '%s' out of range", name);
			continue;
		}
		s = &re->sl[ndx];
		s->name = name;
		s->scn = scn;
		s->off = sh.sh_offset;
		s->sz = sh.sh_size;
		s->entsize = sh.sh_entsize;
		s->align = sh.sh_addralign;
		s->type = sh.sh_type;
		s->flags = sh.sh_flags;
		s->addr = sh.sh_addr;
		s->link = sh.sh_link;
		s->info = sh.sh_info;
	} while ((scn = elf_nextscn(re->elf, scn)) != NULL);
	elferr = elf_errno();
	if (elferr != 0)
		warnx("elf_nextscn failed: %s", elf_errmsg(elferr));

	re->flags |= SECTIONS_LOADED;
}

static void
dump_elf(struct readelf *re)
{

	/* Fetch ELF header. No need to continue if this fails. */
	if (gelf_getehdr(re->elf, &re->ehdr) == NULL) {
		warnx("gelf_getehdr failed: %s", elf_errmsg(-1));
		return;
	}
	if ((re->ec = gelf_getclass(re->elf)) == ELFCLASSNONE) {
		warnx("gelf_getclass failed: %s", elf_errmsg(-1));
		return;
	}

	load_sections(re);

	if (re->options & RE_H)
		dump_ehdr(re);
	if (re->options & RE_L)
		dump_phdr(re);
	if (re->options & RE_SS)
		dump_shdr(re);
	if (re->options & RE_D)
		dump_dynamic(re);
	if (re->options & RE_S)
		dump_symtabs(re);
	if (re->options & RE_II)
		dump_hash(re);
	if (re->options & RE_X)
		hex_dump(re);
}

#ifndef LIBELF_AR

/*
 * Convenient wrapper for general libarchive error handling.
 */
#define	AC(CALL) do {							\
	if ((CALL))							\
		errx(EX_SOFTWARE, "%s", archive_error_string(a));	\
} while (0)

static int
ac_detect_ar(int fd)
{
	struct archive		*a;
	struct archive_entry	*entry;
	int			 r;

	r = -1;
	if ((a = archive_read_new()) == NULL)
		return (0);
	archive_read_support_compression_all(a);
	archive_read_support_format_ar(a);
	if (archive_read_open_fd(a, fd, 10240) == ARCHIVE_OK)
		r = archive_read_next_header(a, &entry);
	archive_read_close(a);
	archive_read_finish(a);

	return (r == ARCHIVE_OK);
}

static void
ac_dump_ar(struct readelf *re, int fd)
{
	struct archive		*a;
	struct archive_entry	*entry;
	const char		*name;
	void			*buff;
	size_t			 size;
	int			 r;

	if (lseek(fd, 0, SEEK_SET) == -1)
		err(EX_IOERR, "lseek failed");
	if ((a = archive_read_new()) == NULL)
		errx(EX_SOFTWARE, "%s", archive_error_string(a));
	archive_read_support_compression_all(a);
	archive_read_support_format_ar(a);
	AC(archive_read_open_fd(a, fd, 10240));
	for(;;) {
		r = archive_read_next_header(a, &entry);
		if (r == ARCHIVE_FATAL)
			errx(EX_DATAERR, "%s", archive_error_string(a));
		if (r == ARCHIVE_EOF)
			break;
		if (r == ARCHIVE_WARN || r == ARCHIVE_RETRY)
			warnx("%s", archive_error_string(a));
		if (r == ARCHIVE_RETRY)
			continue;

		name = archive_entry_pathname(entry);

		/* TODO: handle option '-c' here. */

		/* skip pseudo members. */
		if (strcmp(name, "/") == 0 || strcmp(name, "//") == 0)
			continue;

		size = archive_entry_size(entry);
		if (size > 0) {
			if ((buff = malloc(size)) == NULL)
				err(EX_SOFTWARE, "malloc failed");
			if (archive_read_data(a, buff, size) != (ssize_t)size) {
				warnx("%s", archive_error_string(a));
				free(buff);
				continue;
			}
			if ((re->elf = elf_memory(buff, size)) == NULL) {
				warnx("elf_memroy() failed: %s",
				    elf_errmsg(-1));
				free(buff);
				continue;
			}
			dump_elf(re);
			free(buff);
		}
	}
	AC(archive_read_close(a));
	AC(archive_read_finish(a));
}
#endif	/* ! LIBELF_AR */

static void
dump_object(struct readelf *re)
{
	int fd;

	if ((fd = open(re->filename, O_RDONLY)) == -1) {
		warn("open %s failed", re->filename);
		return;
	}

#ifndef	LIBELF_AR
	/*
	 * Detect and process ar(1) archive using libarchive.
	 */
	if (ac_detect_ar(fd)) {
		ac_dump_ar(re, fd);
		return;
	}
#endif	/* ! LIBELF_AR */

	if ((re->flags & DISPLAY_FILENAME) != 0)
		printf("\nFile: %s\n", re->filename);

	if ((re->elf = elf_begin(fd, ELF_C_READ, NULL)) == NULL) {
		warnx("elf_begin() failed: %s", elf_errmsg(-1));
		return;
	}

	switch (elf_kind(re->elf)) {
	case ELF_K_NONE:
		warnx("Not an ELF file.");
		return;
	case ELF_K_ELF:
		dump_elf(re);
		break;
	case ELF_K_AR:
		/* dump_ar(re); */
		break;
	default:
		warnx("Internal: libelf returned unknown elf kind.");
		return;
	}

	elf_end(re->elf);
}

static void
add_dumpop(struct readelf *re, size_t sn, int op)
{
	struct dumpop *d;

	if ((d = find_dumpop(re, sn, 0)) == NULL) {
		if ((d = malloc(sizeof(*d))) == NULL)
			err(EX_SOFTWARE, "malloc failed");
		d->sn = sn;
		d->op = op;
		STAILQ_INSERT_TAIL(&re->v_dumpop, d, dumpop_list);
	} else
		d->op |= op;
}

static struct dumpop *
find_dumpop(struct readelf *re, size_t sn, int op)
{
	struct dumpop *d;

	STAILQ_FOREACH(d, &re->v_dumpop, dumpop_list) {
		if (d->sn == sn && (op == 0 || op & d->op))
			return (d);
	}

	return (NULL);
}

static void
readelf_version()
{

	exit(EX_OK);
}

static void
readelf_help()
{

	exit(EX_OK);
}

static void
readelf_usage()
{

	fprintf(stderr, "usage: \n");
	exit(EX_USAGE);
}

int
main(int argc, char **argv)
{
	struct readelf	*re, re_storage;
	unsigned long	 sn;
	int		 opt, i;

	if (argc < 3)
		readelf_usage();

	re = &re_storage;
	memset(re, 0, sizeof(*re));
	STAILQ_INIT(&re->v_dumpop);

	while ((opt = getopt_long(argc, argv, "AacDdegHhIi:lNnprSstuVvWw:x:",
	    longopts, NULL)) != -1) {
		switch(opt) {
		case 'A':
			re->options |= RE_AA;
			break;
		case 'a':
			re->options |= RE_AA | RE_D | RE_H | RE_II | RE_L |
			    RE_R | RE_SS | RE_S | RE_VV;
			break;
		case 'c':
			re->options |= RE_C;
			break;
		case 'D':
			re->options |= RE_DD;
			break;
		case 'd':
			re->options |= RE_D;
			break;
		case 'e':
			re->options |= RE_H | RE_L | RE_SS;
			break;
		case 'g':
			re->options |= RE_G;
			break;
		case 'H':
			readelf_help();
			break;
		case 'h':
			re->options |= RE_H;
			break;
		case 'I':
			re->options |= RE_II;
			break;
		case 'i':
			/* Not implemented yet. */
			break;
		case 'l':
			re->options |= RE_L;
			break;
		case 'N':
			re->options |= RE_NN;
			break;
		case 'n':
			re->options |= RE_N;
			break;
		case 'p':
			re->options |= RE_P;
			break;
		case 'r':
			re->options |= RE_R;
			break;
		case 'S':
			re->options |= RE_SS;
			break;
		case 's':
			re->options |= RE_S;
			break;
		case 't':
			re->options |= RE_T;
			break;
		case 'u':
			re->options |= RE_U;
			break;
		case 'V':
			re->options |= RE_VV;
			break;
		case 'v':
			readelf_version();
			break;
		case 'W':
			re->options |= RE_WW;
			break;
		case 'w':
			re->options |= RE_W;
			/* TODO: Parse -w optarg. */
			break;
		case 'x':
			re->options |= RE_X;
			sn = strtoul(optarg, NULL, 10);
			add_dumpop(re, (size_t)sn, HEX_DUMP);
			break;
		case OPTION_DEBUG_DUMP:
			re->options |= RE_W;
			/* TODO: Parse debug-dump optarg. */
		}
	}

	argv += optind;
	argc -= optind;

	if (argc > 1)
		re->flags |= DISPLAY_FILENAME;

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EX_SOFTWARE, "ELF library initialization failed: %s",
		    elf_errmsg(-1));

	for (i = 0; i < argc; i++)
		if (argv[i] != NULL) {
			re->filename = argv[i];
			dump_object(re);
		}

	exit(EX_OK);
}
