/*-
 * Copyright (c) 2007 S.Sam Arun Raj
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

#include <sys/cdefs.h>

#include <err.h>
#include <fcntl.h>
#include <getopt.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sysexits.h>
#include <unistd.h>

#include <libelf.h>
#include <gelf.h>

#include "_elftc.h"

ELFTC_VCSID("$Id$");

#define	BUF_SIZE			40
#define	DEFAULT_SECTION_NAME_LENGTH	19
#define	ELF_ALIGN(val,x) (((val)+(x)-1) & ~((x)-1))
#define	SIZE_VERSION_STRING		"size 1.0"

#ifndef	NT_AUXV
#define	NT_AUXV			6
#endif
#ifndef	NT_LWPSTATUS
#define	NT_LWPSTATUS		16
#endif
#ifndef	NT_PRFPREG
#define	NT_PRFPREG		2
#endif
#ifndef	NT_PRPSINFO
#define	NT_PRPSINFO		3
#endif
#ifndef	NT_PRSTATUS
#define	NT_PRSTATUS		1
#endif
#ifndef	NT_PRXFPREG
#define	NT_PRXFPREG		0x46e62b7f
#endif
#ifndef	NT_PSINFO
#define	NT_PSINFO		13
#endif
#ifndef	NT_PSTATUS
#define	NT_PSTATUS		10
#endif
#ifndef	PT_GNU_EH_FRAME
#define	PT_GNU_EH_FRAME		(PT_LOOS + 0x474e550)
#endif
#ifndef	PT_GNU_STACK
#define	PT_GNU_STACK		(PT_LOOS + 0x474e551)
#endif

enum output_style {
	STYLE_BERKELEY,
	STYLE_SYSV
};

enum radix_style {
	RADIX_OCTAL,
	RADIX_DECIMAL,
	RADIX_HEX
};

uint32_t bss_size, data_size, text_size, total_size;
uint32_t bss_size_total, data_size_total, text_size_total;

int show_totals;

size_t sec_name_len	= DEFAULT_SECTION_NAME_LENGTH;
enum radix_style radix	= RADIX_DECIMAL;
enum output_style style = STYLE_BERKELEY;

const char *default_args[2] = { "a.out", NULL };

enum {
	OPT_FORMAT,
	OPT_RADIX
};

int	size_option;

static struct option size_longopts[] = {
	{ "format",	required_argument, &size_option, OPT_FORMAT },
	{ "help",	no_argument,	NULL,	'h' },
	{ "radix",	required_argument, &size_option, OPT_RADIX },
	{ "totals",	no_argument,	NULL,	't' },
	{ "version",	no_argument,	NULL,	'v' },
	{ NULL, 0, NULL, 0 }  
};

void	berkeley_calc(GElf_Shdr *);
void	berkeley_footer(const char *, const char *, const char *);
void	berkeley_header(void);
void	berkeley_totals(void);
int	handle_core(char const *, Elf *elf, GElf_Ehdr *);
void	handle_core_note(Elf *, GElf_Ehdr *, GElf_Phdr *, char **);
int	handle_elf(char const *);
void	handle_phdr(Elf *, GElf_Ehdr *, GElf_Phdr *, uint32_t,
    	    const char *);
void	print_number(int, uint32_t, enum radix_style, char);
void	show_version(void);
void	sysv_header(const char *, Elf_Arhdr *);
void	sysv_footer(void);
void	sysv_calc(Elf *, GElf_Ehdr *, GElf_Shdr *, int);
void	usage(void);

/*
 * size utility using elf(3) and gelf(3) API to list section sizes and
 * total in elf files. Supports only elf files (core dumps in elf
 * included) that can be opened by libelf, other formats are not supported.
 */
