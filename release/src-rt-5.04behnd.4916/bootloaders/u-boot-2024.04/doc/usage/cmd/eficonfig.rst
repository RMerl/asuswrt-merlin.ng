.. SPDX-License-Identifier: GPL-2.0+
.. (C) Copyright 2022, Masahisa Kojima <masahisa.kojima@linaro.org>

.. index::
   single: eficonfig (command)

eficonfig command
=================

Synopsis
--------
::

    eficonfig

Description
-----------

The "eficonfig" command uses the U-Boot menu interface to provide a
menu-driven UEFI variable maintenance feature. These are the top level menu
entries:

Add Boot Option
    Add a new UEFI Boot Option.
    The user can edit description, file path, and optional_data.
    The new boot opiton is appended to the boot order in the *BootOrder*
    variable. The user may want to update the boot order using the
    *Change Boot Order* menu entry.

Edit Boot Option
    Edit an existing UEFI Boot Option.
    The User can edit description, file path, and optional_data.

Change Boot Order
    Change the boot order updating the UEFI BootOrder variable.

Delete Boot Option
    Delete a UEFI Boot Option

Secure Boot Configuration
    Edit the UEFI Secure Boot Configuration

How to boot the system with a newly added UEFI Boot Option
''''''''''''''''''''''''''''''''''''''''''''''''''''''''''

The "eficonfig" command is used to set the UEFI boot options which are stored
in the UEFI variable Boot#### where #### is a hexadecimal number.

The command *bootefi bootmgr* can be used to boot by trying in sequence all
boot options selected by the variable *BootOrder*.

If the bootmenu is enabled, CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE is enabled,
and "eficonfig" is configured as preboot command, the newly added Boot Options
are enumerated in the bootmenu when the user exits from the eficonfig menu.
The user may select the entry in the bootmenu to boot the system, or follow
the U-Boot configuration the system already has.

Auto boot with the UEFI Boot Option
'''''''''''''''''''''''''''''''''''

To do auto boot according to the UEFI BootOrder variable,
add "bootefi bootmgr" entry as a default or first bootmenu entry::

    CONFIG_PREBOOT="setenv bootmenu_0 UEFI Boot Manager=bootefi bootmgr; setenv bootmenu_1 UEFI Maintenance Menu=eficonfig"

UEFI Secure Boot Configuration
''''''''''''''''''''''''''''''

The user can enroll the variables PK, KEK, db and dbx by selecting a file.
The "eficonfig" command only accepts signed EFI Signature List(s) with an
authenticated header, typically a ".auth" file.

To clear the PK, KEK, db and dbx, the user needs to enroll a null value
signed by PK or KEK.

Configuration
-------------

The "eficonfig" command is enabled by::

    CONFIG_CMD_EFICONFIG=y

If CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE is enabled, the user can not enter
U-Boot console. In this case, the bootmenu can be used to invoke "eficonfig"::

    CONFIG_USE_PREBOOT=y
    CONFIG_PREBOOT="setenv bootmenu_0 UEFI Maintenance Menu=eficonfig"

The only way U-Boot can currently store EFI variables on a tamper
resistant medium is via OP-TEE. The Kconfig option that enables that is::

    CONFIG_EFI_MM_COMM_TEE=y.

It enables storing EFI variables on the RPMB partition of an eMMC device.

The UEFI Secure Boot Configuration menu entry is only available if the following
options are enabled::

    CONFIG_EFI_SECURE_BOOT=y
    CONFIG_EFI_MM_COMM_TEE=y

See also
--------

* :doc:`bootmenu<bootmenu>` provides a simple mechanism for creating menus with
  different boot items
