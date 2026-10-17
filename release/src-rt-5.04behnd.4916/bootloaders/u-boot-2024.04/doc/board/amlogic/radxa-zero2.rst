.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Radxa Zero2 (A311D)
==============================

Radxa Zero2 is a small form factor SBC based on the Amlogic A311D chipset with the
following specification:

- Amlogic A311D (Quad A73 + Dual A53) CPU
- 4GB LPDDR4 RAM
- 32/64/128GB eMMC
- Mali G52-MP4 GPU
- HDMI 2.1 output (micro)
- BCM4345 WiFi (2.4/5GHz a/b/g/n/ac) and BT 5.0
- 1x USB 2.0 port - Type C (OTG)
- 1x USB 3.0 port - Type C (Host)
- 1x micro SD Card slot
- 40 Pin GPIO header

Schematics are available on request from Radxa.

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make radxa-zero2_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh radxa-zero2 /path/to/u-boot/u-boot.bin my-output-dir

U-Boot Manual Signing
---------------------

Amlogic does not provide sources for the firmware and tools needed to create a bootloader
image so it is necessary to obtain binaries from sources published by the board vendor:

.. code-block:: bash

    $ git clone -b radxa-zero-v2021.07 https://github.com/radxa/u-boot.git
    $ git clone https://github.com/radxa/fip.git

    $ sudo apt-get install -y gcc-aarch64-linux-gnu device-tree-compiler libncurses5 libncurses5-dev
    $ sudo apt-get install -y bc python dosfstools flex build-essential libssl-dev mtools

    $ wget https://developer.arm.com/-/media/Files/downloads/gnu-a/10.3-2021.07/binrel/gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz
    $ sudo tar xvf gcc-arm-10.3-2021.07-x86_64-aarch64-none-elf.tar.xz -C /opt

    $ export CROSS_COMPILE=/opt/gcc-arm-10.2-2020.11-x86_64-aarch64-none-elf/bin/aarch64-none-elf-
    $ export ARCH=arm
    $ cd u-boot
    $ make radxa-zero2_defconfig
    $ make

    $ cp u-boot.bin ../fip/radxa-zero2/bl33.bin
    $ cd ../fip/radxa-zero2
    $ make

This will generate the signed U-Boot binaries:

.. code-block:: bash

    $ u-boot.bin u-boot.bin.sd.bin u-boot.bin.usb.bl2 u-boot.bin.usb.tpl

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440
