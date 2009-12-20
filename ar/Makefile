# $Id$

TOP=	..

PROG=	ar
SRCS=	ar.c acplex.l acpyacc.y read.c util.c write.c y.tab.h

WARNS?=	5

DPADD=	${LIBARCHIVE} ${LIBBZ2} ${LIBZ} ${LIBELF}
LDADD=	-larchive -lbz2 -lz -lelf

CFLAGS+=-I. -I${.CURDIR}

NO_SHARED?=	yes

LINKS=	${BINDIR}/ar ${BINDIR}/ranlib

MAN1=	ar.1
MLINKS= ar.1 ranlib.1

.include "${TOP}/mk/elftoolchain.prog.mk"
