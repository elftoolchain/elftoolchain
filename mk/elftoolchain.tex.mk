#
# Rules to build LateX documentation.
#
# $Id$
#

.include "${TOP}/mk/elftoolchain.os.mk"

.if defined(MKTEX) # && ${MKTEX} == "yes"

TEXINPUTS=	`kpsepath tex`:${.CURDIR}
_TEX=		TEXINPUTS=${TEXINPUTS} ${PDFLATEX}

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
	${_TEX} ${DOC}.tex
	${MAKEINDEX} ${DOC}.idx
	${_TEX} ${DOC}.tex
	@if grep 'Rerun to get' ${DOC}.log > /dev/null; then \
		${_TEX} ${DOC}.tex; \
	fi

# Recognize additional suffixes.
.SUFFIXES:	.mp .eps .tex .pdf

# Rules to build MetaPost figures.
.mp.eps:
	@if [ "${.OBJDIR}" != "${.CURDIR}" ]; then cp ${.CURDIR}/${.IMPSRC:T} ${.OBJDIR}/; fi
	TEX=${MPOSTTEX} ${MPOST} -halt-on-error ${.IMPSRC:T}
	mv ${.IMPSRC:T:R}.1 ${.TARGET}
.eps.pdf:
	${EPSTOPDF} ${.IMPSRC} > ${.TARGET}

.for f in ${IMAGES_MP}
${f:R}.eps: ${.CURDIR}/${f}
CLEANFILES+=	${f:R}.eps ${f:R}.log ${f:R}.pdf ${f:R}.mpx
.endfor

${DOC}.pdf:	${SRCS} ${IMAGES_MP:S/.mp$/.pdf/g}
	${_TEX} ${.CURDIR}/${DOC}.tex || (rm ${.TARGET}; exit 1)
	@if grep 'undefined references' ${DOC}.log > /dev/null; then \
		${_TEX} ${.CURDIR}/${DOC}.tex; \
	fi
	@if grep 'Rerun to get' ${DOC}.log > /dev/null; then \
		${_TEX} ${.CURDIR}/${DOC}.tex; \
	fi

.for f in aux log out pdf toc ind idx ilg
CLEANFILES+=	${DOC}.${f}
.endfor

# Do something sensible for the `depend` and `cleandepend` targets.
depend:		.depend
	@true
.depend:
	@echo ${DOC}.pdf: ${SRCS} ${IMAGES_MP:S/.mp$/.pdf/g} > ${.TARGET}
cleandepend:	.PHONY
	rm -f .depend

# Include rules for `make obj` and `make clean`
.include <bsd.obj.mk>

.else

all clean obj depend:
	@echo WARNING: documentation build skipped: MKTEX=\"${MKTEX}\"
	@true
.endif
