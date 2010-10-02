# $Id$

TOP=	..

PROG=	findtextrel
SRCS=	findtextrel.c

WARNS?=	6

DPADD=	${LIBELF} ${LIBDWARF}
LDADD=	-ldwarf -lelf

NO_MAN=

.include "${TOP}/mk/elftoolchain.prog.mk"
