.. SPDX-License-Identifier: GPL-2.0+

U-Boot for Beelink GT-King (S922X)
==================================

The Shenzen AZW (Beelink) GT-King is based on the Amlogic W400 reference board with an
S922X-H chip and the following specifications:

- 4GB LPDDR4 RAM
- 64GB eMMC storage
- 10/100/1000 Base-T Ethernet
- AP6356S Wireless (802.11 a/b/g/n/ac, BT 4.1)
- HDMI 2.1 video
- S/PDIF optical output
- Analogue audio output
- 1x USB 2.0 port
- 2x USB 3.0 ports
- IR receiver
- 1x micro SD card slot

Beelink do not provide public schematics, but have been willing to share them with known
distro developers to assist with development.

U-Boot Compilation
------------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-none-elf-
    $ make beelink-gtking_defconfig
    $ make

U-Boot Signing with Pre-Built FIP repo
--------------------------------------

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh beelink-s922x /path/to/u-boot/u-boot.bin my-output-dir

U-Boot Manual Signing
---------------------

Beelink released an Amlogic "SDK" dump in their forums, but the U-Boot sources included
result in 2GB RAM detected. The following FIPs were generated with newer sources and
detect 4GB RAM: https://github.com/LibreELEC/amlogic-boot-fip/tree/master/beelink-s922x

.. code-block:: bash

    $ wget https://github.com/LibreELEC/amlogic-boot-fip/archive/master.zip
    $ unzip master.zip
    $ export FIPDIR=$PWD/amlogic-boot-fip/beelink-s922x

Go back to the mainline U-Boot source tree then:

.. code-block:: bash

    $ mkdir fip
    $ cp $FIPDIR/* fip/
    $ cp u-boot.bin fip/bl33.bin

    $ sh fip/blx_fix.sh \
         fip/bl30.bin \
         fip/zero_tmp \
         fip/bl30_zero.bin \
         fip/bl301.bin \
         fip/bl301_zero.bin \
         fip/bl30_new.bin \
         bl30

    $ sh fip/blx_fix.sh \
         fip/bl2.bin \
         fip/zero_tmp \
         fip/bl2_zero.bin \
         fip/acs.bin \
         fip/bl21_zero.bin \
         fip/bl2_new.bin \
         bl2

    $ fip/aml_encrypt_g12b --bl30sig --input fip/bl30_new.bin \
                           --output fip/bl30_new.bin.g12a.enc \
                           --level v3
    $ fip/aml_encrypt_g12b --bl3sig --input fip/bl30_new.bin.g12a.enc \
                           --output fip/bl30_new.bin.enc \
                           --level v3 --type bl30
    $ fip/aml_encrypt_g12b --bl3sig --input fip/bl31.img \
                           --output fip/bl31.img.enc \
                           --level v3 --type bl31
    $ fip/aml_encrypt_g12b --bl3sig --input fip/bl33.bin --compress lz4 \
                           --output fip/bl33.bin.enc \
                           --level v3 --type bl33
    $ fip/aml_encrypt_g12b --bl2sig --input fip/bl2_new.bin \
                           --output fip/bl2.n.bin.sig
    $ fip/aml_encrypt_g12b --bootmk \
                           --output fip/u-boot.bin \
                           --bl2 fip/bl2.n.bin.sig \
                           --bl30 fip/bl30_new.bin.enc \
                           --bl31 fip/bl31.img.enc \
                           --bl33 fip/bl33.bin.enc \
                           --ddrfw1 fip/ddr4_1d.fw \
                           --ddrfw2 fip/ddr4_2d.fw \
                           --ddrfw3 fip/ddr3_1d.fw \
                           --ddrfw4 fip/piei.fw \
                           --ddrfw5 fip/lpddr4_1d.fw \
                           --ddrfw6 fip/lpddr4_2d.fw \
                           --ddrfw7 fip/diag_lpddr4.fw \
                           --ddrfw8 fip/aml_ddr.fw \
                           --level v3

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440
