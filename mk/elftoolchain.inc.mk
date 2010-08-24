#
# Rules for handling include files.
#
# $Id$

.if !defined(TOP)
.error	Make variable \"TOP\" has not been defined.
.endif

.include "${TOP}/mk/elftoolchain.os.mk"

# Use the standard rules in <bsd.inc.mk>.
.include <bsd.own.mk>
.include <${BSDINCMK}>
