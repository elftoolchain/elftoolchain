# $Id$

TOP=	..

PROG=	readelf
SRCS=	readelf.c

WARNS?=	6

DPADD=	${LIBDWARF} ${LIBELF}
LDADD=	-ldwarf -lelf
.if !defined(LIBELF_AR)
DPADD+= ${LIBARCHIVE}
LDADD+=	-larchive
.endif

MAN1=	readelf.1

.include "${TOP}/mk/elftoolchain.prog.mk"
