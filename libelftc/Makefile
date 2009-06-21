# $Id$

LIB=	elftc

SRCS=	elftc_demangle.c

CFLAGS+=	-I. -I${.CURDIR}

SHLIB_MAJOR=	1

WARNS?=	6

MAN=	elftc_demangle.3

# Determine the target operating system that we are building for.
.if ${unix:MFreeBSD*}
OS_TARGET=freebsd
.elif ${unix:MNetBSD*}
OS_TARGET=netbsd
.else
.error Unsupported target operating system.
.endif

.if exists(${.CURDIR}/os.${OS_TARGET}.mk)
.include "os.${OS_TARGET}.mk"
.endif

.include <bsd.lib.mk>
