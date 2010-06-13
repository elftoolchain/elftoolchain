# $Id$

TOP=	..

PROG=	elfdump
WARNS?=	6

DPADD=	${LIBELF}
LDADD=	-lelf

USE_LIBARCHIVE?=	0
.if defined(USE_LIBARCHIVE) && (${USE_LIBARCHIVE} > 0)
CFLAGS+=	-DUSE_LIBARCHIVE_AR
DPADD+=	${LIBARCHIVE}
LDADD+=	-larchive
.endif

.include "${TOP}/mk/elftoolchain.prog.mk"
