.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Sam Protsenko <joe.skb7@gmail.com>

Android Boot Image
==================

Overview
--------

Android Boot Image is used to boot Android OS. It usually contains kernel image
(like ``zImage`` file) and ramdisk. Sometimes it can contain additional
binaries. This image is built as a part of AOSP (called ``boot.img``), and being
flashed into ``boot`` partition on eMMC. Bootloader then reads that image from
``boot`` partition to RAM and boots the kernel from it. Kernel then starts
``init`` process from the ramdisk. It should be mentioned that recovery image
(``recovery.img``) also has Android Boot Image format.

Android Boot Image format is described at [1]_. At the moment it can have one of
next image headers:

* v0: it's called *legacy* boot image header; used in devices launched before
  Android 9; contains kernel image, ramdisk and second stage bootloader
  (usually unused)
* v1: used in devices launched with Android 9; adds ``recovery_dtbo`` field,
  which should be used for non-A/B devices in ``recovery.img`` (see [2]_ for
  details)
* v2: used in devices launched with Android 10; adds ``dtb`` field, which
  references payload containing DTB blobs (either concatenated one after the
  other, or in Android DTBO image format)
* v3: used in devices launched with Android 11; adds ``vendor_boot`` partition
  and removes the second-stage bootloader and recovery image support. The new
  ``vendor_boot`` partition holds the device tree blob (DTB) and a vendor ramdisk.
  The generic ramdisk in ``boot`` partition is loaded immediately following
  the vendor ramdisk.
* v4: used in devices launched with Android 12; provides a boot signature in boot
  image header, supports multiple vendor ramdisk fragments in ``vendor_boot``
  partition. This version also adds a bootconfig section at the end of the vendor
  boot image, this section contains boot configuration parameters known at build time
  (see [9]_ for details).

v2, v1 and v0 formats are backward compatible.

The Android Boot Image format is represented by
:c:type:`struct andr_image_data <andr_image_data>` in U-Boot, and can be seen in
``include/android_image.h``. U-Boot supports booting Android Boot Image and also
has associated command

Booting
-------

U-Boot is able to boot the Android OS from Android Boot Image using ``bootm``
command. In order to use Android Boot Image format support, next option should
be enabled::

    CONFIG_ANDROID_BOOT_IMAGE=y

Then one can use next ``bootm`` command call to run Android:

.. code-block:: bash

    => bootm $loadaddr $loadaddr $fdtaddr

where ``$loadaddr`` - address in RAM where boot image was loaded; ``$fdtaddr`` -
address in RAM where DTB blob was loaded.

And parameters are, correspondingly:

  1. Where kernel image is located in RAM
  2. Where ramdisk is located in RAM (can be ``"-"`` if not applicable)
  3. Where DTB blob is located in RAM

``bootm`` command will figure out that image located in ``$loadaddr`` has
Android Boot Image format, will parse that and boot the kernel from it,
providing DTB blob to kernel (from 3rd parameter), passing info about ramdisk to
kernel via DTB.

DTB and DTBO blobs
------------------

``bootm`` command can't just use DTB blob from Android Boot Image (``dtb``
field), because:

* there is no DTB area in Android Boot Image before v2
* there may be several DTB blobs in DTB area (e.g. for different SoCs)
* some DTBO blobs may have to be merged in DTB blobs before booting
  (e.g. for different boards)

So user has to prepare DTB blob manually and provide it in a 3rd parameter
of ``bootm`` command. Next commands can be used to do so:

1. ``abootimg``: manipulates Anroid Boot Image, allows one to extract
   meta-information and payloads from it
2. ``adtimg``: manipulates Android DTB/DTBO image [3]_, allows one to extract
   DTB/DTBO blobs from it

In order to use those, please enable next config options::

    CONFIG_CMD_ABOOTIMG=y
    CONFIG_CMD_ADTIMG=y

For example, let's assume we have next Android partitions on eMMC:

* ``boot``: contains Android Boot Image v2 (including DTB blobs)
* ``dtbo``: contains DTBO blobs

Then next command sequence can be used to boot Android:

.. code-block:: bash

    => mmc dev 1

       # Read boot image to RAM (into $loadaddr)
    => part start mmc 1 boot boot_start
    => part size mmc 1 boot boot_size
    => mmc read $loadaddr $boot_start $boot_size

       # Read DTBO image to RAM (into $dtboaddr)
    => part start mmc 1 dtbo dtbo_start
    => part size mmc 1 dtbo dtbo_size
    => mmc read $dtboaddr $dtbo_start $dtbo_size

       # Copy required DTB blob (into $fdtaddr)
    => abootimg get dtb --index=0 dtb0_start dtb0_size
    => cp.b $dtb0_start $fdtaddr $dtb0_size

       # Merge required DTBO blobs into DTB blob
    => fdt addr $fdtaddr 0x100000
    => adtimg addr $dtboaddr
    => adtimg get dt --index=0 $dtbo0_addr
    => fdt apply $dtbo0_addr

       # Boot Android
    => bootm $loadaddr $loadaddr $fdtaddr

This sequence should be used for Android 10 boot. Of course, the whole Android
boot procedure includes much more actions, like:

* obtaining reboot reason from BCB (see [4]_)
* implementing recovery boot
* implementing fastboot boot
* implementing A/B slotting (see [5]_)
* implementing AVB2.0 (see [6]_)

But Android Boot Image booting is the most crucial part in Android boot scheme.

All Android bootloader requirements documentation is available at [7]_. Some
overview on the whole Android 10 boot process can be found at [8]_.

C API for working with Android Boot Image format
------------------------------------------------

.. kernel-doc:: boot/image-android.c
   :internal:

References
----------

.. [1] https://source.android.com/devices/bootloader/boot-image-header
.. [2] https://source.android.com/devices/bootloader/recovery-image
.. [3] https://source.android.com/devices/architecture/dto/partitions
.. [4] :doc:`bcb`
.. [5] :doc:`ab`
.. [6] :doc:`avb2`
.. [7] https://source.android.com/devices/bootloader
.. [8] https://connect.linaro.org/resources/san19/san19-217/
.. [9] https://source.android.com/docs/core/architecture/bootloader/implementing-bootconfig
