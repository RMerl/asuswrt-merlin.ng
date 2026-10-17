.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: loadm (command)

loadm command
=============

Synopsis
--------

::

    loadm <src_addr> <dst_addr> <len>

Description
-----------

The loadm command is used to copy memory content from source address
to destination address and, if efi is enabled, will setup a "Mem" efi
boot device.

The number of transferred bytes must be set by bytes parameter

src_addr
    start address of the memory location to be loaded

dst_addr
    destination address of the byte stream to be loaded

len
    number of bytes to be copied in hexadecimal. Can not be 0 (zero).

Example
-------

::

    => loadm ${kernel_addr} ${kernel_addr_r} ${kernel_size}
    loaded bin to memory: size: 12582912

Configuration
-------------

The command is only available if CONFIG_CMD_LOADM=y.

Return value
------------

The return value $? is set 0 (true) if the loading is succefull, and
is set to 1 (false) in case of error.

