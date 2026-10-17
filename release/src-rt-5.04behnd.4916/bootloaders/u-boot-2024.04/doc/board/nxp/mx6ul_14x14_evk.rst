.. SPDX-License-Identifier: GPL-2.0+

mx6ul_14x14_evk
===============

How to use U-Boot on Freescale MX6UL 14x14 EVK
----------------------------------------------

- Build U-Boot for MX6UL 14x14 EVK:

.. code-block:: bash

   $ make mrproper
   $ make mx6ul_14x14_evk_defconfig
   $ make

This will generate the SPL image called SPL and the u-boot.img.

1. Booting via SDCard
---------------------

- Flash the SPL image into the micro SD card:

.. code-block:: bash

   sudo dd if=SPL of=/dev/mmcblk0 bs=1k seek=1 conv=notrunc; sync

- Flash the u-boot.img image into the micro SD card:

.. code-block:: bash

   sudo dd if=u-boot.img of=/dev/mmcblk0 bs=1k seek=69 conv=notrunc; sync

- Jumper settings::

   SW601: 0 0 1 0
   Sw602: 1 0

where 0 means bottom position and 1 means top position (from the
switch label numbers reference).

- Connect the USB cable between the EVK and the PC for the console.
  The USB console connector is the one close the push buttons

- Insert the micro SD card in the board, power it up and U-Boot messages should come up.

2. Booting via Serial Download Protocol (SDP)
---------------------------------------------

The mx6ulevk board can boot from USB OTG port using the SDP, target will
enter in SDP mode in case an SD Card is not connect or boot switches are
set as below::

   Sw602: 0 1
   SW601: x x x x

The following tools can be used to boot via SDP, for both tools you must
connect an USB cable in USB OTG port.

- Method 1: Universal Update Utility (uuu)

The UUU binary can be downloaded in release tab from link below:
https://github.com/NXPmicro/mfgtools

The following script should be created to boot SPL + u-boot-dtb.img binaries:

.. code-block:: bash

   $ cat uuu_script
     uuu_version 1.1.4

     SDP: boot -f SPL
     SDPU: write -f u-boot-dtb.img -addr 0x877fffc0
     SDPU: jump -addr 0x877fffc0
     SDPU: done

Please note that the address above is calculated based on TEXT_BASE address:

0x877fffc0 = 0x87800000 (TEXT_BASE) - 0x40 (U-Boot proper Header size)

Power on the target and run the following command from U-Boot root directory:

.. code-block:: bash

   $ sudo ./uuu uuu_script

- Method 2: imx usb loader tool (imx_usb):

The imx_usb_loader tool can be downloaded in link below:
https://github.com/boundarydevices/imx_usb_loader

Build the source code and run the following commands from U-Boot root
directory:

.. code-block:: bash

   $ sudo ./imx_usb SPL
   $ sudo ./imx_usb u-boot-dtb.img
