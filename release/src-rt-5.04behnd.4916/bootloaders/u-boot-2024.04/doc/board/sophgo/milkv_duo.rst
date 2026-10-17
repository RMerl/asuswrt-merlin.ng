.. SPDX-License-Identifier: GPL-2.0+

Milk-V Duo
==========

CV1800B RISC-V SoC
------------------
The CV1800B is a high-performance, low-power 1+1 64-bit RISC-V SoC from Sophgo.

Mainline support
----------------
The support for following drivers are already enabled:
1. ns16550 UART Driver.

Building
~~~~~~~~
1. Add the RISC-V toolchain to your PATH.
2. Setup ARCH & cross compilation environment variable:

.. code-block:: console

   export CROSS_COMPILE=<riscv64 toolchain prefix>
   cd <U-Boot-dir>
   make milkv_duo_defconfig
   make

This will generate u-boot-dtb.bin

Booting
~~~~~~~
Currently, we rely on vendor FSBL(First Stage Boot Loader) to initialize the
clock and load the u-boot image, then bootup from it.

Alternatively, to run u-boot-dtb.bin on top of FSBL, follow these steps:

1. Use the vendor-provided tool to create a unified fip.bin file containing
   FSBL, OpenSBI, and U-Boot.

2. Place the generated fip.bin file into the FAT partition of the SD card.

3. Insert the SD card into the board and power it on.

The board will automatically execute the FSBL from the fip.bin file.
Subsequently, it will transition to OpenSBI, and finally, OpenSBI will invoke
U-Boot.


Sample boot log from Milk-V Duo board
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code-block:: none

   U-Boot 2024.01-rc5-00010-g51965baa36 (Dec 28 2023 - 13:15:53 +0800)milkv_duo

   DRAM:  63.3 MiB
   Core:  10 devices, 8 uclasses, devicetree: separate
   Loading Environment from nowhere... OK
   In:    serial@4140000
   Out:   serial@4140000
   Err:   serial@4140000
   Net:   No ethernet found.
   milkv_duo# cpu detail
     0: cpu@0      rv64imafdc
      ID = 0, freq = 0 Hz: L1 cache, MMU
   milkv_duo#
