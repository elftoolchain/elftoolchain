#
# Configuration information for TET.
#
# $Id$
#

.if !defined(TOP)
.error Make variable \"TOP\" has not been defined.
.endif

# Set TET_ROOT and version.
TET_VERSION?=		3.8
TET_ROOT?=		${TOP}/test/tet/tet${TET_VERSION}

TET_DOWNLOAD_URL=	\
	http://tetworks.opengroup.org/downloads/38/software/Sources/${TET_VERSION}/tet${TET_VERSION}-src.tar.gz
