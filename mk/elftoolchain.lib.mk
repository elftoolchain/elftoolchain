#
# $Id$
#

.if !defined(TOP)
.error	Make variable \"TOP\" has not been defined.
.endif

.include "${TOP}/mk/elftoolchain.os.mk"

.include <bsd.lib.mk>

# Support a 'clobber' target.
clobber:	clean	.PHONY

# Adjust CFLAGS
CFLAGS+=	-I.			# OBJDIR
CFLAGS+=	-I${.CURDIR}		# Sources
CFLAGS+=	-I${TOP}/common		# common code

.if defined(LDADD)
_LDADD_LIBELF=${LDADD:M-lelf}
.if !empty(_LDADD_LIBELF)
CFLAGS+=	-I${TOP}/libelf
LDFLAGS+=	-L${TOP}/libelf
.endif
.endif

# Keep the .SUFFIXES line after the include of bsd.lib.mk
.SUFFIXES:	.m4 .c
.m4.c:
	m4 -D SRCDIR=${.CURDIR} ${.IMPSRC} > ${.TARGET}

.if defined(DEBUG)
CFLAGS:=	${CFLAGS:N-O*} -g
.endif

.if ${OS_HOST} == "DragonFly" || ${OS_HOST} == "FreeBSD"
# Install headers too, in the 'install' phase.
install:	includes
.elif ${OS_HOST} == "Linux" || ${OS_HOST} == "NetBSD" || ${OS_HOST} == "Minix"
install:	incinstall
.endif

# Bring in rules related to test code.
.include "${TOP}/mk/elftoolchain.test.mk"
