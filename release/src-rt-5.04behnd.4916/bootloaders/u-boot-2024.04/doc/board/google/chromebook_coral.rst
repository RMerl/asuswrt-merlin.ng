.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Simon Glass <sjg@chromium.org>

Chromebook Coral
================

Coral is a Chromebook (or really about 20 different Chromebooks) which use the
Intel Apollo Lake platform (APL). The 'reef' Chromebooks use the same APL SoC so
should also work. Some later ones based on Glacier Lake (GLK) need various
changes in GPIOs, etc. but are very similar.

It is hoped that this port can enable ports to embedded APL boards which are
starting to appear.

Note that booting U-Boot on APL is already supported by coreboot and
Slim Bootloader. This documentation refers to a 'bare metal' port.


Building
--------

First, you need the following binary blobs:

   * descriptor.bin - Intel flash descriptor
   * fitimage.bin - Base flash image structure
   * fsp_m.bin - FSP-M, for setting up SDRAM
   * fsp_s.bin - FSP-S, for setting up Silicon
   * vbt.bin - for setting up display

These binaries do not seem to be available publicly. If you have a ROM image,
such as santa.bin then you can do this::

  cbfstool santa.bin extract -n fspm.bin -f fsp-m.bin
  cbfstool santa.bin extract -n fsps.bin -f fsp-s.bin
  cbfstool santa.bin extract -n vbt-santa.bin -f vbt.bin
  mkdir tmp
  cd tmp
  dump_fmap -x ../santa.bin
  mv SI_DESC ../descriptor.bin
  mv IFWI ../fitimage.bin

Put all of these files in `board/google/chromebook_coral` so they can be found
by the build.

To build::

  make O=/tmp/b/chromebook_coral chromebook_coral_defconfig
  make O=/tmp/b/chromebook_coral -s -j30 all

That should produce `/tmp/b/chrombook_coral/u-boot.rom` which you can use with
a Dediprog em100::

  em100 -s -c w25q128fw -d /tmp/b/chromebook_coral/u-boot.rom -r

or you can use flashrom to write it to the board. If you do that, make sure you
have a way to restore the old ROM without booting the board. Otherwise you may
brick it. Having said that, you may find these instructions useful if you want
to unbrick your device:

  https://chromium.googlesource.com/chromiumos/platform/ec/+/cr50_stab/docs/case_closed_debugging.md

You can buy Suzy-Q from Sparkfun:

  https://chromium.googlesource.com/chromiumos/third_party/hdctools/+/main/docs/ccd.md#suzyq-suzyqable

Note that it will hang at the SPL prompt for 21 seconds. When booting into
Chrome OS it will always select developer mode, so will wipe anything you have
on the device if you let it proceed. You have two seconds in U-Boot to stop the
auto-boot prompt and several seconds at the 'developer wipe' screen to stop it
wiping the disk.

