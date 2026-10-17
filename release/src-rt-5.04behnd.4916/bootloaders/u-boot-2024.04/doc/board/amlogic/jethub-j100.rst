.. SPDX-License-Identifier: GPL-2.0+

U-Boot for JetHub J100 (A113X)
==============================

JetHome Jethub D1 (http://jethome.ru/jethub-d1) is a home automation controller device
manufactured by JetHome with the following specifications:

 - Amlogic A113X (ARM Cortex-A53) quad-core up to 1.5GHz
 - no video out
 - 512MB/1GB DDR3 or 2GB DDR4 SDRAM
 - 8/16/32GB eMMC flash
 - 1 x USB 2.0
 - 1 x 10/100Mbps ethernet
 - WiFi / Bluetooth one from:
   - AMPAK AP6255 (Broadcom BCM43455) IEEE 802.11a/b/g/n/ac, Bluetooth 4.2
   - RTL8822CS IEEE 802.11a/b/g/n/ac, Bluetooth 5.0
   - Amlogic W155S1 WiFi5 IEEE 802.11a/b/g/n/ac, Bluetooth 5.2
 - 2 x gpio LEDS
 - GPIO user Button
 - DC source with a voltage of 9 to 56 V / Passive POE
 - DIN Rail Mounting case

The basic version also has:

 - TI CC2538 + CC2592 Zigbee Wireless with upto 20dBm output power and Zigbee 3.0
 - 1 x 1-Wire
 - 2 x RS-485
 - 4 x dry contact digital GPIO inputs
 - 3 x relay GPIO outputs

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make jethub_j100_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh jethub-j100 /path/to/u-boot/u-boot.bin my-output-dir

U-Boot Manual Signing
---------------------

Amlogic does not provide sources for the firmware and tools needed to create a bootloader
image so it is necessary to obtain binaries from sources published by the board vendor:

.. code-block:: bash

    $ git clone https://github.com/jethome-ru/jethub-aml-tools jethub-u-boot
    $ cd jethub-u-boot
    $ export FIPDIR=$PWD

Go back to the mainline U-Boot source tree then:

.. code-block:: bash

    $ mkdir fip

    $ cp $FIPDIR/j100/bl2.bin fip/
    $ cp $FIPDIR/j100/acs.bin fip/
    $ cp $FIPDIR/j100/bl21.bin fip/
    $ cp $FIPDIR/j100/bl30.bin fip/
    $ cp $FIPDIR/j100/bl301.bin fip/
    $ cp $FIPDIR/j100/bl31.img fip/
    $ cp u-boot.bin fip/bl33.bin

    $ $FIPDIR/blx_fix.sh \
        fip/bl30.bin \
        fip/zero_tmp \
        fip/bl30_zero.bin \
        fip/bl301.bin \
        fip/bl301_zero.bin \
        fip/bl30_new.bin \
        bl30

    $ $FIPDIR/acs_tool.pyc fip/bl2.bin fip/bl2_acs.bin fip/acs.bin 0

    $ $FIPDIR/blx_fix.sh \
        fip/bl2_acs.bin \
        fip/zero_tmp \
        fip/bl2_zero.bin \
        fip/bl21.bin \
        fip/bl21_zero.bin \
        fip/bl2_new.bin \
        bl2

    $ $FIPDIR/j100/aml_encrypt_axg --bl3sig --input fip/bl30_new.bin \
                                   --output fip/bl30_new.bin.enc \
                                   --level v3 --type bl30
    $ $FIPDIR/j100/aml_encrypt_axg --bl3sig --input fip/bl31.img \
                                   --output fip/bl31.img.enc \
                                   --level v3 --type bl31
    $ $FIPDIR/j100/aml_encrypt_axg --bl3sig --input fip/bl33.bin --compress lz4 \
                                   --output fip/bl33.bin.enc \
                                   --level v3 --type bl33
    $ $FIPDIR/j100/aml_encrypt_axg --bl2sig --input fip/bl2_new.bin \
                                   --output fip/bl2.n.bin.sig
    $ $FIPDIR/j100/aml_encrypt_axg --bootmk \
                                   --output fip/u-boot.bin \
                                   --bl2 fip/bl2.n.bin.sig \
                                   --bl30 fip/bl30_new.bin.enc \
                                   --bl31 fip/bl31.img.enc \
                                   --bl33 fip/bl33.bin.enc --level v3

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440
