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

old_NEWS_hash = e0a804a2c5d068b972c54600336f0f8e

guix = $(shell command -v guix > /dev/null && echo ,guix)
bootstrap-tools = git,gnulib,autoconf,automake,libtoolize,make,bison,help2man,gengetopt,gtkdocize,tar,gzip$(guix)

# syntax-check.
VC_LIST_ALWAYS_EXCLUDE_REGEX = ^(liboath|libpskc)/man/gdoc|libpskc/schemas/$$

# Project wide exceptions on philosophical grounds.
local-checks-to-skip = sc_GPL_version sc_prohibit_strcmp
# Re-add when we have translation.
local-checks-to-skip += sc_unmarked_diagnostics sc_bindtextdomain
# Revisit these soon.
local-checks-to-skip += sc_prohibit_atoi_atof sc_prohibit_gnu_make_extensions
# The following requires gnulib-srcdir.
local-checks-to-skip += sc_prohibit_intprops_without_use sc_prohibit_always-defined_macros sc_prohibit_always_true_header_tests

# syntax-check: Explicit syntax-check exceptions.
exclude_file_name_regexp--sc_avoid_if_before_free = ^pam_oath/pam_modutil.c$$
exclude_file_name_regexp--sc_codespell = ^m4/pkg.m4$$
exclude_file_name_regexp--sc_error_message_uppercase = ^oathtool/oathtool.c|pskctool/pskctool.c$$
exclude_file_name_regexp--sc_fsf_postal = ^m4/pkg.m4$$
exclude_file_name_regexp--sc_program_name = ^liboath/tests/|libpskc/examples/|libpskc/tests/|pam_oath/tests/
exclude_file_name_regexp--sc_prohibit_empty_lines_at_EOF = ^(liboath/|libpskc/|)gl/override/.*|libpskc/schemas/xenc-schema.xsd$$
exclude_file_name_regexp--sc_prohibit_have_config_h = ^liboath/tests/tst_fopen-wx.c$$
exclude_file_name_regexp--sc_readme_link_copying = ^libpskc/README|pam_oath/README$$
exclude_file_name_regexp--sc_readme_link_install = $(exclude_file_name_regexp--sc_readme_link_copying)
exclude_file_name_regexp--sc_require_config_h = ^libpskc/examples/
exclude_file_name_regexp--sc_require_config_h_first = $(exclude_file_name_regexp--sc_require_config_h)
exclude_file_name_regexp--sc_space_tab = ^m4/pkg.m4$$
exclude_file_name_regexp--sc_trailing_blank = ^m4/pkg.m4|libpskc/examples/pskctool-h.txt|libpskc/schemas/xmldsig-core-schema.xsd|(liboath/|libpskc/|)gl/override/.*|libpskc/schemas/xenc-schema.xsd$$
exclude_file_name_regexp--sc_two_space_separator_in_usage = ^pskctool/tests/

TAR_OPTIONS += --mode=go+u,go-w --mtime=$(abs_top_srcdir)/NEWS

announce_gen_args = --cksum-checksums
url_dir_list = https://download.savannah.nongnu.org/releases/oath-toolkit

DIST_ARCHIVES += $(shell \
	if test -e $(srcdir)/.git && command -v git > /dev/null; then \
		echo $(PACKAGE)-v$(VERSION)-src.tar.gz; \
	fi)

update-copyright-env = UPDATE_COPYRIGHT_HOLDER="Simon Josefsson" UPDATE_COPYRIGHT_USE_INTERVALS=2

my-update-copyright: update-copyright
	perl -pi -e "s/-20.. Simon Josefsson/-`(date +%Y)` Simon Josefsson/" liboath/man/Makefile.am libpskc/man/Makefile.am

review-diff:
	git diff `git describe --abbrev=0`.. \
	| grep -v -e ^index -e '^diff --git' \
	| filterdiff -p 1 -x 'build-aux/*' -x '*/build-aux/*' -x 'gl/*' -x '*/gl/*' -x 'gltests/*' -x '*/gltests/*' -x 'maint.mk' -x '.gitignore' -x '.x-sc*' -x 'ChangeLog' -x 'GNUmakefile' \
	| less

website:
	cd website && ./build-website.sh

website-copy:
	rsync -av --exclude .git --exclude coverage --exclude clang-analyzer --delete website/html/ $(htmldir)/
	ln -s liboath-oath.h.html $(htmldir)/liboath/liboath-oath.html
	ln -s liboath-oath.h.html $(htmldir)/liboath-api/liboath-oath.html
	ln -s liboath $(htmldir)/reference

website-upload:
	cd $(htmldir) && \
		git add . && \
		git commit -m "Auto-update." && \
		git push

release-upload-www: website website-copy website-upload

release-upload-ftp:
	mkdir -p ../releases/$(PACKAGE)/
	cp -v $(distdir).tar.gz $(distdir).tar.gz.sig $(distdir)-src.tar.gz $(distdir)-src.tar.gz.sig ../releases/$(PACKAGE)/
	scp $(distdir).tar.gz $(distdir).tar.gz.sig $(distdir)-src.tar.gz $(distdir)-src.tar.gz.sig jas@dl.sv.nongnu.org:/releases/oath-toolkit/
