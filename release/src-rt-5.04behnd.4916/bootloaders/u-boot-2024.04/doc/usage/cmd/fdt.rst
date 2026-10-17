.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: fdt (command)

fdt command
===========

Synopsis
--------

::

    fdt addr [-cq] [addr [len]]

Description
-----------

The fdt command provides access to flat device tree blobs in memory. It has
many subcommands, some of which are not documented here.

Flags:

-c
    Select the control FDT (otherwise the working FDT is used).
-q
    Don't display errors

The control FDT is the one used by U-Boot itself to control various features,
including driver model. This should only be changed if you really know what you
are doing, since once U-Boot starts it maintains pointers into the FDT from the
various driver model data structures.

The working FDT is the one passed to the Operating System when booting. This
can be freely modified, so far as U-Boot is concerned, since it does not affect
U-Boot's operation.

fdt addr
~~~~~~~~

With no arguments, this shows the address of the current working or control
FDT.

If the `addr` argument is provided, then this sets the address of the working or
control FDT to the provided address.

If the `len` argument is provided, then the device tree is expanded to that
size. This can be used to make space for more nodes and properties. It is
assumed that there is enough space in memory for this expansion.

Example
-------

Get the control address and copy that FDT to free memory::

    => fdt addr -c
    Control fdt: 0aff9fd0
    => cp.b 0aff9fd0 10000 10000
    => md 10000 4
    00010000: edfe0dd0 5b3d0000 78000000 7c270000  ......=[...x..'|

The second word shows the size of the FDT. Now set the working FDT to that
address and expand it to 0xf000 in size::

    => fdt addr 10000 f000
    Working FDT set to 10000
    => md 10000 4
    00010000: edfe0dd0 00f00000 78000000 7c270000  ...........x..'|

Return value
------------

The return value $? indicates whether the command succeeded.
