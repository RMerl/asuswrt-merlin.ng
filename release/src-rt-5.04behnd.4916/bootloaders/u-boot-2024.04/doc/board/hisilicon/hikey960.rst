.. SPDX-License-Identifier: GPL-2.0-or-later

HiKey960 board
##############

Introduction
============

HiKey960 is one of the 96Boards Consumer Edition board from HiSilicon.
The board/SoC has:

* HiSilicon Kirin960 (HI3660) SoC with 4xCortex-A73 and 4xCortex-A53
* ARM Mali G71 MP8 GPU
* 3GB LPDDR4 SDRAM
* 32GB UFS Flash Storage
* microSD
* 802.11a/b/g/n WiFi, Bluetooth

More information about this board can be found in 96Boards website:
https://www.96boards.org/product/hikey960/

Currently the u-boot port supports:

* SD card

Compile from source
===================

First get all the sources

.. code-block:: bash

  mkdir -p ~/hikey960/src ~/hikey960/bin
  cd ~/hikey960/src
  git clone https://github.com/ARM-software/arm-trusted-firmware
  git clone https://github.com/96boards-hikey/OpenPlatformPkg -b testing/hikey960_v1.3.4
  git clone https://github.com/96boards-hikey/l-loader -b testing/hikey960_v1.2
  wget http://snapshots.linaro.org/reference-platform/components/uefi-staging/123/hikey960/release/config
  wget http://snapshots.linaro.org/reference-platform/components/uefi-staging/123/hikey960/release/hisi-sec_usb_xloader.img
  wget http://snapshots.linaro.org/reference-platform/components/uefi-staging/123/hikey960/release/hisi-sec_uce_boot.img
  wget http://snapshots.linaro.org/reference-platform/components/uefi-staging/123/hikey960/release/hisi-sec_xloader.img
  wget http://snapshots.linaro.org/reference-platform/components/uefi-staging/123/hikey960/release/recovery.bin
  wget http://snapshots.linaro.org/reference-platform/components/uefi-staging/123/hikey960/release/hikey_idt

Get the SCP_BL2 lpm3.img binary. It is shipped as part of the UEFI source.
The latest version can be obtained from the OpenPlatformPkg repo.

.. code-block:: bash

  cp OpenPlatformPkg/Platforms/Hisilicon/HiKey960/Binary/lpm3.img ~/hikey960/bin/

Compile U-Boot
==============

.. code-block:: bash

  cd ~/hikey960/src/u-boot
  make CROSS_COMPILE=aarch64-linux-gnu- hikey960_defconfig
  make CROSS_COMPILE=aarch64-linux-gnu-
  cp u-boot.bin ~/hikey960/bin/

Compile ARM Trusted Firmware (ATF)
==================================

.. code-block:: bash

  cd ~/hikey960/src/arm-trusted-firmware
  make CROSS_COMPILE=aarch64-linux-gnu- all fip \
    SCP_BL2=~/hikey960/bin/lpm3.img \
    BL33=~/hikey960/bin/u-boot.bin DEBUG=1 PLAT=hikey960

Copy the resulting FIP binary

.. code-block:: bash

  cp build/hikey960/debug/fip.bin ~/hikey960/bin

Compile l-loader
================

.. code-block:: bash

  cd ~/hikey960/src/l-loader
  ln -sf ~/hikey960/src/arm-trusted-firmware/build/hikey960/debug/bl1.bin
  ln -sf ~/hikey960/src/arm-trusted-firmware/build/hikey960/debug/bl2.bin
  ln -sf ~/hikey960/src/arm-trusted-firmware/build/hikey960/debug/fip.bin
  ln -sf ~/hikey960/bin/u-boot.bin
  make hikey960 PTABLE_LST=linux-32g NS_BL1U=u-boot.bin

Copy the resulting binaries

.. code-block:: bash

  cp *.img ~/hikey960/bin
  cp l-loader.bin ~/hikey960/bin

These instructions are adapted from
https://github.com/ARM-software/arm-trusted-firmware/blob/master/docs/plat/hikey960.rst

