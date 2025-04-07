# Copyright (C) 2012-2025 Simon Josefsson

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

dist_man_MANS = $(gdoc_MANS)
MAINTAINERCLEANFILES = $(dist_man_MANS)

EXTRA_DIST = gdoc

GDOC_SRC = $(top_srcdir)/global.c $(top_srcdir)/coding.c	\
	$(top_srcdir)/usersfile.c $(top_srcdir)/hotp.c		\
	$(top_srcdir)/totp.c $(top_srcdir)/errors.c

BUILT_SOURCES = $(gdoc_MANS)

gdoc_MANS =
gdoc_MANS += oath_init.3
gdoc_MANS += oath_done.3
gdoc_MANS += oath_check_version.3
gdoc_MANS += oath_hex2bin.3
gdoc_MANS += oath_bin2hex.3
gdoc_MANS += oath_base32_decode.3
gdoc_MANS += oath_base32_encode.3
gdoc_MANS += oath_authenticate_usersfile.3
gdoc_MANS += oath_hotp_generate.3
gdoc_MANS += oath_hotp_validate_callback.3
gdoc_MANS += oath_hotp_validate.3
gdoc_MANS += oath_totp_generate.3
gdoc_MANS += oath_totp_generate2.3
gdoc_MANS += oath_totp_validate.3
gdoc_MANS += oath_totp_validate_callback.3
gdoc_MANS += oath_totp_validate2.3
gdoc_MANS += oath_totp_validate2_callback.3
gdoc_MANS += oath_totp_validate3.3
gdoc_MANS += oath_totp_validate3_callback.3
gdoc_MANS += oath_totp_validate4.3
gdoc_MANS += oath_totp_validate4_callback.3
gdoc_MANS += oath_strerror.3
gdoc_MANS += oath_strerror_name.3

.PHONY: compare-makefile

compare-makefile:
	$(AM_V_GEN) \
	MANS=""; \
	FUNCS=`$(srcdir)/gdoc -listfunc $(GDOC_SRC)`; \
	for i in $$FUNCS; do \
		MANS="$$MANS\ngdoc_MANS += $$i.3"; \
	done && \
	grep -v -e '^gdoc_MANS += ' $(srcdir)/Makefile.am | \
		perl -p -e "s,^gdoc_MANS =,gdoc_MANS =$$MANS,;" \
		> tmp-$@ && \
	diff -u $(srcdir)/Makefile.am tmp-$@ && \
	rm -f tmp-$@

EXTRA_DIST += stamp-gdoc

$(gdoc_MANS): stamp-gdoc

clean-local:
	-rm -f stamp-gdoc

stamp-gdoc: $(top_srcdir)/../.version $(GDOC_SRC)
	$(AM_V_GEN)for i in `$(srcdir)/gdoc -listfunc $(GDOC_SRC)`; do \
		echo -n "Creating documentation for $$i... " && \
		$(srcdir)/gdoc -man \
			-module $(PACKAGE) \
			-sourceversion $(VERSION) \
		        -bugsto $(PACKAGE_BUGREPORT) \
			-pkg-name "$(PACKAGE_NAME)" \
			-pkg-url "$(PACKAGE_URL)" \
			-includefuncprefix \
		        -copyright "2009-2025 Simon Josefsson" \
		        -verbatimcopying \
			-function $$i \
			$(GDOC_SRC) > $$i.3 && \
		echo "ok"; \
	done
	$(AM_V_at)touch $@
