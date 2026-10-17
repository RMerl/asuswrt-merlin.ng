i.MX7D/i.MX8MM SRC_GPR10 PERSIST_SECONDARY_BOOT for bootloader A/B switching
============================================================================

Introduction
------------
Since at least iMX53 until iMX8MM, it is possible to have two copies of
bootloader in SD/eMMC and switch between them. The switch is triggered
either by the BootROM in case the bootloader image is faulty OR can be
enforced by the user.

Operation
---------
 #. Upon Power-On Reset (POR)

    - SRC_GPR10 bit PERSIST_SECONDARY_BOOT is set to 0
    - BootROM attempts to start bootloader A-copy

      - if A-copy valid

         - BootROM starts A-copy
         - END

      - if A-copy NOT valid

         - BootROM sets SRC_GPR10 bit PERSIST_SECONDARY_BOOT to 1
         - BootROM triggers WARM reset, GOTO 1)
         - END

 #. Upon COLD Reset

    - GOTO 1)
    - END

 #. Upon WARM Reset

    - SRC_GPR10 bit PERSIST_SECONDARY_BOOT is retained

      - if SRC_GPR10 bit PERSIST_SECONDARY_BOOT is 0

        - BootROM attempts to start bootloader A-copy

          - if A-copy valid

            - BootROM starts A-copy
            - END

          - if A-copy NOT valid

            - BootROM sets SRC_GPR10 bit PERSIST_SECONDARY_BOOT to 1
            - BootROM triggers WARM reset. GOTO 1.3)
            - END

      - if SRC_GPR10 bit PERSIST_SECONDARY_BOOT is 1

        - BootROM attempts to start bootloader B-copy

          - if B-copy valid

            - BootROM starts B-copy
            - END

          - if B-copy NOT valid
            - System hangs
            - END

Setup
-----
The bootloader A-copy must be placed at predetermined offset in SD/eMMC. The
bootloader B-copy area offset is determined by an offset stored in Secondary
Image Table (SIT). The SIT must be placed at predetermined offset in SD/eMMC.

The following table contains offset of SIT, bootloader A-copy and recommended
bootloader B-copy offset. The offsets are in 512 Byte sector units (that is
offset 0x1 means 512 Bytes from the start of SD/eMMC card data partition).
For details on the addition of two numbers in recommended B-copy offset, see
SIT format below.

+----------+-----------------------------+--------------------+-----------------------+-----------------------------+
|   SoC    | Boot Device Type            | SIT offset (fixed) | A-copy offset (fixed) | B-copy offset (recommended) |
+----------+-----------------------------+--------------------+-----------------------+-----------------------------+
| iMX7D    |                             |         0x1        |          0x2          |          0x800+0x2          |
+----------+-----------------------------+--------------------+-----------------------+-----------------------------+
| iMX8MM   | SD/eSD/MMC/eMMC normal boot |        0x41        |         0x42          |         0x1000+0x42         |
+----------+-----------------------------+--------------------+-----------------------+-----------------------------+
| iMX8MM   | eMMC Fast boot fuse blown   |         0x1        |          0x2          |          0x1000+0x2         |
+----------+-----------------------------+--------------------+-----------------------+-----------------------------+

SIT format
~~~~~~~~~~
SIT is a 20 byte long structure containing of 5 32-bit words. Those encode
bootloader B-copy area offset (called "firstSectorNumber"), magic value
(called "tag") that is always 0x00112233, and three unused words set to 0.
SIT is documented in [1]_ and [2]_. Example SIT are below::

  $ hexdump -vC sit-mx7d.bin
    00000000  00 00 00 00
    00000004  00 00 00 00
    00000008  33 22 11 00 <--- This is the "tag"
    0000000c  00 08 00 00 <--- This is the "firstSectorNumber"
    00000010  00 00 00 00

  $ hexdump -vC sit-mx8mm.bin
    00000000  00 00 00 00
    00000004  00 00 00 00
    00000008  33 22 11 00 <--- This is the "tag"
    0000000c  00 10 00 00 <--- This is the "firstSectorNumber"
    00000010  00 00 00 00

B-copy area offset ("firstSectorNumber") is offset, in units of 512 Byte
sectors, that is added to the start of boot media when switching between
A-copy and B-copy. For A-copy, this offset is 0x0. For B-copy, this offset
is determined by SIT (e.g. if firstSectorNumber is 0x1000 as it is above
in sit-mx8mm.bin, then the B-copy offset is 0x1000 sectors = 2 MiB).

Bootloader A-copy (e.g. u-boot.imx or flash.bin) is placed at fixed offset
from A-copy area offset (e.g. 0x2 sectors from sector 0x0 for iMX7D, which
means u-boot.imx A-copy must be written to sector 0x2).

The same applies to bootloader B-copy, which is placed at fixed offset from
B-copy area offset determined by SIT (e.g. 0x2 sectors from sector 0x800 [see
sit-mx7d.bin example above, this can be changed in SIT firstSectorNumber] for
iMX7D, which means u-boot.imx B-copy must be written to sector 0x802)

**WARNING:**
B-copy area offset ("firstSectorNumber") is NOT equal to bootloader
(image, which is u-boot.imx or flash.bin) B-copy offset.

To generate SIT, use for example the following bourne shell printf command::

$ printf '\x0\x0\x0\x0\x0\x0\x0\x0\x33\x22\x11\x00\x00\x08\x00\x00\x0\x0\x0\x0' > sit-mx7d.bin
$ printf '\x0\x0\x0\x0\x0\x0\x0\x0\x33\x22\x11\x00\x00\x10\x00\x00\x0\x0\x0\x0' > sit-mx8mm.bin

Write bootloader A/B copy and SIT to SD/eMMC
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Examples of writing SIT and two copies of bootloader to SD or eMMC:

- iMX8MM, SD card at /dev/sdX, Linux command line
  ::

    $ dd if=sit-mx8mm.bin of=/dev/sdX bs=512 seek=65
    $ dd if=flash.bin     of=/dev/sdX bs=512 seek=66
    $ dd if=flash.bin     of=/dev/sdX bs=512 seek=4162

- iMX8MM, eMMC 1 data partition, U-Boot command line
  ::

    => mmc partconf 1 0 0 0

    => dhcp ${loadaddr} sit-mx8mm.bin
    => mmc dev 1
    => mmc write ${loadaddr} 0x41 0x1

    => dhcp ${loadaddr} flash.bin
    => setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    => mmc dev 1
    => mmc write ${loadaddr} 0x42   ${blkcnt}
    => mmc write ${loadaddr} 0x1042 ${blkcnt}

WARM reset into B-copy using WDT
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

To perform a reboot into B-copy, the PERSIST_SECONDARY_BOOT must be set
in SRC_GPR10 register. Example on iMX8MM::

  => mw 0x30390098 0x40000000

A WARM reset can be triggered using WDT as follows::

  => mw.w 0x30280000 0x25

References
----------

.. [1] i.MX 7Dual Applications Processor Reference Manual, Rev. 1, 01/2018 ; section 6.6.5.3.5 Redundant boot support for expansion device
.. [2] i.MX 8M Mini Applications Processor Reference Manual, Rev. 3, 11/2020 ; section 6.1.5.4.5 Redundant boot support for expansion device
