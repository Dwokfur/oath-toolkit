# OATH Toolkit README

OATH Toolkit provide components to build one-time password
authentication systems.  It contains shared C libraries, command line
tools and a PAM module.  Supported technologies include the
event-based HOTP algorithm (RFC 4226), the time-based TOTP algorithm
(RFC 6238), and Portable Symmetric Key Container (PSKC, RFC 6030) to
manage secret key data.  OATH stands for Open AuTHentication, which is
the organization that specify the algorithms.

The following components are included:

 * `liboath`: A shared and static C library for OATH handling.
 * `oathtool`: A command line tool for generating and validating OTPs.
 * `pam_oath`: A PAM module for pluggable login authentication for OATH.
 * `libpskc`: A shared and static C library for PSKC handling.
 * `pskctool`: A command line tool for manipulating PSKC data.

For updates on the project please visit our web sites:

 * [OATH Toolkit webpage](https://www.nongnu.org/oath-toolkit/)
 * [Savannah project page](https://savannah.nongnu.org/projects/oath-toolkit/)
 * [GitLab project page](https://gitlab.com/oath-toolkit/oath-toolkit)

# License

OATH Toolkit is licensed under the GPLv3+ license, see the file
[COPYING](COPYING).

Liboath and libpskc are available under the LGPLv2.1+, see
[liboath/COPYING](liboath/COPYING), although you may use it under the
GPLv3+ license if you prefer.

Many files in the official release tarball are automatically generated
by various tools, for example Makefile.in by automake and
documentation by GTK-DOC.  For authoritative licensing information,
refer to information in each file.

For any copyright year range specified as YYYY-ZZZZ in this package
note that the range specifies every single year in that closed
interval.

# Building

The file [INSTALL](INSTALL) contains generic installation and
configuration instructions, however briefly the typical process is:

```
./configure
make check install
```

If any of the self-tests fail, please report it as a bug.

# Contact

For e-mail discussions related the OATH Toolkit, please send e-mail to
oath-toolkit-help@nongnu.org after subscribing to the [mailing
list](https://lists.nongnu.org/mailman/listinfo/oath-toolkit-help),
where you can find an archive of earlier posts.

# Contributing

To report a problem please open an issue on the GitLab OATH Toolkit
project page: https://gitlab.com/oath-toolkit/oath-toolkit/-/issues

To build from version controlled sources, and other information for
developers, read the file [CONTRIBUTING.md](CONTRIBUTING.md).

```
Copyright (C) 2009-2024 Simon Josefsson

This file is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation, either version 3 of the License, or (at your
option) any later version.

This file is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this file.  If not, see <http://www.gnu.org/licenses/>.
```
