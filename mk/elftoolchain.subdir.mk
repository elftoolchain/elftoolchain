#
# Rules for recursing into directories
# $Id$

# Pass down 'test' as a valid target.

.include "$(TOP)/mk/elftoolchain.os.mk"

.if ${OS_HOST} == FreeBSD
SUBDIR_TARGETS+=	test
.else
TARGETS+=	test
.endif

.include <bsd.subdir.mk>
