# $Id$

TOP=		../..

.include "${TOP}/mk/elftoolchain.tetvars.mk"

.if !defined(TET_EXECUTE)
TET_EXECUTE=	${.OBJDIR}
.endif

all clean depend:

execute test:
	TET_EXECUTE=${TET_EXECUTE} TET_SUITE_ROOT=${.CURDIR} \
		${TET_ROOT}/bin/tcc -e .
