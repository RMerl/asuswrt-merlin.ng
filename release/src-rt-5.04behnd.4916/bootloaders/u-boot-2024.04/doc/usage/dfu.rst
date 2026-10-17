.. SPDX-License-Identifier: GPL-2.0+

Device Firmware Upgrade (DFU)
=============================

Overview
--------

Device Firmware Upgrade (DFU) enables the download and upload of firmware
to/from U-Boot while connected over USB.

U-Boot follows the Universal Serial Bus Device Class Specification for
Device Firmware Upgrade Version 1.1 from the USB forum (DFU v1.1 in www.usb.org).

U-Boot implements this DFU capability (CONFIG_DFU) with the command dfu
(cmd/dfu.c / CONFIG_CMD_DFU) based on:

- the DFU stack (common/dfu.c and common/spl/spl_dfu.c), based on the
  USB DFU download gadget (drivers/usb/gadget/f_dfu.c)
- The access to mediums is done in DFU backends (driver/dfu)

Today the supported DFU backends are:

- MMC (RAW or FAT / EXT2 / EXT3 / EXT4 file system / SKIP / SCRIPT)
- NAND
- RAM
- SF (serial flash)
- MTD (all MTD device: NAND, SPI-NOR, SPI-NAND,...)
- virtual

These DFU backends are also used by

- the dfutftp (see README.dfutftp)
- the thordown command (cmd/thordown.c and gadget/f_thor.c)

The "virtual" backend is a generic DFU backend to support a board specific
target (for example OTP), only based on the weak functions:

- dfu_write_medium_virt
- dfu_get_medium_size_virt
- dfu_read_medium_virt

Configuration Options
---------------------

The following configuration options are relevant to device firmware upgrade:

* CONFIG_DFU
* CONFIG_DFU_OVER_USB
* CONFIG_DFU_MMC
* CONFIG_DFU_MTD
* CONFIG_DFU_NAND
* CONFIG_DFU_RAM
* CONFIG_DFU_SF
* CONFIG_DFU_SF_PART
* CONFIG_DFU_TIMEOUT
* CONFIG_DFU_VIRTUAL
* CONFIG_CMD_DFU

Environment variables
---------------------

The dfu command uses 3 environment variables:

dfu_alt_info
    The DFU setting for the USB download gadget with a semicolon separated
    string of information on each alternate::

        dfu_alt_info="<alt1>;<alt2>;....;<altN>"

    When several devices are used, the format is:

    - <interface> <dev>'='alternate list (';' separated)
    - each interface is separated by '&'::

        dfu_alt_info=\
            "<interface1> <dev1>=<alt1>;....;<altN>&"\
            "<interface2> <dev2>=<altN+1>;....;<altM>&"\
            ...\
            "<interfaceI> <devI>=<altY+1>;....;<altZ>&"

dfu_bufsiz
    size of the DFU buffer, when absent, defaults to
    CONFIG_SYS_DFU_DATA_BUF_SIZE (8 MiB by default)

dfu_hash_algo
    name of the hash algorithm to use

Commands
--------

dfu <USB_controller> [<interface> <dev>] list
    List the alternate device defined in *dfu_alt_info*.

dfu <USB_controller> [<interface> <dev>] [<timeout>]
    Start the dfu stack on the USB instance with the selected medium
    backend and use the *dfu_alt_info* variable to configure the
    alternate setting and link each one with the medium.
    The dfu command continues until it receives a ^C in the console or
    a DFU detach transaction from the HOST. If the CONFIG_DFU_TIMEOUT option
    is enabled and a <timeout> parameter is present in the command line,
    the DFU operation will be aborted automatically after <timeout>
    seconds of waiting for the remote to initiate a DFU session.

The possible values of <interface> are (with <USB controller> = 0 in the dfu
command example)

