# $Id$

TOP=	..

PROG=	elfcopy

SRCS=	archive.c ascii.c binary.c main.c sections.c segments.c symbols.c

WARNS?=	5

DPADD=	${LIBELF} ${LIBELFTC}
LDADD=	-lelf -lelftc

.if !defined(LIBELF_AR)
LDADD+= -larchive
.endif

MAN=	elfcopy.1 mcs.1

NO_SHARED?=	yes

LINKS=	${BINDIR}/elfcopy ${BINDIR}/strip 	\
	${BINDIR}/elfcopy ${BINDIR}/mcs

.include "${TOP}/mk/elftoolchain.prog.mk"
