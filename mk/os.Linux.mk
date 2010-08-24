#
# Build recipes for Debian GNU/Linux based operating systems.
#
# $Id$

BSDINCMK=	bsd.inc.mk

MKLINT=		no
MKPIC=		no
MKTEX=		yes

OBJECT_FORMAT=	ELF	# work around a bug in the pmake package

YFLAGS+=	-d		# force bison to write y.tab.h

EPSTOPDF=	/usr/bin/epstopdf
MAKEINDEX=	/usr/bin/makeindex
MPOST=		/usr/bin/mpost
MPOSTTEX=	/usr/bin/latex
PDFLATEX=	/usr/bin/pdflatex
