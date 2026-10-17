.. SPDX-License-Identifier: GPL-2.0+

UEFI subsystem
==============

Lauching UEFI images
--------------------

Bootefi command
~~~~~~~~~~~~~~~

The bootefi command is used to start UEFI applications or to install UEFI
drivers. It takes two parameters

    bootefi <image address> [fdt address]

* image address - the memory address of the UEFI binary
* fdt address - the memory address of the flattened device tree

The environment variable 'bootargs' is passed as load options in the UEFI system
table. The Linux kernel EFI stub uses the load options as command line
arguments.

.. kernel-doc:: cmd/bootefi.c
   :internal:

Boot manager
~~~~~~~~~~~~

The UEFI specification foresees to define boot entries and boot sequence via UEFI
variables. Booting according to these variables is possible via

    bootefi bootmgr [fdt address]

* fdt address - the memory address of the flattened device tree

The relevant variables are:

* Boot0000-BootFFFF define boot entries
* BootNext specifies next boot option to be booted
* BootOrder specifies in which sequence the boot options shall be tried if
  BootNext is not defined or booting via BootNext fails

.. kernel-doc:: lib/efi_loader/efi_bootmgr.c
   :internal:

Efidebug command
~~~~~~~~~~~~~~~~

The efidebug command is used to set and display boot options as well as to
display information about internal data of the UEFI subsystem (devices,
drivers, handles, loaded images, and the memory map).

.. kernel-doc:: cmd/efidebug.c
   :internal:

Initialization of the UEFI sub-system
-------------------------------------

.. kernel-doc:: lib/efi_loader/efi_setup.c
   :internal:

Boot services
-------------

.. kernel-doc:: lib/efi_loader/efi_boottime.c
   :internal:

Image relocation
~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_image_loader.c
   :internal:

Memory services
~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_memory.c
   :internal:

SetWatchdogTimer service
~~~~~~~~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_watchdog.c
   :internal:

Runtime services
----------------

.. kernel-doc:: lib/efi_loader/efi_runtime.c
   :internal:

Variable services
~~~~~~~~~~~~~~~~~

.. kernel-doc:: include/efi_variable.h
   :internal:
.. kernel-doc:: lib/efi_loader/efi_variable.c
   :internal:

UEFI drivers
------------

UEFI driver uclass
~~~~~~~~~~~~~~~~~~
.. kernel-doc:: lib/efi_driver/efi_uclass.c
   :internal:

Block device driver
~~~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_driver/efi_block_device.c
   :internal:

Protocols
---------

Block IO protocol
~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_disk.c
   :internal:

File protocol
~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_file.c
   :internal:

Graphical output protocol
~~~~~~~~~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_gop.c
   :internal:

Load file 2 protocol
~~~~~~~~~~~~~~~~~~~~

The load file 2 protocol can be used by the Linux kernel to load the initial
RAM disk. U-Boot can be configured to provide an implementation.

.. kernel-doc:: lib/efi_loader/efi_load_initrd.c
   :internal:

Network protocols
~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_net.c
   :internal:

Random number generator protocol
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_rng.c
   :internal:

Text IO protocols
~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_console.c
   :internal:

Unicode Collation protocol
~~~~~~~~~~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_unicode_collation.c
   :internal:

Firmware management protocol
~~~~~~~~~~~~~~~~~~~~~~~~~~~~

.. kernel-doc:: lib/efi_loader/efi_firmware.c
   :internal:

Driver binding protocol
~~~~~~~~~~~~~~~~~~~~~~~

.. kernel-doc:: include/efi_driver.h
   :internal:

Unit testing
------------

The following library functions are provided to support writing UEFI unit tests.
The should not be used elsewhere.

.. kernel-doc:: include/efi_selftest.h
   :internal:
