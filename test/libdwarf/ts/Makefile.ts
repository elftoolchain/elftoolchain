#
# A hack to build test cases using make(1)
#

.if !defined(TET_ROOT)
.error TET_ROOT must be defined!
.endif

PROG?=	tc_${.CURDIR:T:R}
TS_ROOT?=	${.CURDIR:H}
TS_OBJROOT?=	${.OBJDIR:H}

TET_LIBS:=	${TET_ROOT}/lib/tet3
TET_OBJS:=	${TET_LIBS}/tcm.o

.if defined(TCGEN)
DWARF_INC?=	/usr/local/include
DWARF_LIBS?=	/usr/local/lib
.endif

CFLAGS+=	-I${TET_ROOT}/inc/tet3 -I${TS_ROOT}/common
.if defined(TCGEN)
CFLAGS+=	-DTCGEN -I${DWARF_INC}
.endif

LDADD+=	-lelf ${TET_OBJS} -L${TET_LIBS} -lapi -lbsdxml
.if defined(TCGEN)
LDADD+= -L${DWARF_LIBS}
.endif
LDADD+=	-ldwarf

.if defined(TS_SRCS)
_C_SRCS=	${TS_SRCS:M*.c}
_M4_SRCS=	${TS_SRCS:M*.m4}
CLEANFILES+=	${_M4_SRCS:S/.m4/.c/}
SRCS=	${_C_SRCS} ${_M4_SRCS} # for bsd.prog.mk
.if !defined(TCGEN)
SRCS+=	${.OBJDIR}/ic_count.c 
.endif
.endif

_TS_OBJS:=	${_C_SRCS:S/.c/.o/g} ${_M4_SRCS:S/.m4/.o/}

${PROG}:	${TS_DATA}

NO_MAN?=1

.if !defined(TCGEN)
${.OBJDIR}/ic_count.c:
	${TS_ROOT}/bin/count-ic ${.OBJDIR}
.endif

# Copy
.for f in ${TS_DATA}
.if !exists(${f:R})
${f}:	${TS_OBJROOT}/common/object/${f}.gz
	cp ${.ALLSRC} ${.TARGET}.gz
	gunzip ${.TARGET}.gz
.endif
.endfor

CLEANFILES+=	${TS_DATA}
.for f in ${TS_DATA}
CLEANFILES+=	${f}.xml
.endfor
.if !defined(TCGEN)
CLEANFILES+=	 ${.OBJDIR}/ic_count.c
.endif

CLEANFILES+=	tet_xres

WARNS?=		2

# Bring in rules to build programs
.include <bsd.prog.mk>
