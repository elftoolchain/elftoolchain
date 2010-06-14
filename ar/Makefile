# $Id$

TOP=	..

PROG=	ar
SRCS=	ar.c read.c util.c write.c
LSRC=	acplex.l
YSRC=	acpyacc.y

WARNS?=	5

DPADD=	${LIBARCHIVE} ${LIBELF}
LDADD=	-larchive -lelf

CFLAGS+=-I. -I${.CURDIR}

NO_SHARED?=	yes

LINKS=	${BINDIR}/ar ${BINDIR}/ranlib

CLEANFILES+= ranlib

MAN1=	ar.1
MLINKS= ar.1 ranlib.1

.include "${TOP}/mk/elftoolchain.prog.mk"
