.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: bootmeth (command)

bootmeth command
================

Synopsis
--------

::

    bootmeth list [-a]          - list selected bootmeths (-a for all)
    bootmeth order "[<bm> ...]" - select the order of bootmeths\n"


Description
-----------

The `bootmeth` command is used to manage bootmeths. It can list them and change
the order in which they are used.

See :doc:`../../develop/bootstd` for more information.


.. _bootmeth_order:

bootmeth order
~~~~~~~~~~~~~~

Selects which bootmeths to use and the order in which they are invoked. When
scanning bootdevs, each bootmeth is tried in turn to see if it can find a valid
bootflow. You can use this command to adjust the order or even to omit some
boomeths.

The argument is a quoted list of bootmeths to use, by name. If global bootmeths
are included, they must be at the end, otherwise the scanning mechanism will not
work correctly.


bootmeth list
~~~~~~~~~~~~~

This lists the selected bootmeths, or all of them, if the `-a` flag is used.
The format looks like this:

=====  ===  ==================  =================================
Order  Seq  Name                Description
=====  ===  ==================  =================================
    0    0  extlinunx           Extlinux boot from a block device
    1    1  efi                 EFI boot from an .efi file
    2    2  pxe                 PXE boot from a network device
    3    3  sandbox             Sandbox boot for testing
 glob    4  efi_mgr             EFI bootmgr flow
=====  ===  ==================  =================================

The fields are as follows:

Order:
    The order in which these bootmeths are invoked for each bootdev. If this
    shows as a hyphen, then the bootmeth is not in the current ordering. If it
    shows as 'glob', then this is a global bootmeth and should be at the end.

Seq:
    The sequence number of the bootmeth, i.e. the normal ordering if none is set

Name:
    Name of the bootmeth

Description:
    A friendly description for the bootmeth


Example
-------

This shows listing bootmeths. All are present and in the normal order::

    => bootmeth list
    Order  Seq  Name                Description
    -----  ---  ------------------  ------------------
        0    0  distro              Extlinux boot from a block device
        1    1  efi                 EFI boot from an .efi file
        2    2  pxe                 PXE boot from a network device
        3    3  sandbox             Sandbox boot for testing
        4    4  efi_mgr             EFI bootmgr flow
    -----  ---  ------------------  ------------------
    (5 bootmeths)

Now the order is changed, to include only two of them::

    => bootmeth order "sandbox distro"
    => bootmeth list
    Order  Seq  Name                Description
    -----  ---  ------------------  ------------------
        0    3  sandbox             Sandbox boot for testing
        1    0  distro              Extlinux boot from a block device
    -----  ---  ------------------  ------------------
    (2 bootmeths)

The -a flag shows all bootmeths so you can clearly see which ones are used and
which are not::

    => bootmeth list -a
    Order  Seq  Name                Description
    -----  ---  ------------------  ------------------
        1    0  distro              Extlinux boot from a block device
        -    1  efi                 EFI boot from an .efi file
        -    2  pxe                 PXE boot from a network device
        0    3  sandbox             Sandbox boot for testing
        -    4  efi_mgr             EFI bootmgr flow
    -----  ---  ------------------  ------------------
    (5 bootmeths)
