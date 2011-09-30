#
# Build definitions for FreeBSD
#

# Link programs statically by default.
NO_SHARED?=	yes

# TeX and friends are packaged in the teTeX package.
MKTEX?=		yes

.if defined(MKTEX) && ${MKTEX} == "yes"
EPSTOPDF?=	/usr/local/bin/epstopdf
MAKEINDEX?=	/usr/local/bin/makeindex
MPOSTTEX?=	/usr/local/bin/latex
MPOST?=		/usr/local/bin/mpost
PDFLATEX?=	/usr/local/bin/pdflatex
.endif

# Translate the spelling of build knob (see ticket #316).
.if defined(NOMAN)
NO_MAN=		yes
.endif
