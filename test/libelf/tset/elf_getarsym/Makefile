# $Id$

TOP=	../../..

TS_SRCS=		getarsym.m4

# These names must match those in the test case code.
TS_DATA=		a.ar a2.ar a1.o a2.o
TS_LONGNAME=		"s------------------------2"
CLEANFILES+=		a1.c a2.c s1 ${TS_LONGNAME} "s 3"

a1.c:
	echo "int a1;" > ${.TARGET}
a2.c:
	echo "int a2;" > ${.TARGET}

s1:
	echo 'This is s1.' > ${.TARGET}
${TS_LONGNAME}:
	echo 's2.' > ${.TARGET}

a.ar: a1.o a2.o ${TS_LONGNAME} s1
	rm -f ${.TARGET}
	echo 's-3.' > "s 3"
	ar crv ${.TARGET} s1 a1.o ${TS_LONGNAME} a2.o "s 3"

a2.ar: s1 ${TS_LONGNAME}
	rm -f ${.TARGET}
	ar crv ${.TARGET} ${.ALLSRC}
	
.include "${TOP}/mk/elftoolchain.tet.mk"
