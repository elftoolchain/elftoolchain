#
# $Id$
#

TOP=	../..

.include "${TOP}/mk/elftoolchain.tetvars.mk"

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
	TET_ROOT=${TET_ROOT} TET_SUITE_ROOT=${.CURDIR} \
	${TET_ROOT}/bin/tcc ${TET_OPTIONS} .

tccbuild tccclean execute:	_cmd

SUBDIR=		ts

.include <bsd.subdir.mk>
