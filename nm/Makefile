# $FreeBSD$

PROG=	nm
SRCS=	nm.c vector_str.c cpp_demangle.c cpp_demangle_arm.c \
	cpp_demangle_gnu2.c dwarf_line_number.c
LDADD=	-lelf
CSTD=	c99
NO_SHARED?= yes

.if ${unix:MNetBSD*}
CFLAGS+=	--std=${CSTD}
.endif

.include <bsd.prog.mk>
