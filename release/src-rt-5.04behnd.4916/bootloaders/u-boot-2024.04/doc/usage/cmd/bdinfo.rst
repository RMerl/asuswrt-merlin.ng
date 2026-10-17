.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2023, Heinrich Schuchardt <heinrich.schuchardt@canonical.com>

.. index::
   single: bdinfo (command)

bdinfo command
==============

Synopsis
--------

::

    bdinfo

Description
-----------

The *bdinfo* command prints information about the board.

Example
-------

::

    => bdinfo
    boot_params = 0x0000000000000000
    DRAM bank   = 0x0000000000000000
    -> start    = 0x0000000040000000
    -> size     = 0x0000000100000000
    flashstart  = 0x0000000000000000
    flashsize   = 0x0000000004000000
    flashoffset = 0x00000000000e87f8
    baudrate    = 115200 bps
    relocaddr   = 0x000000013fefb000
    reloc off   = 0x000000013fefb000
    Build       = 64-bit
    current eth = virtio-net#32
    ethaddr     = 52:52:52:52:52:52
    IP addr     = 10.0.2.15
    fdt_blob    = 0x000000013edbadb0
    new_fdt     = 0x000000013edbadb0
    fdt_size    = 0x0000000000100000
    lmb_dump_all:
     memory.cnt  = 0x1
     memory[0]      [0x40000000-0x13fffffff], 0x100000000 bytes flags: 0
     reserved.cnt  = 0x2
     reserved[0]    [0x13ddb3000-0x13fffffff], 0x0224d000 bytes flags: 0
     reserved[1]    [0x13edb6930-0x13fffffff], 0x012496d0 bytes flags: 0
    devicetree  = board
    arch_number = 0x0000000000000000
    TLB addr    = 0x000000013fff0000
    irq_sp      = 0x000000013edbada0
    sp start    = 0x000000013edbada0
    Early malloc usage: 3a8 / 2000
    =>

boot_params
    address of the memory area for boot parameters

DRAM bank
    index, start address and end address of a memory bank

baudrate
    baud rate of the serial console

relocaddr
    address to which U-Boot has relocated itself

reloc off
    relocation offset, difference between *relocaddr* and the text base

Build
    bitness of the system

current eth
    name of the active network device

IP addr
    network address, value of the environment variable *ipaddr*

fdt_blob
    address of U-Boot's own device tree, NULL if none

new_fdt
    location of the relocated device tree

fdt_size
    space reserved for relocated device space

lmb_dump_all
    available memory and memory reservations

devicetree
    source of the device-tree

arch_number
    unique id for the board

TLB addr
    address of the translation lookaside buffer

irq_sp
    address of the IRQ stack pointer

sp start
    initial stack pointer address

Early malloc usage
    amount of memory used in the early malloc memory and its maximum size
    as defined by CONFIG_SYS_MALLOC_F_LEN

Configuration
-------------

The bdinfo command is available if CONFIG_CMD_BDI=y.

Return code
-----------

The return code $? is 0 (true).
