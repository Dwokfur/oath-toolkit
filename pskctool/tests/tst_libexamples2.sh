#!/bin/sh

# tst_libexamples2.sh - keep pskctool output in GTK-DOC manual up to date
# Copyright (C) 2012-2025 Simon Josefsson

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

grep '#define USE_XMLSEC 1' ../../libpskc/config.h > /dev/null \
     || exit 77

srcdir=${srcdir:-.}
PSKCTOOL=${PSKCTOOL:-../pskctool}

$PSKCTOOL --sign --sign-key $srcdir/pskc-ee-key.pem \
    --sign-crt $srcdir/pskc-ee-crt.pem \
    $srcdir/../../libpskc/examples/pskc-hotp.xml \
    | sed 's,4</X509Cert,4\n</X509Cert,' > bar
if ! diff -ur $srcdir/../../libpskc/examples/pskc-hotp-signed.xml bar; then
    echo "FAIL: pskctool --sign output change, commit updated file."
    exit 1
fi

rm -f bar

exit 0
