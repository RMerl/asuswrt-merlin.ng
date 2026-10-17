.. SPDX-License-Identifier: GPL-2.0+

Android Fastboot
================

Overview
--------

The protocol that is used over USB and UDP is described in [1]_.

The current implementation supports the following standard commands:

- ``boot``
- ``continue``
- ``download``
- ``erase`` (if enabled)
- ``flash`` (if enabled)
- ``getvar``
- ``reboot``
- ``reboot-bootloader``
- ``set_active`` (only a stub implementation which always succeeds)
- ``ucmd`` (if enabled)
- ``acmd`` (if enabled)

The following OEM commands are supported (if enabled):

- ``oem format`` - this executes ``gpt write mmc %x $partitions``
- ``oem partconf`` - this executes ``mmc partconf %x <arg> 0`` to configure eMMC
  with <arg> = boot_ack boot_partition
- ``oem bootbus``  - this executes ``mmc bootbus %x %s`` to configure eMMC
- ``oem run`` - this executes an arbitrary U-Boot command
- ``oem console`` - this dumps U-Boot console record buffer

Support for both eMMC and NAND devices is included.

Client installation
-------------------

The counterpart to this is the fastboot client which can be found in
Android's ``platform/system/core`` repository in the fastboot
folder. It runs on Windows, Linux and OSX. The fastboot client is
part of the Android SDK Platform-Tools and can be downloaded from [2]_.

Board specific
--------------

USB configuration
^^^^^^^^^^^^^^^^^

The fastboot gadget relies on the USB download gadget, so the following
options must be configured:

::

   CONFIG_USB_GADGET_DOWNLOAD
   CONFIG_USB_GADGET_VENDOR_NUM
   CONFIG_USB_GADGET_PRODUCT_NUM
   CONFIG_USB_GADGET_MANUFACTURER

NOTE: The ``CONFIG_USB_GADGET_VENDOR_NUM`` must be one of the numbers
supported by the fastboot client. The list of vendor IDs supported can
be found in the fastboot client source code.

General configuration
^^^^^^^^^^^^^^^^^^^^^

The fastboot protocol requires a large memory buffer for
downloads. This buffer should be as large as possible for a
platform. The location of the buffer and size are set with
``CONFIG_FASTBOOT_BUF_ADDR`` and ``CONFIG_FASTBOOT_BUF_SIZE``. These
may be overridden on the fastboot command line using ``-l`` and
``-s``.

Fastboot environment variables
------------------------------

Partition aliases
^^^^^^^^^^^^^^^^^

Fastboot partition aliases can also be defined for devices where GPT
limitations prevent user-friendly partition names such as ``boot``, ``system``
and ``cache``.  Or, where the actual partition name doesn't match a standard
partition name used commonly with fastboot.

The current implementation checks aliases when accessing partitions by
name (flash_write and erase functions).  To define a partition alias
add an environment variable similar to::

    fastboot_partition_alias_<alias partition name>=<actual partition name>

for example::

    fastboot_partition_alias_boot=LNX

Raw partition descriptors
^^^^^^^^^^^^^^^^^^^^^^^^^

In cases where no partition table is present, a raw partition descriptor can be
defined, specifying the offset, size, and optionally the MMC hardware partition
number for a given partition name.

This is useful when using fastboot to flash files (e.g. SPL or U-Boot) to a
specific offset in the eMMC boot partition, without having to update the entire
boot partition.

To define a raw partition descriptor, add an environment variable similar to::

    fastboot_raw_partition_<raw partition name>=<offset> <size> [mmcpart <num>]

for example::

    fastboot_raw_partition_boot=0x100 0x1f00 mmcpart 1

Variable overrides
^^^^^^^^^^^^^^^^^^

Variables retrived through ``getvar`` can be overridden by defining
environment variables of the form ``fastboot.<variable>``. These are
looked up first so can be used to override values which would
otherwise be returned. Using this mechanism you can also return types
for NAND filesystems, as the fully parameterised variable is looked
up, e.g.::

    fastboot.partition-type:boot=jffs2

Boot command
^^^^^^^^^^^^

