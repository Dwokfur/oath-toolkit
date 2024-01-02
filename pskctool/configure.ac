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

AC_INIT([pskctool],
  m4_esyscmd([../build-aux/oath-toolkit-version ../.tarball-version]),
  [oath-toolkit-help@nongnu.org],,
  [https://www.nongnu.org/oath-toolkit/])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIR([gl/m4])
AM_INIT_AUTOMAKE([1.13 -Wall foreign])

AM_MISSING_PROG(HELP2MAN, help2man, $missing_dir)

AC_PROG_CC
gl_EARLY
AM_PROG_AR
LT_INIT([win32-dll])
gl_INIT

AC_CONFIG_FILES([
  Makefile
  gl/Makefile
  tests/Makefile
])
AC_OUTPUT