int
main(int argc, char **argv)
{
	int ch, exitcode, r;
	const char **files, *fn;

	exitcode = EX_OK;

	if (elf_version(EV_CURRENT) == EV_NONE)
		errx(EX_SOFTWARE, "ELF library initialization failed: %s",
		    elf_errmsg(-1));

	while ((ch = getopt_long(argc, argv, "ABVdhotx", size_longopts,
	    NULL)) != -1)
		switch((char)ch) {
		case 'A':
			style = STYLE_SYSV;
			break;
		case 'B':
			style = STYLE_BERKELEY;
			break;
		case 'V':
			show_version();
			break;
		case 'd':
			radix = RADIX_DECIMAL;
			break;
		case 'o':
			radix = RADIX_OCTAL;
			break;
		case 't':
			show_totals = 1;
			break;
		case 'x':
			radix = RADIX_HEX;
			break;
		case 0:
			switch (size_option) {
			case OPT_FORMAT:
				if (*optarg == 's' || *optarg == 'S')
					style = STYLE_SYSV;
				else if (*optarg == 'b' || *optarg == 'B')
					style = STYLE_BERKELEY;
				else {
					warnx("unrecognized format \"%s\".",
					      optarg);
					usage();
				}
				break;
			case OPT_RADIX:
				r = strtol(optarg, NULL, 10);
				if (r == 8)
					radix = RADIX_OCTAL;
				else if (r == 10)
					radix = RADIX_DECIMAL;
				else if (r == 16)
					radix = RADIX_HEX;
				else {
					warnx("unsupported radix \"%s\".",
					      optarg);
					usage();
				}
				break;
			default:
				err(EX_SOFTWARE, "Error in option handling.");
				/*NOTREACHED*/
			}
			break;
		case 'h':
		case '?':
		default:
			usage();
			/* NOTREACHED */
		}
	argc -= optind;
	argv += optind;

	files = (argc == 0) ? default_args : (const char **) argv;

	while ((fn = *files) != NULL) {
		exitcode = handle_elf(fn);
		if (exitcode != EX_OK)
			warnx(exitcode == EX_NOINPUT ?
			      "'%s': No such file" :
			      "%s: File format not recognized", fn);
		files++;
	}
	if (style == STYLE_BERKELEY && show_totals)
		berkeley_totals();
        return (exitcode);
}

static Elf_Data *
xlatetom(Elf *elf, GElf_Ehdr *elfhdr, void *_src, void *_dst,
    Elf_Type type, size_t size)
{
	Elf_Data src, dst;

	src.d_buf = _src;
	src.d_type = type;
	src.d_version = elfhdr->e_version;
	src.d_size = size;
	dst.d_buf = _dst;
	dst.d_version = elfhdr->e_version;
	dst.d_size = size;
	return (gelf_xlatetom(elf, &dst, &src, elfhdr->e_ident[EI_DATA]));
}

#define NOTE_OFFSET_32(nhdr, namesz, offset) 			\
	((char *)nhdr + sizeof(Elf32_Nhdr) +			\
	    ELF_ALIGN((int32_t)namesz, 4) + offset)

#define NOTE_OFFSET_64(nhdr, namesz, offset) 			\
	((char *)nhdr + sizeof(Elf32_Nhdr) +			\
	    ELF_ALIGN((int32_t)namesz, 8) + offset)

#define PID32(nhdr, namesz, offset) 				\
	(pid_t)*((int *)((uintptr_t)NOTE_OFFSET_32(nhdr,	\
	    namesz, offset)));

#define PID64(nhdr, namesz, offset) 				\
	(pid_t)*((int *)((uintptr_t)NOTE_OFFSET_64(nhdr,	\
	    namesz, offset)));

#define NEXT_NOTE(elfhdr, descsz, namesz, offset) do {		\
	if (elfhdr->e_ident[EI_CLASS] == ELFCLASS32) { 		\
		offset += ELF_ALIGN((int32_t)descsz, 4) +	\
		    sizeof(Elf32_Nhdr) + 			\
			ELF_ALIGN((int32_t)namesz, 4); 		\
	} else {						\
		offset += ELF_ALIGN((int32_t)descsz, 8) + 	\
		    sizeof(Elf32_Nhdr) + 			\
		        ELF_ALIGN((int32_t)namesz, 8); 		\
	}							\
} while (0)

/*
 * Parse individual note entries inside a PT_NOTE segment.
 */
