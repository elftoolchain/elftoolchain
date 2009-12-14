#
# Build recipes for Linux based operating systems.
#
# $Id$

_NATIVE_ELF_FORMAT = native-elf-format

.BEGIN:	${_NATIVE_ELF_FORMAT}.h

${_NATIVE_ELF_FORMAT}.h:
	${.CURDIR}/${_NATIVE_ELF_FORMAT} > ${.TARGET} || rm ${.TARGET}

CLEANFILES += ${_NATIVE_ELF_FORMAT}.h
