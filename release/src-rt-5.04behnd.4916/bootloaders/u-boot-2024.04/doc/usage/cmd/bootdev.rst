.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: bootdev (command)

bootdev command
===============

Synopsis
--------

::

    bootdev list [-p]        - list all available bootdevs (-p to probe)
    bootdev hunt [-l|<spec>] - use hunt drivers to find bootdevs
    bootdev select <bm>      - select a bootdev by name
    bootdev info [-p]        - show information about a bootdev

Description
-----------

The `bootdev` command is used to manage bootdevs. It can list available
bootdevs, select one and obtain information about it.

See :doc:`../../develop/bootstd` for more information about bootdevs in general.


bootdev list
~~~~~~~~~~~~

This lists available bootdevs

Scanning with `-p` causes the bootdevs to be probed. This happens automatically
when they are used.

The list looks something like this:

===  ======  ======  ========  =========================
Seq  Probed  Status  Uclass    Name
===  ======  ======  ========  =========================
  0   [ + ]      OK  mmc       mmc@7e202000.bootdev
  1   [   ]      OK  mmc       sdhci@7e300000.bootdev
  2   [   ]      OK  ethernet  smsc95xx_eth.bootdev
===  ======  ======  ========  =========================


The fields are as follows:

Seq:
    Sequence number in the scan, used to reference the bootflow later

Probed:
    Shows a plus (+) if the device is probed, empty if not.

Status:
    Shows the status of the device. Typically this is `OK` meaning that there is
    no error. If you use -p and an error occurs when probing, then this shows
    the error number. You can look up Linux error codes to find the meaning of
    the number.

Uclass:
    Name of the media device's Uclass. This indicates the type of the parent
    device (e.g. MMC, Ethernet).

Name:
    Name of the bootdev. This is generated from the media device appended
    with `.bootdev`


bootdev hunt
~~~~~~~~~~~~

This hunts for new bootdevs, or shows a list of hunters.

Use `-l` to list the available bootdev hunters.

To run hunters, specify the name of the hunter to run, e.g. "mmc". If no
name is provided, all hunters are run.


bootdev select
~~~~~~~~~~~~~~

Use this to select a particular bootdev. You can select it by the sequence
number or name, as shown in `bootdev list`.

Once a bootdev is selected, you can use `bootdev info` to look at it or
`bootflow scan` to scan it.

If no bootdev name or number is provided, then any existing bootdev is
unselected.


bootdev info
~~~~~~~~~~~~

This shows information on the current bootdev, with the format looking like
this:

=========  =======================
Name       `mmc@7e202000.bootdev`
Sequence   0
Status     Probed
Uclass     mmc
Bootflows  1 (1 valid)
=========  =======================

Most of the information is the same as `bootdev list` above. The new fields
are:

Device
    Name of the bootdev

Status
    Shows `Probed` if the device is probed, `OK` if not. If `-p` is used and the
    device fails to probe, an error code is shown.

Bootflows
    Indicates the number of bootflows attached to the bootdev. This is 0
    unless you have used 'bootflow scan' on the bootflow, or on all bootflows.


Example
-------

This example shows listing available bootdev and getting information about
one of them::

   U-Boot> bootdev list
   Seq  Probed  Status  Uclass    Name
   ---  ------  ------  --------  ------------------
     0   [ + ]      OK  mmc       mmc@7e202000.bootdev
     1   [   ]      OK  mmc       sdhci@7e300000.bootdev
     2   [   ]      OK  ethernet  smsc95xx_eth.bootdev
   ---  ------  ------  --------  ------------------
   (3 devices)
   U-Boot> bootdev sel 0
   U-Boot> bootflow scan
   U-Boot> bootdev info
   Name:      mmc@7e202000.bootdev
   Sequence:  0
   Status:    Probed
   Uclass:    mmc
   Bootflows: 1 (1 valid)

This shows using one of the available hunters, then listing them::

    => bootdev hunt usb
    Hunting with: usb
    Bus usb@1: scanning bus usb@1 for devices...
    3 USB Device(s) found
    => bootdev hunt -l
    Prio  Used  Uclass           Hunter
    ----  ----  ---------------  ---------------
    6        ethernet         eth_bootdev
    1        simple_bus       (none)
    5        ide              ide_bootdev
    2        mmc              mmc_bootdev
    4        nvme             nvme_bootdev
    4        scsi             scsi_bootdev
    4        spi_flash        sf_bootdev
    5     *  usb              usb_bootdev
    4        virtio           virtio_bootdev
    (total hunters: 9)
    => usb stor
    Device 0: Vendor: sandbox Rev: 1.0 Prod: flash
                Type: Hard Disk
                Capacity: 4.0 MB = 0.0 GB (8192 x 512)
    Device 1: Vendor: sandbox Rev: 1.0 Prod: flash
                Type: Hard Disk
                Capacity: 0.0 MB = 0.0 GB (1 x 512)
    =>


Return value
------------

The return value $? is always 0 (true).
