.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: addrmap (command)

addrmap command
===============

Synopsis
--------

::

    addrmap

Description
-----------

The addrmap command is used to display non-identity virtual-physical memory
mappings for 32-bit CPUs.

The output may look like:

::

    => addrmap
               vaddr            paddr             size
    ================ ================ ================
            e0000000        fe0000000         00100000
            00000000         00000000         04000000
            04000000         04000000         04000000
            80000000        c00000000         10000000
            90000000        c10000000         10000000
            a0000000        fe1000000         00010000

The first column indicates the virtual address.
The second column indicates the physical address.
The third column indicates the mapped size.

Configuration
-------------

To use the addrmap command you must specify CONFIG_CMD_ADDRMAP=y.
It is automatically turned on when CONFIG_ADDR_MAP is set.
