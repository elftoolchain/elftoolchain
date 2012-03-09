# $Id$
#
# The same 'source' for a literate program is used to build both
# documentation and the program itself.
#
# Keeping the build rules for program code and documentation in
# separate Makefiles help to keep each rule set simple.

TOP=		../..

BUILD_TARGETS=	all clean clobber
BUILD_RULES=	documentation program

NW_SRCS=	build-automation.nw \
		master.nw \
		slave.nw

.MAIN:		all

# Pass standard targets down to both rulesets.
.for target in ${BUILD_TARGETS}
${target}:	.PHONY
.for rules in ${BUILD_RULES}
	@${MAKE} -f Makefile.${rules} ${target} NW_SRCS="${NW_SRCS}" \
		TOP="${TOP}"
.endfor
.endfor

#
# Special targets.
#

# The 'index' target only applies to the documentation.
index:	.PHONY
	@${MAKE} -f Makefile.documentation ${.TARGET} NW_SRCS="${NW_SRCS}" \
		TOP="${TOP}"

# The 'test' target only applies to the program.
test:	.PHONY
	@${MAKE} -f Makefile.program ${.TARGET} NW_SRCS="${NW_SRCS}" \
		TOP="${TOP}"