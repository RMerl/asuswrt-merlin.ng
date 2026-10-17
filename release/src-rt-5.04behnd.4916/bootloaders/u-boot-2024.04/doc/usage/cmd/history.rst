.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: history (command)

history command
===============

Synopsis
--------

::

    history

Description
-----------

The *history* command shows a list of previously entered commands on the
command line. When U-Boot starts, this it is initially empty. Each new command
entered is added to the list.

Normally these commands can be accessed by pressing the `up arrow` and
`down arrow` keys, which cycle through the list. The `history` command provides
a simple way to view the list.

Example
-------

This example shows entering three commands, then `history`. Note that `history`
itself is added to the list.

::

    => bootflow scan -l
    Scanning for bootflows in all bootdevs
    Seq  Method       State   Uclass    Part  Name                      Filename
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    Scanning global bootmeth 'firmware0':
    Hunting with: simple_bus
    Found 2 extension board(s).
    Scanning bootdev 'mmc2.bootdev':
    Scanning bootdev 'mmc1.bootdev':
      0  extlinux     ready   mmc          1  mmc1.bootdev.part_1       /extlinux/extlinux.conf
    No more bootdevs
    ---  -----------  ------  --------  ----  ------------------------  ----------------
    (1 bootflow, 1 valid)
    => bootflow select 0
    => bootflow info
    Name:      mmc1.bootdev.part_1
    Device:    mmc1.bootdev
    Block dev: mmc1.blk
    Method:    extlinux
    State:     ready
    Partition: 1
    Subdir:    (none)
    Filename:  /extlinux/extlinux.conf
    Buffer:    aebdea0
    Size:      253 (595 bytes)
    OS:        Fedora-Workstation-armhfp-31-1.9 (5.3.7-301.fc31.armv7hl)
    Cmdline:   (none)
    Logo:      (none)
    FDT:       <NULL>
    Error:     0
    => history
    bootflow scan -l
    bootflow select 0
    bootflow info
    history
    =>
