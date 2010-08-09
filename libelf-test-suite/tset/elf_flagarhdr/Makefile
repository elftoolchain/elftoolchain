# $Id$

TOP=	../../..

TS_SRCS=		flagarhdr.m4

TS_DATA=		a.ar
CLEANFILES+=		a1

a1:
	echo "a1" > ${.TARGET}

a.ar: a1
	ar crv ${.TARGET} a1

.include "${TOP}/mk/elftoolchain.tet.mk"
