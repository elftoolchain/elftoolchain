# $Id$

TOP=		../..

ELFCOPY=	${TOP}/elfcopy/elfcopy

TEST_LOG=	test.log

.MAIN:	all

execute test: ${ELFCOPY}
	/bin/sh run.sh

clobber:
	rm -f ${TEST_LOG}

SUBDIR=		plugin

.include <bsd.subdir.mk>
