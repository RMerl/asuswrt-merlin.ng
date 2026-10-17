.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2015 Google, Inc

U-Boot on EFI
=============
This document provides information about U-Boot running on top of EFI, either
as an application or just as a means of getting U-Boot onto a new platform.


Motivation
----------
Running U-Boot on EFI is useful in several situations:

- You have EFI running on a board but U-Boot does not natively support it
  fully yet. You can boot into U-Boot from EFI and use that until U-Boot is
  fully ported

- You need to use an EFI implementation (e.g. UEFI) because your vendor
  requires it in order to provide support

- You plan to use coreboot to boot into U-Boot but coreboot support does
  not currently exist for your platform. In the meantime you can use U-Boot
  on EFI and then move to U-Boot on coreboot when ready

- You use EFI but want to experiment with a simpler alternative like U-Boot


Status
------
Only x86 is supported at present. If you are using EFI on another architecture
you may want to reconsider. However, much of the code is generic so could be
ported.

U-Boot supports running as an EFI application for both 32- and 64-bit EFI.

U-Boot supports building itself as a payload for either 32-bit or 64-bit EFI.
U-Boot is packaged up and loaded in its entirety by EFI. Once started, U-Boot
changes to 32-bit mode (currently) and takes over the machine. You can use
devices, boot a kernel, etc.


Build Instructions
------------------
First choose a board that has EFI support and obtain an EFI implementation
for that board. It will be either 32-bit or 64-bit. Alternatively, you can
opt for using QEMU [1] and the OVMF [2], as detailed below.

To build U-Boot as an EFI application, enable CONFIG_EFI and CONFIG_EFI_APP.
The efi-x86_app32 and efi-x86_app64 configs are set up for this. Just build
U-Boot as normal, e.g.::

   make efi-x86_app32_defconfig
   make

To build U-Boot as an EFI payload (32-bit or 64-bit EFI can be used), enable
CONFIG_EFI, CONFIG_EFI_STUB, and select either CONFIG_EFI_STUB_32BIT or
CONFIG_EFI_STUB_64BIT. The efi-x86_payload configs (efi-x86_payload32_defconfig
and efi-x86_payload32_defconfig) are set up for this. Then build U-Boot as
normal, e.g.::

   make efi-x86_payload32_defconfig (or efi-x86_payload64_defconfig)
   make

You will end up with one of these files depending on what you build for:

* u-boot-app.efi - U-Boot EFI application
* u-boot-payload.efi  - U-Boot EFI payload application


Trying it out
-------------
QEMU is an emulator and it can emulate an x86 machine. Please make sure your
QEMU version is 6.0.0 or above to test this. You can run the payload with
something like this::

   mkdir /tmp/efi
   cp /path/to/u-boot*.efi /tmp/efi
   qemu-system-x86_64 -pflash edk2-x86_64-code.fd -hda fat:rw:/tmp/efi/

Add -nographic if you want to use the terminal for output. Once it starts
type 'fs0:u-boot-payload.efi' to run the payload or 'fs0:u-boot-app.efi' to
run the application. 'edk2-x86_64-code.fd' is the EFI 'BIOS'. QEMU already
ships both 32-bit and 64-bit EFI BIOS images. For 32-bit EFI 'BIOS' image,
use 'edk2-i386-code.fd'.


To try it on real hardware, put u-boot-app.efi on a suitable boot medium,
such as a USB stick. Then you can type something like this to start it::

   fs0:u-boot-payload.efi

(or fs0:u-boot-app.efi for the application)

This will start the payload, copy U-Boot into RAM and start U-Boot. Note
that EFI does not support booting a 64-bit application from a 32-bit
EFI (or vice versa). Also it will often fail to print an error message if
you get this wrong.

You may find the script `scripts/build-efi.sh` helpful for building and testing
U-Boot on UEFI on QEMU. It also includes links to UEFI binaries dating from
2021.

See `Example run`_ for an example run.

Inner workings
--------------
Here follow a few implementation notes for those who want to fiddle with
this and perhaps contribute patches.

The application and payload approaches sound similar but are in fact
implemented completely differently.

EFI Application
~~~~~~~~~~~~~~~
For the application the whole of U-Boot is built as a shared library. The
efi_main() function is in lib/efi/efi_app.c. It sets up some basic EFI
functions with efi_init(), sets up U-Boot global_data, allocates memory for
U-Boot's malloc(), etc. and enters the normal init sequence (board_init_f()
and board_init_r()).

Since U-Boot limits its memory access to the allocated regions very little
special code is needed. The CONFIG_EFI_APP option controls a few things
that need to change so 'git grep CONFIG_EFI_APP' may be instructive.
The CONFIG_EFI option controls more general EFI adjustments.

