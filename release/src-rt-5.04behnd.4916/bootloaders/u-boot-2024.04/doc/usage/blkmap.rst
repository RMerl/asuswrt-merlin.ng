.. SPDX-License-Identifier: GPL-2.0+
..
.. Copyright (c) 2023 Addiva Elektronik
.. Author: Tobias Waldekranz <tobias@waldekranz.com>

Block Maps (blkmap)
===================

Block maps are a way of looking at various sources of data through the
lens of a regular block device. It lets you treat devices that are not
block devices, like RAM, as if they were. It also lets you export a
slice of an existing block device, which does not have to correspond
to a partition boundary, as a new block device.

This is primarily useful because U-Boot's filesystem drivers only
operate on block devices, so a block map lets you access filesystems
wherever they might be located.

The implementation is loosely modeled on Linux's "Device Mapper"
subsystem, see `kernel documentation`_ for more information.

.. _kernel documentation: https://www.kernel.org/doc/html/latest/admin-guide/device-mapper/index.html


Example: Netbooting an Ext4 Image
---------------------------------

Say that our system is using an Ext4 filesystem as its rootfs, where
the kernel is stored in ``/boot``. This image is then typically stored
in an eMMC partition. In this configuration, we can use something like
``load mmc 0 ${kernel_addr_r} /boot/Image`` to load the kernel image
into the expected location, and then boot the system. No problems.

Now imagine that during development, or as a recovery mechanism, we
want to boot the same type of image by downloading it over the
network. Getting the image to the target is easy enough:

::

   dhcp ${ramdisk_addr_r} rootfs.ext4

But now we are faced with a predicament: how to we extract the kernel
image? Block maps to the rescue!

We start by creating a new device:

::

   blkmap create netboot

Before setting up the mapping, we figure out the size of the
downloaded file, in blocks:

::

   setexpr fileblks ${filesize} + 0x1ff
   setexpr fileblks ${filesize} / 0x200

Then we can add a mapping to the start of our device, backed by the
memory at `${loadaddr}`:

::

   blkmap map netboot 0 ${fileblks} mem ${fileaddr}

Now we can access the filesystem via the virtual device:

::

   blkmap get netboot dev devnum
   load blkmap ${devnum} ${kernel_addr_r} /boot/Image


Example: Accessing a filesystem inside an FIT image
---------------------------------------------------

In this example, an FIT image is stored in an eMMC partition. We would
like to read the file ``/etc/version``, stored inside a Squashfs image
in the FIT. Since the Squashfs image is not stored on a partition
boundary, there is no way of accessing it via ``load mmc ...``.

What we can to instead is to first figure out the offset and size of
the filesystem:

::

   mmc dev 0
   mmc read ${loadaddr} 0 0x100

   fdt addr ${loadaddr}
   fdt get value squashaddr /images/ramdisk data-position
   fdt get value squashsize /images/ramdisk data-size

   setexpr squashblk  ${squashaddr} / 0x200
   setexpr squashsize ${squashsize} + 0x1ff
   setexpr squashsize ${squashsize} / 0x200

Then we can create a block map that maps to that slice of the full
partition:

::

   blkmap create sq
   blkmap map sq 0 ${squashsize} linear mmc 0 ${squashblk}

Now we can access the filesystem:

::

   blkmap get sq dev devnum
   load blkmap ${devnum} ${loadaddr} /etc/version
