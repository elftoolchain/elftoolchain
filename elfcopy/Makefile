# $Id$

TOP=	..

PROG=	elfcopy
VERSION=	1.0.0

SRCS=	archive.c main.c sections.c segments.c symbols.c

WARNS?=	5

DPADD=	${LIBELF} ${LIBELFTC}
LDADD=	-lelf -lelftc

.if !defined(LIBELF_AR)
LDADD+= -larchive
.endif

NO_SHARED?=	yes

CFLAGS+=	-DELFCOPY_VERSION=\"${VERSION}\"
LINKS=	${BINDIR}/elfcopy ${BINDIR}/strip

.include "${TOP}/mk/elftoolchain.prog.mk"
