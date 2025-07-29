Copyright (C) 2009-2025 Simon Josefsson.  Licensed under the GPLv3+.

# OATH Toolkit Developer Information

Download, build and self-check version controlled sources:

```
git clone https://codeberg.org/oath-toolkit/oath-toolkit.git
cd oath-toolkit
./bootstrap
./configure
make
make check
```

Links to resources that may be useful if you want to get involved the
project:

- Mailing list: https://lists.nongnu.org/mailman/listinfo/oath-toolkit-help
- Codeberg OATH Toolkit Project: https://codeberg.org/oath-toolkit/oath-toolkit
- Savannah OATH Toolkit Project: https://savannah.nongnu.org/projects/oath-toolkit/
- GitLab Pipeline: https://gitlab.com/oath-toolkit/oath-toolkit/-/pipelines
- Code coverage report: https://oath-toolkit.gitlab.io/oath-toolkit/coverage/
- Clang code analysis: https://oath-toolkit.gitlab.io/oath-toolkit/clang-analyzer/
- Pre-release version of website: https://oath-toolkit.gitlab.io/oath-toolkit/

# Dependencies

We rely on several tools to build the software, including:

- Gnulib <https://www.gnu.org/software/gnulib/>
- Make <https://www.gnu.org/software/make/>
- C compiler <https://www.gnu.org/software/gcc/>
- Automake <https://www.gnu.org/software/automake/>
- Autoconf <https://www.gnu.org/software/autoconf/>
- Libtool <https://www.gnu.org/software/libtool/>
- Bison <https://www.gnu.org/software/bison/>
- help2man <https://www.gnu.org/software/help2man/>
- Gengetopt <https://www.gnu.org/software/gengetopt/>
- Tar <https://www.gnu.org/software/tar/>
- Gzip <https://www.gnu.org/software/gzip/>
- GTK-DOC <https://gitlab.gnome.org/GNOME/gtk-doc> (for API manuals)
- Git <https://git-scm.com/>
- Valgrind <https://valgrind.org/> (optional)
- PAM library/headers (optional, required for PAM module)
- XMLSec <https://www.aleksey.com/xmlsec/> (optional, for libpskc)

The software is typically distributed with your operating system, and
the instructions for installing them differ.  Below are some hints.
If you have hints on how to install the required dependencies on other
operating systems, please provide a patch explaining it.  Find
inspiration from build rules in `.gitlab-ci.yml`.

## Debian/Ubuntu dependencies

```
apt-get install make git autoconf automake libtool bison gengetopt valgrind
apt-get install libpam0g-dev libxmlsec1-dev libxml2-utils
apt-get install gnulib help2man gtk-doc-tools libglib2.0-dev dblatex
```

# Valgrind suppression

When building from version controlled sources, some developer specific
flags are automatically enabled.  For example, the self-checks are run
under valgrind if available.  For various reasons, you may run into
valgrind false positives that will cause self-checks to fail.  First
be sure to install debug symbols for system libraries.  We ship a
Valgrind suppression file to address common issues.  You can use it by
putting the following in your `~/.valgrindrc`:

```
--suppressions=/path/to/oath-toolkit/libpskc/tests/libpskc.supp
```

# Release Process

To prepare a release you need some additional tools:

- Guix <https://guix.gnu.org/>
- Groff <https://www.gnu.org/software/groff/>
- Asciidoc <http://www.methods.co.nz/asciidoc/>
- XSLT <http://xmlsoft.org/xslt/>
- Lcov (to produce coverage HTML pages)
- Clang (to produce clang analysis)
- rsync <https://rsync.samba.org/>

Debian/Ubuntu dependencies:

```
apt-get install groff asciidoc xsltproc lcov clang rsync
```

Most of the release process rely on gnulib scripts and maint.mk rules,
and the steps below are inspired by gnulib's README-release.

Here are most of the steps we (maintainers) follow when making a release.

* Start from a clean, up-to-date git directory on "main":

```
git checkout main
git pull origin main
git clean -d -x -f
git restore --staged .
git reset --hard
```

* Ensure that the latest stable versions of autoconf, automake, etc.
  are in your PATH.

* Make sure you have updated to latest gnulib files.  The GitLab CI/CD
  pipeline uses the GNULIB_REVISION setting from `.gitlab-ci.yml`, and
  you ought to use the same locally to be able to reproduce the
  release tarball.

* Make sure `NEWS` reflect all changes made since the last release.

```
make review-diff
```

* Ensure that you have no uncommitted diffs.  This should produce no
  output:

```
git diff
```

* Ensure that you've pushed all changes that belong in the release:

```
git push origin main
```

* Check that the GitLab CI/CD Pipeline is reporting all is well:

https://gitlab.com/oath-toolkit/oath-toolkit/-/pipelines

* Run the following commands:

```
./bootstrap
./configure
make check syntax-check distcheck
```

* To (i) set the date, version number, and release TYPE on line 3 of
  NEWS, (ii) commit that, and (iii) tag the release, run

```
# "TYPE" must be stable, beta or alpha
make release-commit RELEASE='X.Y.Z TYPE'
```

* Push the NEWS-updating changes and the new tag:

```
v=$(cat .prev-version)
git push origin main tag v$v
```

* Run the following to create release tarballs.

```
make release RELEASE='X.Y.Z TYPE' gnulib_dir=../gnulib
```

* Write the release announcement that you will soon post.  Start with
  the template, $HOME/announce-oath-toolkit-X.Y.Z that was just
  created by that "make" command.

* Confirm that the pipeline passes and that your local tarballs are
  bit-by-bit identical to the B-Guix and R-Guix pipeline jobs.

* To upload the tarballs to the Savannah download area run:

```
make release-upload-ftp
```

* Make sure ../www-oath-toolkit/ contains a git checkout of the
  website git repository.

```
cd ..
git clone ssh://git@codeberg.org/oath-toolkit/pages.git www-oath-toolkit
```

* Run the following to upload the website:

```
make release-upload-www
```

* Send the announcement email message.

* Start next development cycle by pushing the post-release commit.

```
git push main
```

* Commit and push updates of the release process depending on your
  experience following these steps.

Happy hacking!
