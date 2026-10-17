.. SPDX-License-Identifier: GPL-2.0+

Android Bootloader Control Block (BCB)
======================================

The purpose behind this file is to:

* give an overview of BCB w/o duplicating public documentation
* describe the main BCB use-cases which concern U-Boot
* reflect current support status in U-Boot
* mention any relevant U-Boot build-time tunables
* precisely exemplify one or more use-cases

Additions and fixes are welcome!

Overview
--------

Bootloader Control Block (BCB) is a well established term/acronym in
the Android namespace which refers to a location in a dedicated raw
(i.e. FS-unaware) flash (e.g. eMMC) partition, usually called ``misc``,
which is used as media for exchanging messages between Android userspace
(particularly recovery [1]_) and an Android-capable bootloader.

On higher level, BCB provides a way to implement a subset of Android
Bootloader Requirements [2]_, amongst which are:

* Android-specific bootloader flow [3]_
* Get the "reboot reason" (and act accordingly) [4]_
* Get/pass a list of commands from/to recovery [1]_
* TODO


'bcb'. Shell command overview
-----------------------------

The ``bcb`` command provides a CLI to facilitate the development of the
requirements enumerated above. Below is the command's help message::

   => bcb
   bcb - Load/set/clear/test/dump/store Android BCB fields

   Usage:
   bcb load <interface> <dev> <part>  - load  BCB from <interface> <dev>:<part>
   load <dev> <part>              - load  BCB from mmc <dev>:<part>
   bcb set   <field> <val>        - set   BCB <field> to <val>
   bcb clear [<field>]            - clear BCB <field> or all fields
   bcb test  <field> <op> <val>   - test  BCB <field> against <val>
   bcb dump  <field>              - dump  BCB <field>
   bcb store                      - store BCB back to <interface>

   Legend:
   <interface> - storage device interface (virtio, mmc, etc)
   <dev>       - storage device index containing the BCB partition
   <part>      - partition index or name containing the BCB
   <field>     - one of {command,status,recovery,stage,reserved}
   <op>        - the binary operator used in 'bcb test':
                 '=' returns true if <val> matches the string stored in <field>
                 '~' returns true if <val> matches a subset of <field>'s string
   <val>       - string/text provided as input to bcb {set,test}
                 NOTE: any ':' character in <val> will be replaced by line feed
                 during 'bcb set' and used as separator by upper layers


'bcb'. Example of getting reboot reason
---------------------------------------

.. code-block:: bash

   if bcb load 1 misc; then
       # valid BCB found
       if bcb test command = bootonce-bootloader; then
           bcb clear command; bcb store;
           # do the equivalent of AOSP ${fastbootcmd}
           # i.e. call fastboot
       else if bcb test command = boot-recovery; then
           bcb clear command; bcb store;
           # do the equivalent of AOSP ${recoverycmd}
           # i.e. do anything required for booting into recovery
       else
           # boot Android OS normally
       fi
   else
       # corrupted/non-existent BCB
       # report error or boot non-Android OS (platform-specific)
   fi


Enable on your board
--------------------

The following Kconfig options must be enabled::

   CONFIG_PARTITIONS=y
   CONFIG_MMC=y
   CONFIG_CMD_BCB=y

.. [1] https://android.googlesource.com/platform/bootable/recovery
.. [2] https://source.android.com/devices/bootloader
.. [3] https://patchwork.ozlabs.org/patch/746835/
       ("[U-Boot,5/6] Initial support for the Android Bootloader flow")
.. [4] https://source.android.com/devices/bootloader/boot-reason
