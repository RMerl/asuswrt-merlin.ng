.. SPDX-License-Identifier: GPL-2.0+
.. sectionauthor:: Daniel Schwierzeck <daniel.schwierzeck@gmail.com>

QEMU MIPS
=========

Qemu for MIPS is based on the MIPS Malta board. The built Malta U-Boot
images can be used for Qemu and on physical hardware. The Malta board
supports all combinations of Little and Big Endian as well as 32 bit
and 64 bit.

Limitations & comments
----------------------
The memory size for Qemu is hard-coded to 256 MiB. For Malta Little Endian
targets an extra endianness swapped image named *u-boot-swap.bin* is
generated and required for Qemu.

Example usage
-------------

Build for 32 bit, big endian:

.. code-block:: bash

  make malta_defconfig
  make
  UBOOT_BIN=u-boot.bin
  QEMU_BIN=qemu-system-mips
  QEMU_CPU=24Kc

Build for 32 bit, little endian:

.. code-block:: bash

  make maltael_defconfig
  make
  UBOOT_BIN=u-boot-swap.bin
  QEMU_BIN=qemu-system-mipsel
  QEMU_CPU=24Kc

Build for 64 bit, big endian:

.. code-block:: bash

  make malta64_defconfig
  make
  UBOOT_BIN=u-boot.bin
  QEMU_BIN=qemu-system-mips64
  QEMU_CPU=MIPS64R2-generic

Build for 64 bit, little endian:

.. code-block:: bash

  make malta64el_defconfig
  make
  UBOOT_BIN=u-boot-swap.bin
  QEMU_BIN=qemu-system-mips64el
  QEMU_CPU=MIPS64R2-generic

Generate NOR flash image with U-Boot binary:

.. code-block:: bash

  dd if=/dev/zero bs=1M count=4 | tr '\000' '\377' > pflash.img
  dd if=${UBOOT_BIN} of=pflash.img conv=notrunc

Start Qemu:

.. code-block:: bash

  mkdir tftproot
  ${QEMU_BIN} -nographic -cpu ${QEMU_CPU} -m 256 -drive if=pflash,file="$(pwd)/pflash.img",format=raw -netdev user,id=net0,tftp="$(pwd)/tftproot" -device pcnet,netdev=net0

.. code-block:: bash

  U-Boot 2021.04-00963-g60279a2b1d (Apr 21 2021 - 19:54:32 +0200)

  Board: MIPS Malta CoreLV
  DRAM:  256 MiB
  Flash: 4 MiB
  Loading Environment from Flash... *** Warning - bad CRC, using default environment

  In:    serial@3f8
  Out:   serial@3f8
  Err:   serial@3f8
  Net:   pcnet#0
  IDE:   Bus 0: not available
  maltael #

How to debug U-Boot
-------------------

In order to debug U-Boot you need to start qemu with gdb server support (-s)
and waiting the connection to start the CPU (-S). Start Qemu in the first console:

.. code-block:: bash

  mkdir tftproot
  ${QEMU_BIN} -s -S -nographic -cpu ${QEMU_CPU} -m 256 -drive if=pflash,file="$(pwd)/pflash.img",format=raw -netdev user,id=net0,tftp="$(pwd)/tftproot" -device pcnet,netdev=net0

In the second console start gdb:

.. code-block:: bash

  gdb-multiarch --eval-command "target remote :1234" u-boot

.. code-block:: bash

  GNU gdb (Ubuntu 9.2-0ubuntu1~20.04) 9.2
  Copyright (C) 2020 Free Software Foundation, Inc.
  License GPLv3+: GNU GPL version 3 or later <http://gnu.org/licenses/gpl.html>
  This is free software: you are free to change and redistribute it.
  There is NO WARRANTY, to the extent permitted by law.
  Type "show copying" and "show warranty" for details.
  This GDB was configured as "x86_64-linux-gnu".
  Type "show configuration" for configuration details.
  For bug reporting instructions, please see:
  <http://www.gnu.org/software/gdb/bugs/>.
  Find the GDB manual and other documentation resources online at:
      <http://www.gnu.org/software/gdb/documentation/>.

  For help, type "help".
  Type "apropos word" to search for commands related to "word"...
  Reading symbols from u-boot...
  Remote debugging using :1234
  0xbfc00000 in ?? ()
  (gdb) c
  Continuing.
