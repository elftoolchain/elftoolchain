# $Id$

TOP = ../..

.include "${TOP}/mk/elftoolchain.tetvars.mk"

.if !defined(TET_EXECUTE)
TET_EXECUTE=	${.OBJDIR}
.endif

.if make(tccbuild)
TET_OPTIONS+=	-b
.endif

.if make(tccclean)
TET_OPTIONS+=	-c
.endif

.if make(execute) || make(test)
TET_OPTIONS+=	-e
.endif

.MAIN: all

tccbuild tccclean test execute:	.PHONY
	TET_EXECUTE=${TET_EXECUTE} TET_SUITE_ROOT=${.CURDIR} \
		${TET_ROOT}/bin/tcc ${TET_OPTIONS} .

SUBDIR=		ts

.include <bsd.subdir.mk>
