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

GDOC_SRC = $(top_srcdir)/global.c $(top_srcdir)/errors.c $(top_srcdir)/enums.c
GDOC_SRC += $(top_srcdir)/container.c $(top_srcdir)/parser.c	\
	$(top_srcdir)/validate.c $(top_srcdir)/build.c		\
	$(top_srcdir)/sign.c $(top_srcdir)/output.c

BUILT_SOURCES = $(gdoc_MANS)

gdoc_MANS =
gdoc_MANS += pskc_global_init.3
gdoc_MANS += pskc_global_done.3
gdoc_MANS += pskc_check_version.3
gdoc_MANS += pskc_free.3
gdoc_MANS += pskc_global_log.3
gdoc_MANS += pskc_strerror.3
gdoc_MANS += pskc_strerror_name.3
gdoc_MANS += pskc_pinusagemode2str.3
gdoc_MANS += pskc_valueformat2str.3
gdoc_MANS += pskc_keyusage2str.3
gdoc_MANS += pskc_str2pinusagemode.3
gdoc_MANS += pskc_str2valueformat.3
gdoc_MANS += pskc_str2keyusage.3
gdoc_MANS += pskc_get_version.3
gdoc_MANS += pskc_set_version.3
gdoc_MANS += pskc_get_id.3
gdoc_MANS += pskc_set_id.3
gdoc_MANS += pskc_get_signed_p.3
gdoc_MANS += pskc_get_keypackage.3
gdoc_MANS += pskc_add_keypackage.3
gdoc_MANS += pskc_get_device_manufacturer.3
gdoc_MANS += pskc_set_device_manufacturer.3
gdoc_MANS += pskc_get_device_serialno.3
gdoc_MANS += pskc_set_device_serialno.3
gdoc_MANS += pskc_get_device_model.3
gdoc_MANS += pskc_set_device_model.3
gdoc_MANS += pskc_get_device_issueno.3
gdoc_MANS += pskc_set_device_issueno.3
gdoc_MANS += pskc_get_device_devicebinding.3
gdoc_MANS += pskc_set_device_devicebinding.3
gdoc_MANS += pskc_get_device_startdate.3
gdoc_MANS += pskc_set_device_startdate.3
gdoc_MANS += pskc_get_device_expirydate.3
gdoc_MANS += pskc_set_device_expirydate.3
gdoc_MANS += pskc_get_device_userid.3
gdoc_MANS += pskc_set_device_userid.3
gdoc_MANS += pskc_get_cryptomodule_id.3
gdoc_MANS += pskc_set_cryptomodule_id.3
gdoc_MANS += pskc_get_key_id.3
gdoc_MANS += pskc_set_key_id.3
gdoc_MANS += pskc_get_key_algorithm.3
gdoc_MANS += pskc_set_key_algorithm.3
gdoc_MANS += pskc_get_key_issuer.3
gdoc_MANS += pskc_set_key_issuer.3
gdoc_MANS += pskc_get_key_algparm_suite.3
gdoc_MANS += pskc_set_key_algparm_suite.3
gdoc_MANS += pskc_get_key_algparm_chall_encoding.3
gdoc_MANS += pskc_set_key_algparm_chall_encoding.3
gdoc_MANS += pskc_get_key_algparm_chall_min.3
gdoc_MANS += pskc_set_key_algparm_chall_min.3
gdoc_MANS += pskc_get_key_algparm_chall_max.3
gdoc_MANS += pskc_set_key_algparm_chall_max.3
gdoc_MANS += pskc_get_key_algparm_chall_checkdigits.3
gdoc_MANS += pskc_set_key_algparm_chall_checkdigits.3
gdoc_MANS += pskc_get_key_algparm_resp_encoding.3
gdoc_MANS += pskc_set_key_algparm_resp_encoding.3
gdoc_MANS += pskc_get_key_algparm_resp_length.3
gdoc_MANS += pskc_set_key_algparm_resp_length.3
gdoc_MANS += pskc_get_key_algparm_resp_checkdigits.3
gdoc_MANS += pskc_set_key_algparm_resp_checkdigits.3
gdoc_MANS += pskc_get_key_profileid.3
gdoc_MANS += pskc_set_key_profileid.3
gdoc_MANS += pskc_get_key_reference.3
gdoc_MANS += pskc_set_key_reference.3
gdoc_MANS += pskc_get_key_friendlyname.3
gdoc_MANS += pskc_set_key_friendlyname.3
gdoc_MANS += pskc_get_key_userid.3
gdoc_MANS += pskc_set_key_userid.3
gdoc_MANS += pskc_get_key_data_secret.3
gdoc_MANS += pskc_set_key_data_secret.3
gdoc_MANS += pskc_get_key_data_b64secret.3
gdoc_MANS += pskc_set_key_data_b64secret.3
gdoc_MANS += pskc_get_key_data_counter.3
gdoc_MANS += pskc_set_key_data_counter.3
gdoc_MANS += pskc_get_key_data_time.3
gdoc_MANS += pskc_set_key_data_time.3
gdoc_MANS += pskc_get_key_data_timeinterval.3
gdoc_MANS += pskc_set_key_data_timeinterval.3
gdoc_MANS += pskc_get_key_data_timedrift.3
gdoc_MANS += pskc_set_key_data_timedrift.3
gdoc_MANS += pskc_get_key_policy_startdate.3
gdoc_MANS += pskc_set_key_policy_startdate.3
gdoc_MANS += pskc_get_key_policy_expirydate.3
gdoc_MANS += pskc_set_key_policy_expirydate.3
gdoc_MANS += pskc_get_key_policy_pinkeyid.3
gdoc_MANS += pskc_set_key_policy_pinkeyid.3
gdoc_MANS += pskc_get_key_policy_pinusagemode.3
gdoc_MANS += pskc_set_key_policy_pinusagemode.3
gdoc_MANS += pskc_get_key_policy_pinmaxfailedattempts.3
gdoc_MANS += pskc_set_key_policy_pinmaxfailedattempts.3
gdoc_MANS += pskc_get_key_policy_pinminlength.3
gdoc_MANS += pskc_set_key_policy_pinminlength.3
gdoc_MANS += pskc_get_key_policy_pinmaxlength.3
gdoc_MANS += pskc_set_key_policy_pinmaxlength.3
gdoc_MANS += pskc_get_key_policy_pinencoding.3
gdoc_MANS += pskc_set_key_policy_pinencoding.3
gdoc_MANS += pskc_get_key_policy_keyusages.3
gdoc_MANS += pskc_set_key_policy_keyusages.3
gdoc_MANS += pskc_get_key_policy_numberoftransactions.3
gdoc_MANS += pskc_set_key_policy_numberoftransactions.3
gdoc_MANS += pskc_init.3
gdoc_MANS += pskc_done.3
gdoc_MANS += pskc_parse_from_memory.3
gdoc_MANS += pskc_validate.3
gdoc_MANS += pskc_build_xml.3
gdoc_MANS += pskc_sign_x509.3
gdoc_MANS += pskc_verify_x509crt.3
gdoc_MANS += pskc_output.3

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
