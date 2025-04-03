#!/bin/sh
# tst_fopen-wx.sh - Setup and invoke tst_fopen-wx and check output.
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

rm -f cve.oath cve.oath.lock cve.oath.new cve.sshd-config

# Run 1: Vulnerable *.new file
printf 'HOTP/E/8\tsilver\t4711\t3132333435363738393031323334353637383930313233343536373839303132\n' > cve.oath
echo my-magic-cookie > cve.sshd-config
ln -s cve.sshd-config cve.oath.new
./tst_fopen-wx$EXEEXT cve.oath silver 670691 4711
rc=$?
if test $rc != 0; then
    echo "FAIL: tst_fopen-wx appears to be linked to vulnerable liboath"
    exit 1
fi
if ! grep my-magic-cookie cve.sshd-config > /dev/null; then
    echo "FAIL: sshd-config modified"
    exit 1
fi
if test -f cve.oath.lock; then
    echo "FAIL: lock file still exists"
    exit 1
fi
if grep 670691 cve.oath > /dev/null; then
    echo "FAIL: cve.oath updated with new OTP incorrectly"
    exit 1
fi

rm -f cve.oath cve.oath.lock cve.oath.new cve.sshd-config

# Run 2: Vulnerable *.lock file
printf 'HOTP/E/8\tsilver\t4711\t3132333435363738393031323334353637383930313233343536373839303132\n' > cve.oath
echo my-magic-cookie > cve.sshd-config
ln -s cve.sshd-config cve.oath.lock
./tst_fopen-wx$EXEEXT cve.oath silver 670691 4711
rc=$?
if test $rc != 0; then
    echo "FAIL: tst_fopen-wx appears to be linked to vulnerable liboath"
    exit 1
fi
if ! grep my-magic-cookie cve.sshd-config > /dev/null; then
    echo "FAIL: sshd-config modified"
    exit 1
fi
if test -f cve.oath.new; then
    echo "FAIL: new file still exists"
    exit 1
fi
if grep 670691 cve.oath > /dev/null; then
    echo "FAIL: cve.oath updated with new OTP incorrectly"
    exit 1
fi

rm -f cve.oath cve.oath.lock cve.oath.new cve.sshd-config

echo "PASS: tst_fopen-wx.sh"

exit 0
