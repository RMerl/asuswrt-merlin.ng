libConfuse
==========
[![Travis Status][]][Travis] [![Coverity Status][]][Coverity Scan]

Table of Contents
-----------------

* [Introduction](#introduction)
* [Examples](#examples)
* [Download](#download)
* [Building](#building)
* [Documentation](#documentation)
* [News](#news)


Introduction
------------

libConfuse is a configuration file parser library, licensed under the
terms of the [ISC license][1], and written in C.  It supports sections
and (lists of) values (strings, integers, floats, booleans or other
sections), as well as some other features (such as single/double-quoted
strings, environment variable expansion, functions and nested include
statements).  It makes it very easy to add configuration file capability
to a program using a simple API.

The goal of libConfuse is not to be _the_ configuration file parser
library with a gazillion of features.  Instead, it aims to be easy to
use and quick to integrate with your code.

libConfuse was called libcfg before, but was changed to not confuse with
other similar libraries.

Please report bugs to the GitHub [issue tracker][2].  If you want to
contribute fixes or new features, see the file
[CONTRIBUTING.md](CONTRIBUTING.md).


Examples
--------

Example configuration files:

* [test.conf](examples/test.conf) and the
  [source code](examples/cfgtest.c) shows most of the
  features of confuse, including lists and functions.
* [simple.conf](examples/simple.conf) shows how to use the
  "simple" versions of options. See the corresponding
  [source](examples/simple.c).


Download
--------

The source code is distributed in three files: Two tarballs in .tar.gz
and .tar.xz formats with source code and Makefiles for UNIX systems, and
a .zip file with Windows build files.

* confuse v3.2:
  [UNIX tar.gz](https://github.com/martinh/libconfuse/releases/download/v3.2/confuse-3.2.tar.gz),
  [Windows ZIP](https://github.com/martinh/libconfuse/releases/download/v3.2/confuse-3.2.zip)
* [New releases](https://github.com/martinh/libconfuse/releases)
* [Old releases](http://savannah.nongnu.org/download/confuse/)


Building
--------

libConfuse employs the GNU configure and build system.  Simply enter
<kbd>./configure --help</kbd> to list available options and see the
INSTALL file for the full installation instructions.

When checking out the code from GitHub, use <kbd>./autogen.sh</kbd> to
generate a `configure` script.


Documentation
-------------

For the time being, the following documentation is published at the
[old homepage](http://www.nongnu.org/confuse/), but also distributed
with the source:

* [API reference/manual](http://www.nongnu.org/confuse/manual/) (generated with doxygen)
* [Tutorial](http://www.nongnu.org/confuse/tutorial-html/) (a work in progress)
* [ChangeLog](ChangeLog.md) (check what's new!)


News
----

* 2018-08-19: libConfuse version 3.2.2 released (CVE-2018-14447)
* 2017-08-17: libConfuse version 3.2.1 released (major ABI bump!)
* 2017-06-03: libConfuse version 3.2 released
* 2017-05-24: libConfuse version 3.1 released
* 2016-03-03: libConfuse version 3.0 released
* 2015-10-14: libConfuse version 2.8 released
* 2015-10-12: New patch monkey at the helm of GitHub.
  [Joachim](https://github.com/troglobit) will help out auditing pull
  requests and do occasional releases.
* 2014-01-10: code moved to
  [github](https://github.com/martinh/libconfuse)
* 2010-02-21: libConfuse version 2.7 released. This is primarily a
  bugfix release
* 2007-12-29: libConfuse version 2.6 released
* 2007-11-29: libConfuse 2.6 will be released as soon as I can get some
  time for it. promise!
* 2004-10-17: libConfuse version 2.5 released
* 2004-09-23: There is now a
  [tutorial](http://www.nongnu.org/confuse/tutorial-html/index.html)
  available
* 2004-08-09: libConfuse version 2.4 released
* 2004-05-22: libConfuse version 2.3 released
* 2003-09-25: libConfuse version 2.2 released
* 2003-07-13: libConfuse version 2.1 released
* 2003-07-03: libConfuse webpage moved to
  [Savannah](http://www.nongnu.org/confuse/)
* 2003-04-05: libConfuse version 2.0 released
* 2003-03-02: There is now a mailing list and a project page at
  [savannah.nongnu.org](http://savannah.nongnu.org/projects/confuse/)
* 2002-12-18: version 1.2.3 released that fixes a segfault due to an
  uninitialized user-defined error function. Support for callbacks are
  added, however they are not yet properly documented


[1]:                http://en.wikipedia.org/wiki/ISC_license
[2]:                https://github.com/martinh/libconfuse/issues
[Travis]:           https://travis-ci.org/troglobit/libconfuse
[Travis Status]:    https://travis-ci.org/troglobit/libconfuse.png?branch=master
[Coverity Scan]:    https://scan.coverity.com/projects/6674
[Coverity Status]:  https://scan.coverity.com/projects/6674/badge.svg

<!--
  -- Local Variables:
  -- mode: markdown
  -- End:
  -->
