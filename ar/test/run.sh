#!/bin/sh
# $FreeBSD$
#
# Run all the tests.

# load functions.
. func.sh

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
