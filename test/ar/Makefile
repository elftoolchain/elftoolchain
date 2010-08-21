# $Id$

TOP=		../..
AR=		${TOP}/ar/ar
RANLIB=		${TOP}/ar/ranlib

.MAIN:	all ${RANLIB}

# check if ar(1) executable is available.
_check_ar:
	test -f ${AR}

execute: _check_ar
	/bin/sh run.sh

clean: ranlibclean

${RANLIB}: ${AR}
	ln -f ${AR} ${RANLIB}

ranlibclean:
	rm -f ${RANLIB}

SUBDIR=		plugin

.include <bsd.subdir.mk>