void
handle_core_note(Elf *elf, GElf_Ehdr *elfhdr, GElf_Phdr *phdr,
    char **cmd_line)
{
	size_t max_size;
	uint64_t raw_size;
	GElf_Off offset;
	static pid_t pid;
	uintptr_t ver;
	Elf32_Nhdr *nhdr, nhdr_l;
	static int reg_pseudo = 0, reg2_pseudo = 0, regxfp_pseudo = 0;
	char buf[BUF_SIZE], *data, *name;

 	if (elf == NULL || elfhdr == NULL || phdr == NULL)
		return;

	data = elf_rawfile(elf, &max_size);
	offset = phdr->p_offset;
	while (data != NULL && offset < phdr->p_offset + phdr->p_filesz) {
		nhdr = (Elf32_Nhdr *)(uintptr_t)((char*)data + offset);
		memset(&nhdr_l, 0, sizeof(Elf32_Nhdr));
		if (!xlatetom(elf, elfhdr, &nhdr->n_type, &nhdr_l.n_type,
			ELF_T_WORD, sizeof(Elf32_Word)) ||
		    !xlatetom(elf, elfhdr, &nhdr->n_descsz, &nhdr_l.n_descsz,
			ELF_T_WORD, sizeof(Elf32_Word)) ||
		    !xlatetom(elf, elfhdr, &nhdr->n_namesz, &nhdr_l.n_namesz,
			ELF_T_WORD, sizeof(Elf32_Word)))
			break;

		name = (char *)((char *)nhdr + sizeof(Elf32_Nhdr));
		switch (nhdr_l.n_type) {
		case NT_PRSTATUS: {
			raw_size = 0;
			if (elfhdr->e_ident[EI_OSABI] == ELFOSABI_FREEBSD &&
			    nhdr_l.n_namesz == 0x8 &&
			    !strcmp(name,"FreeBSD")) {
				if (elfhdr->e_ident[EI_CLASS] == ELFCLASS32) {
					raw_size = (uint64_t)*((uint32_t *)
					    (uintptr_t)(name +
						ELF_ALIGN((int32_t)
						nhdr_l.n_namesz, 4) + 8));
					ver = (uintptr_t)NOTE_OFFSET_32(nhdr,
					    nhdr_l.n_namesz,0);
					if (*((int *)ver) == 1)
						pid = PID32(nhdr,
						    nhdr_l.n_namesz, 24);
				} else {
					raw_size = *((uint64_t *)(uintptr_t)
					    (name + ELF_ALIGN((int32_t)
						nhdr_l.n_namesz, 8) + 16));
					ver = (uintptr_t)NOTE_OFFSET_64(nhdr,
					    nhdr_l.n_namesz,0);
					if (*((int *)ver) == 1)
						pid = PID64(nhdr,
						    nhdr_l.n_namesz, 40);
				}
				xlatetom(elf, elfhdr, &raw_size, &raw_size,
				    ELF_T_WORD, sizeof(uint64_t));
				xlatetom(elf, elfhdr, &pid, &pid, ELF_T_WORD,
				    sizeof(pid_t));
			}

			if (raw_size != 0 && style == STYLE_SYSV) {
				(void) snprintf(buf, BUF_SIZE, "%s/%d",
				    ".reg", pid);
				(void) printf("%-18s ", buf);
				print_number(10, (uint32_t)raw_size,
				    radix, ' ');
				print_number(10, (uint32_t)0,
				    radix, '\n');
				if (!reg_pseudo) {
					(void) printf("%-18s ", ".reg");
					print_number(10, (uint32_t)raw_size,
					    radix, ' ');
					print_number(10, (uint32_t)0, radix,
					    '\n');
					reg_pseudo = 1;
					text_size_total += raw_size;
				}
				text_size_total += raw_size;
			}
		}
		break;
		case NT_PRFPREG: /* same as NT_FPREGSET */
			if (style == STYLE_SYSV) {
				(void) snprintf(buf, BUF_SIZE,
				    "%s/%d", ".reg2", pid);
				(void) printf("%-18s ", buf);
				print_number(10, (uint32_t)nhdr_l.n_descsz,
				    radix, ' ');
				print_number(10, (uint32_t)0, radix, '\n');
				if (!reg2_pseudo) {
					(void) printf("%-18s ", ".reg2");
					print_number(10,
					    (uint32_t)nhdr_l.n_descsz,
					    radix, ' ');
					print_number(10, (uint32_t)0, radix,
					    '\n');
					reg2_pseudo = 1;
					text_size_total += nhdr_l.n_descsz;
				}
				text_size_total += nhdr_l.n_descsz;
			}
			break;
		case NT_AUXV:
			if (style == STYLE_SYSV) {
				(void) printf("%-18s ", ".auxv");
				print_number(10, (uint32_t)nhdr_l.n_descsz,
				    radix, ' ');
				print_number(10, (uint32_t)0, radix, '\n');
				text_size_total += nhdr_l.n_descsz;
			}
			break;
		case NT_PRXFPREG:
			if (style == STYLE_SYSV) {
				(void) snprintf(buf, BUF_SIZE,
				    "%s/%d", ".reg-xfp", pid);
				(void) printf("%-18s ", buf);
				print_number(10, (uint32_t)nhdr_l.n_descsz,
				    radix, ' ');
				print_number(10, (uint32_t)0, radix, '\n');
				if (!regxfp_pseudo) {
					(void) printf("%-18s ", ".reg-xfp");
					print_number(10,
					    (uint32_t)nhdr_l.n_descsz,
					    radix, ' ');
					print_number(10, (uint32_t)0,
					    radix, '\n');
					regxfp_pseudo = 1;
					text_size_total += nhdr_l.n_descsz;
				}
				text_size_total += nhdr_l.n_descsz;
			}
			break;
		case NT_PSINFO:
		case NT_PRPSINFO: {
			/* FreeBSD 64-bit */
			if (nhdr_l.n_descsz == 0x78 &&
				!strcmp(name,"FreeBSD")) {
				*cmd_line = strdup(NOTE_OFFSET_64(nhdr,
				    nhdr_l.n_namesz, 33));
			/* FreeBSD 32-bit */
			} else if (nhdr_l.n_descsz == 0x6c &&
				!strcmp(name,"FreeBSD")) {
				*cmd_line = strdup(NOTE_OFFSET_32(nhdr,
				    nhdr_l.n_namesz, 25));
			}
			/* Strip any trailing spaces */
			if (*cmd_line != NULL) {
				char *s;

				s = *cmd_line + strlen(*cmd_line);
				while (s > *cmd_line) {
					if (*(s-1) != 0x20) break;
					s--;
				}
				*s = 0;
			}
			break;
		}
		case NT_PSTATUS:
		case NT_LWPSTATUS:
		default:
			break;
		}
		NEXT_NOTE(elfhdr, nhdr_l.n_descsz, nhdr_l.n_namesz, offset);
	}
}

