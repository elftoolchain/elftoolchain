# $Id$
#
# Build recipes for NetBSD.

LDSTATIC?=	-static		# link programs statically

MKDOC?=		yes		# Build documentation.
MKLINT?=	no		# lint dies with a sigbus
MKTESTS?=	yes		# Enable the test suites.
