# Copyright (C) 2009-2024 Simon Josefsson

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

AM_LDFLAGS = -no-install

LDADD = ../pam_oath.la -lpam

EXTRA_DIST = expect.oath

TESTS =

check_PROGRAMS =

if TEST_PAM
TESTS += test-pam_oath
check_PROGRAMS += test-pam_oath$(EXEEXT)

if ROOT_TESTS
TESTS += run-root-tests.sh
check_PROGRAMS += test-pam_oath-root$(EXEEXT)
dist_check_SCRIPTS = run-root-tests.sh
endif
endif
