.. SPDX-License-Identifier: GPL-2.0+:

.. index::
   single: printenv (command)

printenv command
================

Synopsis
--------

::

    printenv [-a] [name ...]
    printenv -e [-guid guid][-n] [name]

Description
-----------

The printenv command is used to print environment or UEFI variables.

\-a
    Print environment variables starting with a period ('.').

\-e
    Print UEFI variables. Without -e environment variables are printed.

\-guid *guid*
    Specify vendor GUID *guid*. If none is specified, all UEFI variables with
    the specified name are printed irrespective of their vendor GUID.

\-n
    don't show hexadecimal dump of value

name
    Variable name. If no name is provided, all variables are printed.
    Multiple environment variable names may be specified.

Examples
--------

The following examples demonstrates the effect of the *-a* flag when displaying
environment variables:

::

    => setenv .foo bar
    => printenv
    arch=sandbox
    baudrate=115200
    board=sandbox
    ...
    stdout=serial,vidconsole

    Environment size: 644/8188 bytes
    => printenv -a
    .foo=bar
    arch=sandbox
    baudrate=115200
    board=sandbox
    ...
    stdout=serial,vidconsole

    Environment size: 653/8188 bytes
    =>

The next example shows the effect of the *-n* flag when displaying an UEFI
variable and how to specify a vendor GUID:

::

    => printenv -e -guid 8be4df61-93ca-11d2-aa0d-00e098032b8c PlatformLangCodes
    PlatformLangCodes:
        8be4df61-93ca-11d2-aa0d-00e098032b8c (EFI_GLOBAL_VARIABLE_GUID)
        BS|RT|RO, DataSize = 0x6
        00000000: 65 6e 2d 55 53 00                                en-US.
    => printenv -e -n PlatformLangCodes
    PlatformLangCodes:
        8be4df61-93ca-11d2-aa0d-00e098032b8c (EFI_GLOBAL_VARIABLE_GUID)
        BS|RT|RO, DataSize = 0x6
    =>

Configuration
-------------

UEFI variables are only supported if CONFIG_CMD_NVEDIT_EFI=y. The value of UEFI
variables can only be displayed if CONFIG_HEXDUMP=y.

Return value
------------

The return value $? is 1 (false) if a specified variable is not found.
Otherwise $? is set to 0 (true).
