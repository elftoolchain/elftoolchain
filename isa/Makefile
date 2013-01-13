# $Id$

TOP=	..

PROG=	isa
SRCS=	isa.c
LSRC=
YSRC=
LDADD=	-lelftc

ISA=

MAN=	isa.1

check-specifications:	.PHONY
.for f in ${ISA}
	${.OBJDIR}/${PROG} -n query ${f}
.endfor

.include "${TOP}/mk/elftoolchain.prog.mk"
