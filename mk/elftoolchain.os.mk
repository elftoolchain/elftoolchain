#
# $Id$
#

# OS specific build instructions

# Determine the target operating system that we are building for.
.if ${unix:MFreeBSD*}
OS_TARGET=freebsd
.elif ${unix:MNetBSD*}
OS_TARGET=netbsd
.else
.error Unknown target operating system.
.endif

# Bring in OS-specific Makefiles, if they exist
.if exists(${.CURDIR}/os.${OS_TARGET}.mk)
.include "${.CURDIR}/os.${OS_TARGET}.mk"
.endif