mmc
    for eMMC and SD card::

        dfu 0 mmc <dev>

    each element in *dfu_alt_info* being

    * <name> raw <offset> <size> [mmcpart <num>]   raw access to mmc device
    * <name> part <dev> <part_id> [offset <byte>]  raw access to partition
    * <name> fat <dev> <part_id>                   file in FAT partition
    * <name> ext4 <dev> <part_id>                  file in EXT4 partition
    * <name> skip 0 0                              ignore flashed data
    * <name> script 0 0                            execute commands in shell

    with

    offset
        is the offset in the device (hexadecimal without "0x")
    size
        is the size of the access area (hexadecimal without "0x")
        or 0 which means whole device
    partid
        being the GPT or DOS partition index,
    num
         being the eMMC hardware partition number.

    A value of environment variable *dfu_alt_info* for eMMC could be::

        u-boot raw 0x3e 0x800 mmcpart 1;bl2 raw 0x1e 0x1d mmcpart 1

    A value of environment variable *dfu_alt_info* for SD card could be::

        u-boot raw 0x80 0x800;uImage ext4 0 2

    If you don't want to flash the given image file to storage, use the "skip"
    type entity.

    - It can be used to protect from flashing the wrong image for the specific board.
    - Especially, this layout will be useful when the thor protocol is used,
      which performs flashing in batch mode, where more than one file is
      processed.

    For example, if one makes a single tar file with support for the two
    boards with u-boot-<board1>.bin and u-boot-<board2>.bin files, one
    can use it to flash a proper u-boot image on both without a failure::

        u-boot-<board1>.bin raw 0x80 0x800; u-boot-<board2>.bin skip 0 0

    When flashing a new system image requires you to do some more complex
    things than just writing data to the storage medium, one can use 'script'
    type. Data written to such an entity will be executed as a command list
    in the u-boot's shell. This for example allows you to re-create a partition
    layout and even set a new *dfu_alt_info* for the newly created partitions.
    Such a script would look like::

        setenv dfu_alt_info ...
        setenv mbr_parts ...
        mbr write ...

    Please note that this means the user will be able to execute any
    arbitrary commands just like in the u-boot's shell.

nand
    raw slc nand device::

         dfu 0 nand <dev>

    each element in *dfu_alt_info* being either of

    * <name> raw <offset> <size>        raw access to nand device
    * <name> part <dev_id> <part_id>     raw access to partition
    * <name> partubi <dev_id> <part_id>  raw access to ubi partition

    with

    offset
        is the offset in the nand device (hexadecimal without "0x")
    size
        is the size of the access area (hexadecimal without "0x")
    dev_id
        is the NAND device index (decimal only)
    part_id
        is the NAND partition index (decimal only)

ram
    raw access to ram::

         dfu 0 ram <dev>

    dev
        is not used for RAM target

    each element in *dfu_alt_info* being::

      <name> ram <offset> <size>  raw access to ram

    with

    offset
        is the offset in the ram device (hexadecimal without "0x")
    size
        is the size of the access area (hexadecimal without "0x")

sf
    serial flash : NOR::

        cmd: dfu 0 sf <dev>

    each element in *dfu_alt_info* being either of:

    * <name> raw <offset> <size>  raw access to sf device
    * <name> part <dev_id> <part_id>  raw access to partition
    * <name> partubi <dev_id> <part_id>  raw access to ubi partition

    with

    offset
        is the offset in the sf device (hexadecimal without "0x")
    size
        is the size of the access area (hexadecimal without "0x")
    dev_id
        is the sf device index (the device is "nor<dev_id>") (deximal only)
    part_id
        is the MTD partition index (decimal only)

mtd
    all MTD device: NAND, SPI-NOR, SPI-NAND,...::

        cmd: dfu 0 mtd <dev>

    with

    dev
        the mtd identifier as defined in mtd command
        (nand0, nor0, spi-nand0,...)

    each element in *dfu_alt_info* being either of:

    * <name> raw <offset> <size>  for raw access to mtd device
    * <name> part <part_id>       for raw access to partition
    * <name> partubi <part_id>    for raw access to ubi partition

    with

    offset
        is the offset in the mtd device (hexadecimal without "0x")
    size
        is the size of the access area (hexadecimal without "0x")
    part_id
        is the MTD partition index (decimal only)

virt
    virtual flash back end for DFU

    ::

        cmd: dfu 0 virt <dev>

    each element in *dfu_alt_info* being:

    * <name>

<interface> and <dev> are absent, the dfu command to use multiple devices::

    cmd: dfu 0 list
    cmd: dfu 0

*dfu_alt_info* variable provides the list of <interface> <dev> with
alternate list separated by '&' with the same format for each <alt>::

    mmc <dev>=<alt1>;....;<altN>
    nand <dev>=<alt1>;....;<altN>
    ram <dev>=<alt1>;....;<altN>
    sf <dev>=<alt1>;....;<altN>
    mtd <dev>=<alt1>;....;<altN>
    virt <dev>=<alt1>;....;<altN>

Callbacks
---------

The weak callback functions can be implemented to manage specific behavior

dfu_initiated_callback
   called when the DFU transaction is started, used to initialize the device

dfu_flush_callback
    called at the end of the DFU write after DFU manifestation, used to manage
    the device when the DFU transaction is closed

Host tools
----------

When U-Boot runs the dfu stack, the DFU host tools can be used
to send/receive firmware images on each configured alternate.

