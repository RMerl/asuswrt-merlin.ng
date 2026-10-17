.. SPDX-License-Identifier: GPL-2.0
.. Copyright (C) 2021 Arm Ltd.

Arm Juno development platform
=============================

The `Juno development board`_ is an open, vendor-neutral, Armv8-A development
platform, made by Arm Ltd. It is part of the Versatile Express family.
There are three revisions of the board:

* Juno r0, with two Cortex-A57 and four Cortex-A53 cores, without PCIe.
* Juno r1, with two Cortex-A57 and four Cortex-A53 cores, in later silicon
  revisions, and with PCIe slots, Gigabit Ethernet and two SATA ports.
* Juno r2, with two Cortex-A72 and four Cortex-A53 cores, otherwise the
  same as r1.

Among other things, the motherboard contains a management controller (MCC),
an FPGA providing I/O interfaces (IOFPGA) and 64MB of NOR flash. The provided
platform devices resemble the VExpress peripherals.
The actual SoC also contains a Cortex-M3 based System Control Processor (SCP).
The `V2M-Juno TRM`_ contains more technical details.

U-Boot build
------------
There is only one defconfig and one binary build that covers all three board
revisions, so to generate the needed ``u-boot.bin``:

.. code-block:: bash

    $ make vexpress_aemv8a_juno_defconfig
    $ make

The automatic distro boot sequence looks for UEFI boot applications and
``boot.scr`` scripts on various boot media, starting with USB, then on disks
connected to the two SATA ports, PXE, DHCP and eventually on the NOR flash.

U-Boot installation
-------------------
This assumes there is some firmware on the SD card or NOR flash (see below
for more details). The U-Boot binary is included in the Trusted Firmware
FIP image, so after building U-Boot, this needs to be repackaged or recompiled.

The NOR flash will be updated by the MCC, based on the content of a micro-SD
card, which is exported as a USB mass storage device via the rear USB-B
socket. So to access that SD card, connect a cable to some host computer, and
mount the FAT16 partition of the UMS device.
If there is no device, check the upper serial port for a prompt, and
explicitly enable the USB interface::

    Cmd> usb_on
    Enabling debug USB...

Repackaging an existing FIP image
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
To prevent problems, it is probably a good idea to backup the existing firmware,
for instance by just copying the entire ``SOFTWARE/`` directory, or at least
the current ``fip.bin``, beforehand.

To just replace the BL33 image in the exising FIP image, you can use
`fiptool`_ from the Trusted Firmware repository, on the image file:

.. code-block:: bash

    git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    cd trusted-firmware-a
    make fiptool
    tools/fiptool/fiptool update --nt-fw=/path/to/your/u-boot.bin /mnt/juno/SOFTWARE/fip.bin

Unmount the USB mass storage device and reboot the board, the new ``fip.bin``
will be automatically written to the NOR flash and then used.

Rebuilding Trusted Firmware
^^^^^^^^^^^^^^^^^^^^^^^^^^^
You can also generate a new FIP image by compiling Arm Trusted Firmware,
and providing ``u-boot.bin`` as the BL33 file. For that you can either build
the required `SCP firmware`_ yourself, or just extract the existing
version from your ``fip.bin``, using `fiptool`_ (see above):

.. code-block:: bash

    mkdir /tmp/juno; cd /tmp/juno
    fiptool unpack /mnt/juno/SOFTWARE/fip.bin

Then build TF-A:

.. code-block:: bash

    git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    cd trusted-firmware-a
    make CROSS_COMPILE=aarch64-linux-gnu- PLAT=juno DEBUG=1 \
    SCP_BL2=/tmp/juno/scp-fw.bin BL33=/path/to/your/u-boot.bin fiptool all fip
    cp build/juno/debug/bl1.bin build/juno/debug/fip.bin /mnt/juno/SOFTWARE

Then umount the USB device, and reboot, as above.

Device trees
------------
The device tree files for the boards are maintained in the Linux kernel
repository. They end up in the ``SOFTWARE/`` directory of the SD card, as
``juno.dtb``, ``juno-r1.dtb``, and ``juno-r2.dtb``, respectively. The MCC
firmware will look into the images.txt file matching the board revision, from
the ``SITE1/`` directory. Each version there will reference its respective DTB
file in ``SOFTWARE/``, and so the correct version will end in the NOR flash, in
the ``board.dtb`` partition. U-Boot picks its control DTB from there, you can
pass this on to a kernel using ``$fdtcontroladdr``.

You can update the DTBs anytime, by building them using the ``dtbs`` make
target from a Linux kernel tree, then just copying the generated binaries
to the ``SOFTWARE/`` directory of the SD card.

.. _`Juno development board`: https://developer.arm.com/tools-and-software/development-boards/juno-development-board
.. _`V2M-Juno TRM`: https://developer.arm.com/documentation/100113/latest
.. _`fiptool`: https://github.com/ARM-software/arm-trusted-firmware/tree/master/tools/fiptool
.. _`SCP firmware`: https://github.com/ARM-software/SCP-firmware.git
