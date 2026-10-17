.. SPDX-License-Identifier: GPL-2.0+

Devicetree in QEMU
==================

For QEMU on ARM, RISC-V and one PPC target, the devicetree is created on-the-fly
by QEMU. It is intended for use in Linux but can be used by U-Boot also, so long
as any nodes/properties needed by U-Boot are merged in.

When `CONFIG_OF_BOARD` is enabled


Obtaining the QEMU devicetree
-----------------------------

Where QEMU generates its own devicetree to pass to U-Boot you can use
`-dtb u-boot.dtb` to force QEMU to use U-Boot's in-tree version.

To obtain the devicetree that qemu generates, add `-machine dumpdtb=qemu.dtb`,
e.g.::

    qemu-system-arm -machine virt -machine dumpdtb=qemu.dtb

    qemu-system-aarch64 -machine virt -machine dumpdtb=qemu.dtb

    qemu-system-riscv64 -machine virt -machine dumpdtb=qemu.dtb


Merging in U-Boot nodes/properties
----------------------------------

Various U-Boot features require nodes and properties in the U-Boot devicetree
and at present QEMU is unaware of these. To use these you must manually merge
in the appropriate pieces.

One way to do this is with dtc. This command runs dtc on each .dtb file in turn,
to produce a text file. It drops the duplicate header on the qemu one. Then it
joins them up and runs them through dtc to compile the output::

    qemu-system-arm -machine virt -machine dumpdtb=qemu.dtb
    cat  <(dtc -I dtb qemu.dtb) <(dtc -I dtb u-boot.dtb | grep -v /dts-v1/) | dtc - -o merged.dtb

You can then run qemu with the merged devicetree, e.g.::

    qemu-system-arm -machine virt -nographic -bios u-boot.bin -dtb merged.dtb

Note that there seems to be a bug in some versions of qemu where the output of
dumpdtb does not quite match what is provided to U-Boot.
