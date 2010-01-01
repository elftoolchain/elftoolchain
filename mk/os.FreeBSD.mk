#
# Build definitions for FreeBSD
#


# TeX and friends are packaged in the teTeX package.
MKTEX?=yes

.if defined(MKTEX) && ${MKTEX} == "yes"
EPSTOPDF?=	/usr/local/bin/epstopdf
MPOSTTEX?=	/usr/local/bin/latex
MPOST?=		/usr/local/bin/mpost
PDFLATEX?=	/usr/local/bin/pdflatex
.endif