/*
 * Handles program headers except for PT_NOTE, when sysv output stlye is
 * choosen, prints out the segment name and length. For berkely output
 * style only PT_LOAD segments are handled, and text,
 * data, bss size is calculated for them.
 */
void
handle_phdr(Elf *elf, GElf_Ehdr *elfhdr, GElf_Phdr *phdr,
    uint32_t idx, const char *name)
{
	uint32_t addr, size;
	int split;
	char buf[BUF_SIZE];

	if (elf == NULL || elfhdr == NULL || phdr == NULL)
		return;

	size = addr = 0;
	split = (phdr->p_memsz > 0) && 	(phdr->p_filesz > 0) &&
	    (phdr->p_memsz > phdr->p_filesz);

	if (style == STYLE_SYSV) {
		(void) snprintf(buf, BUF_SIZE,
		    "%s%d%s", name, idx, (split ? "a" : ""));
		(void) printf("%-18s ", buf);
		print_number(10, (uint32_t)phdr->p_filesz, radix, ' ');
		print_number(10, (uint32_t)phdr->p_vaddr, radix, '\n');
		text_size_total += phdr->p_filesz;
		if (split) {
			size = (uint32_t)(phdr->p_memsz - phdr->p_filesz);
			addr = (uint32_t)(phdr->p_vaddr + phdr->p_filesz);
			(void) snprintf(buf, BUF_SIZE, "%s%d%s", name,
			    idx, "b");
			text_size_total += phdr->p_memsz - phdr->p_filesz;
			(void) printf("%-18s ", buf);
			print_number(10, size, radix, ' ');
			print_number(10, addr, radix, '\n');
		}
	} else {
		if (phdr->p_type != PT_LOAD)
			return;
		if ((phdr->p_flags & PF_W) && !(phdr->p_flags & PF_X)) {
			data_size += phdr->p_filesz;
			if (split)
				data_size += phdr->p_memsz - phdr->p_filesz;
		} else {
			text_size += phdr->p_filesz;
			if (split)
				text_size += phdr->p_memsz - phdr->p_filesz;
		}
	}
}

