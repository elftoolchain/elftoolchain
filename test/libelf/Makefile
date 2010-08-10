# $Id$

TET_ROOT?=	/home/tmp/tet

.if !defined(TET_EXECUTE)
TET_EXECUTE=	${.OBJDIR}
.endif

.if make(tccbuild)
TET_OPTIONS+=	-b
.endif

.if make(tccclean)
TET_OPTIONS+=	-c
.endif

.if make(execute)
TET_OPTIONS+=	-e
.endif

.MAIN: all

_cmd:	.USE
	TET_EXECUTE=${TET_EXECUTE} TET_SUITE_ROOT=${.CURDIR} \
		${TET_ROOT}/bin/tcc ${TET_OPTIONS} .

tccbuild tccclean execute:	_cmd

SUBDIR=		tset

.include <bsd.subdir.mk>
