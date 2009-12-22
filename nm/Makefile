# $Id$

TOP=	..

PROG=	nm
SRCS=	nm.c vector_str.c cpp_demangle.c cpp_demangle_arm.c \
	cpp_demangle_gnu2.c dwarf_line_number.c
LDADD=	-lelf -ldwarf

.include "${TOP}/mk/elftoolchain.prog.mk"
