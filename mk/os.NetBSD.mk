#
# Build recipes for NetBSD.
#
# $Id$
#

LDSTATIC?=	-static		# link programs statically

MKLINT?=	no		# lint dies with a sigbus

# Enable the test suites.
MKTESTS?=	yes
