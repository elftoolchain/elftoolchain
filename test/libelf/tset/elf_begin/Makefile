# $Id$

TOP=	../../../..

TS_SRCS=	begin.m4
TS_DATA=	check_elf.msb32 check_elf.lsb32 check_elf.msb64 check_elf.lsb64 \
		a.ar a.o
CLEANFILES+=	a.c

a.c:	.SILENT
	echo "int dummy;" > a.c

a.ar:	a.o .SILENT
	${AR} crv ${.TARGET} ${.ALLSRC}	> /dev/null

.include "${TOP}/mk/elftoolchain.tet.mk"
