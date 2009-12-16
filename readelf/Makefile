# $Id$

TOP=	..

PROG=	readelf
SRCS=	readelf.c

WARNS?=	6

DPADD=	${LIBELF} ${LIBDWARF}
LDADD=	-lelf -ldwarf
.if !defined(LIBELF_AR)
LDADD+=	-larchive -lbz2 -lz
.endif

MAN1=	readelf.1

.include "${TOP}/mk/elftoolchain.prog.mk"