/*
 * Given a core dump file, this function maps program headers to segments.
 */
int
handle_core(char const *name, Elf *elf, GElf_Ehdr *elfhdr)
{
	GElf_Phdr phdr;
	uint32_t i;
	char *core_cmdline;
	const char *seg_name;

	if (name == NULL || elf == NULL || elfhdr == NULL)
		return (EX_DATAERR);
	if  (elfhdr->e_shnum != 0 || elfhdr->e_type != ET_CORE)
		return (EX_DATAERR);

	seg_name = core_cmdline = NULL;
	if (style == STYLE_SYSV)
		sysv_header(name, NULL);
	else
		berkeley_header();

	for (i = 0; i < elfhdr->e_phnum; i++) {
		if (gelf_getphdr(elf, i, &phdr) != NULL) {
			if (phdr.p_type == PT_NOTE) {
				handle_phdr(elf, elfhdr, &phdr, i, "note");
				handle_core_note(elf, elfhdr, &phdr,
				    &core_cmdline);
			} else {
				switch(phdr.p_type) {
				case PT_NULL:
					seg_name = "null";
					break;
				case PT_LOAD:
					seg_name = "load";
					break;
				case PT_DYNAMIC:
					seg_name = "dynamic";
					break;
				case PT_INTERP:
					seg_name = "interp";
					break;
				case PT_SHLIB:
					seg_name = "shlib";
					break;
				case PT_PHDR:
					seg_name = "phdr";
					break;
				case PT_GNU_EH_FRAME:
					seg_name = "eh_frame_hdr";
					break;
				case PT_GNU_STACK:
					seg_name = "stack";
					break;
				default:
					seg_name = "segment";
				}
				handle_phdr(elf, elfhdr, &phdr, i, seg_name);
			}
		}
	}

	if (style == STYLE_BERKELEY) {
		if (core_cmdline != NULL) {
			berkeley_footer(core_cmdline, name,
			    "core file invoked as");
		} else {
			berkeley_footer(core_cmdline, name, "core file");
		}
	} else {
		sysv_footer();
		if (core_cmdline != NULL) {
			(void) printf(" (core file invoked as %s)\n\n",
			    core_cmdline);
		} else {
			(void) printf(" (core file)\n\n");
		}
	}
	free(core_cmdline);
	return (EX_OK);
}

/*
 * Given an elf object,ar(1) filename, and based on the output style
 * and radix format the various sections and their length will be printed
 * or the size of the text, data, bss sections will be printed out.
 */
