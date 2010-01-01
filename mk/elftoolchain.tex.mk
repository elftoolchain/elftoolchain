#
# Rules to build LateX documentation.
#
# $Id$
#

.include "${TOP}/mk/elftoolchain.os.mk"

.if defined(MKTEX) # && ${MKTEX} == "yes"

all:	${DOC}.pdf

# Recognize additional suffixes.
.SUFFIXES:	.mp .eps .tex .pdf

# Rules to build MetaPost figures.
.mp.eps:
	TEX=${MPOSTTEX} ${MPOST} -halt-on-error ${.IMPSRC}
	mv ${.IMPSRC:T:R}.1 ${.TARGET}
.eps.pdf:
	${EPSTOPDF} ${.IMPSRC} > ${.TARGET}

.for f in ${IMAGES_MP}
${f:R}.eps: ${.CURDIR}/${f}
CLEANFILES+=	${f:R}.eps ${f:R}.log ${f:R}.pdf ${f:R}.mpx
.endfor

${DOC}.pdf:	${SRCS} ${IMAGES_MP:S/.mp$/.pdf/g}
	${PDFLATEX} ${SRCS}
	${PDFLATEX} ${SRCS}

.for f in aux log out
CLEANFILES+=	${DOC}.${f}
.endfor

clean:
	rm -f ${CLEANFILES}
.else

all clean:
	@echo WARNING: documentation build skipped: MKTEX=\"${MKTEX}\"
	@true
.endif
