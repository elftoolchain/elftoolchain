#
# Rules to build LateX documentation.
#
# $Id$
#

.include "${TOP}/mk/elftoolchain.os.mk"

.if defined(MKTEX) # && ${MKTEX} == "yes"

.MAIN:	all

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
	${PDFLATEX} ${DOC}.tex || (rm ${.TARGET}; exit 1)
	${PDFLATEX} ${DOC}.tex

.for f in aux log out pdf toc
CLEANFILES+=	${DOC}.${f}
.endfor

clean:
	rm -f ${CLEANFILES}
.else

all clean:
	@echo WARNING: documentation build skipped: MKTEX=\"${MKTEX}\"
	@true
.endif
