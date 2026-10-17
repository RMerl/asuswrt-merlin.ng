.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Bin Meng <bmeng.cn@gmail.com>

Coreboot
========

Build Instructions for U-Boot as coreboot payload
-------------------------------------------------
Building U-Boot as a coreboot payload is just like building U-Boot for targets
on other architectures, like below::

   $ make coreboot_defconfig
   $ make all

Test with coreboot
------------------
For testing U-Boot as the coreboot payload, there are things that need be paid
attention to. coreboot supports loading an ELF executable and a 32-bit plain
binary, as well as other supported payloads. With the default configuration,
U-Boot is set up to use a separate Device Tree Blob (dtb). As of today, the
generated u-boot-dtb.bin needs to be packaged by the cbfstool utility (a tool
provided by coreboot) manually as coreboot's 'make menuconfig' does not provide
this capability yet. The command is as follows::

   # in the coreboot root directory
   $ ./build/util/cbfstool/cbfstool build/coreboot.rom add-flat-binary \
     -f u-boot-dtb.bin -n fallback/payload -c lzma -l 0x1110000 -e 0x1110000

Make sure 0x1110000 matches CONFIG_TEXT_BASE, which is the symbol address
of _x86boot_start (in arch/x86/cpu/start.S).

If you want to use ELF as the coreboot payload, change U-Boot configuration to
use CONFIG_OF_EMBED instead of CONFIG_OF_SEPARATE.

To enable video you must enable CONFIG_GENERIC_LINEAR_FRAMEBUFFER in coreboot:

   - Devices->Display->Framebuffer mode->Linear "high resolution" framebuffer

At present it seems that for Minnowboard Max, coreboot does not pass through
the video information correctly (it always says the resolution is 0x0). This
works correctly for link though.

You can run via QEMU using::

  qemu-system-x86_64 -bios build/coreboot.rom -serial mon:stdio

The `-serial mon:stdio` part shows both output in the display and on the
console. It is optional. You can add `nographic` as well to *only* get console
output.

To run with a SATA drive called `$DISK`::

  qemu-system-x86_64 -bios build/coreboot.rom -serial mon:stdio \
	-drive id=disk,file=$DISK,if=none \
	-device ahci,id=ahci \
	-device ide-hd,drive=disk,bus=ahci.0

Then you can scan it with `scsi scan` and access it normally.

To use 4GB of memory, typically necessary for booting Linux distros, add
`-m 4GB`.

64-bit U-Boot
-------------

In addition to the 32-bit 'coreboot' build there is a 'coreboot64' build. This
produces an image which can be booted from coreboot (32-bit). Internally it
works by using a 32-bit SPL binary to switch to 64-bit for running U-Boot. It
can be useful for running UEFI applications, for example with the coreboot
build in `$CBDIR`::

   DISK=ubuntu-23.04-desktop-amd64.iso
   CBDIR=~/coreboot/build

   cp $CBDIR/coreboot.rom.in coreboot.rom
   cbfstool coreboot.rom add-flat-binary -f u-boot-x86-with-spl.bin \
      -n fallback/payload -c lzma -l 0x1110000 -e 0x1110000

   qemu-system-x86_64 -m 2G -smp 4 -bios coreboot.rom \
      -drive id=disk,file=$DISK,if=none \
      -device ahci,id=ahci \
      -device ide-hd,drive=disk,bus=ahci.0 \

This allows booting and installing various distros, many of which are
64-bit-only, so cannot work with the 32-bit 'coreboot' build.

USB keyboard
------------

The `CONFIG_USE_PREBOOT` option is enabled by default, meaning that USB starts
up just before the command-line starts. This allows user interaction on
non-laptop devices which use a USB keyboard.

CBFS access
-----------

You can use the 'cbfs' commands to access the Coreboot filesystem::

   => cbfsinit
   => cbfsinfo

   CBFS version: 0x31313132
   ROM size: 0x100000
   Boot block size: 0x4
   CBFS size: 0xffdfc
   Alignment: 64
   Offset: 0x200

   => cbfsls
        size              type  name
   ------------------------------------------
          32       cbfs header  cbfs master header
       16720                17  fallback/romstage
       53052                17  fallback/ramstage
         398               raw  config
         715               raw  revision
         117               raw  build_info
        4044               raw  fallback/dsdt.aml
         640       cmos layout  cmos_layout.bin
       17804                17  fallback/postcar
      335797           payload  fallback/payload
      607000              null  (empty)
       10752         bootblock  bootblock

   12 file(s)

   =>

Memory map
----------

  ==========  ==================================================================
     Address  Region at that address
  ==========  ==================================================================
    ffffffff  Top of ROM (and last byte of 32-bit address space)
    7a9fd000  Typical top of memory available to U-Boot
              (use cbsysinfo to see where memory range 'table' starts)
    10000000  Memory reserved by coreboot for mapping PCI devices
              (typical size 2151000, includes framebuffer)
     1920000  CONFIG_SYS_CAR_ADDR, fake Cache-as-RAM memory, used during startup
     1110000  CONFIG_TEXT_BASE (start address of U-Boot code, before reloc)
      110000  CONFIG_BLOBLIST_ADDR (before being relocated)
      100000  CONFIG_PRE_CON_BUF_ADDR
       f0000  ACPI tables set up by U-Boot
              (typically redirects to 7ab10030 or similar)
         500  Location of coreboot sysinfo table, used during startup
  ==========  ==================================================================


Debug UART
----------

It is possible to enable the debug UART with coreboot. To do this, use the
info from the cbsysinfo command to locate the UART base. For example::

   => cbsysinfo
   ...
   Serial I/O port: 00000000
      base        : 00000000
      pointer     : 767b51bc
      type        : 2
      base        : fe03e000
      baud        : 0d115200
      regwidth    : 4
      input_hz    : 0d1843200
      PCI addr    : 00000010
   ...

Here you can see that the UART base is fe03e000, regwidth is 4 (1 << 2) and the
input clock is 1843200. So you can add the following CONFIG options::

   CONFIG_DEBUG_UART=y
   CONFIG_DEBUG_UART_BASE=fe03e000
   CONFIG_DEBUG_UART_CLOCK=1843200
   CONFIG_DEBUG_UART_SHIFT=2
   CONFIG_DEBUG_UART_ANNOUNCE=y

coreboot in CI
--------------

CI runs tests using a pre-built coreboot image. This ensures that U-Boot can
boot as a coreboot payload, based on a known-good build of coreboot.

To update the `coreboot.rom` file which is used:

#. Build coreboot with `CONFIG_GENERIC_LINEAR_FRAMEBUFFER=y`. If using
   `make menuconfig`, this is under
   `Devices->Display->Framebuffer mode->Linear "high resolution" framebuffer`.

#. Compress the resulting `coreboot.rom`::

      xz -c /path/to/coreboot/build/coreboot.rom > coreboot.rom.xz

#. Upload the file to Google drive

#. Send a patch to change the file ID used by wget in the CI yaml files.
