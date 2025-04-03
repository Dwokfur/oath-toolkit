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

AC_INIT([OATH Toolkit],
  m4_esyscmd([build-aux/git-version-gen --prefix oath-toolkit- .tarball-version]),
  [oath-toolkit-help@nongnu.org],
  [oath-toolkit],
  [https://www.nongnu.org/oath-toolkit/])

AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_HEADERS([config.h])
AC_CONFIG_MACRO_DIRS([m4 gl/m4])
AM_INIT_AUTOMAKE([1.13 -Wall])

AM_MISSING_PROG(HELP2MAN, help2man, $missing_dir)

AC_PROG_CC
gl_EARLY
AM_PROG_AR
LT_INIT([win32-dll])
gl_INIT

AC_CHECK_HEADERS_ONCE([security/pam_appl.h])
AC_ARG_ENABLE([pam],
  [AS_HELP_STRING([--disable-pam], [disable PAM module])],
   [],
   [enable_pam=$ac_cv_header_security_pam_appl_h])
AM_CONDITIONAL([ENABLE_PAM], [test "x$enable_pam" = xyes])

PKG_CHECK_MODULES(XML2, libxml-2.0, enable_pskc=yes, enable_pskc=no)
AC_ARG_ENABLE([pskc],
  [AS_HELP_STRING([--disable-pskc], [disable PSKC support])],
   [enable_pskc=$enableval],
   [enable_pskc=yes])
AM_CONDITIONAL([ENABLE_PSKC], [test "x$enable_pskc" = xyes])

AC_CONFIG_SUBDIRS([
  liboath
])
if test "x$enable_pskc" = xyes; then
  AC_CONFIG_SUBDIRS([libpskc])
fi
if test "x$enable_pam" = xyes; then
  AC_CONFIG_SUBDIRS([pam_oath])
fi
AC_CONFIG_FILES([
  Makefile
  gl/Makefile
  oathtool/Makefile
  oathtool/tests/Makefile
  pskctool/Makefile
  pskctool/tests/Makefile
])
AC_OUTPUT

AC_MSG_NOTICE([summary of build options:

  version:          ${VERSION}
  Install prefix:   ${prefix}
  Compiler:         ${CC}
  Build PAM module: $enable_pam
  PSKC:             $enable_pskc
])
