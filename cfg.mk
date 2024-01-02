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

CFGFLAGS = --enable-gtk-doc --enable-gtk-doc-pdf --enable-gcc-warnings	\
	--enable-root-tests --enable-valgrind-tests

INDENT_SOURCES = `find . -name '*.[ch]' | grep -v -e /gl/ -e build-aux -e /config.h -e _cmd.`

# syntax-check
VC_LIST_ALWAYS_EXCLUDE_REGEX = ^GNUmakefile|maint.mk|build-aux/|gl/|m4/libxml2.m4|oathtool/doc/parse-datetime.texi|(liboath|libpskc)/man/gdoc|liboath/gtk-doc.make|libpskc/gtk-doc.make|libpskc/schemas/|(oathtool|liboath)/(build-aux|gl)/.*$$
# syntax-check: Project wide exceptions on philosophical grounds.
local-checks-to-skip = sc_GPL_version sc_immutable_NEWS	\
	sc_prohibit_strcmp
# syntax-check: Re-add when we have translation.
local-checks-to-skip += sc_unmarked_diagnostics sc_bindtextdomain
# syntax-check: Revisit these soon.
local-checks-to-skip += sc_prohibit_atoi_atof sc_prohibit_gnu_make_extensions
# syntax-check: The following requires gnulib-srcdir.
local-checks-to-skip += sc_prohibit_intprops_without_use sc_prohibit_always-defined_macros
# syntax-check: Explicit syntax-check exceptions.
exclude_file_name_regexp--sc_program_name = ^liboath/tests/|libpskc/examples/|libpskc/tests/|pam_oath/tests/
exclude_file_name_regexp--sc_texinfo_acronym = ^oathtool/doc/parse-datetime.texi
exclude_file_name_regexp--sc_error_message_uppercase = ^oathtool/oathtool.c|pskctool/pskctool.c
exclude_file_name_regexp--sc_require_config_h = ^libpskc/examples/
exclude_file_name_regexp--sc_require_config_h_first = $(exclude_file_name_regexp--sc_require_config_h)
exclude_file_name_regexp--sc_trailing_blank = ^m4/pkg.m4|libpskc/examples/pskctool-h.txt$$
exclude_file_name_regexp--sc_two_space_separator_in_usage = ^pskctool/tests/
exclude_file_name_regexp--sc_space_tab = ^m4/pkg.m4$$
exclude_file_name_regexp--sc_avoid_if_before_free = ^pam_oath/pam_modutil.c$$
exclude_file_name_regexp--sc_readme_link_copying = ^libpskc/README|pam_oath/README$$
exclude_file_name_regexp--sc_readme_link_install = $(exclude_file_name_regexp--sc_readme_link_copying)

update-copyright-env = UPDATE_COPYRIGHT_HOLDER="Simon Josefsson" UPDATE_COPYRIGHT_USE_INTERVALS=2

# maint.mk's public-submodule-commit breaks on shallow gnulib
# https://lists.gnu.org/archive/html/bug-gnulib/2022-08/msg00040.html
# so let's disable it - XXX FIXME let's revisit this later
submodule-checks =
gl_public_submodule_commit =

glimport:
	cd liboath && gtkdocize --copy
	cd libpskc && gtkdocize --copy
	gnulib-tool --add-import
	cd liboath && gnulib-tool --add-import
	cd oathtool && gnulib-tool --add-import
	cd libpskc && gnulib-tool --add-import
	cd pskctool && gnulib-tool --add-import

review-diff:
	git diff `git describe --abbrev=0`.. \
	| grep -v -e ^index -e '^diff --git' \
	| filterdiff -p 1 -x 'build-aux/*' -x '*/build-aux/*' -x 'gl/*' -x '*/gl/*' -x 'gltests/*' -x '*/gltests/*' -x 'maint.mk' -x '.gitignore' -x '.x-sc*' -x 'ChangeLog' -x 'GNUmakefile' \
	| less

# Release

tag = $(PACKAGE)-$(VERSION)
htmldir = ../www-$(PACKAGE)

tarball:
	test `git describe` = `git tag -l $(tag)`
	$(MAKE) distcheck

.PHONY: website
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

release-check: syntax-check tarball website website-copy

release-upload-www: website-upload

release-upload-ftp:
	gpg -b $(distdir).tar.gz
	gpg --verify $(distdir).tar.gz.sig
	mkdir -p ../releases/$(PACKAGE)/
	cp $(distdir).tar.gz $(distdir).tar.gz.sig ../releases/$(PACKAGE)/
	scp $(distdir).tar.gz $(distdir).tar.gz.sig jas@dl.sv.nongnu.org:/releases/oath-toolkit/
	git push
	git push --tags
	sha1sum $(distdir).tar.gz
	sha224sum $(distdir).tar.gz

tag: # Use "make tag VERSION=1.2.3"
	git tag -s -m $(VERSION) $(tag)

release: release-check release-upload-www release-upload-ftp
