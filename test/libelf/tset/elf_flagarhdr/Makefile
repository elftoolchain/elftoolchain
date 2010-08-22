# $Id$

TOP=	../../../..

TS_SRCS=		flagarhdr.m4

TS_DATA=		a.ar
CLEANFILES+=		a1

a1:	.SILENT
	echo "a1" > ${.TARGET}

a.ar:	a1 .SILENT
	ar crv ${.TARGET} a1

.include "${TOP}/mk/elftoolchain.tet.mk"
