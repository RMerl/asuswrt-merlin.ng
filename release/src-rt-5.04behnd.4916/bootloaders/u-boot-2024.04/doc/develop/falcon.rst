.. SPDX-License-Identifier: GPL-2.0-or-later

Falcon Mode
===========

Introduction
------------

This document provides an overview of how to add support for Falcon Mode
to a board.

Falcon Mode is introduced to speed up the booting process, allowing
to boot a Linux kernel (or whatever image) without a full blown U-Boot.

Falcon Mode relies on the SPL framework. In fact, to make booting faster,
U-Boot is split into two parts: the SPL (Secondary Program Loader) and U-Boot
image. In most implementations, SPL is used to start U-Boot when booting from
a mass storage, such as NAND or SD-Card. SPL has now support for other media,
and can generally be seen as a way to start an image performing the minimum
required initialization. SPL mainly initializes the RAM controller, and then
copies U-Boot image into the memory.

The Falcon Mode extends this way allowing to start the Linux kernel directly
from SPL. A new command is added to U-Boot to prepare the parameters that SPL
must pass to the kernel, using ATAGS or Device Tree.

In normal mode, these parameters are generated each time before
loading the kernel, passing to Linux the address in memory where
the parameters can be read.
With Falcon Mode, this snapshot can be saved into persistent storage and SPL is
informed to load it before running the kernel.

To boot the kernel, these steps under a Falcon-aware U-Boot are required:

1. Boot the board into U-Boot.
    After loading the desired legacy-format kernel image into memory (and DT as
    well, if used), use the "spl export" command to generate the kernel
    parameters area or the DT.  U-Boot runs as when it boots the kernel, but
    stops before passing the control to the kernel.

2. Save the prepared snapshot into persistent media.
    The address where to save it must be configured into board configuration
    file (CONFIG_CMD_SPL_NAND_OFS for NAND).

3. Boot the board into Falcon Mode. SPL will load the kernel and copy
    the parameters which are saved in the persistent area to the required
    address. If a valid uImage is not found at the defined location, U-Boot
    will be booted instead.

It is required to implement a custom mechanism to select if SPL loads U-Boot
or another image.

The value of a GPIO is a simple way to operate the selection, as well as
reading a character from the SPL console if CONFIG_SPL_CONSOLE is set.

Falcon Mode is generally activated by setting CONFIG_SPL_OS_BOOT. This tells
SPL that U-Boot is not the only available image that SPL is able to start.

Configuration
-------------

CONFIG_CMD_SPL
    Enable the "spl export" command.
    The command "spl export" is then available in U-Boot mode.

CONFIG_SPL_PAYLOAD_ARGS_ADDR
    Address in RAM where the parameters must be copied by SPL.
    In most cases, it is <start_of_ram> + 0x100.

CONFIG_SYS_NAND_SPL_KERNEL_OFFS
    Offset in NAND where the kernel is stored

CONFIG_CMD_SPL_NAND_OFS
    Offset in NAND where the parameters area was saved.

CONFIG_CMD_SPL_NOR_OFS
    Offset in NOR where the parameters area was saved.

CONFIG_CMD_SPL_WRITE_SIZE
    Size of the parameters area to be copied

CONFIG_SPL_OS_BOOT
    Activate Falcon Mode.

Function that a board must implement
------------------------------------

void spl_board_prepare_for_linux(void)
    optional, called from SPL before starting the kernel

spl_start_uboot()
    required, returns "0" if SPL should start the kernel, "1" if U-Boot
    must be started.

Environment variables
---------------------

A board may chose to look at the environment for decisions about falcon
mode.  In this case the following variables may be supported:

boot_os
    Set to yes/Yes/true/True/1 to enable booting to OS,
    any other value to fall back to U-Boot (including unset)

falcon_args_file
    Filename to load as the 'args' portion of falcon mode rather than the
    hard-coded value.

falcon_image_file
    Filename to load as the OS image portion of falcon mode rather than the
    hard-coded value.

Using spl command
-----------------

spl - SPL configuration