Setup console
=============

Install ser2net. Use telnet as the console since UEFI in recovery mode
output window fails to display in minicom.

.. code-block:: bash

  sudo apt-get install ser2net

Configure ser2net

.. code-block:: bash

  sudo vi /etc/ser2net.conf

Append one line for serial-over-USB in #ser2net.conf

  2004:telnet:0:/dev/ttyUSB0:115200 8DATABITS NONE 1STOPBIT banner

Start ser2net

.. code-block:: bash

  sudo killall ser2net
  sudo ser2net -u

Open the console.

.. code-block:: bash

  telnet localhost 2004

And you could open the console remotely, too.

Flashing
========

1. Boot Hikey960 into recovery mode as per the below document:
https://github.com/96boards/documentation/blob/master/consumer/hikey/hikey960/installation/board-recovery.md

Once Hikey960 is in recovery mode, flash the recovery binary:

.. code-block:: bash

  cd ~/hikey960/src
  chmod +x ./hikey_idt
  sudo ./hikey_idt -c config -p /dev/ttyUSB1

Now move to the Hikey960 console and press `f` during UEFI boot. This
will allow the board to boot into fastboot mode. Once the board is in
fastboot mode, you should see the ID of the HiKey960 board using the
following command

.. code-block:: bash

  sudo fastboot devices

  1ED3822A018E3372	fastboot

3. Flash the images

Now, the images can be flashed using fastboot:

.. code-block:: bash

  sudo fastboot flash ptable ~/hikey960/bin/prm_ptable.img
  sudo fastboot flash xloader ~/hikey960/bin/hisi-sec_xloader.img
  sudo fastboot flash fastboot ~/hikey960/bin/l-loader.bin
  sudo fastboot flash fip ~/hikey960/bin/fip.bin

4. Set the "Boot Mode" switch to OFF position for normal boot mode.
Then power on HiKey960

