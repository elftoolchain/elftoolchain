# $FreeBSD$

PROG=	addr2line
SRCS=	addr2line.c

WARNS?=	6

DPADD=	${LIBELF} ${LIBELFTC} ${LIBDWARF}
LDADD=	-lelftc -ldwarf -lelf

NO_MAN=

.include <bsd.prog.mk>
