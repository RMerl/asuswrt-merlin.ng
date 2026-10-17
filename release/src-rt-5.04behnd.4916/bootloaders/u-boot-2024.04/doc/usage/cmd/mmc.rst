.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: mmc (command)

mmc command
===========

Synopsis
--------

::

    mmc info
    mmc read addr blk# cnt
    mmc write addr blk# cnt
    mmc erase blk# cnt
    mmc rescan [mode]
    mmc part
    mmc dev [dev] [part] [mode]
    mmc list
    mmc wp
    mmc bootbus <dev> <boot_bus_width> <reset_boot_bus_width> <boot_mode>
    mmc bootpart-resize <dev> <dev part size MB> <RPMB part size MB>
    mmc partconf <dev> [[varname] | [<boot_ack> <boot_partition> <partition_access>]]
    mmc rst-function <dev> <value>
    mmc reg read <reg> <offset> [env]

Description
-----------

The mmc command is used to control MMC(eMMC/SD) device.

The 'mmc info' command displays information (Manufacturer ID, OEM, Name, Bus Speed, Mode, ...) of MMC device.

The 'mmc read' command reads raw data to memory address from MMC device with block offset and count.

The 'mmc write' command writes raw data to MMC device from memory address with block offset and count.

    addr
        memory address
    blk#
        start block offset
    cnt
        block count

The 'mmc erase' command erases *cnt* blocks on the MMC device starting at block *blk#*.

    blk#
        start block offset
    cnt
        block count

The 'mmc rescan' command scans the available MMC device.

   mode
       speed mode to set.
       CONFIG_MMC_SPEED_MODE_SET should be enabled. The requested speed mode is
       passed as a decimal number according to the following table:

       ========== ==========================
       Speed mode Description
       ========== ==========================
           0      MMC legacy
           1      MMC High Speed (26MHz)
           2      SD High Speed (50MHz)
           3      MMC High Speed (52MHz)
           4      MMC DDR52 (52MHz)
           5      UHS SDR12 (25MHz)
           6      UHS SDR25 (50MHz)
           7      UHS SDR50 (100MHz)
           8      UHS DDR50 (50MHz)
           9      UHS SDR104 (208MHz)
          10      HS200 (200MHz)
          11      HS400 (200MHz)
          12      HS400ES (200MHz)
       ========== ==========================

       A speed mode can be set only if it has already been enabled in the device tree

The 'mmc part' command displays the list available partition on current mmc device.

The 'mmc dev' command shows or set current mmc device.

    dev
        device number to change
    part
        partition number to change

   mode
       speed mode to set.
       CONFIG_MMC_SPEED_MODE_SET should be enabled. The requested speed mode is
       passed as a decimal number according to the following table:

       ========== ==========================
       Speed mode Description
       ========== ==========================
           0      MMC legacy
           1      MMC High Speed (26MHz)
           2      SD High Speed (50MHz)
           3      MMC High Speed (52MHz)
           4      MMC DDR52 (52MHz)
           5      UHS SDR12 (25MHz)
           6      UHS SDR25 (50MHz)
           7      UHS SDR50 (100MHz)
           8      UHS DDR50 (50MHz)
           9      UHS SDR104 (208MHz)
          10      HS200 (200MHz)
          11      HS400 (200MHz)
          12      HS400ES (200MHz)
       ========== ==========================

       A speed mode can be set only if it has already been enabled in the device tree

The 'mmc list' command displays the list available devices.

The 'mmc wp' command enables "power on write protect" function for boot partitions.

The 'mmc bootbus' command sets the BOOT_BUS_WIDTH field. (*Refer to eMMC specification*)

    boot_bus_width
        0x0
            x1 (sdr) or x4(ddr) buswidth in boot operation mode (default)
        0x1
            x4 (sdr/ddr) buswidth in boot operation mode
        0x2
            x8 (sdr/ddr) buswidth in boot operation mode
        0x3
            Reserved

    reset_boot_bus_width
        0x0
            Reset buswidth to x1, Single data reate and backward compatible timing after boot operation (default)
        0x1
            Retain BOOT_BUS_WIDTH and BOOT_MODE value after boot operation. This is relevant to Push-pull mode operation only

    boot_mode
        0x0
            Use single data rate + backward compatible timing in boot operation (default)
        0x1
            Use single data rate + High Speed timing in boot operation mode
        0x2
            Use dual data rate in boot operation
        0x3
            Reserved

