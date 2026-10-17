.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Aiden Park <aiden.park@intel.com>

Slim Bootloader
===============

Introduction
------------

This target is to enable U-Boot_ as a payload of `Slim Bootloader`_ (a.k.a SBL)
boot firmware which currently supports QEMU, Apollolake, Whiskeylake,
Coffeelake-R platforms.

The `Slim Bootloader`_ is designed with multi-stages (Stage1A/B, Stage2, Payload)
architecture to cover from reset vector to OS booting and it consumes
`Intel FSP`_ for silicon initialization.

* Stage1A: Reset vector, CAR init with FSP-T
* Stage1B: Memory init with FSP-M, CAR teardown, Continue execution in memory
* Stage2 : Rest of Silicon init with FSP-S, Create HOB, Hand-off to Payload
* Payload: Payload init with HOB, Load OS from media, Booting OS

The Slim Bootloader stages (Stage1A/B, Stage2) focus on chipset, hardware and
platform specific initialization, and it provides useful information to a
payload in a HOB (Hand-Off Block) which has serial port, memory map, performance
data info and so on. This is Slim Bootloader architectural design to make a
payload light-weight, platform independent and more generic across different
boot solutions or payloads, and to minimize hardware re-initialization in a
payload.

Build Instruction for U-Boot as a Slim Bootloader payload
---------------------------------------------------------

Build U-Boot and obtain u-boot-dtb.bin::

   $ make distclean
   $ make slimbootloader_defconfig
   $ make all

Prepare Slim Bootloader
-----------------------

1. Setup Build Environment for Slim Bootloader.

   Refer to `Getting Started`_ page in `Slim Bootloader`_ document site.

2. Get source code. Let's simply clone the repo::

   $ git clone https://github.com/slimbootloader/slimbootloader.git

3. Copy u-boot-dtb.bin to Slim Bootloader.
   Slim Bootloader looks for a payload from the specific location.
   Copy the build u-boot-dtb.bin to the expected location::

   $ mkdir -p <Slim Bootloader Dir>/PayloadPkg/PayloadBins/
   $ cp <U-Boot Dir>/u-boot-dtb.bin <Slim Bootloader Dir>/PayloadPkg/PayloadBins/u-boot-dtb.bin

Build Instruction for Slim Bootloader for QEMU target
-----------------------------------------------------

Slim Bootloader supports multiple payloads, and a board of Slim Bootloader
detects its target payload by PayloadId in board configuration.
The PayloadId can be any 4 Bytes value.

1. Update PayloadId. Let's use 'U-BT' as an example::

    $ vi Platform/QemuBoardPkg/CfgData/CfgDataExt_Brd1.dlt
    -GEN_CFG_DATA.PayloadId                     | 'AUTO'
    +GEN_CFG_DATA.PayloadId                     | 'U-BT'

2. Update payload text base. PAYLOAD_EXE_BASE must be the same as U-Boot
   CONFIG_TEXT_BASE in board/intel/slimbootloader/Kconfig.
   PAYLOAD_LOAD_HIGH must be 0::

    $ vi Platform/QemuBoardPkg/BoardConfig.py
    +               self.PAYLOAD_LOAD_HIGH    = 0
    +               self.PAYLOAD_EXE_BASE     = 0x00100000

3. Build QEMU target. Make sure u-boot-dtb.bin and U-BT PayloadId
   in build command. The output is Outputs/qemu/SlimBootloader.bin::

   $ python BuildLoader.py build qemu -p "OsLoader.efi:LLDR:Lz4;u-boot-dtb.bin:U-BT:Lzma"

4. Launch Slim Bootloader on QEMU.
   You should reach at U-Boot serial console::

   $ qemu-system-x86_64 -machine q35 -nographic -serial mon:stdio -pflash Outputs/qemu/SlimBootloader.bin

Test Linux booting on QEMU target
---------------------------------