When executing the fastboot ``boot`` command, if ``fastboot_bootcmd`` is set
then that will be executed in place of ``bootm <CONFIG_FASTBOOT_BUF_ADDR>``.

Partition Names
---------------

The Fastboot implementation in U-Boot allows to write images into disk
partitions. Target partitions are referred on the host computer by
their names.

For GPT/EFI the respective partition name is used.

For MBR the partitions are referred by generic names according to the
following schema::

    <device type><device index letter><partition index>

Example: ``hda3``, ``sdb1``, ``usbda1``.

The device type is as follows:

  * IDE, ATAPI and SATA disks: ``hd``
  * SCSI disks: ``sd``
  * USB media: ``usbd``
  * MMC and SD cards: ``mmcsd``
  * Disk on chip: ``docd``
  * other: ``xx``

The device index starts from ``a`` and refers to the interface (e.g. USB
controller, SD/MMC controller) or disk index. The partition index starts
from ``1`` and describes the partition number on the particular device.

Alternatively, partition types may be specified using :ref:`U-Boot's partition
syntax <partitions>`. This allows specifying partitions like ``0.1``,
``0#boot``, or ``:3``. The interface is always ``mmc``.

Writing Partition Table
-----------------------

Fastboot also allows to write the partition table to the media. This can be
done by writing the respective partition table image to a special target
"gpt" or "mbr". These names can be customized by defining the following
configuration options:

::

   CONFIG_FASTBOOT_GPT_NAME
   CONFIG_FASTBOOT_MBR_NAME

In Action
---------

Enter into fastboot by executing the fastboot command in U-Boot for either USB::

   => fastboot usb 0

or UDP::

   => fastboot udp
   link up on port 0, speed 100, full duplex
   Using ethernet@4a100000 device
   Listening for fastboot command on 192.168.0.102

On the client side you can fetch the bootloader version for instance::

   $ fastboot getvar version-bootloader
   version-bootloader: U-Boot 2019.07-rc4-00240-g00c9f2a2ec
   Finished. Total time: 0.005s

or initiate a reboot::

   $ fastboot reboot

and once the client comes back, the board should reset.

You can also specify a kernel image to boot. You have to either specify
the an image in Android format *or* pass a binary kernel and let the
fastboot client wrap the Android suite around it. On OMAP for instance you
take zImage kernel and pass it to the fastboot client::

   $ fastboot -b 0x80000000 -c "console=ttyO2 earlyprintk root=/dev/ram0 mem=128M" boot zImage
   creating boot image...
   creating boot image - 1847296 bytes
   downloading 'boot.img'...
   OKAY [  2.766s]
   booting...
   OKAY [ -0.000s]
   finished. total time: 2.766s

and on the U-Boot side you should see::

   Starting download of 1847296 bytes
   ........................................................
   downloading of 1847296 bytes finished
   Booting kernel..
   ## Booting Android Image at 0x81000000 ...
   Kernel load addr 0x80008000 size 1801 KiB
   Kernel command line: console=ttyO2 earlyprintk root=/dev/ram0 mem=128M
      Loading Kernel Image ... OK
   OK

   Starting kernel ...

Running Shell Commands
^^^^^^^^^^^^^^^^^^^^^^

Normally, arbitrary U-Boot command execution is not enabled. This is so
fastboot can be used to update systems using verified boot. However, such
functionality can be useful for production or when verified boot is not in use.
Enable ``CONFIG_FASTBOOT_OEM_RUN`` to use this functionality. This will enable
``oem run`` command, which can be used with the fastboot client. For example,
to print "Hello at 115200 baud" (or whatever ``CONFIG_BAUDRATE`` is), run::

    $ fastboot oem run:'echo Hello at $baudrate baud'

You can run any command you would normally run on the U-Boot command line,
including multiple commands (using e.g. ``;`` or ``&&``) and control structures
(``if``, ``while``, etc.). The exit code of ``fastboot`` will reflect the exit
code of the command you ran.

References
----------

.. [1] :doc:`fastboot-protocol`
.. [2] https://developer.android.com/studio/releases/platform-tools