The only available driver is the serial driver. This calls back into EFI
'boot services' to send and receive characters. Although it is implemented
as a serial driver the console device is not necessarilly serial. If you
boot EFI with video output then the 'serial' device will operate on your
target devices's display instead and the device's USB keyboard will also
work if connected. If you have both serial and video output, then both
consoles will be active. Even though U-Boot does the same thing normally,
These are features of EFI, not U-Boot.

Very little code is involved in implementing the EFI application feature.
U-Boot is highly portable. Most of the difficulty is in modifying the
Makefile settings to pass the right build flags. In particular there is very
little x86-specific code involved - you can find most of it in
arch/x86/cpu. Porting to ARM (which can also use EFI if you are brave
enough) should be straightforward.

Use the 'reset' command to get back to EFI.

EFI Payload
~~~~~~~~~~~
The payload approach is a different kettle of fish. It works by building
U-Boot exactly as normal for your target board, then adding the entire
image (including device tree) into a small EFI stub application responsible
for booting it. The stub application is built as a normal EFI application
except that it has a lot of data attached to it.

The stub application is implemented in lib/efi/efi_stub.c. The efi_main()
function is called by EFI. It is responsible for copying U-Boot from its
original location into memory, disabling EFI boot services and starting
U-Boot. U-Boot then starts as normal, relocates, starts all drivers, etc.

The stub application is architecture-dependent. At present it has some
x86-specific code and a comment at the top of efi_stub.c describes this.

While the stub application does allocate some memory from EFI this is not
used by U-Boot (the payload). In fact when U-Boot starts it has all of the
memory available to it and can operate as it pleases (but see the next
section).

Tables
~~~~~~
The payload can pass information to U-Boot in the form of EFI tables. At
present this feature is used to pass the EFI memory map, an inordinately
large list of memory regions. You can use the 'efi mem all' command to
display this list. U-Boot uses the list to work out where to relocate
itself.

Although U-Boot can use any memory it likes, EFI marks some memory as used
by 'run-time services', code that hangs around while U-Boot is running and
is even present when Linux is running. This is common on x86 and provides
a way for Linux to call back into the firmware to control things like CPU
fan speed. U-Boot uses only 'conventional' memory, in EFI terminology. It
will relocate itself to the top of the largest block of memory it can find
below 4GB.

Interrupts
~~~~~~~~~~
U-Boot drivers typically don't use interrupts. Since EFI enables interrupts
it is possible that an interrupt will fire that U-Boot cannot handle. This
seems to cause problems. For this reason the U-Boot payload runs with
interrupts disabled at present.

32/64-bit
~~~~~~~~~
While the EFI application can be built as either 32- or 64-bit, you need to be
careful to build the correct one so that your UEFI firmware can start it. Most
UEFI images are 64-bit at present.

The payload stub can be build as either 32- or 64-bits. Only a small amount
of code is built this way (see the extra- line in lib/efi/Makefile).
Everything else is built as a normal U-Boot, so is always 32-bit on x86 at
present.

Example run
-----------

This shows running with serial enabled (see `include/configs/efi-x86_app.h`)::

   $ scripts/build-efi.sh -wsPr
   Packaging efi-x86_app32
   Running qemu-system-i386

   BdsDxe: failed to load Boot0001 "UEFI QEMU HARDDISK QM00005 " from PciRoot(0x0)/Pci(0x3,0x0)/Sata(0x0,0xFFFF,0x0): Not Found
   BdsDxe: loading Boot0002 "EFI Internal Shell" from Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)
   BdsDxe: starting Boot0002 "EFI Internal Shell" from Fv(7CB8BDC9-F8EB-4F34-AAEA-3EE4AF6516A1)/FvFile(7C04A583-9E3E-4F1C-AD65-E05268D0B4D1)

   UEFI Interactive Shell v2.2
   EDK II
   UEFI v2.70 (EDK II, 0x00010000)
   Mapping table
         FS0: Alias(s):HD0a65535a1:;BLK1:
             PciRoot(0x0)/Pci(0x3,0x0)/Sata(0x0,0xFFFF,0x0)/HD(1,GPT,0FFD5E61-3B0C-4326-8049-BDCDC910AF72,0x800,0xB000)
        BLK0: Alias(s):
             PciRoot(0x0)/Pci(0x3,0x0)/Sata(0x0,0xFFFF,0x0)

   Press ESC in 5 seconds to skip startup.nsh or any other key to continue.
   Shell> fs0:u-boot-app.efi
   U-Boot EFI App (using allocated RAM address 47d4000) key=8d4, image=06a6f610
   starting


   U-Boot 2022.01-rc4 (Sep 19 2021 - 14:03:20 -0600)

   CPU: x86, vendor Intel, device 663h
   DRAM:  32 MiB
    0: efi_media_0  PciRoot(0x0)/Pci(0x3,0x0)/Sata(0x0,0xFFFF,0x0)
    1: <partition>  PciRoot(0x0)/Pci(0x3,0x0)/Sata(0x0,0xFFFF,0x0)/HD(1,GPT,0FFD5E61-3B0C-4326-8049-BDCDC910AF72,0x800,0xB000)
   Loading Environment from nowhere... OK
   Model: EFI x86 Application
   Hit any key to stop autoboot:  0

   Partition Map for EFI device 0  --   Partition Type: EFI

   Part    Start LBA       End LBA            Name
           Attributes
           Type GUID
           Partition GUID
     1     0x00000800      0x0000b7ff      "boot"
           attrs:  0x0000000000000000
           type:   ebd0a0a2-b9e5-4433-87c0-68b6b72699c7
           guid:   0ffd5e61-3b0c-4326-8049-bdcdc910af72
          19   startup.nsh
      528384   u-boot-app.efi
       10181   NvVars

   3 file(s), 0 dir(s)

   => QEMU: Terminated

