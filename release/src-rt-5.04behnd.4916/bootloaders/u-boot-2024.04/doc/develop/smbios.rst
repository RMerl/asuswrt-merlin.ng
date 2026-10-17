.. SPDX-License-Identifier: GPL-2.0+

SMBIOS tables
=============

The System Management BIOS (SMBIOS) table is used to deliver management
information from the firmware to the operating system. The content is
standardized in [1]_.

In Linux you can use the dmidecode command to view the contents of the SMBIOS
table.

When booting via UEFI the SMBIOS table is transferred as an UEFI configuration
table to the operating system.

To generate SMBIOS tables in U-Boot, the CONFIG_GENERATE_SMBIOS_TABLE option
must be enabled. The easiest way to provide the values to use is via the device
tree. For details see
:download:`smbios.txt <../device-tree-bindings/sysinfo/smbios.txt>`.

.. [1] `System Management BIOS (SMBIOS) Reference, version 3.5
   <https://www.dmtf.org/content/dmtf-releases-smbios-35>`_