Usage::

    spl export <img=atags|fdt> [kernel_addr] [initrd_addr] [fdt_addr ]

img
    "atags" or "fdt"

kernel_addr
    kernel is loaded as part of the boot process, but it is not started.
    This is the address where a kernel image is stored.

initrd_addr
    Address of initial ramdisk
    can be set to "-" if fdt_addr without initrd_addr is used

fdt_addr
    in case of fdt, the address of the device tree.

The *spl export* command does not write to a storage media. The user is
responsible to transfer the gathered information (assembled ATAGS list
or prepared FDT) from temporary storage in RAM into persistent storage
after each run of *spl export*. Unfortunately the position of temporary
storage can not be predicted nor provided at command line, it depends
highly on your system setup and your provided data (ATAGS or FDT).
However at the end of an successful *spl export* run it will print the
RAM address of temporary storage. The RAM address of FDT will also be
set in the environment variable *fdtargsaddr*, the new length of the
prepared FDT will be set in the environment variable *fdtargslen*.
These environment variables can be used in scripts for writing updated
FDT to persistent storage.

Now the user have to save the generated BLOB from that printed address
to the pre-defined address in persistent storage
(CONFIG_CMD_SPL_NAND_OFS in case of NAND).
The following example shows how to prepare the data for Falcon Mode on
twister board with ATAGS BLOB.

The *spl export* command is prepared to work with ATAGS and FDT. However,
using FDT is at the moment untested. The ppc port (see a3m071 example
later) prepares the fdt blob with the fdt command instead.


Usage on the twister board
--------------------------

Using mtd names with the following (default) configuration
for mtdparts::

    device nand0 <omap2-nand.0>, # parts = 9
     #: name        size        offset      mask_flags
     0: MLO                 0x00080000      0x00000000      0
     1: u-boot              0x00100000      0x00080000      0
     2: env1                0x00040000      0x00180000      0
     3: env2                0x00040000      0x001c0000      0
     4: kernel              0x00600000      0x00200000      0
     5: bootparms           0x00040000      0x00800000      0
     6: splashimg           0x00200000      0x00840000      0
     7: mini                0x02800000      0x00a40000      0
     8: rootfs              0x1cdc0000      0x03240000      0

::

    twister => nand read 82000000 kernel

    NAND read: device 0 offset 0x200000, size 0x600000
    6291456 bytes read: OK

Now the kernel is in RAM at address 0x82000000::

    twister => spl export atags 0x82000000
    ## Booting kernel from Legacy Image at 82000000 ...
       Image Name:   Linux-3.5.0-rc4-14089-gda0b7f4
       Image Type:   ARM Linux Kernel Image (uncompressed)
       Data Size:    3654808 Bytes = 3.5 MiB
       Load Address: 80008000
       Entry Point:  80008000
       Verifying Checksum ... OK
       Loading Kernel Image ... OK
    OK
    cmdline subcommand not supported
    bdt subcommand not supported
    Argument image is now in RAM at: 0x80000100

The result can be checked at address 0x80000100::

    twister => md 0x80000100
    80000100: 00000005 54410001 00000000 00000000    ......AT........
    80000110: 00000000 00000067 54410009 746f6f72    ....g.....ATroot
    80000120: 65642f3d 666e2f76 77722073 73666e20    =/dev/nfs rw nfs

The parameters generated with this step can be saved into NAND at the offset
0x800000 (value for twister for CONFIG_CMD_SPL_NAND_OFS)::

    nand erase.part bootparms
    nand write 0x80000100 bootparms 0x4000

Now the parameters are stored into the NAND flash at the address
CONFIG_CMD_SPL_NAND_OFS (=0x800000).

Next time, the board can be started into Falcon Mode moving the
setting the GPIO (on twister GPIO 55 is used) to kernel mode.

The kernel is loaded directly by the SPL without passing through U-Boot.

Example with FDT: a3m071 board
------------------------------

To boot the Linux kernel from the SPL, the DT blob (fdt) needs to get
prepared/patched first. U-Boot usually inserts some dynamic values into
the DT binary (blob), e.g. autodetected memory size, MAC addresses,
clocks speeds etc. To generate this patched DT blob, you can use
the following command:

