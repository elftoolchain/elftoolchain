PROG=	elfcopy
VERSION=	1.0.0
SRCS=	archive.c main.c sections.c segments.c symbols.c
WARNS?=	5
DPADD=	${LIBELF} ${LIBELFTC}
LDADD=	-lelf -lelftc
.if !defined(LIBELF_AR)
LDADD+= -larchive -lbz2 -lz
.endif
NO_SHARED?=	yes
CFLAGS+=	-g -DELFCOPY_VERSION=\"${VERSION}\"
LINKS=	${BINDIR}/elfcopy ${BINDIR}/strip

.include <bsd.prog.mk>