The 'mmc partconf' command shows or changes PARTITION_CONFIG field.

    varname
        When showing the PARTITION_CONFIG, an optional environment variable to store the current boot_partition value into.
    boot_ack
        boot acknowledge value
    boot_partition
        boot partition to enable for boot
            0x0
                Device not boot enabled(default)
            0x1
                Boot partition1 enabled for boot
            0x2
                Boot partition2 enabled for boot
            0x7
                User area enabled for boot
            others
                Reserved
    partition_access
        partitions to access

The 'mmc bootpart-resize' command changes sizes of boot and RPMB partitions.

    dev
        device number
    boot part size MB
        target size of boot partition
    RPMB part size MB
        target size of RPMB partition

The 'mmc rst-function' command changes the RST_n_FUNCTION field.
**WARNING** : This is a write-once field. (*Refer to eMMC specification*)

    value
        0x0
            RST_n signal is temporarily disabled (default)
        0x1
            RST_n signal is permanently enabled
        0x2
            RST_n signal is permanently disabled
        0x3
            Reserved

The 'mmc reg read <reg> <offset> [env]' reads eMMC card register and
either print it to standard output, or store the value in environment
variable.

<reg> with
optional offset <offset> into the register array, and print it to
standard output or store it into environment variable [env].

    reg
        cid
            The Device IDentification (CID) register. Uses offset.
        csd
            The Device-Specific Data (CSD) register. Uses offset.
        dsr
            The driver stage register (DSR).
        ocr
            The operation conditions register (OCR).
        rca
            The relative Device address (RCA) register.
        extcsd
            The Extended CSD register. Uses offset.
    offset
        For 'cid'/'csd' 128 bit registers '[0..3]' in 32-bit increments. For 'extcsd' 512 bit register '[0..512,all]' in 8-bit increments, or 'all' to read the entire register.
    env
        Optional environment variable into which 32-bit value read from register should be stored.

Examples
--------

The 'mmc info' command displays device's capabilities:
::

    => mmc info
    Device: EXYNOS DWMMC
    Manufacturer ID: 45
    OEM: 100
    Name: SDW16
    Bus Speed: 52000000
    Mode: MMC DDR52 (52MHz)
    Rd Block Len: 512
    MMC version 5.0
    High Capacity: Yes
    Capacity: 14.7 GiB
    Bus Width: 8-bit DDR
    Erase Group Size: 512 KiB
    HC WP Group Size: 8 MiB
    User Capacity: 14.7 GiB WRREL
    Boot Capacity: 4 MiB ENH
    RPMB Capacity: 4 MiB ENH
    Boot area 0 is not write protected
    Boot area 1 is not write protected

The raw data can be read/written via 'mmc read/write' command:
::

    => mmc read 40000000 5000 100
    MMC read: dev # 0, block # 20480, count 256 ... 256 blocks read: OK

    => mmc write 40000000 5000 100
    MMC write: dev # 0, block # 20480, count 256 ... 256 blocks written: OK

The partition list can be shown via 'mmc part' command:
::

    => mmc part
    Partition Map for MMC device 0  --   Partition Type: DOS

    Part    Start Sector    Num Sectors     UUID            Type
      1     8192            131072          dff8751a-01     0e Boot
      2     139264          6291456         dff8751a-02     83
      3     6430720         1048576         dff8751a-03     83
      4     7479296         23298048        dff8751a-04     05 Extd
      5     7481344         307200          dff8751a-05     83
      6     7790592         65536           dff8751a-06     83
      7     7858176         16384           dff8751a-07     83
      8     7876608         22900736        dff8751a-08     83

The current device can be shown or set via 'mmc dev' command:
::

    => mmc dev
    switch to partitions #0, OK
    mmc0(part0) is current device
    => mmc dev 2 0
    switch to partitions #0, OK
    mmc2 is current device
    => mmc dev 0 1 4
    switch to partitions #1, OK
    mmc0(part 1) is current device

The list of available devices can be shown via 'mmc list' command:
::

    => mmc list
    mmc list
    EXYNOS DWMMC: 0 (eMMC)
    EXYNOS DWMMC: 2 (SD)

Configuration
-------------

The mmc command is only available if CONFIG_CMD_MMC=y.
Some commands need to enable more configuration.

write, erase
    CONFIG_MMC_WRITE
bootbus, bootpart-resize, partconf, rst-function
    CONFIG_SUPPORT_EMMC_BOOT=y
