.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2013, Miao Yan <miao.yan@windriver.com>
.. Copyright (C) 2015-2018, Bin Meng <bmeng.cn@gmail.com>
.. Copyright (C) 2019, Lihua Zhao <lihua.zhao@windriver.com>

VxWorks
=======

This document describes the information about U-Boot loading VxWorks kernel.

Status
------
U-Boot supports loading VxWorks kernels via 'bootvx' and 'bootm' commands.
For booting old kernels (6.9.x) on PowerPC and ARM, and all kernel versions
on other architectures, 'bootvx' shall be used. For booting VxWorks 7 kernels
on PowerPC/ARM/RISC-V, 'bootm' shall be used.

With CONFIG_EFI_LOADER option, it's possible to chain load a VxWorks x86 kernel
via the UEFI boot loader application for VxWorks loaded by 'bootefi' command.

VxWorks 7 on PowerPC/ARM/RISC-V
-------------------------------
From VxWorks 7, VxWorks starts adopting device tree as its hardware description
mechanism (for PowerPC and ARM), thus requiring boot interface changes.
This section will describe the new interface.

Since VxWorks 7 SR0640 release, VxWorks starts using Linux compatible standard
DTB for some boards. With that, the exact same bootm flow as used by Linux is
used, which includes board-specific DTB fix up. To keep backward compatibility,
only when the least significant bit of flags in bootargs is set, the standard
DTB will be used. Otherwise it falls back to the legacy bootm flow.

For legacy bootm flow, make sure the least significant bit of flags in bootargs
is cleared. The calling convention is described below:

For PowerPC, the calling convention of the new VxWorks entry point conforms to
the ePAPR standard, which is shown below (see ePAPR for more details):

.. code-block:: c

    void (*kernel_entry)(fdt_addr, 0, 0, EPAPR_MAGIC, boot_IMA, 0, 0)

For ARM, the calling convention is shown below:

.. code-block:: c

    void (*kernel_entry)(void *fdt_addr)

When using the Linux compatible standard DTB, the calling convention of VxWorks
entry point is exactly the same as the Linux kernel.

For RISC-V, there is no legacy bootm flow as VxWorks always uses the same boot
interface as the Linux kernel, with the calling convention below::

    void (*kernel_entry)(unsigned long hartid, void *fdt_addr)

When booting a VxWorks 7 kernel (uImage format), the parameters passed to bootm
is like below::

    bootm <kernel image address> - <device tree address>

VxWorks bootline
----------------
When using 'bootvx', the kernel bootline must be prepared by U-Boot at a
board-specific address before loading VxWorks. U-Boot supplies its address
via "bootaddr" environment variable. To check where the bootline should be
for a specific board, go to the VxWorks BSP for that board, and look for a
parameter called BOOT_LINE_ADRS. Assign its value to "bootaddr". A typical
value for "bootaddr" on an x86 board is 0x101200.

If a "bootargs" variable is defined, its content will be copied to the memory
location pointed by "bootaddr" as the kernel bootline. If "bootargs" is not
there, command 'bootvx' can construct a valid bootline using the following
environments variables: bootdev, bootfile, ipaddr, netmask, serverip,
gatewayip, hostname, othbootargs.

When using 'bootm', just define "bootargs" in the environment and U-Boot will
handle bootline fix up for the kernel dtb automatically.

When using 'bootefi' to chain load an x86 kernel, the UEFI boot loader
application for VxWorks takes care of the kernel bootline preparation.

Serial console
--------------
It's very common that VxWorks BSPs configure a different baud rate for the
serial console from what is being used by U-Boot. For example, VxWorks tends
to use 9600 as the default baud rate on all x86 BSPs while U-Boot uses 115200.
Please configure both U-Boot and VxWorks to use the same baud rate, or it may
look like VxWorks hangs somewhere as nothing outputs on the serial console.

x86-specific information
------------------------
Before direct loading an x86 kernel via 'bootvx', one additional environment
variable need to be provided. This is "vx_phys_mem_base", which represent the
physical memory base address of VxWorks.

Check VxWorks kernel configuration to look for LOCAL_MEM_LOCAL_ADRS. For
VxWorks 7, this is normally a virtual address and you need find out its
corresponding physical address and assign its value to "vx_phys_mem_base".

For boards on which ACPI is not supported by U-Boot yet, VxWorks kernel must
be configured to use MP table and virtual wire interrupt mode. This requires
INCLUDE_MPTABLE_BOOT_OP and INCLUDE_VIRTUAL_WIRE_MODE to be included in a
VxWorks kernel configuration.

Both 32-bit x86 and 64-bit x64 kernels can be loaded.

There are two types of graphics console drivers in VxWorks. One is the 80x25
VGA text mode driver. The other one is the EFI console bitmapped graphics mode
driver. To make these drivers function, U-Boot needs to load and run the VGA
BIOS of the graphics card first.

    - If the kernel is configured with 80x25 VGA text mode driver,
      CONFIG_FRAMEBUFFER_SET_VESA_MODE must be unset in U-Boot.
    - If the kernel is configured with bitmapped graphics mode driver,
      CONFIG_FRAMEBUFFER_SET_VESA_MODE need remain set but care must be taken
      at which VESA mode is to be set. The supported pixel format is 32-bit
      RGBA, hence the available VESA mode can only be one of the following:

        * FRAMEBUFFER_VESA_MODE_10F
        * FRAMEBUFFER_VESA_MODE_112
        * FRAMEBUFFER_VESA_MODE_115
        * FRAMEBUFFER_VESA_MODE_118
        * FRAMEBUFFER_VESA_MODE_11B
