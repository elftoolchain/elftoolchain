# $FreeBSD$

ELFCOPY=	../elfcopy
ECPDIR!=	dirname ${ELFCOPY}
STRIP:=		${ECPDIR}/strip
MCS:=		${ECPDIR}/mcs
PLUGINDIR=	plugin

# list all the C plugins here.
CPLUGIN=	ardiff teraser

test: ${ELFCOPY} ${STRIP} ${MCS} ${CPLUGIN}
	/bin/sh run.sh

clean: ecpclean stripclean mcsclean pluginclean

${ELFCOPY}:
	cd ${ECPDIR} && ${MAKE}

${STRIP}: ${ELFCOPY}
	ln -f ${ELFCOPY} ${STRIP}

${MCS}: ${ELFCOPY}
	ln -f ${ELFCOPY} ${MCS}

${CPLUGIN}:
.for plugin in ${.TARGET}
	cd ${PLUGINDIR} && ${MAKE} -f Makefile.${plugin}
.endfor

ecpclean:
	cd ${ECPDIR} && ${MAKE} clean

stripclean:
	rm -f ${STRIP}

mcsclean:
	rm -f ${MCS}

pluginclean:
.for plugin in ${CPLUGIN}
	cd ${PLUGINDIR} && ${MAKE} -f Makefile.${plugin} clean
.endfor
