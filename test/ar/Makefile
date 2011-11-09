# $Id$

TOP=		../..
AR=		${TOP}/ar/ar

TEST_LOG=	test.log

.MAIN:	all

.PHONY:	clobber execute test

execute test: ${AR}
	/bin/sh run.sh

clobber:
	rm -f ${TEST_LOG}

SUBDIR=		plugin

.include <bsd.subdir.mk>
