# $Id$

TOP=	..

PROG=	elfdump
WARNS?=	6

DPADD=	${LIBELF}
LDADD=	-lelf

USE_LIBARCHIVE?=	0
CFLAGS+= -g
.if defined(USE_LIBARCHIVE) && (${USE_LIBARCHIVE} > 0)
CFLAGS+=	-DUSE_LIBARCHIVE_AR
DPADD+=	${LIBARCHIVE} ${LIBBZ2} ${LIBZ}
LDADD+=	-larchive -lbz2 -lz
.endif

.include "${TOP}/mk/elftoolchain.prog.mk"
