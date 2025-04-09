# Copyright (C) 2009-2025 Simon Josefsson

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

.NOTPARALLEL:

SUBDIRS = . tests

AM_CPPFLAGS = -I$(builddir)/../liboath
AM_CPPFLAGS += -I$(srcdir)/../gl -I$(builddir)/../gl

bin_PROGRAMS = oathtool
oathtool_SOURCES = oathtool.c
oathtool_LDADD = ../liboath/liboath.la ../gl/libgnu.la liboathtool_cmd.la

noinst_LTLIBRARIES = liboathtool_cmd.la
BUILT_SOURCES = oathtool_cmd.c oathtool_cmd.h
liboathtool_cmd_la_SOURCES = $(BUILT_SOURCES) oathtool.ggo
liboathtool_cmd_la_LIBADD = ../gl/libgnu.la
liboathtool_cmd_la_CFLAGS =

oathtool_cmd.h oathtool_cmd.c: oathtool.ggo Makefile.am
	$(AM_V_GEN)cp -v $(srcdir)/oathtool.ggo repro-gengetopt.ggo && \
	gengetopt --unamed-opts="KEY [OTP]" \
		--set-package="oathtool" \
		--no-handle-version --no-handle-help \
		--input repro-gengetopt.ggo --file-name oathtool_cmd && \
	rm -v repro-gengetopt.ggo && \
	perl -pi -e 's/HAVE_LONG_LONG/HAVE_UNSIGNED_LONG_LONG/g' oathtool_cmd.h oathtool_cmd.c && \
	perl -pi -e 's/  long long int/  unsigned long long int/g' oathtool_cmd.h oathtool_cmd.c && \
	perl -pi -e 's/int\) strtoll /int\) strtoull /' oathtool_cmd.c

dist_man_MANS = oathtool.1
MAINTAINERCLEANFILES = $(dist_man_MANS)
EXTRA_DIST = oathtool.h2m

oathtool.1: $(srcdir)/oathtool.c $(srcdir)/oathtool.ggo $(srcdir)/oathtool.h2m $(top_srcdir)/.version
	$(AM_V_GEN)env SOURCE_DATE_EPOCH=$(SOURCETIME_HELP2MAN) $(HELP2MAN) \
		--include $(srcdir)/oathtool.h2m \
		--no-info \
		--output=$@ $(builddir)/oathtool$(EXEEXT)
