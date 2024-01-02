# Copyright (C) 2012-2024 Simon Josefsson

# This library is free software; you can redistribute it and/or modify
# it under the terms of the GNU Lesser General Public License as
# published by the Free Software Foundation; either version 2.1 of the
# License, or (at your option) any later version.

# This library is distributed in the hope that it will be useful, but
# WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
# 02110-1301 USA

AM_CPPFLAGS = -I$(top_srcdir)/include -I$(top_builddir)/include
AM_LDFLAGS = -no-install

AM_CFLAGS = $(WARN_CFLAGS) $(WERROR_CFLAGS)

LDADD = ../libpskc.la

noinst_PROGRAMS = serialno pskc2csv pskcsign pskcverify pskccreate

# Remember to update EXAMPLES_SOURCE_FILES in ../gtk-doc/Makefile.am
# if you add examples.

# Manually maintained.
EXTRA_DIST = pskc-mini.xml pskc-hotp.xml

# The following files used to be generated -- but there is a
# bootstrapping issue so we added the files to git instead, and verify
# in pskctool/tests/tst_libexamples.sh that they match with how
# they would have be generated.
EXTRA_DIST += pskctool-h.txt
EXTRA_DIST += pskc-hotp-human.txt pskc-hotp-signed.xml
EXTRA_DIST += pskc-figure6-debug.txt
