.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Crown Bay CRB
=============

U-Boot support of Intel `Crown Bay`_ board relies on a binary blob called
Firmware Support Package (`FSP`_) to perform all the necessary initialization
steps as documented in the BIOS Writer Guide, including initialization of the
CPU, memory controller, chipset and certain bus interfaces.

Download the Intel FSP for Atom E6xx series and Platform Controller Hub EG20T,
install it on your host and locate the FSP binary blob. Note this platform
also requires a Chipset Micro Code (CMC) state machine binary to be present in
the SPI flash where u-boot.rom resides, and this CMC binary blob can be found
in this FSP package too.

   * ./FSP/QUEENSBAY_FSP_GOLD_001_20-DECEMBER-2013.fd
   * ./Microcode/C0_22211.BIN

Rename the first one to fsp.bin and second one to cmc.bin and put them in the
board directory.

Note the FSP release version 001 has a bug which could cause random endless
loop during the FspInit call. This bug was published by Intel although Intel
did not describe any details. We need manually apply the patch to the FSP
binary using any hex editor (eg: bvi). Go to the offset 0x1fcd8 of the FSP
binary, change the following five bytes values from orginally E8 42 FF FF FF
to B8 00 80 0B 00.

As for the video ROM, you need manually extract it from the Intel provided
BIOS for Crown Bay `here`_, using the AMI `MMTool`_. Check PCI option
ROM ID 8086:4108, extract and save it as vga.bin in the board directory.

Now you can build U-Boot and obtain u-boot.rom::

   $ make crownbay_defconfig
   $ make all

.. _`Crown Bay`: http://www.intel.com/content/www/us/en/embedded/design-tools/evaluation-platforms/atom-e660-eg20t-development-kit.html
.. _`FSP`: http://www.intel.com/fsp
.. _`here`: http://www.intel.com/content/www/us/en/secure/intelligent-systems/privileged/e6xx-35-b1-cmc22211.html
.. _`MMTool`: http://www.ami.com/products/bios-uefi-tools-and-utilities/bios-uefi-utilities/
