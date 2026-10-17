.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: host (command)

host command
============

Synopsis
--------

::

    host bind [-r] <label> [<filename>]
    host unbind <label|seq>
    host info [<label|seq>]
    host dev [<label|seq>]

Description
-----------

The host command provides a way to attach disk images on the host to U-Boot
sandbox. This can be useful for testing U-Boot's filesystem implementations.

Common arguments:

<label|seq>
    This is used to specify a host device. It can either be a label (a string)
    or the sequence number of the device. An invalid value causes the command
    to fail.


host bind
~~~~~~~~~

This creates a new host device and binds a file to it.

Arguments:

label
    Label to use to identify this binding. This can be any string.

filename:
    Host filename to bind to

Flags:

-r
    Mark the device as removable


host unbind
~~~~~~~~~~~

This unbinds a host device that was previously bound. The sequence numbers of
other devices remain unchanged.


host info
~~~~~~~~~

Provides information about a particular host binding, or all of them.


host dev
~~~~~~~~

Allowing selecting a particular device, or (with no arguments) seeing which one
is selected.


Example
-------

Initially there are no devices::

    => host info
    dev       blocks label           path

Bind a device::

    => host bind -r test2 2MB.ext2.img
    => host bind fat 1MB.fat32.img
    => host info
    dev       blocks label           path
      0         4096 test2           2MB.ext2.img
      1         2048 fat             1MB.fat32.img

Select a device by label or sequence number::

    => host dev fat
    Current host device: 1: fat
    => host dev 0
    Current host device: 0: test2

Write a file::

    => ext4write host 0 0 /dump 1e00
    File System is consistent
    7680 bytes written in 3 ms (2.4 MiB/s)
    => ext4ls host 0
    <DIR>       4096 .
    <DIR>       4096 ..
    <DIR>      16384 lost+found
                4096 testing
                7680 dump

Unbind a device::

    => host unbind test2
    => host info
    dev       blocks label           path
      1         2048 fat             1MB.fat32.img


Return value
------------

The return value $? indicates whether the command succeeded.
