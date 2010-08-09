# $Id$

TOP=	../../..

TS_SRCS=		cntl.m4
TS_DATA=		a.ar
CLEANFILES+=		a.c a.o

a.c:
	echo "int foobar;" > ${.TARGET}

a.ar:	a.o
	rm -f ${.TARGET}
	ar crv ${.TARGET} ${.ALLSRC}

.include "${TOP}/mk/elftoolchain.tet.mk"
