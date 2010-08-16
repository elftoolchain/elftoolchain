#
# Rules for handling TET based test suites.
#

.if !defined(TOP)
.error Make variable \"TOP\" has not been defined.
.endif

# Set TET_ROOT
TET_ROOT?= ${TOP}/test/tet/

# Inform make(1) about the suffixes we use.
.SUFFIXES: .lsb32 .lsb64 .m4 .msb32 .msb64 .yaml

TS_ROOT?=	${.CURDIR:H}
TS_OBJROOT?=	${.OBJDIR:H}

TET_LIBS=	${TET_ROOT}/lib/tet3
TET_OBJS=	${TET_LIBS}/tcm.o

CFLAGS+=	-I${TET_ROOT}/inc/tet3 -I${TS_ROOT}/common

# Bring in test-suite specific definitions, if any.
.if exists(${.CURDIR}/../Makefile.tset)
.include "${.CURDIR}/../Makefile.tset"
.endif

.if defined(TS_SRCS)
PROG=		tc_${.CURDIR:T:R}

_C_SRCS=	${TS_SRCS:M*.c}
_M4_SRCS=	${TS_SRCS:M*.m4}

SRCS=		${_C_SRCS} ${_M4_SRCS}	# See <bsd.prog.mk>.
CLEANFILES+=	${_M4_SRCS:S/.m4$/.c/g} ${TS_DATA}

${PROG}:	${TS_DATA} 

NO_MAN?=	1

.if defined(GENERATE_TEST_SCAFFOLDING)
_TC_SRC=	${.OBJDIR}/tc.c				# Test driver.

SRCS+=		${_TC_SRC}
CLEANFILES+=	${_TC_SRC}

# Generate the driver file "tc.c" from the objects comprising the test case.
_TS_OBJS=	${_C_SRCS:S/.c$/.o/g} ${_M4_SRCS:S/.m4$/.o/g}
_MUNGE_TS=	${TS_ROOT}/bin/munge-ts
${_TC_SRC}:	${_TS_OBJS}
	${_MUNGE_TS} -o ${.TARGET} ${.ALLSRC}
.endif
.endif

# M4->C translation.
M4FLAGS+=	-I${TS_ROOT}/common
.m4.c:
	m4 ${M4FLAGS} ${.IMPSRC} > ${.TARGET}

LDADD+=		${TET_OBJS} -L${TET_LIBS} -lapi
CLEANFILES+=	tet_xres

.include "${TOP}/mk/elftoolchain.prog.mk"
