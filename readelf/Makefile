# $FreeBSD$

PROG=	readelf
SRCS=	readelf.c

WARNS?=	6

DPADD=	${LIBELF}
LDADD=	-lelf
.if !defined(LIBELF_AR)
LDADD+=	-larchive -lbz2 -lz
.endif
NO_MAN=

.include <bsd.prog.mk>
