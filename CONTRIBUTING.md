Copyright (C) 2009-2024 Simon Josefsson.  Licensed under the GPLv3+.

.Download, build and self-check version controlled sources:
----
git clone https://gitlab.com/oath-toolkit/oath-toolkit.git
cd oath-toolkit
make
make check
----

Links to resources that may be useful if you want to get involved the
project:

- Mailing list: https://lists.nongnu.org/mailman/listinfo/oath-toolkit-help
- Savannah OATH Toolkit Project: https://savannah.nongnu.org/projects/oath-toolkit/
- GitLab OATH Toolkit Project: https://gitlab.com/oath-toolkit/oath-toolkit
- GitLab Bug tracker: https://gitlab.com/oath-toolkit/oath-toolkit/issues
- Savannah bug tracker: https://savannah.nongnu.org/support/?group=oath-toolkit
- Code browser: https://gitlab.com/oath-toolkit/oath-toolkit/tree/master
- Code coverage report: https://oath-toolkit.gitlab.io/oath-toolkit/coverage/
- Clang code analysis: https://oath-toolkit.gitlab.io/oath-toolkit/clang-analyzer/
- Pre-release version of website: https://oath-toolkit.gitlab.io/oath-toolkit/

We rely on several tools to build the software, including:

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
the instructions for installing them differ.  Here are some hints:

.Debian/Ubuntu dependencies:
-----
apt-get install make git autoconf automake libtool bison gengetopt valgrind
apt-get install libpam0g-dev libxmlsec1-dev libxml2-utils
apt-get install help2man gtk-doc-tools libglib2.0-dev dblatex
-----

If you have hints on how to install the required dependencies on other
operating systems, please let us know.

When building from version controlled sources, some developer specific
flags are automatically enabled.  For example, the self-checks are run
under valgrind if available.  For various reasons, you may run into
valgrind false positives that will cause self-checks to fail.  First
be sure to install debug symbols for system libraries.  We ship a
Valgrind suppression file to address common issues.  You can use it by
putting the following in your ~/.valgrindrc:

----
--suppressions=/path/to/oath-toolkit/libpskc/tests/libpskc.supp
----

To prepare a release you need some additional tools:

- Groff <https://www.gnu.org/software/groff/>
- Asciidoc <http://www.methods.co.nz/asciidoc/>
- XSLT <http://xmlsoft.org/xslt/>
- Lcov (to produce coverage HTML pages)
- Clang (to produce clang analysis)
- rsync <https://rsync.samba.org/>

.Debian/Ubuntu dependencies:
-----
apt-get install groff asciidoc xsltproc lcov clang rsync
-----

The release rules are implemented in cfg.mk, and to make new official
release the following steps are made:

x. Make sure you have updated to latest gnulib files.
   make glimport
   git rm -f ...
   git add ...
   git commit -m "Update gnulib files." -a

x. Make sure you have pushed git to GitLab and that CI/CD passes.
   https://gitlab.com/oath-toolkit/oath-toolkit/-/pipelines

x. Make sure NEWS reflect all changes made since the last release.
   make review-diff

x. Make sure the '(unreleased)' string in NEWS is changed into
   '(released XXXX-YY-ZZ)' for the release.

x. Make sure you have committed everything and have a clean checkout.
   git clean -d -x -f
   git status
   git reset --hard

x. Run 'make tag VERSION=1.2.3' for the version number.
   Use 'git tag -d ...' to remove tags if you made mistakes.

x. Make sure ../www-oath-toolkit/ contains a git checkout of the
   website git repository, and ../www-oath-toolkit-cvs/ contains a CVS
   checkout of the website.
   cd ..
   git clone git@gitlab.com:oath-toolkit/website.git www-oath-toolkit
   cvs -z3 -d:ext:USER@cvs.savannah.nongnu.org:/web/oath-toolkit co -d www-oath-toolkit-cvs oath-toolkit

x. Create release artifacts.
   ./bootstrap
   ./configure --enable-gtk-doc --enable-gtk-doc-pdf --enable-gcc-warnings --enable-root-tests --enable-valgrind-tests
   make
   make release-check

x. Upload release artifacts.
   make release-upload-www release-upload-ftp

x. Manually update the CVS website that will be synchronized to the
   main website via savannah
   cd ../www-oath-toolkit-cvs/
   sh -x ./synk-from-git.sh
   cvs add ...
   cvs rm -f ...
   cvs commit -mUpdate.

x. Post release announcement based on doc/announce.txt, updating
   announcement in git.

x. On major releases, update webpages with information.

x. Start next development cycle by bumping version number in NEWS and
   use the '(unreleased)' to indicate that it is not released yet.

Happy hacking!
