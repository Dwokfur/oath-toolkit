#!/bin/sh

# run-e2e-tests.sh - Test pam_oath end-to-end
# Copyright (C) 2011-2021 Simon Josefsson

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

if ! ldconfig -p | grep -q libpam_wrapper.so; then
    echo "pam_wrapper not installed, skipping"
    exit 77
fi

srcdir=${srcdir:-.}
pamcfgdir="${srcdir}/pam.d"
pamcfg="${pamcfgdir}/pam_oath1"
usrcfg="${srcdir}/tst-pam_oath.users"

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

mkdir -p "${pamcfgdir}"
echo "auth requisite [${so_path}] debug [usersfile=${usrcfg}] window=20 digits=6" > "${pamcfg}"

echo "HOTP user1 - 00" > "${usrcfg}"
echo "HOTP user2 pw 00" >> "${usrcfg}"
echo "HOTP/T30 user3 - 00" >> "${usrcfg}"

export PAM_WRAPPER=1 \
       PAM_WRAPPER_SERVICE_DIR="${pamcfgdir}" \
       PAM_WRAPPER_DEBUGLEVEL=2

TSTAMP=`TZ=UTC datefudge "2006-09-23" date -u +%s`
if test "$TSTAMP" != "1158969600"; then
    echo "Cannot fake timestamp, install datefudge to check better. ($TSTAMP)"
    LD_PRELOAD=libpam_wrapper.so "${srcdir}/test-pam_oath-e2e"
    rc=$?
else
    LD_PRELOAD=libpam_wrapper.so TZ=UTC datefudge "2006-12-07" \
        "${srcdir}/test-pam_oath-e2e"
    rc=$?
fi

if test "$rc" != "77"; then
    diff -u "${srcdir}/expect.oath" "${usrcfg}" || rc=1
fi

exit $rc
