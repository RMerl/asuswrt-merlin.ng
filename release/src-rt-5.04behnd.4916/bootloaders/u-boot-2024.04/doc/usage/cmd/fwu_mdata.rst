.. SPDX-License-Identifier: GPL-2.0+

.. index::
   single: fwu_mdata_read (command)

fwu_mdata_read command
======================

Synopsis
--------

::

    fwu_mdata_read

Description
-----------

The fwu_mdata_read command is used to read the FWU metadata
structure. The command prints out information about the current active
bank, the previous active bank, image GUIDs, image acceptance etc.

The output may look like:

::

    => fwu_mdata_read
            FWU Metadata
    crc32: 0xec4fb997
    version: 0x1
    active_index: 0x0
    previous_active_index: 0x1
            Image Info

    Image Type Guid: 19D5DF83-11B0-457B-BE2C-7559C13142A5
    Location Guid: 49272BEB-8DD8-46DF-8D75-356C65EFF417
    Image Guid:  D57428CC-BB9A-42E0-AA36-3F5A132059C7
    Image Acceptance: yes
    Image Guid:  2BE37D6D-8281-4938-BD7B-9A5BBF80869F
    Image Acceptance: yes

Configuration
-------------

To use the fwu_mdata_read command, CONFIG_CMD_FWU_METADATA needs to be
enabled.
