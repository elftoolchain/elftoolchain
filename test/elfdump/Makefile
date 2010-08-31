# $Id$

TET_ROOT?=	/home/tmp/tet

.if !defined(TET_EXECUTE)
TET_EXECUTE=	${.OBJDIR}
.endif

all clean depend:

execute:
	TET_SUITE_ROOT=${.CURDIR} ${TET_ROOT}/bin/tcc -e .
