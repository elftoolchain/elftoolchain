TET_ROOT?=	/usr/tet
TET_CMD=	TET_ROOT=${TET_ROOT} TET_SUITE_ROOT=${.CURDIR} ${TET_ROOT}/bin/tcc

all: build

build:
	${TET_CMD} -b .

clean:
	${TET_CMD} -c .

test:
	${TET_CMD} -e .

