.. SPDX-License-Identifier: GPL-2.0-or-later

imxrt1170-evk
=============

How to use U-Boot on NXP i.MXRT1170 EVK
---------------------------------------

- Build U-Boot for i.MXRT1170 EVK:

  .. code-block:: bash

      $ make mrproper
      $ make imxrt1170-evk_defconfig
      $ make

  This will generate the SPL image called SPL and the u-boot.img.

- Flash the SPL image into the micro SD card:

  .. code-block:: bash

      $sudo dd if=SPL of=/dev/sdX bs=1k seek=1 conv=notrunc; sync

  This location is not compatible with GPT partioning. Please, use MBR
  partitioning instead.

- Flash the u-boot.img image into the micro SD card:

  .. code-block:: bash

      $sudo dd if=u-boot.img of=/dev/sdX bs=1k seek=128 conv=notrunc; sync

- Jumper settings

  .. list-table::
     :stub-columns: 1

     * - SW1
       - 1 0 1 0
     * - SW2
       - 0 0 0 0 | 0 0 0 0 | 1 0 0 0

  where 0 means bottom position and 1 means top position (from the
  switch label numbers reference).

- Connect the USB cable between the EVK and the PC for the console.
  The USB console connector is the one close the ethernet connector

- Insert the micro SD card in the board, power it up and U-Boot messages should come up.
