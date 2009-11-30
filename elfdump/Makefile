# $Id$

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
