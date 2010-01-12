#
# Rules to build LateX documentation.
#
# $Id$
#

.include "${TOP}/mk/elftoolchain.os.mk"

.if defined(MKTEX) # && ${MKTEX} == "yes"

.MAIN:	all

all:	${DOC}.pdf

# Build an index.
#
# First, we need to remove the existing ".ind" file and run `latex` once
# to generate it afresh.  This generates the appropriate ".idx" files used
# by `makeindex`.
# Next, `makeindex` is used to create the ".ind" file.
# Then another set of `latex` runs serves to typeset the index.
index:	.PHONY
	rm -f ${DOC}.ind
	${PDFLATEX} ${DOC}.tex
	${MAKEINDEX} ${DOC}.idx
	${PDFLATEX} ${DOC}.tex
	@if grep 'Rerun to get' ${DOC}.log > /dev/null; then \
		${PDFLATEX} ${DOC}.tex; \
	fi

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

${DOC}.pdf:	${SRCS} ${IMAGES_MP:S/.mp$/.pdf/g} ${DOC}.ind
	${PDFLATEX} ${DOC}.tex || (rm ${.TARGET}; exit 1)
	@if grep 'undefined references' ${DOC}.log > /dev/null; then \
		${PDFLATEX} ${DOC}.tex; \
	fi
	@if grep 'Rerun to get' ${DOC}.log > /dev/null; then \
		${PDFLATEX} ${DOC}.tex; \
	fi

.for f in aux log out pdf toc ind idx ilg
CLEANFILES+=	${DOC}.${f}
.endfor

clean:
	rm -f ${CLEANFILES}
.else

all clean:
	@echo WARNING: documentation build skipped: MKTEX=\"${MKTEX}\"
	@true
.endif
