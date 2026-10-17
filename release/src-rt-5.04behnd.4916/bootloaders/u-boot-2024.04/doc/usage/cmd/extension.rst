.. SPDX-License-Identifier: GPL-2.0+
.. Copyright 2021, Kory Maincent <kory.maincent@bootlin.com>

.. index::
   single: extension (command)

extension command
=================

Synopsis
--------

::

    extension scan
    extension list
    extension apply <extension number|all>

Description
-----------

The "extension" command proposes a generic U-Boot mechanism to detect
extension boards connected to the HW platform, and apply the appropriate
Device Tree overlays depending on the detected extension boards.

The "extension" command comes with three sub-commands:

 - "extension scan" makes the generic code call the board-specific
   extension_board_scan() function to retrieve the list of detected
   extension boards.

 - "extension list" allows to list the detected extension boards.

 - "extension apply <number>|all" allows to apply the Device Tree
   overlay(s) corresponding to one, or all, extension boards

The latter requires two environment variables to exist:

 - extension_overlay_addr: the RAM address where to load the Device
   Tree overlays

 - extension_overlay_cmd: the U-Boot command to load one overlay.
   Indeed, the location and mechanism to load DT overlays is very setup
   specific.

In order to enable this mechanism, board-specific code must implement
the extension_board_scan() function that fills in a linked list of
"struct extension", each describing one extension board. In addition,
the board-specific code must select the SUPPORT_EXTENSION_SCAN Kconfig
boolean.

Usage example
-------------

1. Make sure your devicetree is loaded and set as the working fdt tree.

::

    => run loadfdt
    => fdt addr $fdtaddr

2. Prepare the environment variables

::

    => setenv extension_overlay_addr 0x88080000
    => setenv extension_overlay_cmd 'load mmc 0:1 ${extension_overlay_addr} /boot/${extension_overlay_name}'

3. Detect the plugged extension board

::

    => extension scan

4. List the plugged extension board information and the devicetree
   overlay name

::

    => extension list

5. Apply the appropriate devicetree overlay

For apply the selected overlay:

::

    => extension apply 0

For apply all the overlays:

::

    => extension apply all

Simple extension_board_scan function example
--------------------------------------------

.. code-block:: c

    int extension_board_scan(struct list_head *extension_list)
    {
        struct extension *extension;

        extension = calloc(1, sizeof(struct extension));
        snprintf(extension->overlay, sizeof(extension->overlay), "overlay.dtbo");
        snprintf(extension->name, sizeof(extension->name), "extension board");
        snprintf(extension->owner, sizeof(extension->owner), "sandbox");
        snprintf(extension->version, sizeof(extension->version), "1.1");
        snprintf(extension->other, sizeof(extension->other), "Extension board information");
        list_add_tail(&extension->list, extension_list);

        return 1;
    }
