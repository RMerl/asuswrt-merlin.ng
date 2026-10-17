.. SPDX-License-Identifier: GPL-2.0+

U-Boot for ODROID-GO-ULTRA (S922X)
==================================

The ODROID GO ULTRA is a portable gaming device with the following characteristics:

 - Amlogic S922X SoC
 - RK817 & RK818 PMICs
 - 2GiB LPDDR4
 - On board 16GiB eMMC
 - Micro SD Card slot
 - 5inch 854×480 MIPI-DSI TFT LCD
 - Earphone stereo jack, 0.5Watt 8Ω Mono speaker
 - Li-Polymer 3.7V/4000mAh Battery
 - USB-A 2.0 Host Connector
 - x16 GPIO Input Buttons
 - 2x ADC Analog Joysticks
 - USB-C Port for USB2 Device and Charging

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make odroid-go-ultra_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh odroid-go-ultra /path/to/u-boot/u-boot.bin my-output-dir

Then write the image to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440
