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

AM_DISTCHECK_CONFIGURE_FLAGS = --enable-gtk-doc

SUBDIRS = gl liboath oathtool

if ENABLE_PSKC
SUBDIRS += libpskc
SUBDIRS += pskctool
endif

if ENABLE_PAM
SUBDIRS += pam_oath
endif

EXTRA_DIST = GNUmakefile cfg.mk maint.mk
EXTRA_DIST += CONTRIBUTING.md

# Website
EXTRA_DIST += website/asciidoc.css website/build-website.sh		\
	website/docs.txt website/download.txt website/intro-1.txt	\
	website/intro-2.txt website/config.cfg website/layout.css	\
	website/oath-toolkit.xsl website/style.css

# gnulib version-stamp
EXTRA_DIST += $(top_srcdir)/.version
BUILT_SOURCES = $(top_srcdir)/.version

# git-version-gen
dist-hook: dist-tarball-version
.PHONY: dist-tarball-version
dist-tarball-version:
	echo '$(VERSION)' > $(distdir)/.tarball-version

dist-hook: srcdist
.PHONY: srcdist
srcdist:
	$(AM_V_GEN)if test -e $(srcdir)/.git \
			&& command -v git > /dev/null; then \
	 cd $(srcdir) && \
	   git archive --prefix=$(PACKAGE)-v$(VERSION)/ \
	       -o $(abs_builddir)/$(PACKAGE)-v$(VERSION)-src.tar.gz HEAD; \
	fi

dist-hook: gen-ChangeLog
.PHONY: gen-ChangeLog
gen-ChangeLog:
	$(AM_V_GEN)if test -e $(srcdir)/.git; then			\
	  LC_ALL=en_US.UTF-8 TZ=UTC0					\
	    $(top_srcdir)/build-aux/gitlog-to-changelog			\
	      --srcdir=$(srcdir) --					\
	      v$(PREV_VERSION)~.. > $(distdir)/cl-t &&			\
	    { printf '\n\nSee the source repo for older entries\n'	\
	      >> $(distdir)/cl-t &&					\
	      rm -f $(distdir)/ChangeLog &&				\
	      mv $(distdir)/cl-t $(distdir)/ChangeLog; }		\
	fi
