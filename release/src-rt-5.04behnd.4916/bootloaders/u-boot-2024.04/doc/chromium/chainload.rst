.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2020 Google LLC

Running U-Boot from coreboot on Chromebooks
===========================================

U-Boot can be used as a secondary boot loader in a few situations such as from
UEFI and coreboot (see README.x86). Recent Chromebooks use coreboot even on
ARM platforms to start up the machine.

This document aims to provide a guide to booting U-Boot on a Chromebook. It
is only a starting point, and there are many guides on the interwebs. But
placing this information in the U-Boot tree should make it easier to find for
those who use U-Boot habitually.

Most of these platforms are supported by U-Boot natively, but it is risky to
replace the ROM unless you have a servo board and cable to restore it with.


For all of these the standard U-Boot build instructions apply. For example on
ARM::

   sudo apt install gcc-arm-linux-gnueabi
   mkdir b
   make O=b/nyan_big CROSS_COMPILE=arm-linux-gnueabi- nyan-big_defconfig all

You can obtain the vbutil_kernel utility here:

   https://drive.google.com/open?id=0B7WYZbZ9zd-3dHlVVXo4VXE2T0U


Snow (Samsung ARM Chromebook)
-----------------------------

See here:

https://www.chromium.org/chromium-os/firmware-porting-guide/using-nv-u-boot-on-the-samsung-arm-chromebook


Nyan-big
--------

Compiled based on information here::

   https://lists.denx.de/pipermail/u-boot/2015-March/209530.html
   https://git.collabora.com/cgit/user/tomeu/u-boot.git/commit/?h=nyan-big
   https://lists.denx.de/pipermail/u-boot/2017-May/289491.html
   https://github.com/chromeos-nvidia-androidtv/gnu-linux-on-acer-chromebook-13#copy-data-to-the-sd-card

1. Build U-Boot

Steps::

   mkdir b
   make -j8 O=b/nyan-big CROSS_COMPILE=arm-linux-gnueabi- nyan-big_defconfig all


2. Select a .its file

Select something from doc/chromium which matches your board, or create your
own.

Note that the device tree node is required, even though it is not actually
used by U-Boot. This is because the Chromebook expects to pass it to the
kernel, and crashes if it is not present.


3. Build and sign an image

Steps::

   ./b/nyan-big/tools/mkimage -f doc/chromium/files/nyan-big.its u-boot-chromium.fit
   echo test >dummy.txt
   vbutil_kernel --arch arm \
     --keyblock doc/chromium/files/devkeys/kernel.keyblock \
     --signprivate doc/chromium/files/devkeys/kernel_data_key.vbprivk \
     --version 1 --config dummy.txt --vmlinuz u-boot-chromium.fit \
     --bootloader dummy.txt --pack u-boot.kpart


4. Prepare an SD card

Steps::

   DISK=/dev/sdc   # Replace with your actual SD card device
   sudo cgpt create $DISK
   sudo cgpt add -b 34 -s 32768 -P 1 -S 1 -t kernel $DISK
   sudo cgpt add -b 32802 -s 2000000 -t rootfs $DISK
   sudo gdisk $DISK   # Enter command 'w' to write a protective MBR to the disk


5. Write U-Boot to the SD card

Steps::

   sudo dd if=u-boot.kpart of=/dev/sdc1; sync


6. Start it up

Reboot the device in dev mode. Make sure that you have USB booting enabled. To
do this, login as root (via Ctrl-Alt-forward_arrow) and type
'enable_dev_usb_boot'. You only need to do this once.

Reboot the device with the SD card inserted. Press Clrl-U at the developer
mode screen. It should show something like the following on the display::

   U-Boot 2017.07-00637-g242eb42-dirty (May 22 2017 - 06:14:21 -0600)

   Model: Acer Chromebook 13 CB5-311
   Board: Google/NVIDIA Nyan-big, ID: 1

   Net:   No ethernet found.
   Hit any key to stop autoboot:  0
   Tegra124 (Nyan-big) #


7. Known problems

On the serial console the word MMC is chopped at the start of the line::

   C:   sdhci@700b0000: 2, sdhci@700b0400: 1, sdhci@700b0600: 0