1. Load fdt blob to SDRAM::

        => tftp 1800000 a3m071/a3m071.dtb

2. Set bootargs as desired for Linux booting (e.g. flash_mtd)::

        => run mtdargs addip2 addtty

3. Use "fdt" commands to patch the DT blob::

        => fdt addr 1800000
        => fdt boardsetup
        => fdt chosen

4. Display patched DT blob (optional)::

        => fdt print

5. Save fdt to NOR flash::

        => erase fc060000 fc07ffff
        => cp.b 1800000 fc060000 10000
        ...


Falcon Mode was presented at the RMLL 2012. Slides are available at:

http://schedule2012.rmll.info/IMG/pdf/LSM2012_UbootFalconMode_Babic.pdf

Falcon Mode Boot on RISC-V
--------------------------

Introduction
~~~~~~~~~~~~

In the RISC-V environment, OpenSBI is required to enable a supervisor mode
binary to execute certain privileged operations. The typical boot sequence on
RISC-V is SPL -> OpenSBI -> U-Boot -> Linux kernel. SPL will load and start
the OpenSBI initializations, then OpenSBI will bring up the next image, U-Boot
proper. The OpenSBI binary must be prepared in advance of the U-Boot build
process and it will be packed together with U-Boot into a file called
u-boot.itb.

The Falcon Mode on RISC-V platforms is a distinct boot sequence. Borrowing
ideas from the U-Boot Falcon Mode on ARM, it skips the U-Boot proper phase
in the normal boot process and allows OpenSBI to load and start the Linux
kernel. Its boot sequence is SPL -> OpenSBI -> Linux kernel. The OpenSBI
binary and Linux kernel binary must be prepared prior to the U-Boot build
process and they will be packed together as a FIT image named linux.itb in
this process.

CONFIG_SPL_LOAD_FIT_OPENSBI_OS_BOOT enables the Falcon Mode boot on RISC-V.
This configuration setting tells OpenSBI that Linux kernel is its next OS
image and makes it load and start the kernel afterwards.

Note that the Falcon Mode boot bypasses a lot of initializations by U-Boot.
If the Linux kernel expects hardware initializations by U-Boot, make sure to
port the relevant code to the SPL build process.

Configuration
~~~~~~~~~~~~~

CONFIG_SPL_LOAD_FIT_ADDRESS
    Specifies the address to load u-boot.itb in a normal boot. When the Falcon
    Mode boot is enabled, it specifies the load address of linux.itb.

CONFIG_SYS_TEXT_BASE
    Specifies the address of the text section for a u-boot proper in a normal
    boot. When the Falcon Mode boot is enabled, it specifies the text section
    address for the Linux kernel image.

CONFIG_SPL_PAYLOAD_ARGS_ADDR
    The address in the RAM to which the FDT blob is to be moved by the SPL.
    SPL places the FDT blob right after the kernel. As the kernel does not
    include the BSS section in its size calculation, SPL ends up placing
    the FDT blob within the BSS section of the kernel. This may cause the
    FDT blob to be cleared during kernel BSS initialization. To avoid the
    issue, be sure to move the FDT blob out of the kernel first.

CONFIG_SPL_LOAD_FIT_OPENSBI_OS_BOOT
    Activates the Falcon Mode boot on RISC-V.

Example for Andes AE350 Board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

A FDT blob is required to boot the Linux kernel from the SPL. Andes AE350
platforms generally come with a builtin dtb. To load a custom DTB, follow
these steps:

1. Load the custom DTB to SDRAM::

        => fatload mmc 0:1 0x20000000 user_custom.dtb

2. Set the SPI speed::

        => sf probe 0:0 50000000 0

3. Erase sectors from the SPI Flash::

        => sf erase 0xf0000 0x10000

4. Write the FDT blob to the erased sectors of the Flash::

        => sf write 0x20000000 0xf0000 0x10000

Console Log of AE350 Falcon Mode Boot
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

