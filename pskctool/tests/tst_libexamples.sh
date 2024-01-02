#!/bin/sh

# tst_libexamples.sh - keep pskctool output in GTK-DOC manual up to date
# Copyright (C) 2012-2024 Simon Josefsson

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

set -e

srcdir=${srcdir:-.}
PSKCTOOL=${PSKCTOOL:-../pskctool}

if $PSKCTOOL -h | head -1 | grep ^pskctool >/dev/null; then
    # gengetopt < 2.23
    ($PSKCTOOL -h | grep ^Usage: | sed 's/ \+$//;s/OPTIONS/OPTION/';
     $PSKCTOOL -h | sed '1,2d;4,5d;s/ \+$//;') > foo
else
    # gengetopt >= 2.23
    $PSKCTOOL -h | sed 's/ \+$//' > foo
fi
if ! diff -ur $srcdir/../../libpskc/examples/pskctool-h.txt foo; then
    set -x
    $PSKCTOOL -h
    cat foo
    echo "FAIL: pskctool --help output change, commit updated file."
    exit 1
fi

$PSKCTOOL -i $srcdir/../../libpskc/examples/pskc-hotp.xml > foo
if ! diff -ur $srcdir/../../libpskc/examples/pskc-hotp-human.txt foo; then
    echo "FAIL: pskctool --info output change, commit updated file."
    exit 1
fi

$PSKCTOOL --sign --sign-key $srcdir/pskc-ee-key.pem \
    --sign-crt $srcdir/pskc-ee-crt.pem \
    $srcdir/../../libpskc/examples/pskc-hotp.xml \
    | sed 's,4</X509Cert,4\n</X509Cert,' > foo
if ! diff -ur $srcdir/../../libpskc/examples/pskc-hotp-signed.xml foo; then
    echo "FAIL: pskctool --sign output change, commit updated file."
    exit 1
fi

$PSKCTOOL --info --debug --quiet $srcdir/pskc-figure6.xml > foo 2>&1
if ! diff -ur $srcdir/../../libpskc/examples/pskc-figure6-debug.txt foo; then
    echo "FAIL: pskctool --info output change, commit updated file."
    exit 1
fi

rm -f foo

exit 0
