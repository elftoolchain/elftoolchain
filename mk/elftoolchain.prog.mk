#
# Rules for building programs.
#
# $Id$

.if !defined(TOP)
.error	Make variable \"TOP\" has not been defined.
.endif

.include "${TOP}/mk/elftoolchain.os.mk"

LIBDWARF?=	${TOP}/libdwarf
LIBELF?=	${TOP}/libelf
LIBELFTC?=	${TOP}/libelftc

CFLAGS+=	-I. -I${.CURDIR} -I${TOP}/common

.if defined(LDADD)
_LDADD_LIBDWARF=${LDADD:M-ldwarf}
.if !empty(_LDADD_LIBDWARF)
CFLAGS+= -I${TOP}/libdwarf
LDFLAGS+= -L${TOP}/libdwarf
.endif

_LDADD_LIBELF=${LDADD:M-lelf}
.if !empty(_LDADD_LIBELF)
CFLAGS+= -I${TOP}/libelf
LDFLAGS+= -L${TOP}/libelf
.endif

_LDADD_LIBELFTC=${LDADD:M-lelftc}
.if !empty(_LDADD_LIBELFTC)
CFLAGS+= -I${TOP}/libelftc
LDFLAGS+= -L${TOP}/libelftc
.endif
.endif

.if ${SRCS:M*.y}
CLEANFILES+=	y.tab.h
.endif

.include <bsd.prog.mk>
