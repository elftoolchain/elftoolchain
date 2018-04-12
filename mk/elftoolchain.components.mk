#
# $Id$
#

# Knobs to turn parts of the source tree on or off.

# Build the automation tools.
WITH_BUILD_TOOLS?=	no

# Build additional tutorial documentation. (Manual page generation is
# controlled by the 'MKDOC' knob).
WITH_ADDITIONAL_DOCUMENTATION?=yes

# Build the instruction set analyser.
WITH_ISA?=	no

# Build PE support.
WITH_PE?=	yes

# Build test suites.
.if defined(MAKEOBJDIR) || defined(MAKEOBJDIRPREFIX)
.if defined(WITH_TESTS) && ${WITH_TESTS:tl} == "yes"
.error "Only in-tree builds are supported for tests currently [#271]."
.endif
WITH_TESTS=no
.else
WITH_TESTS?=	yes
.endif