int
handle_elf(char const *name)
{
	GElf_Ehdr elfhdr;
	GElf_Shdr shdr;
	Elf *elf, *elf1;
	Elf_Arhdr *arhdr;
	Elf_Scn *scn;
	Elf_Cmd elf_cmd;
	int exit_code, fd;

	if (name == NULL)
		return (EX_NOINPUT);

	if ((fd = open(name, O_RDONLY, 0)) < 0)
		return (EX_NOINPUT);

	elf_cmd = ELF_C_READ;
	elf1 = elf_begin(fd, elf_cmd, NULL);
	while ((elf = elf_begin(fd, elf_cmd, elf1)) != NULL) {
		arhdr = elf_getarhdr(elf);
		if (elf_kind(elf) == ELF_K_NONE && arhdr == NULL) {
			(void) elf_end(elf);
			(void) elf_end(elf1);
			(void) close(fd);
			return (EX_DATAERR);
		}
		if (elf_kind(elf) != ELF_K_ELF ||
		    (gelf_getehdr(elf, &elfhdr) == NULL)) {
			elf_cmd = elf_next(elf);
			(void) elf_end(elf);
			warnx("%s: File format not recognized",
			    arhdr->ar_name);
			continue;
		}
		/* Core dumps are handled seperately */
		if (elfhdr.e_shnum == 0 && elfhdr.e_type == ET_CORE) {
			exit_code = handle_core(name, elf, &elfhdr);
			(void) elf_end(elf);
			(void) elf_end(elf1);
			(void) close(fd);
			return (exit_code);
		} else {
			scn = NULL;
			if (style == STYLE_BERKELEY) {
				berkeley_header();
				while ((scn = elf_nextscn(elf, scn)) != NULL) {
					if (gelf_getshdr(scn, &shdr) != NULL)
						berkeley_calc(&shdr);
				}
			} else {
				/*
				 * Perform a dry run to find the length of
				 * the largest segment name.
				 */
				while ((scn = elf_nextscn(elf, scn)) != NULL) {
					if (gelf_getshdr(scn, &shdr) !=	NULL) {
						sysv_calc(elf, &elfhdr,
						    &shdr, 1);
					}
				}
				sysv_header(name, arhdr);
				scn = NULL;
				while ((scn = elf_nextscn(elf, scn)) != NULL) {
					if (gelf_getshdr(scn, &shdr) !=	NULL)
						sysv_calc(elf, &elfhdr,
						    &shdr, 0);
				}
			}
			if (style == STYLE_BERKELEY) {
				if (arhdr != NULL) {
					berkeley_footer(name, arhdr->ar_name,
					    "ex");
				} else {
					berkeley_footer(name, NULL, "ex");
				}
			} else {
				sysv_footer();
			}
		}
		elf_cmd = elf_next(elf);
		(void) elf_end(elf);
	}
	(void) elf_end(elf1);
	(void) close(fd);
	return (EX_OK);
}

void
print_number(int width, uint32_t num, enum radix_style rad, char c)
{
	char buffer[BUF_SIZE];

	(void) snprintf(buffer, BUF_SIZE, (rad == RADIX_DECIMAL ? "%lu" :
	    ((rad == RADIX_OCTAL) ? "0%lo" : "0x%lx")),
	    (unsigned long int)num);
	(void) printf("%-*s%c", width, buffer, c);
}

/*
 * Sysv formatting helper functions.
 */
void
sysv_header(const char *name, Elf_Arhdr *arhdr)
{
	text_size_total = 0;
	if (arhdr != NULL) {
		(void) printf("%s   (ex %s):\n%-*s%-10s %-10s\n",
		    arhdr->ar_name, name, (int)sec_name_len,
		    "section","size","addr");
	} else {
		(void) printf("%s  :\n%-*s%-10s %-10s\n",
		    name, (int)sec_name_len, "section",
		    "size", "addr");
	}
}

void
sysv_calc(Elf *elf, GElf_Ehdr *elfhdr, GElf_Shdr *shdr, int dry_run)
{
	char *section_name;

	section_name = elf_strptr(elf, elfhdr->e_shstrndx,
					(size_t)shdr->sh_name);
	if (!dry_run) {
		if ((shdr->sh_type == SHT_SYMTAB ||
		    shdr->sh_type == SHT_STRTAB || shdr->sh_type == SHT_RELA ||
		    shdr->sh_type == SHT_REL) && shdr->sh_addr == 0)
			return;
		(void) printf("%-*s", (int)sec_name_len, section_name);
		print_number(10, (uint32_t)shdr->sh_size, radix, ' ');
		print_number(10, (uint32_t)shdr->sh_addr, radix, '\n');
		text_size_total += shdr->sh_size;
	} else {
		if (sec_name_len < strlen(section_name))
			sec_name_len = strlen(section_name) + 3;
	}
}