Here is the console output::

  U-Boot TPL 2021.04-rc1-00128-g344eefcdfec-dirty (Feb 11 2021 - 20:13:08 -0700)
  Trying to boot from Mapped SPI

  U-Boot SPL 2021.04-rc1-00128-g344eefcdfec-dirty (Feb 11 2021 - 20:13:08 -0700)
  Trying to boot from Mapped SPI


  U-Boot 2021.04-rc1-00128-g344eefcdfec-dirty (Feb 11 2021 - 20:13:08 -0700)

  CPU:   Intel(R) Celeron(R) CPU N3450 @ 1.10GHz
  DRAM:  3.9 GiB
  MMC:   sdmmc@1b,0: 1, emmc@1c,0: 2
  Video: 1024x768x32 @ b0000000
  Model: Google Coral
  Net:   No ethernet found.
  SF: Detected w25q128fw with page size 256 Bytes, erase size 4 KiB, total 16 MiB
  Hit any key to stop autoboot:  0
  cmdline=console= loglevel=7 init=/sbin/init cros_secure oops=panic panic=-1 root=PARTUUID=${uuid}/PARTNROFF=1 rootwait rw dm_verity.error_behavior=3 dm_verity.max_bios=-1 dm_verity.dev_wait=0 dm="1 vroot none rw 1,0 3788800 verity payload=ROOT_DEV hashtree=HASH_DEV hashstart=3788800 alg=sha1 root_hexdigest=55052b629d3ac889f25a9583ea12cdcd3ea15ff8 salt=a2d4d9e574069f4fed5e3961b99054b7a4905414b60a25d89974a7334021165c" noinitrd vt.global_cursor_default=0 kern_guid=${uuid} add_efi_memmap noresume i915.modeset=1 Kernel command line: "console= loglevel=7 init=/sbin/init cros_secure oops=panic panic=-1 root=PARTUUID=35c775e7-3735-d745-93e5-d9e0238f7ed0/PARTNROFF=1 rootwait rw dm_verity.error_behavior=3 dm_verity.max_bios=-1 dm_verity.dev_wait=0 dm="1 vroot none rw 1,0 3788800 verity payload=ROOT_DEV hashtree=HASH_DEV hashstart=3788800 alg=sha1 root_hexdigest=55052b629d3ac889f25a9583ea12cdcd3ea15ff8 salt=a2d4d9e574069f4fed5e3961b99054b7a4905414b60a25d89974a7334021165c" noinitrd vt.global_
  Setup located at 00090000:

  ACPI RSDP addr      : 7991f000
  E820: 14 entries
                Addr              Size  Type
      d0000000     1000000  <NULL>
             0       a0000  RAM
         a0000       60000  Reserved
      7b000000      800000  Reserved
      7b800000     4800000  Reserved
      7ac00000      400000  Reserved
        100000     ff00000  RAM
      10000000     2151000  Reserved
      12151000    68aaf000  RAM
     100000000    80000000  RAM
      e0000000    10000000  Reserved
      7991bfd0     12e4030  Reserved
      d0000000    10000000  Reserved
      fed10000        8000  Reserved
  Setup sectors       : 1e
  Root flags          : 1
  Sys size            : 63420
  RAM size            : 0
  Video mode          : ffff
  Root dev            : 0
  Boot flag           : 0
  Jump                : 66eb
  Header              : 53726448
                        Kernel V2
  Version             : 20d
  Real mode switch    : 0
  Start sys           : 1000
  Kernel version      : 38cc
     @00003acc:
  Type of loader      : 80
                        U-Boot, version 0
  Load flags          : 81
                      : loaded-high can-use-heap
  Setup move size     : 8000
  Code32 start        : 100000
  Ramdisk image       : 0
  Ramdisk size        : 0
  Bootsect kludge     : 0
  Heap end ptr        : 8e00
  Ext loader ver      : 0
  Ext loader type     : 0
  Command line ptr    : 99000
     console= loglevel=7 init=/sbin/init cros_secure oops=panic panic=-1 root=PARTUUID=35c775e7-3735-d745-93e5-d9e0238f7ed0/PARTNROFF=1 rootwait rw dm_verity.error_behavior=3 dm_verity.max_bios=-1 dm_verity.dev_wait=0 dm="1 vroot none rw 1,0 3788800 verity payload=ROOT_DEV hashtree=HASH_DEV hashstart=3788800 alg=sha1 root_hexdigest=55052b629d3ac889f25a9583ea12cdcd3ea15ff8 salt=a2d4d9e574069f4fed5e3961b99054b7a4905414b60a25d89974a7334021165c" noinitrd vt.global_cursor_default=0 kern_guid=35c775e7-3735-d745-93e5-d9e0238f7ed0 add_efi_memmap noresume i915.modeset=1 tpm_tis.force=1 tpm_tis.interrupts=0 nmi_watchdog=panic,lapic disablevmx=off
  Initrd addr max     : 7fffffff
  Kernel alignment    : 200000
  Relocatable kernel  : 1
  Min alignment       : 15
                      : 200000
  Xload flags         : 3
                      : 64-bit-entry can-load-above-4gb
  Cmdline size        : 7ff
  Hardware subarch    : 0
  HW subarch data     : 0
  Payload offset      : 26e
  Payload length      : 612045
  Setup data          : 0
  Pref address        : 1000000
  Init size           : 1383000
  Handover offset     : 0

  Starting kernel ...

  Timer summary in microseconds (17 records):
         Mark    Elapsed  Stage
            0          0  reset
      155,279    155,279  TPL
      237,088     81,809  end phase
      237,533        445  SPL
      816,456    578,923  end phase
      817,357        901  board_init_f
    1,061,751    244,394  board_init_r
    1,402,435    340,684  id=64
    1,430,071     27,636  main_loop
    5,532,057  4,101,986  start_kernel

  Accumulated time:
                     685  dm_r
                   2,817  fast_spi
                  33,095  dm_spl
                  52,468  dm_f
                 208,242  fsp-m
                 242,221  fsp-s
                 332,710  mmap_spi


Boot flow - TPL
---------------

Apollo Lake boots via an IFWI (Integrated Firmware Image). TPL is placed in
this, in the IBBL entry.

On boot, an on-chip microcontroller called the CSE (Converged Security Engine)
sets up some SDRAM at ffff8000 and loads the TPL image to that address. The
SRAM extends up to the top of 32-bit address space, but the last 2KB is the
start16 region, so the TPL image must be 30KB at most, and CONFIG_TPL_TEXT_BASE
must be ffff8000. Actually the start16 region is small and it could probably
move from f800 to fe00, providing another 1.5KB, but TPL is only about 19KB so
there is no need to change it at present. The size limit is enforced by
CONFIG_TPL_SIZE_LIMIT to avoid producing images that won't boot.

TPL (running from start.S) first sets up CAR (Cache-as-RAM) which provides
larger area of RAM for use while booting. CAR is mapped at CONFIG_SYS_CAR_ADDR
(fef00000) and is 768KB in size. It then sets up the stack in the botttom 64KB
of this space (i.e. below fef10000). This means that the stack and early
malloc() region in TPL can be 64KB at most.

TPL operates without CONFIG_TPL_PCI enabled so PCI config access must use the
x86-specific functions pci_x86_write_config(), etc. SPL creates a simple-bus
device so that PCI devices are bound by driver model. Then arch_cpu_init_tpl()
is called to early init on various devices. This includes placing PCI devices
at hard-coded addresses in the memory map. PCI auto-config is not used.

Most of the 16KB ROM is mapped into the very top of memory, except for the
Intel descriptor (first 4KB) and the space for SRAM as above.

TPL does not set up a bloblist since at present it does not have anything to
pass to SPL.

Once TPL is done it loads SPL from ROM using either the memory-mapped SPI or by
using the Intel fast SPI driver. SPL is loaded into CAR, at the address given
by CONFIG_SPL_TEXT_BASE, which is normally fef10000.

Note that booting using the SPI driver results in an TPL image that is about
26KB in size instead of 19KB. Also boot speed is worse by about 340ms. If you
really want to use the driver, enable CONFIG_APL_SPI_FLASH_BOOT and set
BOOT_FROM_FAST_SPI_FLASH to true[2].


Boot flow - SPL
---------------

SPL (running from start_from_tpl.S) continues to use the same stack as TPL.
It calls arch_cpu_init_spl() to set up a few devices, then init_dram() loads
the FSP-M binary into CAR and runs to, to set up SDRAM. The address of the
output 'HOB' list (Hand-off-block) is stored into gd->arch.hob_list for parsing.
There is a 2GB chunk of SDRAM starting at 0 and the rest is at 4GB.

PCI auto-config is not used in SPL either, but CONFIG_SPL_PCI is defined, so
proper PCI access is available and normal dm_pci_read_config() calls can be
used. However PCI auto-config is not used so the same static memory mapping set
up by TPL is still active.

SPL on x86 always runs with CONFIG_SPL_SEPARATE_BSS=y and BSS is at 120000
(see u-boot-spl.lds). This works because SPL doesn't access BSS until after
board_init_r(), as per the rules, and DRAM is available then.

SPL sets up a bloblist and passes the SPL hand-off information to U-Boot proper.
This includes a pointer to the HOB list as well as DRAM information. See
struct arch_spl_handoff. The bloblist address is set by CONFIG_BLOBLIST_ADDR,
normally 100000.

SPL uses SPI flash to update the MRC caches in ROM. This speeds up subsequent
boots. Be warned that SPL can take 30 seconds without this cache! This is a
known issue with Intel SoCs with modern DRAM and apparently cannot be improved.
The MRC caches are used to work around this.

Once SPL is finished it loads U-Boot into SDRAM at CONFIG_TEXT_BASE, which
is normally 1110000. Note that CAR is still active.


Boot flow - U-Boot pre-relocation
---------------------------------

U-Boot (running from start_from_spl.S) starts running in RAM and uses the same
stack as SPL. It does various init activities before relocation. Notably
fsp_setup_pinctrl() sets up the pin muxing for the chip using a very large table
in the device tree.

PCI auto-config is not used before relocation, but CONFIG_PCI of course is
defined, so proper PCI access is available. The same static memory mapping set
up by TPL is still active until relocation.

As per usual, U-Boot allocates memory at the top of available RAM (a bit below
2GB in this case) and copies things there ready to relocate itself. Notably
reserve_arch() does not reserve space for the HOB list returned by FSP-M since
this is already located in RAM.

U-Boot then shuts down CAR and jumps to its relocated version.


Boot flow - U-Boot post-relocation
----------------------------------

U-Boot starts up normally, running near the top of RAM. After driver model is
running, arch_fsp_init_r() is called which loads and runs the FSP-S binary.
This updates the HOB list to include graphics information, used by the fsp_video
driver.

PCI autoconfig is done and a few devices are probed to complete init. Most
others are started only when they are used.

Note that FSP-S is supposed to run after CAR has been shut down, which happens
immediately before U-Boot starts up in its relocated position. Therefore we
cannot run FSP-S before relocation. On the other hand we must run it before
PCI auto-config is done, since FSP-S may show or hide devices. The first device
that probes PCI after relocation is the serial port, in initr_serial(), so FSP-S
must run before that. A corollary is that loading FSP-S must be done without
using the SPI driver, to avoid probing PCI and causing an autoconfig, so
memory-mapped reading is always used for FSP-S.

It would be possible to tear down CAR in SPL instead of U-Boot. The SPL handoff
information could make sure it does not include any pointers into CAR (in fact
it doesn't). But tearing down CAR in U-Boot allows the initial state used by TPL
and SPL to be read by U-Boot, which seems useful. It also matches how older
platforms start up (those that don't use SPL).


Performance
-----------

Bootstage is used through all phases of U-Boot to keep accurate timimgs for
boot. Use 'bootstage report' in U-Boot to see the report, e.g.::

    Timer summary in microseconds (16 records):
           Mark    Elapsed  Stage
              0          0  reset
        155,325    155,325  TPL
        204,014     48,689  end TPL
        204,385        371  SPL
        738,633    534,248  end SPL
        739,161        528  board_init_f
        842,764    103,603  board_init_r
      1,166,233    323,469  main_loop
      1,166,283         50  id=175

    Accumulated time:
                        62  fast_spi
                       202  dm_r
                     7,779  dm_spl
                    15,555  dm_f
                   208,357  fsp-m
                   239,847  fsp-s
                   292,143  mmap_spi

CPU performance is about 3500 DMIPS::

    => dhry
    1000000 iterations in 161 ms: 6211180/s, 3535 DMIPS


Partial memory map
------------------

::

    ffffffff       Top of ROM (and last byte of 32-bit address space)
    ffff8000       TPL loaded here (from IFWI)
    ff000000       Bottom of ROM
    fefc0000       Top of CAR region
    fef96000       Stack for FSP-M
    fef40000 59000 FSP-M (also VPL loads here)
    fef11000       SPL loaded here
    fef10000       CONFIG_BLOBLIST_ADDR
    fef10000       Stack top in TPL, SPL and U-Boot before relocation
    fef00000  1000 CONFIG_BOOTSTAGE_STASH_ADDR
    fef00000       Base of CAR region

       30000       AP_DEFAULT_BASE (used to start up additional CPUs)
       f0000       CONFIG_ROM_TABLE_ADDR
      120000       BSS (defined in u-boot-spl.lds)
      200000       FSP-S (which is run after U-Boot is relocated)
     1110000       CONFIG_TEXT_BASE


Speeding up SPL for development
-------------------------------

The 21-second wait for memory training is annoying during development, since
every new image incurs this cost when booting. There is no cache to fall back on
since that area of the image is empty on start-up.

You can add suitable cache contents to the image to fix this, for development
purposes only, like this::

   # Read the image back after booting through SPL
   em100 -s -c w25q128fw -u image.bin

   # Extract the two cache regions
   binman extract -i image.bin extra *cache

   # Move them into the source directory
   mv *cache board/google/chromebook_coral

Then add something like this to the devicetree::

  #if IS_ENABLED(CONFIG_HAVE_MRC) || IS_ENABLED(CONFIG_FSP_VERSION2)
     /* Provide initial contents of the MRC data for faster development */
     rw-mrc-cache {
        type = "blob";
        /* Mirror the offset in spi-flash@0 */
        offset = <0xff8e0000>;
        size = <0x10000>;
        filename = "board/google/chromebook_coral/rw-mrc-cache";
     };
     rw-var-mrc-cache {
        type = "blob";
        size = <0x1000>;
        filename = "board/google/chromebook_coral/rw-var-mrc-cache";
     };
  #endif

This tells binman to put the cache contents in the same place as the
`rw-mrc-cache` and `rw-var-mrc-cache` regions defined by the SPI-flash driver.


Supported peripherals
---------------------

The following have U-Boot drivers:

   - UART
   - SPI flash
   - Video
   - MMC (dev 0) and micro-SD (dev 1)
   - Chrome OS EC
   - Cr50 (security chip)
   - Keyboard
   - USB


To do
-----

- Finish peripherals
   - Sound (Intel I2S support exists, but need da7219 driver)
- Use FSP-T binary instead of our own CAR implementation
- Use the official FSP package instead of the coreboot one
- Suspend / resume
- Fix MMC which seems to try to read even though the card is empty
- Fix USB3 crash "WARN halted endpoint, queueing URB anyway."


Credits
-------

This is a spare-time project conducted slowly over a long period of time.

Much of the code for this port came from Coreboot, an open-source firmware
project similar to U-Boot's SPL in terms of features.

Also see [2] for information about the boot flow used by coreboot. It is
similar, but has an extra postcar stage. U-Boot doesn't need this since it
supports relocating itself in memory.


[2] Intel PDF https://www.coreboot.org/images/2/23/Apollolake_SoC.pdf
