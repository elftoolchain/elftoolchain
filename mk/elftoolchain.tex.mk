#
# $Id$
#

# Recognize additional suffixes
.SUFFIXES:	.tex .pdf

#
# Support for the ConTeXT format
#
.if defined(CONTEXTSRCS)

TEXEXEC ?=	texexec --pdf

.tex.pdf:
	${TEXEXEC} ${.IMPSRC}

all:	${CONTEXTSRCS:C/.tex$/.pdf/g}

_CLEANFILES = mpgraph.mp

.for ct in ${CONTEXTSRCS}
_T = ${ct:T:C/.tex$//}
_CLEANFILES += ${_T}.pdf ${_T}.log ${_T}.tmp ${_T}.tui ${_T}.tuo ${_T}-mpgraph.mp
${_T}.pdf: ${ct}
.endfor
.endif

clean:
	rm -f ${_CLEANFILES}
