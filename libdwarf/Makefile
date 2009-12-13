# $FreeBSD$

LIB=	dwarf

SRCS=	\
	dwarf_abbrev.c		\
	dwarf_arange.c		\
	dwarf_attr.c		\
	dwarf_attrval.c		\
	dwarf_cu.c		\
	dwarf_dealloc.c		\
	dwarf_die.c		\
	dwarf_dump.c		\
	dwarf_errmsg.c		\
	dwarf_finish.c		\
	dwarf_form.c		\
	dwarf_frame.c		\
	dwarf_funcs.c		\
	dwarf_init.c		\
	dwarf_lineno.c		\
	dwarf_loclist.c		\
	dwarf_macinfo.c		\
	dwarf_pro_attr.c	\
	dwarf_pro_init.c	\
	dwarf_pro_die.c		\
	dwarf_pubnames.c	\
	dwarf_pubtypes.c	\
	dwarf_ranges.c		\
	dwarf_str.c		\
	dwarf_types.c		\
	dwarf_vars.c		\
	dwarf_weaks.c		\
	libdwarf.c		\
	libdwarf_abbrev.c	\
	libdwarf_arange.c	\
	libdwarf_attr.c		\
	libdwarf_die.c		\
	libdwarf_elf_access.c	\
	libdwarf_elf_init.c	\
	libdwarf_frame.c	\
	libdwarf_info.c		\
	libdwarf_init.c		\
	libdwarf_lineno.c	\
	libdwarf_loc.c		\
	libdwarf_loclist.c	\
	libdwarf_macinfo.c	\
	libdwarf_nametbl.c	\
	libdwarf_ranges.c	\
	libdwarf_rw.c		\
	libdwarf_str.c

INCS=	dwarf.h libdwarf.h

GENSRCS=	dwarf_pubnames.c dwarf_pubtypes.c dwarf_weaks.c \
		dwarf_funcs.c dwarf_vars.c dwarf_types.c
CLEANFILES=	${GENSRCS}
CFLAGS+=	-I. -I${.CURDIR} -g

SHLIB_MAJOR=	3

WARNS?=	6

MAN=	dwarf_errmsg.3					\
	dwarf_errno.3					\
	dwarf_finish.3					\
	dwarf_get_elf.3					\
	dwarf_init.3

MLINKS+= \
	dwarf_init.3	dwarf_elf_init.3

dwarf_pubnames.c:	dwarf_nametbl.m4 dwarf_pubnames.m4
dwarf_pubtypes.c:	dwarf_nametbl.m4 dwarf_pubtypes.m4
dwarf_weaks.c:		dwarf_nametbl.m4 dwarf_weaks.m4
dwarf_funcs.c:		dwarf_nametbl.m4 dwarf_funcs.m4
dwarf_vars.c:		dwarf_nametbl.m4 dwarf_vars.m4
dwarf_types.c:		dwarf_nametbl.m4 dwarf_types.m4

.include <bsd.lib.mk>

.SUFFIXES:	.m4 .c
.m4.c:
	m4 -D SRCDIR=${.CURDIR} ${.IMPSRC} > ${.TARGET}
