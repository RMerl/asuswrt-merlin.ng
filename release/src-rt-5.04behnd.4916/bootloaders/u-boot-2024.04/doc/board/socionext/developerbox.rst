.. SPDX-License-Identifier: GPL-2.0+

Introduction
============

DeveloperBox is a certified 96boards Enterprise Edition board. The board/SoC has: -

* Socionext SC2A11 24-cores ARM Cortex-A53 on tbe Mini-ATX form factor motherboard
* 4 DIMM slots (4GB DDR4-2400 UDIMM shipped by default)
* 1 4xPCIe Gen2 slot and 2 1xPCIe Gen2 slots
  (1x slots are connected via PCIe bridge chip)
* 4 USB-3.0 ports
* 2 SATA ports
* 1 GbE network port
* 1 USB-UART serial port (micro USB)
* 64MB SPI NOR Flash
* 8GB eMMC Flash Storage
* 96boards LS connector

The DeveloperBox schematic can be found here: -
https://www.96boards.org/documentation/enterprise/developerbox/hardware-docs/mzsc2am_v03_20180115_a.pdf

And the other documents can be found here: -
https://www.96boards.org/documentation/enterprise/developerbox/


Currently, the U-Boot port supports: -

* USB
* eMMC
* SPI-NOR
* SATA
* GbE

The DeveloperBox boots the TF-A and EDK2 as a main bootloader by default.
The DeveloperBox U-Boot port will replace the EDK2 and boot from TF-A as
BL33, but no need to combine with it.

Compile from source
===================

You can build U-Boot without any additinal source code.::

  cd u-boot
  git checkout v2023.07
  export ARCH=arm64
  export CROSS_COMPILE=aarch64-linux-gnu-
  make synquacer_developerbox_defconfig
  make -j `noproc`

Then, expand the binary to 1MB for preparing flash.::

  cp u-boot.bin SPI_NOR_UBOOT.fd
  truncate -s 1M SPI_NOR_UBOOT.fd

Installation
============

You can install the SNI_NOR_UBOOT.fd via NOR flash writer.

Flashing the U-Boot image on DeveloperBox requires a 96boards UART mezzanine
or other mezzanine which can connect to the LS-UART0 port.
Connect USB cable from host to the LS-UART0 and set DSW2-7 to ON, and turn the
board on again. The flash writer program will be started automatically;
don't forget to turn the DSW2-7 off again after flashing.

*!!CAUTION!! If you write the U-Boot image on wrong address, the board can
be bricked. See below page if you need to recover the bricked board. See
the following page for more details*

https://www.96boards.org/documentation/enterprise/developerbox/installation/board-recovery.md.html

When the serial flasher is running correctly it will show the following boot
messages printed to the LS-UART0 console::


  /*------------------------------------------*/
  /*  SC2A11 "SynQuacer" series Flash writer  */
  /*                                          */
  /*  Version: cd254ac                        */
  /*  Build: 12/15/17 11:25:45                */
  /*------------------------------------------*/

  Command Input >

Once the flasher tool is running we are ready flash the UEFI image::

  flash rawwrite 200000 100000
  >> Send SPI_NOR_UBOOT.fd via XMODEM (Control-A S in minicom) <<

*!!NOTE!! The flasher command parameter is different from the command for
board recovery. U-Boot uses the offset 200000 (2-five-0, 2M in hex) and the
size 100000 (1-five-0, 1M in hex).*

After transferring the SPI_NOR_UBOOT.fd, turn off the DSW2-7 and
reset the board.


Enable FWU Multi Bank Update
============================

DeveloperBox supports the FWU Multi Bank Update. You *MUST* update both
*SCP firmware* and *TF-A* for this feature. This will change the layout and
the boot process but you can switch back to the normal one by changing
the DSW 1-4 off.

Configure U-Boot
----------------

To enable the FWU Multi Bank Update on the DeveloperBox board the
configs/synquacer_developerbox_defconfig enables default FWU configuration ::

 CONFIG_FWU_MULTI_BANK_UPDATE=y
 CONFIG_FWU_MDATA=y
 CONFIG_FWU_MDATA_MTD=y
 CONFIG_FWU_NUM_BANKS=2
 CONFIG_FWU_NUM_IMAGES_PER_BANK=1
 CONFIG_CMD_FWU_METADATA=y

And build it::

  cd u-boot/
  export ARCH=arm64
  export CROSS_COMPILE=aarch64-linux-gnu-
  make synquacer_developerbox_defconfig
  make -j `noproc`
  cd ../

By default, the CONFIG_FWU_NUM_BANKS and CONFIG_FWU_NUM_IMAGES_PER_BANKS are
set to 2 and 1 respectively. This uses FIP (Firmware Image Package) type image
which contains TF-A, U-Boot and OP-TEE (the OP-TEE is optional).
You can use fiptool to compose the FIP image from those firmware images.

Rebuild SCP firmware
--------------------

