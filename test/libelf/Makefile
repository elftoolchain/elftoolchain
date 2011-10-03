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

execute tccbuild tccclean test:	.PHONY
	TET_ROOT=${TET_ROOT} TET_EXECUTE=${TET_EXECUTE} \
		TET_SUITE_ROOT=${.CURDIR} ${TET_ROOT}/bin/tcc ${TET_OPTIONS} .

SUBDIR=		tset

.include <bsd.subdir.mk>
