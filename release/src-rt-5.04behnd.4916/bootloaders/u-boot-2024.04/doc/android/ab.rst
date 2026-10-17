.. SPDX-License-Identifier: GPL-2.0+

Android A/B updates
===================

Overview
--------

A/B system updates ensures modern approach for system update. This feature
allows one to use two sets (or more) of partitions referred to as slots
(normally slot A and slot B). The system runs from the current slot while the
partitions in the unused slot can be updated [1]_.

A/B enablement
--------------

The A/B updates support can be activated by specifying next options in
your board configuration file::

    CONFIG_ANDROID_AB=y
    CONFIG_CMD_AB_SELECT=y

The disk space on target device must be partitioned in a way so that each
partition which needs to be updated has two or more instances. The name of
each instance must be formed by adding suffixes: ``_a``, ``_b``, ``_c``, etc.
For example: ``boot_a``, ``boot_b``, ``system_a``, ``system_b``, ``vendor_a``,
``vendor_b``.

As a result you can use ``ab_select`` command to ensure A/B boot process in your
boot script. This command analyzes and processes A/B metadata stored on a
special partition (e.g. ``misc``) and determines which slot should be used for
booting up.

If the A/B metadata partition has a backup bootloader_message block that is used
to ensure one is always valid even in the event of interruption when writing, it
can be enabled in your board configuration file::

    CONFIG_ANDROID_AB_BACKUP_OFFSET=0x1000

Command usage
-------------

.. code-block:: none

    ab_select <slot_var_name> <interface> <dev[:part_number|#part_name]>

for example::

    => ab_select slot_name mmc 1:4

or::

    => ab_select slot_name mmc 1#misc

Result::

    => printenv slot_name
    slot_name=a

Based on this slot information, the current boot partition should be defined,
and next kernel command line parameters should be generated:

* ``androidboot.slot_suffix=``
* ``root=``

For example::

    androidboot.slot_suffix=_a root=/dev/mmcblk1p12

A/B metadata is organized according to AOSP reference [2]_. On the first system
start with A/B enabled, when ``misc`` partition doesn't contain required data,
the default A/B metadata will be created and written to ``misc`` partition.

References
----------

.. [1] https://source.android.com/devices/tech/ota/ab
.. [2] https://android.googlesource.com/platform/bootable/recovery/+/refs/tags/android-10.0.0_r25/bootloader_message/include/bootloader_message/bootloader_message.h
