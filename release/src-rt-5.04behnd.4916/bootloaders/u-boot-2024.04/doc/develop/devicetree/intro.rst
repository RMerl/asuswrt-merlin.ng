.. SPDX-License-Identifier: GPL-2.0+

Devicetree Introduction
=======================

U-Boot uses a devicetree for configuration. This includes the devices used by
the board, the format of the image created with binman, which UART to use for
the console, public keys used for secure boot and many other things.

See :doc:`control` for more information.

Why does U-Boot put <thing> in the devicetree?
----------------------------------------------

This question comes up a lot with people new to U-Boot, particular those coming
from Linux who are used to quite strict rules about what can go into the
devicetree.

U-Boot uses the same devicetree as Linux but adds more things necessary for the
bootloader environment (see :ref:`dttweaks`).

U-Boot does not have a user space to provide policy and configuration. It cannot
do what Linux does and run programs and look up filesystems to figure out how to
boot. So configuration and runtime information goes into the devicetree in
U-Boot.

Of course it is possible to:

- add tables into the rodata section of the U-Boot binary
- append some info to the end of U-Boot in a different format
- modify the linker script to bring in a file with some info in it
- put things in ACPI tables
- link in a UEFI hand-off block structure and put things in there

but *please don't*. In general, devicetree is the sane place to hold U-Boot's
configuration.

So, please, do NOT ask why U-Boot puts <thing> in the devicetree. It is the only
place it can go. It is a highly suitable data structure for just about anything
that U-Boot needs to know at runtime.

Note, it is possible to use platdata directly so drivers avoid devicetreee in
SPL. But of-platdata is the modern way of avoiding devicetree overhead, so
please use that instead.
