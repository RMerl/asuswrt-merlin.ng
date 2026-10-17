.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2020, Heinrich Schuchardt <xypron.glpk@gmx.de>

.. index::
   single: bootefi (command)

bootefi command
===============

Synopsis
--------

::

    bootefi <image_addr>[:<image_size>] [<fdt_addr>]
    bootefi bootmgr [<fdt_addr>]
    bootefi hello [<fdt_addr>]
    bootefi selftest [<fdt_addr>]

Description
-----------

The *bootefi* command is used to launch a UEFI binary which can be either of

* UEFI application
* UEFI boot services driver
* UEFI run-time services driver

An operating system requires a hardware description which can either be
presented as ACPI table (CONFIG\_GENERATE\_ACPI\_TABLE=y) or as device-tree.
The load address of the device-tree may be provided as parameter *fdt\_addr*. If
this address is not specified, the bootefi command will try to fall back in
sequence to:

* the device-tree specified by environment variable *fdt\_addr*
* the device-tree specified by environment variable *fdtcontroladdr*

The load address of the binary is specified by parameter *image_address*. A
command sequence to run a UEFI application might look like

::

    load mmc 0:2 $fdt_addr_r dtb
    load mmc 0:1 $kernel_addr_r /EFI/grub/grubaa64.efi
    bootefi $kernel_addr_r $fdt_addr_r

The last UEFI binary loaded defines the image file path in the loaded image
protocol.

The value of the environment variable *bootargs* is converted from UTF-8 to
UTF-16 and passed as load options in the loaded image protocol to the UEFI
binary.

image_addr
    Address of the UEFI binary.

fdt_addr
    Address of the device-tree or '-'. If no address is specifiy, the
    environment variable $fdt_addr is used as first fallback, the address of
    U-Boot's internal device-tree $fdtcontroladdr as second fallback.
    When using ACPI no device-tree shall be specified.

image_size
    Size of the UEFI binary file. This argument is only needed if *image_addr*
    does not match the address of the last loaded UEFI binary. In this case
    a memory device path will be used as image file path in the loaded image
    protocol.

Note
    UEFI binaries that are contained in FIT images are launched via the
    *bootm* command.

UEFI boot manager
'''''''''''''''''

The UEFI boot manager is invoked by the *bootefi bootmgr* sub-command.
Here boot options are defined by UEFI variables with a name consisting of the
letters *Boot* followed by a four digit hexadecimal number, e.g. *Boot0001* or
*BootA03E*. The boot variable defines a label, the device path of the binary to
execute as well as the load options passed in the loaded image protocol.

If the UEFI variable *BootNext* is defined, it specifies the number of the boot
option to execute next. If no binary can be loaded via *BootNext* the variable
*BootOrder* specifies in which sequence boot options shalled be tried.

The values of these variables can be managed using the U-Boot command
*efidebug*.

UEFI hello world application
''''''''''''''''''''''''''''

U-Boot can be compiled with a hello world application that can be launched using
the *bootefi hello* sub-command. A session might look like

::

    => setenv bootargs 'Greetings to the world'
    => bootefi hello
    Booting /MemoryMapped(0x0,0x10001000,0x1000)
    Hello, world!
    Running on UEFI 2.8
    Have SMBIOS table
    Have device tree
    Load options: Greetings to the world

UEFI selftest
'''''''''''''

U-Boot can be compiled with UEFI unit tests. These unit tests are invoked using
the *bootefi selftest* sub-command.

Which unit test is executed is controlled by the environment variable
*efi\_selftest*. If this variable is not set, all unit tests that are not marked
as 'on request' are executed.

To show a list of the available unit tests the value *list* can be used

::

    => setenv efi_selftest list
    => bootefi selftest

    Available tests:
    'block image transfer' - on request
    'block device'
    'configuration tables'
    ...

A single test is selected for execution by setting the *efi\_selftest*
environment variable to match one of the listed identifiers

::

    => setenv efi_selftest 'block image transfer'
    => bootefi selftest

Some of the tests execute the ExitBootServices() UEFI boot service and will not
return to the command line but require a board reset.

Configuration
-------------

To use the *bootefi* command you must specify CONFIG\_CMD\_BOOTEFI=y.
The *bootefi bootmgr* sub-command requries CMD\_BOOTEFI\_BOOTMGR=y.
The *bootefi hello* sub-command requries CMD\_BOOTEFI\_HELLO=y.
The *bootefi selftest* sub-command depends on CMD\_BOOTEFI\_SELFTEST=y.

See also
--------

* *bootm* for launching UEFI binaries packed in FIT images
* :doc:`booti<booti>`, *bootm*, *bootz* for launching a Linux kernel without
  using the UEFI sub-system
* *efidebug* for setting UEFI boot variables and boot options