void
sysv_footer(void)
{
	(void) printf("%-*s", (int)sec_name_len, "Total");
	print_number(10, text_size_total, radix, '\n');
	(void) printf("\n");
}

/*
 * berkeley style output formatting helper functions.
 */
void
berkeley_header(void)
{
	text_size = data_size = bss_size = 0;
}

void
berkeley_calc(GElf_Shdr *shdr)
{
	if (shdr != NULL) {
		if (!(shdr->sh_flags & SHF_ALLOC))
			return;
		if ((shdr->sh_flags & SHF_ALLOC) &&
		    ((shdr->sh_flags & SHF_EXECINSTR) ||
		    !(shdr->sh_flags & SHF_WRITE)))
			text_size += shdr->sh_size;
		else if ((shdr->sh_flags & SHF_ALLOC) &&
		    (shdr->sh_flags & SHF_WRITE) &&
		    (shdr->sh_type != SHT_NOBITS))
			data_size += shdr->sh_size;
		else
			bss_size += shdr->sh_size;
	}
}

void
berkeley_totals(void)
{
	long unsigned int grand_total;

	grand_total = text_size_total + data_size_total + bss_size_total;
	print_number(10, text_size_total, radix, ' ');
	print_number(10, data_size_total, radix, ' ');
	print_number(10, bss_size_total, radix, ' ');
	if (radix == RADIX_OCTAL)
		print_number(10, grand_total, RADIX_OCTAL, ' ');
	else
		print_number(10, grand_total, RADIX_DECIMAL, ' ');
	(void) printf("%-10lx (TOTALS)\n", grand_total);
}

void
berkeley_footer(const char *name, const char *ar_name, const char *msg)
{
	static int header_printed;
	const char *col_name;

	if (!header_printed) {
		(radix == RADIX_OCTAL) ? (col_name = "oct") :
		    (col_name = "dec");
		(void) printf("%-10s %-10s %-10s %-10s %-10s filename\n",
		    "text","data","bss",col_name,"hex");
		header_printed = 1;
	}

	total_size = text_size + data_size + bss_size;
	if (show_totals) {
		text_size_total += text_size;
		bss_size_total += bss_size;
		data_size_total += data_size;
	}

	print_number(10, text_size, radix, ' ');
	print_number(10, data_size, radix, ' ');
	print_number(10, bss_size, radix, ' ');
	if (radix == RADIX_OCTAL)
		print_number(10, total_size, RADIX_OCTAL, ' ');
	else
		print_number(10, total_size, RADIX_DECIMAL, ' ');
	(void) printf("%-10lx\t", (long unsigned int)total_size);
	if (ar_name != NULL && name != NULL)
		(void) printf("%s (%s %s)\n", ar_name, msg, name);
	else if (ar_name != NULL && name == NULL)
		(void) printf("%s (%s)\n", ar_name, msg);
	else
		(void) printf("%s\n", name);
}

static const char *usagemsg = "\
Usage: size [options] file ...\n\
  Display sizes of ELF sections.\n\n\
  Options:\n\
  --format=format    Display output in specified format.  Supported\n\
                     values are `berkeley' and `sysv'.\n\
  --help             Display this help message and exit.\n\
  --radix=radix      Display numeric values in the specified radix.\n\
                     Supported values are: 8, 10 and 16.\n\
  --totals           Show cumulative totals of section sizes.\n\
  --version          Display program version and exit.\n\
  -A                 Equivalent to `--format=sysv'.\n\
  -B                 Equivalent to `--format=berkeley'.\n\
  -V                 Equivalent to `--version;.\n\
  -d                 Equivalent to `--radix=10'.\n\
  -h                 Same as option --help.\n\
  -o                 Equivalent to `--radix=8'.\n\
  -t                 Equivalent to option --totals.\n\
  -x                 Equivalent to `--radix=16'.\n";

void
usage(void)
{
	(void) fprintf(stderr, "%s", usagemsg);
	exit(EX_USAGE);
}

void
show_version(void)
{
	(void) fprintf(stdout, SIZE_VERSION_STRING "\n");
	exit(EX_OK);
}
