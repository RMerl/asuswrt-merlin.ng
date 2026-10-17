.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Chromebook Samus
================

First, you need the following binary blobs:

   * descriptor.bin - Intel flash descriptor
   * me.bin - Intel Management Engine
   * mrc.bin - Memory Reference Code, which sets up SDRAM
   * refcode.elf - Additional Reference code
   * vga.bin - video ROM, which sets up the display

If you have a samus you can obtain them from your flash, for example, in
developer mode on the Chromebook (use Ctrl-Alt-F2 to obtain a terminal and
log in as 'root')::

   cd /tmp
   flashrom -w samus.bin
   scp samus.bin username@ip_address:/path/to/somewhere

If not see the coreboot tree where you can use::

   bash crosfirmware.sh samus

to get the image. There is also an 'extract_blobs.sh' scripts that you can use
on the 'coreboot-Google_Samus.*' file to short-circuit some of the below.

Then 'ifdtool -x samus.bin' on your development machine will produce::

   flashregion_0_flashdescriptor.bin
   flashregion_1_bios.bin
   flashregion_2_intel_me.bin

Rename flashregion_0_flashdescriptor.bin to descriptor.bin
Rename flashregion_2_intel_me.bin to me.bin
You can ignore flashregion_1_bios.bin - it is not used.

To get the rest, use 'cbfstool samus.bin print'::

   samus.bin: 8192 kB, bootblocksize 2864, romsize 8388608, offset 0x700000
   alignment: 64 bytes, architecture: x86

============================   ========   ===========  ======
Name                           Offset     Type         Size
============================   ========   ===========  ======
cmos_layout.bin                0x700000   cmos_layout  1164
pci8086,0406.rom               0x7004c0   optionrom    65536
spd.bin                        0x710500   (unknown)    4096
cpu_microcode_blob.bin         0x711540   microcode    70720
fallback/romstage              0x722a00   stage        54210
fallback/ramstage              0x72fe00   stage        96382
config                         0x7476c0   raw          6075
fallback/vboot                 0x748ec0   stage        15980
fallback/refcode               0x74cd80   stage        75578
fallback/payload               0x75f500   payload      62878
u-boot.dtb                     0x76eb00   (unknown)    5318
(empty)                        0x770000   null         196504
mrc.bin                        0x79ffc0   (unknown)    222876
(empty)                        0x7d66c0   null         167320
============================   ========   ===========  ======

You can extract what you need::

   cbfstool samus.bin extract -n pci8086,0406.rom -f vga.bin
   cbfstool samus.bin extract -n fallback/refcode -f refcode.rmod
   cbfstool samus.bin extract -n mrc.bin -f mrc.bin
   cbfstool samus.bin extract -n fallback/refcode -f refcode.bin -U

Note that the -U flag is only supported by the latest cbfstool. It unpacks
and decompresses the stage to produce a coreboot rmodule. This is a simple
representation of an ELF file. You need the patch "Support decoding a stage
with compression".

Put all 5 files into board/google/chromebook_samus.

Now you can build U-Boot and obtain u-boot.rom::

   $ make chromebook_samus_defconfig
   $ make all

If you are using em100, then this command will flash write -Boot::

   em100 -s -d filename.rom -c W25Q64CV -r

Flash map for samus / broadwell:

   :fffff800:	SYS_X86_START16
   :ffff0000:	RESET_SEG_START
   :fffd8000:	TPL_TEXT_BASE
   :fffa0000:	X86_MRC_ADDR
   :fff90000:	VGA_BIOS_ADDR
   :ffed0000:	TEXT_BASE
   :ffea0000:	X86_REFCODE_ADDR
   :ffe70000:	SPL_TEXT_BASE
   :ffbf8000:	CONFIG_ENV_OFFSET (environemnt offset)
   :ffbe0000:	rw-mrc-cache (Memory-reference-code cache)
   :ffa00000:	<spare>
   :ff801000:	intel-me (address set by descriptor.bin)
   :ff800000:	intel-descriptor