This is likely due to some problem with change-over of the serial driver
during relocation (or perhaps updating the clock setup in board_init()).


9. Notes

To check that you copied the u-boot.its file correctly, use these commands.
You should see that the data at 0x100 in u-boot-chromium.fit is the first few
bytes of U-Boot::

   hd u-boot-chromium.fit |head -20
   ...
   00000100  b8 00 00 ea 14 f0 9f e5  14 f0 9f e5 14 f0 9f e5  |................|

   hd b/nyan-big/u-boot.bin |head
   00000000  b8 00 00 ea 14 f0 9f e5  14 f0 9f e5 14 f0 9f e5  |................|


The 'data' property of the FIT is set up to start at offset 0x100 bytes into
the file. The change to CONFIG_TEXT_BASE is also an offset of 0x100 bytes
from the load address. If this changes, you either need to modify U-Boot to be
fully relocatable, or expect it to hang.


chromebook_jerry
----------------

The instruction are similar to those for Nyan with changes as noted below:

1. Patch U-Boot

Open include/configs/rk3288_common.h

Change::

   #define CONFIG_TEXT_BASE		0x00100000

to::

   #define CONFIG_TEXT_BASE		0x02000100



2. Build U-Boot

Steps::

   mkdir b
   make -j8 O=b/chromebook_jerry CROSS_COMPILE=arm-linux-gnueabi- \
      chromebook_jerry_defconfig all


3. See above

4. Build and sign an image

Steps::

   ./b/chromebook_jerry/tools/mkimage -f doc/chromium/chromebook_jerry.its \
      u-boot-chromium.fit
   echo test >dummy.txt
   vbutil_kernel --arch arm \
      --keyblock doc/chromium/files/devkeys/kernel.keyblock \
      --signprivate doc/chromium/files/devkeys/kernel_data_key.vbprivk \
      --version 1 --config dummy.txt --vmlinuz u-boot-chromium.fit \
      --bootloader dummy.txt --pack u-boot.kpart


5. See above

6. See above

7. Start it up

Reboot the device in dev mode. Make sure that you have USB booting enabled. To
do this, login as root (via Ctrl-Alt-forward_arrow) and type
'enable_dev_usb_boot'. You only need to do this once.

Reboot the device with the SD card inserted. Press Clrl-U at the developer
mode screen. It should show something like the following on the display::

   U-Boot 2017.05-00649-g72acdbf-dirty (May 29 2017 - 14:57:05 -0600)

   Model: Google Jerry
   Net:   Net Initialization Skipped
   No ethernet found.
   Hit any key to stop autoboot:  0


8. Known problems

None as yet.


9. Notes

None as yet.


Other notes
-----------

flashrom
~~~~~~~~

Used to make a backup of your firmware, or to replace it.

See: https://www.chromium.org/chromium-os/packages/cros-flashrom


coreboot
~~~~~~~~

Coreboot itself is not designed to actually boot an OS. Instead, a program
called Depthcharge is used. This originally came out of U-Boot and was then
heavily hacked and modified such that is is almost unrecognisable. It does
include a very small part of the U-Boot command-line interface but is not
usable as a general-purpose boot loader.

In addition, it has a very unusual design in that it does not do device init
itself, but instead relies on coreboot. This is similar to (in U-Boot) having
a SPI driver with an empty probe() method, relying on whatever was set up
beforehand. It can be quite hard to figure out between these two code bases
what settings are actually used. When chain-loading into U-Boot we must be
careful to reinit anything that U-Boot expects. If not, some peripherals (or
the whole machine) may not work. This makes the process of chainloading more
complicated than it could be on some platforms.

Finally, it supports only a subset of the U-Boot's FIT format. In particular
it uses a fixed address to load the FIT and does not support load/exec
addresses. This means that U-Boot must be able to boot from whatever
address Depthcharge happens to use (it is the CONFIG_KERNEL_START setting
in Depthcharge). In practice this means that the data in the kernel@1 FIT node
(see above) must start at the same address as U-Boot's CONFIG_TEXT_BASE.
