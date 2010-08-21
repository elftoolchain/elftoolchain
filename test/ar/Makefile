# $FreeBSD$

AR=		../ar
ARDIR!=		dirname ${AR}
RANLIB:=	${ARDIR}/ranlib
PLUGINDIR=	plugin

# list all the C plugins here.
CPLUGIN=	ardiff teraser

test: ${AR} ${RANLIB} ${CPLUGIN}
	/bin/sh run.sh

clean: arclean ranlibclean pluginclean

${AR}:
	cd ${ARDIR} && ${MAKE} WITH_BSDAR=yes

${RANLIB}: ${AR}
	ln -f ${AR} ${RANLIB}

${CPLUGIN}:
.for plugin in ${.TARGET}
	cd ${PLUGINDIR} && ${MAKE} -f Makefile.${plugin}
.endfor

arclean:
	cd ${ARDIR} && ${MAKE} clean

ranlibclean:
	rm -f ${RANLIB}

pluginclean:
.for plugin in ${CPLUGIN}
	cd ${PLUGINDIR} && ${MAKE} -f Makefile.${plugin} clean
.endfor
