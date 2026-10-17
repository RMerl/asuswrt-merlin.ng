.. SPDX-License-Identifier: GPL-2.0+

imxrt1050-evk
=============

How to use U-Boot on NXP i.MXRT1050 EVK
---------------------------------------

- Build U-Boot for i.MXRT1050 EVK:

.. code-block:: bash

   $ make mrproper
   $ make imxrt1050-evk_defconfig
   $ make

This will generate the SPL image called SPL and the u-boot.img.

- Flash the SPL image into the micro SD card:

.. code-block:: bash

   $sudo dd if=SPL of=/dev/sdX bs=1k seek=1 conv=notrunc; sync

- Flash the u-boot.img image into the micro SD card:

.. code-block:: bash

   $sudo dd if=u-boot.img of=/dev/sdX bs=1k seek=128 conv=notrunc; sync

- Jumper settings::

   SW7: 1 0 1 0

where 0 means bottom position and 1 means top position (from the
switch label numbers reference).

- Connect the USB cable between the EVK and the PC for the console.
  The USB console connector is the one close the ethernet connector

- Insert the micro SD card in the board, power it up and U-Boot messages should come up.


How to use U-Boot with SPI flash on NXP i.MXRT1050 EVK
------------------------------------------------------

- Build U-Boot for i.MXRT1050 EVK:

.. code-block:: bash

   $ make mrproper
   $ make imxrt1050-evk_fspi_defconfig
   $ make

This will generate SPL, uboot.img, fspi_header.bin, and the final image (flash.bin).

To boot from SPI flash on other boards, you may need to change the flash header config,
which is specific to your flash chip, in Kconfig.
The flash config is 4K in size and is documented on page 217 of the imxrt1050 RM.
The default flash chip on the i.MXRT1050 EVK is the S26KS512SDPBHI02 HYPERFLASH.

- Jumper settings::

   SW7: 0 1 1 0

where 0 means bottom position and 1 means top position (from the
switch label numbers reference).

- Connect the USB cable between the EVK and the PC for the console.

- Use either JTAG or SWD to write `flash.bin` to the NOR. I used Mcuexpresso IDE's GUI flash tool.
