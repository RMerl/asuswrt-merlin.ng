.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: qfw (command)

qfw command
===========

Synopsis
--------

::

    qfw list
    qfw cpus
    qfw load [kernel_addr [initrd_addr]]

Description
-----------

The *qfw* command is used to retrieve information from the QEMU firmware.

The *qfw list* sub-command displays the QEMU firmware files.

The *qfw cpus* sub-command displays the available CPUs.

The *qfw load* command is used to load a kernel and an initial RAM disk.

kernel_addr
    address to which the file specified by the -kernel parameter of QEMU shall
    be loaded. Defaults to environment variable *loadaddr* and further to
    the value of *CONFIG_SYS_LOAD_ADDR*.

initrd_addr
    address to which the file specified by the -initrd parameter of QEMU shall
    be loaded. Defaults to environment variable *ramdiskaddr* and further to
    the value of *CFG_RAMDISK_ADDR*.

Examples
--------

QEMU firmware files are listed via the *qfw list* command:

::

    => qfw list
    00000000 bios-geometry
    00000000 bootorder
    000f0060 etc/acpi/rsdp
    bed14040 etc/acpi/tables
    00000000 etc/boot-fail-wait
    00000000 etc/e820
    00000000 etc/smbios/smbios-anchor
    00000000 etc/smbios/smbios-tables
    00000000 etc/system-states
    00000000 etc/table-loader
    00000000 etc/tpm/log
    00000000 genroms/kvmvapic.bin

Where an address is shown, it indicates where the data is available for
inspection, e.g. using the :doc:`md`.

The available CPUs can be shown via the *qfw cpus* command:

::

    => qfw cpu
    2 cpu(s) online

The *-kernel* and *-initrd* parameters allow to specify a kernel and an
initial RAM disk for QEMU:

.. code-block:: bash

   $ qemu-system-x86_64 -machine pc-i440fx-2.5 -bios u-boot.rom -m 1G \
       -nographic -kernel vmlinuz -initrd initrd

Now the kernel and the initial RAM disk can be loaded to the U-Boot memory via
the *qfw load* command and booted thereafter.

::

    => qfw load ${kernel_addr_r} ${ramdisk_addr_r}
    loading kernel to address 0000000001000000 size 5048f0 initrd 0000000004000000 size 3c94891
    => zboot 1000000 5048f0 4000000 3c94891
    Valid Boot Flag
    Magic signature found
    Linux kernel version 4.19.0-14-amd64 (debian-kernel@lists.debian.org) #1 SMP Debian 4.19.171-2 (2021-01-30)
    Building boot_params at 0x00090000
    Loading bzImage at address 100000 (5260160 bytes)

Configuration
-------------

The qfw command is only available if CONFIG_CMD_QFW=y.