Let's use LeafHill (APL) Yocto image for testing.
Download it from http://downloads.yoctoproject.org/releases/yocto/yocto-2.0/machines/leafhill/.

1. Prepare Yocto hard disk image::

   $ wget http://downloads.yoctoproject.org/releases/yocto/yocto-2.0/machines/leafhill/leafhill-4.0-jethro-2.0.tar.bz2
   $ tar -xvf leafhill-4.0-jethro-2.0.tar.bz2
   $ ls -l leafhill-4.0-jethro-2.0/binary/core-image-sato-intel-corei7-64.hddimg

2. Launch Slim Bootloader on QEMU with disk image::

   $ qemu-system-x86_64 -machine q35 -nographic -serial mon:stdio -pflash Outputs/qemu/SlimBootloader.bin -drive id=mydrive,if=none,file=/path/to/core-image-sato-intel-corei7-64.hddimg,format=raw -device ide-hd,drive=mydrive

3. Update boot environment values on shell::

   => setenv bootfile vmlinuz
   => setenv bootdev scsi
   => boot

Build Instruction for Slim Bootloader for LeafHill (APL) target
---------------------------------------------------------------

Prepare U-Boot and Slim Bootloader as described at the beginning of this page.
Also, the PayloadId needs to be set for APL board.

1. Update PayloadId. Let's use 'U-BT' as an example::

    $ vi Platform/ApollolakeBoardPkg/CfgData/CfgData_Int_LeafHill.dlt
    -GEN_CFG_DATA.PayloadId                     | 'AUTO
    +GEN_CFG_DATA.PayloadId                     | 'U-BT'

2. Update payload text base.

* PAYLOAD_EXE_BASE must be the same as U-Boot CONFIG_TEXT_BASE
  in board/intel/slimbootloader/Kconfig.
* PAYLOAD_LOAD_HIGH must be 0::

    $ vi Platform/ApollolakeBoardPkg/BoardConfig.py
    +               self.PAYLOAD_LOAD_HIGH    = 0
    +               self.PAYLOAD_EXE_BASE     = 0x00100000

3. Build APL target. Make sure u-boot-dtb.bin and U-BT PayloadId
   in build command. The output is Outputs/apl/Stitch_Components.zip::

   $ python BuildLoader.py build apl -p "OsLoader.efi:LLDR:Lz4;u-boot-dtb.bin:U-BT:Lzma"

4. Stitch IFWI.

   Refer to Apollolake_ page in Slim Bootloader document site::

   $ python Platform/ApollolakeBoardPkg/Script/StitchLoader.py -i <Existing IFWI> -s Outputs/apl/Stitch_Components.zip -o <Output IFWI>

5. Flash IFWI.

   Use DediProg to flash IFWI. You should reach at U-Boot serial console.


Build Instruction to use ELF U-Boot
-----------------------------------

1. Enable CONFIG_OF_EMBED::

    $ vi configs/slimbootloader_defconfig
    +CONFIG_OF_EMBED=y

2. Build U-Boot::

   $ make distclean
   $ make slimbootloader_defconfig
   $ make all
   $ strip u-boot (removing symbol for reduced size)

3. Do same steps as above

* Copy u-boot (ELF) to PayloadBins directory
* Update PayloadId 'U-BT' as above.
* No need to set PAYLOAD_LOAD_HIGH and PAYLOAD_EXE_BASE.
* Build Slim Bootloader. Use u-boot instead of u-boot-dtb.bin::

   $ python BuildLoader.py build <qemu or apl> -p "OsLoader.efi:LLDR:Lz4;u-boot:U-BT:Lzma"

.. _U-Boot: https://source.denx.de/
.. _`Slim Bootloader`: https://github.com/slimbootloader/
.. _`Intel FSP`: https://github.com/IntelFsp/
.. _`Getting Started`: https://slimbootloader.github.io/getting-started/
.. _Apollolake: https://slimbootloader.github.io/supported-hardware/apollo-lake-crb.html#stitching
