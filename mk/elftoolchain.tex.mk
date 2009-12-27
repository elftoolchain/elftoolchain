#
# Rules to build LateX documentation.
#
# $Id$
#

.include "${TOP}/mk/elftoolchain.os.mk"

.if defined(MKTEX) # && ${MKTEX} == "YES"

# Recognize additional suffixes.
.SUFFIXES:	.mp .eps .tex .pdf

# Rules to build MetaPost figures.
.mp.eps:
	TEX=${LATEX} ${MPOST} -halt-on-error ${.IMPSRC}
	mv ${.IMPSRC:T:R}.1 ${.TARGET}
.eps.pdf:
	ps2pdf ${.IMPSRC} ${.TARGET}

.for f in ${IMAGES_MP}
${f:R}.eps: ${.CURDIR}/${f}
CLEANFILES+=	${f:R}.eps ${f:R}.log ${f:R}.pdf
.endfor

all:	${DOC}

${DOC}:	${SRCS} ${IMAGES_MP:S/.mp$/.pdf/g}

clean:
	rm -f ${CLEANFILES}
.else

all clean:
	@echo WARNING: documentation build skipped: MKTEX=\"${MKTEX}\"
	@true
.endif
