.. SPDX-License-Identifier: GPL-2.0+

mx6sabreauto
============

How to use and build U-Boot on mx6sabreauto
-------------------------------------------

mx6sabreauto_defconfig target supports mx6q/mx6dl/mx6qp sabreauto variants.

In order to build it:

.. code-block:: bash

   $ make mx6sabreauto_defconfig
   $ make

This will generate the SPL and u-boot-dtb.img binaries.

- Flash the SPL binary into the SD card:

.. code-block:: bash

   $ sudo dd if=SPL of=/dev/sdX bs=1K seek=1 conv=notrunc && sync

- Flash the u-boot-dtb.img binary into the SD card:

.. code-block:: bash

   $ sudo dd if=u-boot-dtb.img of=/dev/sdX bs=1K seek=69 conv=notrunc && sync

Booting via Falcon mode
-----------------------

Write in mx6sabreauto_defconfig the following define below:

CONFIG_SPL_OS_BOOT=y

In order to build it:

.. code-block:: bash

   $ make mx6sabreauto_defconfig
   $ make

This will generate the SPL image called SPL and the u-boot-dtb.img.

- Flash the SPL image into the SD card:

.. code-block:: bash

   $ sudo dd if=SPL of=/dev/sdb bs=1K seek=1 conv=notrunc && sync

- Flash the u-boot-dtb.img image into the SD card:

.. code-block:: bash

   $ sudo dd if=u-boot-dtb.img of=/dev/sdb bs=1K seek=69 conv=notrunc && sync

Create a FAT16 boot partition to store uImage and the dtb file, then copy the files there:

.. code-block:: bash

   $ sudo cp uImage /media/boot
   $ sudo cp imx6dl-sabreauto.dtb /media/boot

Create a partition for root file system and extract it there:

.. code-block:: bash

   $ sudo tar xvf rootfs.tar.gz -C /media/root

The SD card must have enough space for raw "args" and "kernel".
To configure Falcon mode for the first time, on U-Boot do the following commands:

- Load dtb file from boot partition::

   # load mmc 0:1 ${fdt_addr} imx6dl-sabreauto.dtb

- Load kernel image from boot partition::

   # load mmc 0:1 ${loadaddr} uImage

- Write kernel at 2MB offset::

   # mmc write ${loadaddr} 0x1000 0x4000

- Setup kernel bootargs::

   # setenv bootargs "console=ttymxc3,115200 root=/dev/mmcblk0p1 rootfstype=ext4 rootwait quiet rw"

- Prepare args::

   # spl export fdt ${loadaddr} - ${fdt_addr}

- Write args 1MB data (0x800 sectors) to 1MB offset (0x800 sectors)::

   # mmc write 18000000 0x800 0x800

- Restart the board and then SPL binary will launch the kernel directly.
