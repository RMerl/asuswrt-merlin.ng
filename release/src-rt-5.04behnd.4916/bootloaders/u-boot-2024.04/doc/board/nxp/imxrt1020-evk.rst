.. SPDX-License-Identifier: GPL-2.0+

imxrt1020-evk
=============

How to use U-Boot on NXP i.MXRT1020 EVK
---------------------------------------

- Build U-Boot for i.MXRT1020 EVK:

.. code-block:: bash

   $ make mrproper
   $ make imxrt1020-evk_defconfig
   $ make

This will generate the SPL image called SPL and the u-boot.img.

- Flash the SPL image into the micro SD card:

.. code-block:: bash

   $sudo dd if=SPL of=/dev/sdX bs=1k seek=1 conv=notrunc; sync

- Flash the u-boot.img image into the micro SD card:

.. code-block:: bash

   $sudo dd if=u-boot.img of=/dev/sdX bs=1k seek=128 conv=notrunc; sync

- Jumper settings::

   SW8: 0 1 1 0

where 0 means bottom position and 1 means top position (from the
switch label numbers reference).

- Connect the USB cable between the EVK and the PC for the console.
  The USB console connector is the one close the ethernet connector

- Insert the micro SD card in the board, power it up and U-Boot messages should come up.
