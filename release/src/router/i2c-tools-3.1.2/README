I2C TOOLS FOR LINUX
===================

This package contains an heterogeneous set of I2C tools for the Linux kernel.
These tools were originally part of the lm-sensors project but were finally
split into their own package for convenience. They compile, run and have been
tested on GNU/Linux.


CONTENTS
--------

The various tools included in this package are grouped by category, each
category has its own sub-directory:

* eeprom
  Perl scripts for decoding different types of EEPROMs (SPD, EDID...) These
  scripts rely on the "eeprom" kernel driver. They are installed by default.

* eepromer
  Tools for writing to EEPROMs. These tools rely on the "i2c-dev" kernel
  driver. They are not installed by default.

* include
  C/C++ header files for I2C and SMBus access over i2c-dev. Installed by
  default.

* py-smbus
  Python wrapper for SMBus access over i2c-dev. Not installed by default.

* stub
  A helper script to use with the i2c-stub kernel driver. Installed by
  default.

* tools
  I2C device detection and register dump tools. These tools rely on the
  "i2c-dev" kernel driver. They are installed by default.


INSTALLATION
------------

There's no configure script, so simply run "make" to build the tools, and
"make install" to install them. You also can use "make uninstall" to remove
all the files you installed. By default, files are installed in /usr/local
but you can change this behavior by editing the Makefile file and setting
prefix to wherever you want. You may change the C compiler and the
compilation flags as well.

Optionally, you can run "make strip" prior to "make install" if you want
smaller binaries. However, be aware that this will prevent any further
attempt to debug the programs.

If you wish to include sub-directories that are not enabled by default, then
just set them via the EXTRA make variable. For example, to build py-smbus,
do:
  $ make EXTRA="py-smbus"


DOCUMENTATION
-------------

The main tools have manual pages, which are installed by "make install".
See these manual pages for command line interface details and tool specific
information.

The other tools come with simple text documentation, which isn't installed.


QUESTIONS AND BUG REPORTS
-------------------------

Please post your questions and bug reports to the linux-i2c mailing list:
  linux-i2c@vger.kernel.org
For additional information about this list, see:
  http://vger.kernel.org/vger-lists.html#linux-i2c
