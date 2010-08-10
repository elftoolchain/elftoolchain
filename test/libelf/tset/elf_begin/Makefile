# $Id$

TOP=	../../../..

TS_SRCS=	begin.m4
TS_DATA=	check_elf.msb32 check_elf.lsb32 check_elf.msb64 check_elf.lsb64 \
		a.ar a.o
CLEANFILES+=	a.c

a.c:
	echo "int dummy;" > a.c

a.ar:	a.o
	ar crv ${.TARGET} ${.ALLSRC}	


.include "${TOP}/mk/elftoolchain.tet.mk"
