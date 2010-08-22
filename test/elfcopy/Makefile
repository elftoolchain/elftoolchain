# $Id$

TOP=		../..
ELFCOPY=	${TOP}/elfcopy/elfcopy
STRIP=		${TOP}/elfcopy/strip
MCS=		${TOP}/elfcopy/mcs

.MAIN:	all ${STRIP} ${MCS}

_check_elfcopy:
	test -f ${ELFCOPY}

execute: _check_elfcopy
	/bin/sh run.sh

clean: stripclean mcsclean

${STRIP}: ${ELFCOPY}
	ln -f ${ELFCOPY} ${STRIP}

${MCS}: ${ELFCOPY}
	ln -f ${ELFCOPY} ${MCS}

stripclean:
	rm -f ${STRIP}

mcsclean:
	rm -f ${MCS}

SUBDIR=		plugin

.include <bsd.subdir.mk>
