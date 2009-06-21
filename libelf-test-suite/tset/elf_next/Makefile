# $Id$

TS_SRCS=		next.m4
TS_DATA=		a1.ar a1.o a2.ar a2.o a3.ar
TS_LONGNAME=		"s------------------------2"
CLEANFILES+=		a1.c a2.c s1 ${TS_LONGNAME} "s 3"

s1:
	echo 'This is s1.' > ${.TARGET}
${TS_LONGNAME}:
	echo ${TS_LONGNAME} > ${.TARGET}

# a1.ar contains short names only; a2.ar contains long names; a3.ar contains ELF binaries
# and a symbol table.

a1.c:
	echo "int a1;" > ${.TARGET}
a2.c:
	echo "int a2;" > ${.TARGET}

a1.ar: s1
	rm -f ${.TARGET}
	echo 's-3.' > "s 3"
	ar crv ${.TARGET} s1 "s 3"

a2.ar: s1 ${TS_LONGNAME}
	rm -f ${.TARGET}
	ar crv ${.TARGET} ${.ALLSRC}

a3.ar: s1 ${TS_LONGNAME} a1.o a2.o
	rm -f ${.TARGET}
	ar crv ${.TARGET} ${.ALLSRC}

.include "../Makefile.tset"
