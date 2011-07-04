# $Id$

TOP=	../../../..

TS_SRCS=		elf.m4
TS_DATA=		a.ar

a.ar:	/dev/null
	rm -f ${.TARGET}
	ar crv ${.TARGET} ${.ALLSRC}

.include "${TOP}/mk/elftoolchain.tet.mk"