Rebuild SCP firmware which supports FWU Multi Bank Update as below::

  cd SCP-firmware/
  OUT=./build/product/synquacer
  ROMFW_FILE=$OUT/scp_romfw/$SCP_BUILD_MODE/bin/scp_romfw.bin
  RAMFW_FILE=$OUT/scp_ramfw/$SCP_BUILD_MODE/bin/scp_ramfw.bin
  ROMRAMFW_FILE=scp_romramfw_release.bin

  make CC=arm-none-eabi-gcc PRODUCT=synquacer MODE=release
  tr "\000" "\377" < /dev/zero | dd of=${ROMRAMFW_FILE} bs=1 count=196608
  dd if=${ROMFW_FILE} of=${ROMRAMFW_FILE} bs=1 conv=notrunc seek=0
  dd if=${RAMFW_FILE} of=${ROMRAMFW_FILE} bs=1 seek=65536
  cd ../

And you can get the `scp_romramfw_release.bin` file.

Rebuild OPTEE firmware
----------------------

Rebuild OPTEE to use in new-layout FIP as below::

  cd optee_os/
  make -j`nproc` PLATFORM=synquacer ARCH=arm \
    CROSS_COMPILE64=aarch64-linux-gnu- CFG_ARM64_core=y \
    CFG_CRYPTO_WITH_CE=y CFG_CORE_HEAP_SIZE=524288 CFG_CORE_DYN_SHM=y \
    CFG_CORE_ARM64_PA_BITS=48 CFG_TEE_CORE_LOG_LEVEL=1 CFG_TEE_TA_LOG_LEVEL=1
  cp out/arm-plat-synquacer/core/tee-pager_v2.bin ../arm-trusted-firmware/

The produced `tee-pager_v2.bin` is to be used while building TF-A next.


Rebuild TF-A and FIP
--------------------

Rebuild TF-A which supports FWU Multi Bank Update as below::

  cd arm-trusted-firmware/
  make CROSS_COMPILE=aarch64-linux-gnu- -j`nproc` PLAT=synquacer \
     TRUSTED_BOARD_BOOT=1 SPD=opteed SQ_RESET_TO_BL2=1 GENERATE_COT=1 \
     MBEDTLS_DIR=../mbedtls BL32=tee-pager_v2.bin \
     BL33=../u-boot/u-boot.bin all fip fiptool

And make a FIP image.::

  cp build/synquacer/release/fip.bin SPI_NOR_NEWFIP.fd
  tools/fiptool/fiptool update --tb-fw build/synquacer/release/bl2.bin SPI_NOR_NEWFIP.fd

UUIDs for the FWU Multi Bank Update
-----------------------------------

FWU multi-bank update requires some UUIDs. The DeveloperBox platform uses
following UUIDs.

 - Location UUID for the FIP image: 17e86d77-41f9-4fd7-87ec-a55df9842de5
 - Image type UUID for the FIP image: 10c36d7d-ca52-b843-b7b9-f9d6c501d108
 - Image UUID for Bank0 : 5a66a702-99fd-4fef-a392-c26e261a2828
 - Image UUID for Bank1 : a8f868a1-6e5c-4757-878d-ce63375ef2c0

These UUIDs are used for making a FWU metadata image.

u-boot$ ./tools/mkfwumdata -i 1 -b 2 \
	17e86d77-41f9-4fd7-87ec-a55df9842de5,10c36d7d-ca52-b843-b7b9-f9d6c501d108,5a66a702-99fd-4fef-a392-c26e261a2828,a8f868a1-6e5c-4757-878d-ce63375ef2c0 \
	../devbox-fwu-mdata.img

Create Accept & Revert capsules

u-boot$ ./tools/mkeficapsule -A -g 7d6dc310-52ca-43b8-b7b9-f9d6c501d108 NEWFIP_accept.Cap
u-boot$ ./tools/mkeficapsule -R NEWFIP_revert.Cap

Install via flash writer
------------------------

As explained in above section, the new FIP image and the FWU metadata image
can be installed via NOR flash writer.

Once the flasher tool is running we are ready to flash the images.::
Write the FIP image to the Bank-0 & 1 at 6MB and 10MB offset.::

  flash rawwrite 600000 400000
  flash rawwrite a00000 400000
  >> Send SPI_NOR_NEWFIP.fd via XMODEM (Control-A S in minicom) <<

  flash rawwrite 500000 1000
  flash rawwrite 530000 1000
  >> Send devbox-fwu-mdata.img via XMODEM (Control-A S in minicom) <<

And write the new SCP firmware.::

  flash write cm3
  >> Send scp_romramfw_release.bin via XMODEM (Control-A S in minicom) <<

At last, turn on the DSW 3-4 on the board, and reboot.
Note that if DSW 3-4 is turned off, the DeveloperBox will boot from
the original EDK2 firmware (or non-FWU U-Boot if you already installed).
