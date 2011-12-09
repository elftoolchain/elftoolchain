# $Id$

TOP=	..

PROG=	ld
WARNS?=	6

SRCS=	amd64.c			\
	ld_error.c 		\
	ld_exp.c		\
	ld_file.c		\
	ld_layout.c		\
	ld_main.c 		\
	ld_options.c		\
	ld_output.c		\
	ld_script.c		\
	ld_script_lexer.l	\
	ld_script_parser.y	\
	ld_symbols.c		\
	y.tab.h

amd64.c:	amd64.ld
	awk -f ld_script.awk ${.ALLSRC} > ${.TARGET}

GENSRCS=	amd64.c

CLEANFILES+=	y.tab.h ${GENSRCS}

DPADD=	${LIBELFTC} ${LIBELF}
LDADD=	-lelftc -lelf

CFLAGS+= -I. -I${.CURDIR}

NOMAN=

.include "${TOP}/mk/elftoolchain.prog.mk"
