# $Id$

PLUGINS=	ardiff teraser

all:	${PLUGINS}

${PLUGINS}:
.for plugin in ${.TARGET}
	${MAKE} -f Makefile.${plugin}
.endfor

clean cleandepend depend:
.for plugin in ${PLUGINS}
	${MAKE} -f Makefile.${plugin} ${.TARGET}
.endfor
