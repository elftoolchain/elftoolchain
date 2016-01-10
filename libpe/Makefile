# $Id$

TOP=	${.CURDIR}/..

LIB=	pe

SRCS=	libpe_buffer.c	\
	libpe_coff.c	\
	libpe_dos.c	\
	libpe_init.c	\
	libpe_rich.c	\
	libpe_section.c	\
	libpe_utils.c	\
	pe_cntl.c	\
	pe_coff.c	\
	pe_dos.c	\
	pe_buffer.c	\
	pe_init.c	\
	pe_rich.c	\
	pe_section.c

INCS=		libpe.h pe.h
INCSDIR=	/usr/include

RELEASE=	HEAD		# Change this on release branches.

SHLIB_MAJOR=	1

WARNS?=	6

.include "${TOP}/mk/elftoolchain.lib.mk"
