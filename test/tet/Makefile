#
# Build TET from source.
#

TOP=	../..

.include "${TOP}/mk/elftoolchain.tetvars.mk"

TET_BUILD_MARKER=	.tet-build-done

.if exists(${TET_ROOT}/configure)
all: ${TET_BUILD_MARKER}

${TET_BUILD_MARKER}:
	cd ${TET_ROOT} && sh ./configure -t lite
	cd ${TET_ROOT}/src && ${MAKE} all install
	touch ${TET_BUILD_MARKER}

clean:
	cd ${TET_ROOT}/src && ${MAKE} clean
	rm -f ${TET_BUILD_MARKER}

depend:	.SILENT
	true
.else
all depend:	.SILENT
	echo "ERROR: The distribution of TET v${TET_VERSION} is missing."
	echo
	echo "Please download the distribution from:"
	echo "	${TET_DOWNLOAD_URL}"
	echo "and unpack it into directory \"${TET_ROOT}\"."
	echo
	@false
.endif
