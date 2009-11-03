# $FreeBSD$

PROG=	readelf
SRCS=	readelf.c

WARNS?=	6

DPADD=	${LIBELF} ${LIBDWARF}
LDADD=	-lelf -ldwarf
.if !defined(LIBELF_AR)
LDADD+=	-larchive -lbz2 -lz
.endif
NO_MAN=

.include <bsd.prog.mk>
