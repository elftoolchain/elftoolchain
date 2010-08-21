# $Id$

TOP=		../..
AR=		${TOP}/ar/ar
RANLIB=		${TOP}/ar/ranlib
PLUGINDIR=	plugin

# list all the C plugins here.
CPLUGIN=	ardiff teraser

all:	test

# check if ar(1) executable is available.
_check_ar:
	/bin/test -f ${AR}

test: _check_ar ${RANLIB} ${CPLUGIN}
	/bin/sh run.sh

clean: ranlibclean pluginclean

${RANLIB}: ${AR}
	ln -f ${AR} ${RANLIB}

${CPLUGIN}:
.for plugin in ${.TARGET}
	cd ${PLUGINDIR} && ${MAKE} -f Makefile.${plugin}
.endfor

ranlibclean:
	rm -f ${RANLIB}

pluginclean:
.for plugin in ${CPLUGIN}
	cd ${PLUGINDIR} && ${MAKE} -f Makefile.${plugin} clean
.endfor
