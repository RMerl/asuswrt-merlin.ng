.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2022, Heinrich Schuchardt <xypron.glpk@gmx.de>

.. index::
   single: mtest (command)

mtest command
=============

Synopsis
--------

::

    mtest [start [end [pattern [iterations]]]]

Description
-----------

The *mtest* command tests the random access memory. It writes long values, reads
them back and checks for differences. The test can be interrupted with CTRL+C.

The default test uses *pattern* as first value to be written and varies it
between memory addresses.

An alternative test can be selected with CONFIG_SYS_ALT_MEMTEST=y. It uses
multiple hard coded bit patterns.

With CONFIGSYS_ALT_MEMTEST_BITFLIP=y a further test is executed. It writes long
values offset by half the size of long and checks if writing to the one address
causes bit flips at the other address.

start
	start address of the memory range tested, defaults to
	CONFIG_SYS_MEMTEST_START

end
	end address of the memory range tested, defaults to
	CONFIG_SYS_MEMTEST_END. If CONFIGSYS_ALT_MEMTEST_BITFLIP=y, a value will
	be written to this address. Otherwise it is excluded from the range.

pattern
	pattern to be written to memory. This is a 64bit value on 64bit systems
	and a 32bit value on 32bit systems. It defaults to 0. The value is
	ignored if CONFIG_SYS_ALT_MEMTEST=y.

iterations
	number of test repetitions. If the value is not provided the test will
	not terminate automatically. Enter CTRL+C instead.

Examples
--------

::

    => mtest 1000 2000 0x55aa55aa55aa55aa 10
    Testing 00001000 ... 00002000:
    Pattern AA55AA55AA55AA55  Writing...  Reading...
    Tested 16 iteration(s) with 0 errors.

Configuration
-------------

The mtest command is enabled by CONFIG_CMD_MEMTEST=y.

Return value
------------

The return value $? is 0 (true) if the command succeeds, 1 (false) otherwise.
