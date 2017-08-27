# <a name="title"></a> Libical [![Build Status](https://travis-ci.org/libical/libical.svg?branch=master)](https://travis-ci.org/libical/libical) [![Coverity Scan Build Status](https://scan.coverity.com/projects/2367/badge.svg)](https://scan.coverity.com/projects/2367)

Introduction
============
Libical -- an implementation of iCalendar protocols and data formats

Most of the code in here was written by Eric Busboom with help from
dozens of contributors.  It is currently maintained by Art Cancro
and Wilfried Goesgens.

Libical is an Open Source implementation of the iCalendar protocols
and protocol data units. The iCalendar specification describes how
calendar clients can communicate with calendar servers so users can
store their calendar data and arrange meetings with other users. 

Libical implements RFC5545, RFC5546, RFC7529; the iCalendar extensions
in RFC6638; and some of RFC6047.


License
========
The code and datafiles in this distribution are licensed under the
Mozilla Public License (MPL) v1.0. See http://www.mozilla.org/MPL/1.0
for a copy of the license. 

Alternately, you may use libical under the terms of the GNU Library
General Public License (LGPL) v2.1. See http://www.gnu.org/licenses/lgpl-2.1.txt
for a copy of the license.

This dual license ensures that the library can be incorporated into
both proprietary code and GPL'd programs, and will benefit from improvements
made by programmers in both realms. I will only accept changes into
my version of the library if they are similarly dual-licensed.


Acknowledgments
===============
Portions of this distribution are (C) Copyright 1996 Apple Computer,
Inc., AT&T Corp., International Business Machines Corporation and
Siemens Rolm Communications Inc. See src/libicalvcal/README.TXT for
details.


Get Involved
============
Subscribe to our mailing lists:

For developer discussions
  http://lists.infradead.org/mailman/listinfo/libical-devel

For general discussions about libical and related projects
  http://lists.infradead.org/mailman/listinfo/libical-interest

Report bugs to our issue tracker at
  https://github.com/libical/libical/issues


Building the library
====================
See the top-level Install.txt file.


Using the Library
=================
There is rudimentary, unfinished documentation in the /doc directory,
and annotated examples in /examples and the test code in src/test.
