# $Id$

TOP=	${.CURDIR}/..

LIB=	elftc

SRCS=	elftc_bfdtarget.c			\
	elftc_demangle.c			\
	elftc_version.c				\
	libelftc_bfdtarget.c			\
	libelftc_dem_arm.c			\
	libelftc_dem_gnu2.c			\
	libelftc_dem_gnu3.c			\
	libelftc_vstr.c
INCS=	libelftc.h

BRANCH=		HEAD		# Change this on release branches.

SHLIB_MAJOR=	1

WARNS?=	6

CLEANFILES+=	elftc_version.c

LDADD+=	-lelf

MAN=	elftc_bfd_find_target.3 \
	elftc_demangle.3 \
	elftc_version.3

MLINKS=	elftc_bfd_find_target.3 elftc_bfd_target_byteorder.3 \
	elftc_bfd_find_target.3 elftc_bfd_target_class.3 \
	elftc_bfd_find_target.3 elftc_bfd_target_flavor.3

.if	!make(clean) && !make(clobber)
.BEGIN:	.SILENT
	${.CURDIR}/make-toolchain-version -t ${TOP} -b ${BRANCH} \
	    -h ${OS_HOST}
.endif

.include "${TOP}/mk/elftoolchain.lib.mk"