Run on VirtualBox (x86_64)
--------------------------

Enable EFI
~~~~~~~~~~
At settings for virtual machine the flag at **System->Motherboard->Enable EFI
(special OSes only)** has to be enabled.

Installation
~~~~~~~~~~~~
Provide the preinstalled Linux system as a Virtual Disk Image (VDI) and assign
it to a SATA controller (type AHCI) using the settings for the virtual machine
at menu item **System->Storage->Controller:SATA**.

For the following description three GPT partitions are assumed:

- Partition 1: formatted as FAT file-system and marked as EFI system partition
  (partition type 0xEF00) used for the U-Boot EFI binary. (If VirtualBox is UEFI
  compliant, it should recognize the ESP as the boot partition.)

- Partition 2: formatted as **ext4**, used for root file system

Create an extlinux.conf or a boot script
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Following files are assumed to be located at system for boot configuration::

 Partition  File                    Comment
 1          EFI/BOOT/BOOTX64.efi    # renamed U-Boot EFI image
 1          Image                   # Linux image
 1          Initrd                  # Initramfs of Linux

**EFI/BOOT/BOOTX64.efi** is a renamed build result **u-boot-payload.efi**, built with
**efi-x86_payload64_defconfig** configuration.

Boot script
~~~~~~~~~~~

The boot script **boot.scr** is assumed to be located at::

 Partition  File        Comment
 1          boot.scr    # Boot script, generated with mkimage from template

Content of **boot.scr**:

.. code-block:: bash

  ext4load ${devtype} ${devnum}:${distro_bootpart} ${kernel_addr_r} ${prefix}Image
  setenv kernel_size ${filesize}
  ext4load ${devtype} ${devnum}:${distro_bootpart} ${ramdisk_addr_r} ${prefix}Initrd
  setenv initrd_size ${filesize}
  zboot  ${kernel_addr_r} ${kernel_size} ${ramdisk_addr_r} ${initrd_size}

Extlinux configuration
~~~~~~~~~~~~~~~~~~~~~~

Alternatively a configuration **extlinux.conf** can be used. **extlinux.conf**
is assumed to be located at::

 Partition  File                        Comment
 1          extlinux/extlinux.conf      # Extlinux boot configuration

Content of **extlinux.conf**:

.. code-block:: bash

  default l0
  menu title U-Boot menu
  prompt 0
  timeout 50

  label l0
    menu label Linux
    linux /Image
    initrd /Initrd


Additionally something like (sda is assumed as disk device):

.. code-block:: bash

	append  root=/dev/sda2 console=tty0 console=ttyS0,115200n8 rootwait rw



Future work
-----------
This work could be extended in a number of ways:

- Add ARM support

- Figure out how to solve the interrupt problem

- Add more drivers to the application side (e.g.USB, environment access).

- Avoid turning off boot services in the stub. Instead allow U-Boot to make
  use of boot services in case it wants to. It is unclear what it might want
  though. It is better to use the app.

Where is the code?
------------------
lib/efi
	payload stub, application, support code. Mostly arch-neutral

arch/x86/cpu/efi
	x86 support code for running as an EFI application and payload

board/efi/efi-x86_app/efi.c
	x86 board code for running as an EFI application

board/efi/efi-x86_payload
	generic x86 EFI payload board support code

common/cmd_efi.c
	the 'efi' command

--
Ben Stoltz, Simon Glass
Google, Inc
July 2015

* [1] http://www.qemu.org
* [2] https://github.com/tianocore/tianocore.github.io/wiki/OVMF
