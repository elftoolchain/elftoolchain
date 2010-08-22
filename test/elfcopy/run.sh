#!/bin/sh
# $FreeBSD$
#
# Run all the tests.

# setup cleanup trap
trap 'rm -rf /tmp/elfcopy-*; rm -rf /tmp/strip-*; exit' 0 2 3 15

# load functions.
. ./func.sh

# global initialization.
init

# run tests.
for f in tc/*; do
    if [ -d $f ]; then
	. $f/`basename $f`.sh
    fi
done

# show statistics.
statistic