::

        U-Boot SPL 2023.01-00031-g777ecdea66 (Oct 31 2023 - 18:41:36 +0800)
        Trying to boot from RAM

        OpenSBI v1.2-51-g7304e42
           ____                    _____ ____ _____
          / __ \                  / ____|  _ \_   _|
         | |  | |_ __   ___ _ __ | (___ | |_) || |
         | |  | | '_ \ / _ \ '_ \ \___ \|  _ < | |
         | |__| | |_) |  __/ | | |____) | |_) || |_
          \____/| .__/ \___|_| |_|_____/|____/_____|
                | |
                |_|

        Platform Name             : andestech,ax25
        Platform Features         : medeleg
        Platform HART Count       : 1
        Platform IPI Device       : andes_plicsw
        Platform Timer Device     : andes_plmt @ 60000000Hz
        Platform Console Device   : uart8250
        Platform HSM Device       : andes_smu
        Platform PMU Device       : andes_pmu
        Platform Reboot Device    : atcwdt200
        Platform Shutdown Device  : ---
        Firmware Base             : 0x0
        Firmware Size             : 196 KB
        Runtime SBI Version       : 1.0

        Domain0 Name              : root
        Domain0 Boot HART         : 0
        Domain0 HARTs             : 0*
        Domain0 Region00          : 0x0000000000000000-0x000000000003ffff ()
        Domain0 Region01          : 0x00000000e6000000-0x00000000e60fffff (I,R)
        Domain0 Region02          : 0x00000000e6400000-0x00000000e67fffff (I)
        Domain0 Region03          : 0x0000000000000000-0xffffffffffffffff (R,W,X)
        Domain0 Next Address      : 0x0000000001800000
        Domain0 Next Arg1         : 0x0000000001700000
        Domain0 Next Mode         : S-mode
        Domain0 SysReset          : yes

        Boot HART ID              : 0
        Boot HART Domain          : root
        Boot HART Priv Version    : v1.11
        Boot HART Base ISA        : rv64imafdcx
        Boot HART ISA Extensions  : none
        Boot HART PMP Count       : 8
        Boot HART PMP Granularity : 4
        Boot HART PMP Address Bits: 31
        Boot HART MHPM Count      : 4
        Boot HART MHPM Bits       : 64
        Boot HART MIDELEG         : 0x0000000000000222
        Boot HART MEDELEG         : 0x000000000000b109
        [    0.000000] Linux version 6.1.47-09019-g0584b09ad862-dirty
        [    0.000000] OF: fdt: Ignoring memory range 0x0 - 0x1800000
        [    0.000000] Machine model: andestech,ax25
        [    0.000000] earlycon: sbi0 at I/O port 0x0 (options '')
        [    0.000000] printk: bootconsole [sbi0] enabled
        [    0.000000] Disabled 4-level and 5-level paging
        [    0.000000] efi: UEFI not found.
        [    0.000000] Zone ranges:
        [    0.000000]   DMA32    [mem 0x0000000001800000-0x000000003fffffff]
        [    0.000000]   Normal   empty
        [    0.000000] Movable zone start for each node
        [    0.000000] Early memory node ranges
        [    0.000000]   node   0: [mem 0x0000000001800000-0x000000003fffffff]
        [    0.000000] Initmem setup node 0 [mem 0x0000000001800000-0x000000003fffffff]
        [    0.000000] SBI specification v1.0 detected
        [    0.000000] SBI implementation ID=0x1 Version=0x10002
        [    0.000000] SBI TIME extension detected
        [    0.000000] SBI IPI extension detected
        [    0.000000] SBI RFENCE extension detected
        [    0.000000] SBI SRST extension detected
        [    0.000000] SBI HSM extension detected
        [    0.000000] riscv: base ISA extensions acim
        [    0.000000] riscv: ELF capabilities acim
        [    0.000000] percpu: Embedded 18 pages/cpu s35000 r8192 d30536 u73728
        [    0.000000] Built 1 zonelists, mobility grouping on.  Total pages: 252500
