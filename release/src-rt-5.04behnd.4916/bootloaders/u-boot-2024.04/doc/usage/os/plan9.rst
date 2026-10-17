.. SPDX-License-Identifier: GPL-2.0+
.. Steven Stallion
.. June 2013

Plan 9
======

Plan 9 from Bell Labs kernel images require additional setup to pass
configuration information to the kernel.  An environment variable named
confaddr must be defined with the same value as CONFADDR (see mem.h).
Use of this facility is optional, but should be preferable to manual
configuration.

When booting an image, arguments supplied to the bootm command will be
copied to CONFADDR.  If no arguments are specified, the contents of the
bootargs environment variable will be copied.

If no command line arguments or bootargs are defined, CONFADDR is left
uninitialized to permit manual configuration.  For example, PC-style
configuration could be simulated by issuing a fatload in bootcmd::

  # setenv bootcmd fatload mmc 0 $confaddr plan9.ini; ...; bootm
