# $FreeBSD: src/usr.bin/elfdump/Makefile,v 1.4 2003/02/03 01:59:27 obrien Exp $

PROG=	elfdump
WARNS?=	6

DPADD=	${LIBELF}
LDADD=	-lelf

USE_LIBARCHIVE_AR?=	0
.if defined(USE_LIBARCHIVE_AR) && (${USE_LIBARCHIVE_AR} > 0)
CFLAGS+=	-DUSE_LIBARCHIVE_AR
DPADD+=	${LIBARCHIVE} ${LIBBZ2} ${LIBZ}
LDADD+=	-larchive -lbz2 -lz
.endif

.include <bsd.prog.mk>
