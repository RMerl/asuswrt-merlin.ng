.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: pstore (command)

pstore command
==============

Synopsis
--------

::

    pstore set <addr> <len> [record-size] [console-size] [ftrace-size] [pmsg_size] [ecc-size]
    pstore display [record-type] [nb]
    pstore save <interface> <dev[:part]> <directory-path>

Design
------

Linux PStore and Ramoops modules (Linux config options PSTORE and PSTORE_RAM)
allow to use memory to pass data from the dying breath of a crashing kernel to
its successor. This command allows to read those records from U-Boot command
line.

Ramoops is an oops/panic logger that writes its logs to RAM before the system
crashes. It works by logging oopses and panics in a circular buffer. Ramoops
needs a system with persistent RAM so that the content of that area can survive
after a restart.

Ramoops uses a predefined memory area to store the dump.

Ramoops parameters can be passed as kernel parameters or through Device Tree,
i.e.::

    ramoops.mem_address=0x30000000 ramoops.mem_size=0x100000 ramoops.record_size=0x2000 ramoops.console_size=0x2000 memmap=0x100000$0x30000000

The same values should be set in U-Boot to be able to retrieve the records.
This values can be set at build time in U-Boot configuration file, or at runtime.
U-Boot automatically patches the Device Tree to pass the Ramoops parameters to
the kernel.

The PStore configuration parameters are:

======================= ==========
 Name                   Default
======================= ==========
CMD_PSTORE_MEM_ADDR
CMD_PSTORE_MEM_SIZE     0x10000
CMD_PSTORE_RECORD_SIZE  0x1000
CMD_PSTORE_CONSOLE_SIZE 0x1000
CMD_PSTORE_FTRACE_SIZE  0x1000
CMD_PSTORE_PMSG_SIZE    0x1000
CMD_PSTORE_ECC_SIZE     0
======================= ==========

Records sizes should be a power of 2.
The memory size and the record/console size must be non-zero.

Multiple 'dump' records can be stored in the memory reserved for PStore.
The memory size has to be larger than the sum of the record sizes, i.e.::

    MEM_SIZE >= RECORD_SIZE * n + CONSOLE_SIZE + FTRACE_SIZE + PMSG_SIZE

Usage
-----

Generate kernel crash
~~~~~~~~~~~~~~~~~~~~~

For test purpose, you can generate a kernel crash by setting reboot timeout to
10 seconds and trigger a panic

.. code-block:: console

    $ sudo sh -c "echo 1 > /proc/sys/kernel/sysrq"
    $ sudo sh -c "echo 10 > /proc/sys/kernel/panic"
    $ sudo sh -c "echo c > /proc/sysrq-trigger"

Retrieve logs in U-Boot
~~~~~~~~~~~~~~~~~~~~~~~

First of all, unless PStore parameters as been set during U-Boot configuration
and match kernel ramoops parameters, it needs to be set using 'pstore set', e.g.::

    => pstore set 0x30000000 0x100000 0x2000 0x2000

Then all available dumps can be displayed
using::

    => pstore display

Or saved to an existing directory in an Ext2 or Ext4 partition, e.g. on root
directory of 1st partition of the 2nd MMC::

    => pstore save mmc 1:1 /
