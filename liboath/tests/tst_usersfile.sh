#!/bin/sh
# tst_usersfile.sh - Invoke tst_usersfile and check output.
# Copyright (C) 2009-2025 Simon Josefsson

# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, see
# <https://www.gnu.org/licenses/>.

set -e

srcdir=${srcdir:-.}

TZ=UTC
export TZ

FAKETIME=datefudge
TSTAMP=$($FAKETIME "2006-09-23" date -u +%s || true)
if test "$TSTAMP" != "1158969600"; then
    FAKETIME=faketime
    TSTAMP=$($FAKETIME "2006-09-23" date -u +%s || true)
    if test "$TSTAMP" != "1158969600" && test "$TSTAMP" != "1158969601"; then
	echo "Faketime or datefudge missing ($TSTAMP)" >&2
	exit 77
    fi
fi

rm -f tmp.oath tmp2.oath tmp.oath.new tmp.oath.lock

cp $srcdir/users.oath tmp.oath

$FAKETIME 2006-12-07 ./tst_usersfile$EXEEXT
rc=$?
sed 's/2006-12-07T00:00:0.L/2006-12-07T00:00:00L/g' < tmp.oath > tmp2.oath
diff -ur $srcdir/expect.oath tmp2.oath || rc=1

rm -f tmp.oath tmp2.oath

exit $rc