For example dfu-util is a host side implementation of the DFU 1.1
specifications(http://dfu-util.sourceforge.net/) which works with U-Boot.

Usage
-----

Example 1: firmware located in eMMC or SD card, with:

- alternate 1 (alt=1) for SPL partition (GPT partition 1)
- alternate 2 (alt=2) for U-Boot partition (GPT partition 2)

The U-Boot configuration is::

  U-Boot> env set dfu_alt_info "spl part 0 1;u-boot part 0 2"

  U-Boot> dfu 0 mmc 0 list
  DFU alt settings list:
  dev: eMMC alt: 0 name: spl layout: RAW_ADDR
  dev: eMMC alt: 1 name: u-boot layout: RAW_ADDR

  Boot> dfu 0 mmc 0

On the Host side:

list the available alternate setting::

  $> dfu-util -l
  dfu-util 0.9

  Copyright 2005-2009 Weston Schmidt, Harald Welte and OpenMoko Inc.
  Copyright 2010-2016 Tormod Volden and Stefan Schmidt
  This program is Free Software and has ABSOLUTELY NO WARRANTY
  Please report bugs to http://sourceforge.net/p/dfu-util/tickets/

  Found DFU: [0483:5720] ver=0200, devnum=45, cfg=1, intf=0, path="3-1.3.1", \
     alt=1, name="u-boot", serial="003A00203438510D36383238"
  Found DFU: [0483:5720] ver=0200, devnum=45, cfg=1, intf=0, path="3-1.3.1", \
     alt=0, name="spl", serial="003A00203438510D36383238"

  To download to U-Boot, use -D option

  $> dfu-util -a 0 -D u-boot-spl.bin
  $> dfu-util -a 1 -D u-boot.bin

  To upload from U-Boot, use -U option

  $> dfu-util -a 0 -U u-boot-spl.bin
  $> dfu-util -a 1 -U u-boot.bin

  To request a DFU detach and reset the USB connection:
  $> dfu-util -a 0 -e  -R


Example 2: firmware located in NOR (sf) and NAND, with:

- alternate 1 (alt=1) for SPL partition (NOR GPT partition 1)
- alternate 2 (alt=2) for U-Boot partition (NOR GPT partition 2)
- alternate 3 (alt=3) for U-Boot-env partition (NOR GPT partition 3)
- alternate 4 (alt=4) for UBI partition (NAND GPT partition 1)

::

  U-Boot> env set dfu_alt_info \
  "sf 0:0:10000000:0=spl part 0 1;u-boot part 0 2; \
  u-boot-env part 0 3&nand 0=UBI partubi 0,1"

  U-Boot> dfu 0 list

  DFU alt settings list:
  dev: SF alt: 0 name: spl layout: RAW_ADDR
  dev: SF alt: 1 name: ssbl layout: RAW_ADDR
  dev: SF alt: 2 name: u-boot-env layout: RAW_ADDR
  dev: NAND alt: 3 name: UBI layout: RAW_ADDR

  U-Boot> dfu 0

::

  $> dfu-util -l
  Found DFU: [0483:5720] ver=9999, devnum=96, cfg=1,\
     intf=0, alt=3, name="UBI", serial="002700333338511934383330"
  Found DFU: [0483:5720] ver=9999, devnum=96, cfg=1,\
     intf=0, alt=2, name="u-boot-env", serial="002700333338511934383330"
  Found DFU: [0483:5720] ver=9999, devnum=96, cfg=1,\
     intf=0, alt=1, name="u-boot", serial="002700333338511934383330"
  Found DFU: [0483:5720] ver=9999, devnum=96, cfg=1,\
     intf=0, alt=0, name="spl", serial="002700333338511934383330"

Same example with MTD backend

::

  U-Boot> env set dfu_alt_info \
     "mtd nor0=spl part 1;u-boot part 2;u-boot-env part 3&"\
     "mtd nand0=UBI partubi 1"

  U-Boot> dfu 0 list
  using id 'nor0,0'
  using id 'nor0,1'
  using id 'nor0,2'
  using id 'nand0,0'
  DFU alt settings list:
  dev: MTD alt: 0 name: spl layout: RAW_ADDR
  dev: MTD alt: 1 name: u-boot layout: RAW_ADDR
  dev: MTD alt: 2 name: u-boot-env layout: RAW_ADDR
  dev: MTD alt: 3 name: UBI layout: RAW_ADDR

Example 3

firmware located in SD Card (mmc) and virtual partition on OTP and PMIC
non-volatile memory

- alternate 1 (alt=1) for scard
- alternate 2 (alt=2) for OTP (virtual)
- alternate 3 (alt=3) for PMIC NVM (virtual)

::

   U-Boot> env set dfu_alt_info \
      "mmc 0=sdcard raw 0 0x100000&"\
      "virt 0=otp" \
      "virt 1=pmic"

::

   U-Boot> dfu 0 list
   DFU alt settings list:
   dev: eMMC alt: 0 name: sdcard layout: RAW_ADDR
   dev: VIRT alt: 1 name: otp layout: RAW_ADDR
   dev: VIRT alt: 2 name: pmic layout: RAW_ADDR
