#
# Build recipes for Linux based operating systems.
#
# $Id$

MKLINT=		no

OBJECT_FMT=	ELF		# work around a bug in the pmake package.

_NATIVE_ELF_FORMAT = native-elf-format

.BEGIN:	${_NATIVE_ELF_FORMAT}.h

${_NATIVE_ELF_FORMAT}.h:
	${.CURDIR}/${_NATIVE_ELF_FORMAT} > ${.TARGET} || rm ${.TARGET}

CLEANFILES += ${_NATIVE_ELF_FORMAT}.h