Observe the console traces using UART6 on the Low Speed Expansion header::

  NOTICE:  BL2: v2.1(debug):v2.1-531-g3ee48f40
  NOTICE:  BL2: Built : 18:15:58, Aug  2 2019
  INFO:    BL2: Doing platform setup
  INFO:    UFS LUN0 contains 1024 blocks with 4096-byte size
  INFO:    UFS LUN1 contains 1024 blocks with 4096-byte size
  INFO:    UFS LUN2 contains 2048 blocks with 4096-byte size
  INFO:    UFS LUN3 contains 7805952 blocks with 4096-byte size
  INFO:    ufs: change power mode success
  INFO:    BL2: Loading image id 2
  INFO:    Loading image id=2 at address 0x89c80000
  INFO:    Image id=2 loaded: 0x89c80000 - 0x89cb5088
  INFO:    BL2: Initiating SCP_BL2 transfer to SCP
  INFO:    BL2: SCP_BL2: 0x89c80000@0x35088
  INFO:    BL2: SCP_BL2 HEAD:
  INFO:    BL2: SCP_BL2 0x7000 0x179 0x159 0x149
  INFO:    BL2: SCP_BL2 0x189 0x18b 0x18d 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x18f
  INFO:    BL2: SCP_BL2 0x191 0x0 0x193 0x195
  INFO:    BL2: SCP_BL2 0x18fd 0x18fd 0x18fd 0x18fd
  INFO:    BL2: SCP_BL2 0x18fd 0x18fd 0x18fd 0x18fd
  INFO:    BL2: SCP_BL2 0x18fd 0x18fd 0x18fd 0x18fd
  INFO:    BL2: SCP_BL2 0x4d454355 0x43494741 0x424d554e 0x21215245
  INFO:    BL2: SCP_BL2 0x4a054904 0x42912000 0xf841bfbc 0xe7fa0b04
  INFO:    BL2: SCP_BL2 0xb88cf000 0x3b18 0x3d1c 0x6809493e
  INFO:    BL2: SCP_BL2 0x4613680a 0x201f102 0xf0002a04 0x600a804c
  INFO:    BL2: SCP_BL2 0x204f04f 0xf203fb02 0xf102440a 0x60100204
  INFO:    BL2: SCP_BL2 0x160f04f 0xf103fb01 0x68004834 0x61044408
  INFO:    BL2: SCP_BL2 0x61866145 0xf8c061c7 0xf8c08020 0xf8c09024
  INFO:    BL2: SCP_BL2 0xf8c0a028 0xf3efb02c 0xf3ef8208 0x68118309
  INFO:    BL2: SCP_BL2 0xf1026401 0xf0110204 0xbf070f04 0x46113220
  INFO:    BL2: SCP_BL2 TAIL:
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x19cad151 0x19b80040 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 0x0 0x0 0x0 0x0
  INFO:    BL2: SCP_BL2 transferred to SCP
  INFO:    start fw loading
  INFO:    fw load success
  WARNING: BL2: Platform setup already done!!
  INFO:    BL2: Loading image id 3
  INFO:    Loading image id=3 at address 0x1ac58000
  INFO:    Image id=3 loaded: 0x1ac58000 - 0x1ac63024
  INFO:    BL2: Loading image id 5
  INFO:    Loading image id=5 at address 0x1ac98000
  INFO:    Image id=5 loaded: 0x1ac98000 - 0x1ad0819c
  NOTICE:  BL2: Booting BL31
  INFO:    Entry point address = 0x1ac58000
  INFO:    SPSR = 0x3cd
  NOTICE:  BL31: v2.1(debug):v2.1-531-g3ee48f40
  NOTICE:  BL31: Built : 18:16:01, Aug  2 2019
  INFO:    ARM GICv2 driver initialized
  INFO:    BL31: Initializing runtime services
  INFO:    BL31: cortex_a53: CPU workaround for 855873 was applied
  INFO:    plat_setup_psci_ops: sec_entrypoint=0x1ac580fc
  INFO:    BL31: Preparing for EL3 exit to normal world
  INFO:    Entry point address = 0x1ac98000
  INFO:    SPSR = 0x3c9

  U-Boot 2019.07-00628-g286f05a6fc-dirty (Aug 02 2019 - 17:14:05 +0530)
  Hikey960

  DRAM:  3 GiB
  PSCI:  v1.1
  MMC:   dwmmc1@ff37f000: 0
  Loading Environment from EXT4... ** File not found /uboot.env **

  ** Unable to read "/uboot.env" from mmc0:2 **
  In:    serial@fff32000
  Out:   serial@fff32000
  Err:   serial@fff32000
  Net:   Net Initialization Skipped
  No ethernet found.
  Hit any key to stop autoboot:  0
  switch to partitions #0, OK
  mmc0 is current device
  Scanning mmc 0:1...
  Found /extlinux/extlinux.conf
  Retrieving file: /extlinux/extlinux.conf
  201 bytes read in 12 ms (15.6 KiB/s)
  1:      hikey960-kernel
  Retrieving file: /Image
  24689152 bytes read in 4377 ms (5.4 MiB/s)
  append: earlycon=pl011,mmio32,0xfff32000 console=ttyAMA6,115200 rw root=/dev/mmcblk0p2 rot
  Retrieving file: /hi3660-hikey960.dtb
  35047 bytes read in 14 ms (2.4 MiB/s)
  ## Flattened Device Tree blob at 10000000
     Booting using the fdt blob at 0x10000000
     Using Device Tree in place at 0000000010000000, end 000000001000b8e6

  Starting kernel ...

  [  0.000000] Booting Linux on physical CPU 0x0000000000 [0x410fd034]
  [  0.000000] Linux version 5.2.0-03138-gd75da80dce39 (mani@Mani-XPS-13-9360) (gcc versi9
  [  0.000000] Machine model: HiKey960
  [  0.000000] earlycon: pl11 at MMIO32 0x00000000fff32000 (options '')
  [  0.000000] printk: bootconsole [pl11] enabled
  [  0.000000] efi: Getting EFI parameters from FDT:
