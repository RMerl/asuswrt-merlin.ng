.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2020 Google LLC
.. sectionauthor:: Simon Glass <sjg@chromium.org>


Running U-Boot with Chromium OS verified boot
=============================================

Note: Once you use the source below you can obtain extra documentation with
'make htmldocs'. See the 'Internal Documentation' link, under
'Chromium OS-specific doc'.

To obtain::

   git clone https://github.com/sjg20/u-boot.git
   cd u-boot
   git checkout cros-2021.04

   cd ..
   git clone https://chromium.googlesource.com/chromiumos/platform/vboot_reference
   cd vboot_reference
   git checkout 45964294
   #  futility: updater: Correct output version for Snow

To build for sandbox::

   UB=/tmp/b/chromeos_sandbox    # U-Boot build directory
   cd u-boot
   make O=$UB chromeos_sandbox_defconfig
   make O=$UB -j20 -s VBOOT_SOURCE=/path/to/vboot_reference \
     MAKEFLAGS_VBOOT=DEBUG=1 QUIET=1

Replace sandbox with another supported target.

This produces $UB/image.bin which contains the firmware binaries in a SPI
flash image.

To run on sandbox::

   CROS=~/cosarm
   IMG=$CROS/src/build/images/coral/latest/chromiumos_image.bin
   $UB/tpl/u-boot-tpl -d $UB/u-boot.dtb.out \
     -L6 -c "host bind 0 $IMG; vboot go auto" \
     -l -w -s state.dtb -r -n -m $UB/ram

   $UB/tpl/u-boot-tpl -d $UB/u-boot.dtb.out -L6 -l \
     -c "host bind 0 $IMG; vboot go auto" -w -s $UB/state.dtb -r -n -m $UB/mem


To run on other boards:

   - Install image.bin in the SPI flash of your device
   - Boot your system


Sandbox
-------

Most Chromium OS development with U-Boot is undertaken using sandbox. There is
a sandbox target available (chromeos_sandbox) which allows running U-Boot on
a Linux machine completion with emulations of the display, TPM, disk, etc.

Running sandbox starts TPL, which contains the first phase of vboot, providing
a device tree and binding a Chromium OS disk image for use to find kernels
(any Chromium OS image will do). It also saves driver state between U-Boot
phases into state.dtb and will automatically ensure that memory is shared
between all phases. TPL will jump to SPL and then on to U-Boot proper.

It is possible to run with debugging on, e.g.::

   gdb --args $UB/tpl/u-boot-tpl -d ....

Breakpoints can be set in any U-Boot phase. Overall this is a good debugging
environment for new verified-boot features.


Samus
-----

Basic support is available for samus, using the chromeos_samus target. If you
have an em100, use::

   sudo em100 -s -c W25Q128FW -d $UB/image.bin -t -r

to write the image and then boot samus (Power-Refresh).


Boot flow
---------

Verified boot starts in TPL, which selects the A or B SPL, which in turn selects
the A or B U-Boot. Then this jumps to the selected kernel. If anything goes
wrong, the device reboots and the recovery SPL and U-Boot are used instead.

More details are available here:

   https://www.chromium.org/chromium-os/chromiumos-design-docs/firmware-boot-and-recovery


New uclasses
------------

Several uclasses are provided in cros/:

UCLASS_CROS_AUX_FW
   Chrome OS auxiliary firmware

UCLASS_CROS_FWSTORE
   Chrome OS firmware storage

UCLASS_CROS_NVDATA
   Chrome OS non-volatile data device

UCLASS_CROS_VBOOT_EC
   Chrome OS vboot EC operations

UCLASS_CROS_VBOOT_FLAG
   Chrome OS verified boot flag

The existing UCLASS_CROS_EC is also used.


Commands
--------

A new 'vboot' command is provided to run particular vboot stages. The most
useful command is 'vboot go auto', which continues where the last stage left
off.

Note that TPL and SPL do not supports commands as yet, so the vboot code is
called directly from the SPL boot devices (BOOT_DEVICE_CROS_VBOOT). See
cros_load_image_tpl() and cros_load_image_spl() which both call
vboot_run_auto().


Config options
--------------

The main option is CONFIG_CHROMEOS, which enables a wide array of other options
so that the required features are present.


Device-tree config
------------------

Various options are available which control the operation of verified boot.
See cros/dts/bindings/config.txt for details. Most config is handled at run-
time, although build-time config (with Kconfig) could also be added fairly
easily.


Porting to other hardware
-------------------------

A basic port to samus (Chromebook Pixel 2015) is in a basic working state,
using the chromeos_samus target. Patches will likely be forthcoming in early
2019. Ports to an ARM board and coreboot (for x86 Chromebooks) are in the
dreaming state.


Tests
-----

Chromium OS firmware has a very limited set of tests. The tests that originally
existed in U-Boot were not brought over to coreboot or depthcharge.

The U-Boot tests ('make check') do operate, but at present there are no
Chromium OS tests available. These will hopefully come together over time. Of
course the above sandbox feature provides a sort of functional test and can
detect problems that affect the flow or particular vboot features.


U-Boot without Chromium OS verified boot
----------------------------------------

The following script can be used to boot a Chrome OS image on coral. It is
defined as the boot command in mainline::

   # Read the image header and obtain the address of the kernel
   # The offset 4f0 is defined by verified boot and may change for other
   # Chromebooks
   read mmc 2:2 100000 0 80; setexpr loader *001004f0;

   # Get the kernel size and calculate the number of blocks (0x200 bytes each)
   setexpr size *00100518; setexpr blocks $size / 200;

   # Read the full kernel and calculate the address of the setup block
   read mmc 2:2 100000 80 $blocks; setexpr setup $loader - 1000;

   # Locate the command line
   setexpr cmdline $loader - 2000;

   # Start the zboot process with the loaded kernel, setup block and cmdline
   zboot start 100000 0 0 0 $setup $cmdline;

   # Load the kernel, fix up the 'setup' block, dump information
   zboot load; zboot setup; zboot dump

   # Boot into Chrome OS
   zboot go


7 October 2018
