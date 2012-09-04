# $Id$
#
# Build recipes for NetBSD.

LDSTATIC?=	-static		# link programs statically

MKDOC?=		yes		# Build documentation.
MKLINT?=	no		# lint dies with a sigbus
MKTESTS?=	yes		# Enable the test suites.
MKNOWEB?=	no		# Build literate programs.

# Literate programming utility.
NOWEB?=		/usr/pkgsrc/bin/noweb
