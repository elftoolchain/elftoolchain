# $Id$

PLUGINS=	ardiff teraser

all:	${PLUGINS}

${PLUGINS}:
.for plugin in ${.TARGET}
	${MAKE} -f Makefile.${plugin}
.endfor

clean:
.for plugin in ${PLUGINS}
	${MAKE} -f Makefile.${plugin} clean
.endfor
