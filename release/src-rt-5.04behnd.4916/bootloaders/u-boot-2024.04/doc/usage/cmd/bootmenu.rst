.. SPDX-License-Identifier: GPL-2.0+
.. (C) Copyright 2011-2012 Pali Rohár <pali@kernel.org>

.. index::
   single: bootmenu (command)

bootmenu command
================

Synopsis
--------
::

    bootmenu [delay]

Description
-----------

The "bootmenu" command uses U-Boot menu interfaces and provides
a simple mechanism for creating menus with different boot items.
The cursor keys "Up" and "Down" are used for navigation through
the items. Current active menu item is highlighted and can be
selected using the "Enter" key. The selection of the highlighted
menu entry invokes an U-Boot command (or a list of commands)
associated with this menu entry.

The "bootmenu" command interprets ANSI escape sequences, so
an ANSI terminal is required for proper menu rendering and item
selection.

The assembling of the menu is done via a set of environment variables
"bootmenu_<num>" and "bootmenu_delay", i.e.::

    bootmenu_delay=<delay>
    bootmenu_<num>="<title>=<commands>"

<delay>
    is the autoboot delay in seconds, after which the first
    menu entry will be selected automatically

<num>
    is the boot menu entry number, starting from zero

<title>
    is the text of the menu entry shown on the console
    or on the boot screen

<commands>
    are commands which will be executed when a menu
    entry is selected

Title and commands are separated by the first appearance of a '='
character in the value of the environment variable.

The first (optional) argument of the "bootmenu" command is a delay specifier
and it overrides the delay value defined by "bootmenu_delay" environment
variable. If the environment variable "bootmenu_delay" is not set or if
the argument of the "bootmenu" command is not specified, the default delay
will be CONFIG_BOOTDELAY. If delay is 0, no menu entries will be shown on
the console (or on the screen) and the command of the first menu entry will
be called immediately. If delay is less then 0, bootmenu will be shown and
autoboot will be disabled.

Bootmenu always adds menu entry "U-Boot console" at the end of all menu
entries specified by environment variables. When selecting this entry
the bootmenu terminates and the usual U-Boot command prompt is presented
to the user.

Example environment::

    setenv bootmenu_0 Boot 1. kernel=bootm 0x82000000  # Set first menu entry
    setenv bootmenu_1 Boot 2. kernel=bootm 0x83000000  # Set second menu entry
    setenv bootmenu_2 Reset board=reset                # Set third menu entry
    setenv bootmenu_3 U-Boot boot order=boot           # Set fourth menu entry
    bootmenu 20        # Run bootmenu with autoboot delay 20s


The above example will be rendered as below::

    *** U-Boot Boot Menu ***

       Boot 1. kernel
       Boot 2. kernel
       Reset board
       U-Boot boot order
       U-Boot console

    Hit any key to stop autoboot: 20
    Press UP/DOWN to move, ENTER to select

The selected menu entry will be highlighted - it will have inverted
background and text colors.

UEFI boot variable enumeration
''''''''''''''''''''''''''''''
If enabled, the bootmenu command will automatically generate and add
UEFI-related boot menu entries for the following items.

 * possible bootable media with default file names
 * user-defined UEFI boot options

The bootmenu automatically enumerates the possible bootable
media devices supporting EFI_SIMPLE_FILE_SYSTEM_PROTOCOL.
This auto generated entry is named as "<interface> <devnum>:<part>" format.
(e.g. "usb 0:1")

The bootmenu displays the UEFI-related menu entries in order of "BootOrder".
When the user selects the UEFI boot menu entry, the bootmenu sets
the selected boot variable index to "BootNext" without non-volatile attribute,
then call the uefi boot manager with the command "bootefi bootmgr".

Example bootmenu is as below::

    *** U-Boot Boot Menu ***

       mmc 0:1
       mmc 0:2
       debian
       nvme 0:1
       ubuntu
       nvme 0:2
       usb 0:2
       U-Boot console

Default behavior when user exits from the bootmenu
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
User can exit from bootmenu by selecting the last entry
"U-Boot console"/"Quit" or ESC key.

When the CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE is disabled,
user exits from the bootmenu and returns to the U-Boot console.

When the CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE is enabled, user can not
enter the U-Boot console. When the user exits from the bootmenu,
the bootmenu invokes the following default behavior.

 * if CONFIG_CMD_BOOTEFI_BOOTMGR is enabled, execute "bootefi bootmgr" command
 * "bootefi bootmgr" fails or is not enabled, then execute "run bootcmd" command.

Configuration
-------------

The "bootmenu" command is enabled by::

    CONFIG_CMD_BOOTMENU=y

To run the bootmenu at startup add these additional settings::

    CONFIG_AUTOBOOT_KEYED=y
    CONFIG_BOOTDELAY=30
    CONFIG_AUTOBOOT_MENU_SHOW=y

UEFI boot variable enumeration is enabled by::

    CONFIG_CMD_BOOTEFI_BOOTMGR=y

To improve the product security, entering U-Boot console from bootmenu
can be disabled by::

    CONFIG_BOOTMENU_DISABLE_UBOOT_CONSOLE=y

To scan the discoverable devices connected to the buses such as
USB and PCIe prior to bootmenu showing up, CONFIG_PREBOOT can be
used to run the command before showing the bootmenu, i.e.::

    CONFIG_USE_PREBOOT=y
    CONFIG_PREBOOT="pci enum; usb start; scsi scan; nvme scan; virtio scan"
