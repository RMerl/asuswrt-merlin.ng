.. SPDX-License-Identifier: GPL-2.0+
.. Copyright (C) 2023, Yu Chien Peter Lin <peterlin@andestech.com>

RISC-V
======

Overview
--------

This document outlines the U-Boot boot process for the RISC-V architecture.
RISC-V is an open-source instruction set architecture (ISA) based on the
principles of reduced instruction set computing (RISC). It has been designed
to be flexible and customizable, allowing it to be adapted to different use
cases, from embedded systems to high performance servers.

Typical Boot Process
--------------------

U-Boot can run in either M-mode or S-mode, depending on whether it runs before
the initialization of the firmware providing SBI (Supervisor Binary Interface).
The firmware is necessary in the RISC-V boot process as it serves as a SEE
(Supervisor Execution Environment) to handle exceptions for the S-mode U-Boot
or Operating System.

In between the boot phases, the hartid is passed through the a0 register, and
the start address of the devicetree is passed through the a1 register.

As a reference, OpenSBI is an SBI implementation that can be used with U-Boot
in different modes, see the
`OpenSBI firmware document <https://github.com/riscv-software-src/opensbi/tree/master/docs/firmware>`_
for more details.

M-mode U-Boot
^^^^^^^^^^^^^

When running in M-mode U-Boot, it will load the payload image (e.g.
`fw_payload <https://github.com/riscv-software-src/opensbi/blob/master/docs/firmware/fw_payload.md>`_)
which contains the firmware and the S-mode Operating System; in this case, you
can use mkimage to package the payload image into an uImage format, and boot it
using the bootm command.

The following diagram illustrates the boot process::

	<-----------( M-mode )----------><--( S-mode )-->
	+----------+   +--------------+    +------------+
	|  U-Boot  |-->| SBI firmware |--->|     OS     |
	+----------+   +--------------+    +------------+

To examine the boot process with the QEMU virt machine, you can follow the
steps in the "Building U-Boot" section of the following document:
:doc:`../board/emulation/qemu-riscv`.

S-mode U-Boot
^^^^^^^^^^^^^

RISC-V production boot images may include a U-Boot SPL for platform-specific
initialization. The U-Boot SPL then loads a FIT image (u-boot.itb), which
contains a firmware (e.g.
`fw_dynamic <https://github.com/riscv-software-src/opensbi/blob/master/docs/firmware/fw_dynamic.md>`_)
providing the SBI, as well as a regular U-Boot (or U-Boot proper) running in
S-mode. Finally, the S-mode Operating
System is loaded.

The following diagram illustrates the boot process::

	<-------------( M-mode )----------><----------( S-mode )------->
	+------------+   +--------------+    +----------+   +----------+
	| U-Boot SPL |-->| SBI firmware |--->|  U-Boot  |-->|    OS    |
	+------------+   +--------------+    +----------+   +----------+

To examine the boot process with the QEMU virt machine, you can follow the
steps in the "Running U-Boot SPL" section of the following document:
:doc:`../board/emulation/qemu-riscv`.

Toolchain
---------

You can build the
`RISC-V GNU toolchain <https://github.com/riscv-collab/riscv-gnu-toolchain>`_
from scratch, or download a pre-built toolchain from the
`releases page <https://github.com/riscv-collab/riscv-gnu-toolchain/releases>`_.
