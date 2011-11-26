# $Id$

TOP=	..

PROG=	ld
WARNS?=	6

SRCS=	ld_error.c 		\
	ld_file.c		\
	ld_main.c 		\
	ld_options.c		\
	ld_script_lexer.l	\
	ld_script_parser.y	\
	ld_symbols.c		\
	ld_script.c		\
	y.tab.h

LD_SCRIPT=	amd64.ld

ld_script.c:	${LD_SCRIPT}
	awk -f ld_script.awk ${.ALLSRC} > ${.TARGET}

CLEANFILES+=	ld_script.c y.tab.h

DPADD=	${LIBELFTC} ${LIBELF}
LDADD=	-lelftc -lelf

CFLAGS+= -I. -I${.CURDIR}

NOMAN=

.include "${TOP}/mk/elftoolchain.prog.mk"
