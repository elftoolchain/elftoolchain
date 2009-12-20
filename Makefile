# $Id$

# Build configuration information first.
SUBDIR += common

# Build the base libraries next.
SUBDIR += libelf
SUBDIR += libdwarf

# Build additional APIs.
SUBDIR += libelftc

# Build tools after the libraries.
SUBDIR += addr2line
SUBDIR += ar
SUBDIR += brandelf
SUBDIR += elfcopy
SUBDIR += elfdump
SUBDIR += nm
SUBDIR += readelf
SUBDIR += size
SUBDIR += strings

# Build test suites and documentation at the end.
SUBDIR += libelf-test-suite
SUBDIR += libelf-tutorial

.include <bsd.subdir.mk>
