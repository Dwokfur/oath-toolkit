#!/bin/sh

# Copyright (C) 2011-2024 Simon Josefsson

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

if test `id -u` != "0"; then
    echo "Root access required to run this test, skipping..."
    exit 77
fi

srcdir=${srcdir:-.}

FAKETIME=datefudge
TSTAMP=`TZ=UTC $FAKETIME "2006-09-23" date -u +%s`
if test "$TSTAMP" != "1158969600"; then
    FAKETIME=faketime
    TSTAMP=`TZ=UTC $FAKETIME "2006-09-23" date -u +%s`
    if test "$TSTAMP" != "1158969600"; then
	echo "Faketime or datefudge missing ($TSTAMP)" >&2
	exit 77
    fi
fi

ETCPAMCFG=/etc/pam.d/pam_oath1
ETCUSRCFG=/etc/tst-pam_oath.users

if test -f $ETCPAMCFG; then
    echo "Test PAM configuration already exists?!"
    echo "Remove $ETCPAMCFG and retry..."
    exit 1
fi

if test -f $ETCUSRCFG; then
    echo "Test pam_oath user configuration already exists?!"
    echo "Remove $ETCUSRCFG and retry..."
    exit 1
fi

so_path_rel="${srcdir}/../.libs/pam_oath.so"
so_path="$(readlink -f "${so_path_rel}")"
if test -z "${so_path}"; then
    echo "Unable to resolve path to pam_oath.so: ${so_path_rel}"
    exit 1
fi
if ! test -f "${so_path}"; then
    echo "pam_oath.so not found at: ${so_path}"
    exit 1
fi

echo "auth requisite [${so_path}] debug usersfile=$ETCUSRCFG window=20 digits=6" > $ETCPAMCFG
echo "HOTP user1 - 00" > $ETCUSRCFG
echo "HOTP user2 pw 00" >> $ETCUSRCFG
echo "HOTP/T30 user3 - 00" >> $ETCUSRCFG

if ! test -f $ETCPAMCFG; then
    echo "Writing to $ETCPAMCFG failed, skipping..."
    exit 77
fi

if ! test -f $ETCUSRCFG; then
    echo "Writing to $ETCUSRCFG failed, skipping..."
    exit 77
fi

TZ=UTC $FAKETIME "2006-12-07" ./test-pam_oath-root user3
rc=$?

if test "$rc" != "77"; then
    diff -u "$srcdir/expect.oath" $ETCUSRCFG || rc=1
fi

rm -f $ETCPAMCFG $ETCUSRCFG

exit $rc
