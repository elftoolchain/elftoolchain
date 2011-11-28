# $Id$
#
# Build definitions for FreeBSD

MKDOC?=		yes	# Build documentation.
MKTESTS?=	yes	# Enable the test suites.
MKTEX?=		yes	# TeX and friends are packaged in the teTeX package.

# Link programs statically by default.
NO_SHARED?=	yes

.if defined(MKTEX) && ${MKTEX} == "yes"
EPSTOPDF?=	/usr/local/bin/epstopdf
MAKEINDEX?=	/usr/local/bin/makeindex
MPOSTTEX?=	/usr/local/bin/latex
MPOST?=		/usr/local/bin/mpost
PDFLATEX?=	/usr/local/bin/pdflatex
.endif

# Translate the spelling of a build knob (see ticket #316).
.if defined(NOMAN)
NO_MAN=		yes
.endif
