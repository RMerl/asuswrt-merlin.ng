.. SPDX-License-Identifier: GPL-2.0+

Android Verified Boot 2.0
=========================

This file contains information about the current support of Android Verified
Boot 2.0 in U-Boot.

Overview
--------

Verified Boot establishes a chain of trust from the bootloader to system images:

* Provides integrity checking for:

  * Android Boot image: Linux kernel + ramdisk. RAW hashing of the whole
    partition is done and the hash is compared with the one stored in
    the VBMeta image
  * ``system``/``vendor`` partitions: verifying root hash of dm-verity hashtrees

* Provides capabilities for rollback protection

Integrity of the bootloader (U-Boot BLOB and environment) is out of scope.

For additional details check [1]_.

AVB using OP-TEE (optional)
^^^^^^^^^^^^^^^^^^^^^^^^^^^

If AVB is configured to use OP-TEE (see `Enable on your board`_) rollback
indexes and device lock state are stored in RPMB. The RPMB partition is managed
by OP-TEE (see [2]_ for details) which is a secure OS leveraging ARM
TrustZone.

AVB 2.0 U-Boot shell commands
-----------------------------

Provides CLI interface to invoke AVB 2.0 verification + misc. commands for
different testing purposes::

    avb init <dev> - initialize avb 2 for <dev>
    avb read_rb <num> - read rollback index at location <num>
    avb write_rb <num> <rb> - write rollback index <rb> to <num>
    avb is_unlocked - returns unlock status of the device
    avb get_uuid <partname> - read and print uuid of partition <part>
    avb read_part <partname> <offset> <num> <addr> - read <num> bytes from
        partition <partname> to buffer <addr>
    avb read_part_hex <partname> <offset> <num> - read <num> bytes from
        partition <partname> and print to stdout
    avb write_part <partname> <offset> <num> <addr> - write <num> bytes to
        <partname> by <offset> using data from <addr>
    avb read_pvalue <name> <bytes> - read a persistent value <name>
    avb write_pvalue <name> <value> - write a persistent value <name>
    avb verify [slot_suffix] - run verification process using hash data
        from vbmeta structure
        [slot_suffix] - _a, _b, etc (if vbmeta partition is slotted)

Partitions tampering (example)
------------------------------

Boot or system/vendor (dm-verity metadata section) is tampered::

   => avb init 1
   => avb verify
   avb_slot_verify.c:175: ERROR: boot: Hash of data does not match digest in
   descriptor.
   Slot verification result: ERROR_IO

Vbmeta partition is tampered::

   => avb init 1
   => avb verify
   avb_vbmeta_image.c:206: ERROR: Hash does not match!
   avb_slot_verify.c:388: ERROR: vbmeta: Error verifying vbmeta image:
   HASH_MISMATCH
   Slot verification result: ERROR_IO

Enable on your board
--------------------

The following options must be enabled::

   CONFIG_LIBAVB=y
   CONFIG_AVB_VERIFY=y
   CONFIG_CMD_AVB=y

In addtion optionally if storing rollback indexes in RPMB with help of
OP-TEE::

   CONFIG_TEE=y
   CONFIG_OPTEE=y
   CONFIG_OPTEE_TA_AVB=y
   CONFIG_SUPPORT_EMMC_RPMB=y

Then add ``avb verify`` invocation to your android boot sequence of commands,
e.g.::

   => avb_verify=avb init $mmcdev; avb verify;
   => if run avb_verify; then                       \
           echo AVB verification OK. Continue boot; \
           set bootargs $bootargs $avb_bootargs;    \
      else                                          \
           echo AVB verification failed;            \
           exit;                                    \
      fi;                                           \

   => emmc_android_boot=                                   \
          echo Trying to boot Android from eMMC ...;       \
          ...                                              \
          run avb_verify;                                  \
          mmc read ${fdtaddr} ${fdt_start} ${fdt_size};    \
          mmc read ${loadaddr} ${boot_start} ${boot_size}; \
               bootm $loadaddr $loadaddr $fdtaddr;         \

If partitions you want to verify are slotted (have A/B suffixes), then current
slot suffix should be passed to ``avb verify`` sub-command, e.g.::

   => avb verify _a

To switch on automatic generation of vbmeta partition in AOSP build, add these
lines to device configuration mk file::

   BOARD_AVB_ENABLE := true
   BOARD_AVB_ALGORITHM := SHA512_RSA4096
   BOARD_BOOTIMAGE_PARTITION_SIZE := <boot partition size>

After flashing U-Boot don't forget to update environment and write new
partition table::

   => env default -f -a
   => setenv partitions $partitions_android
   => env save
   => gpt write mmc 1 $partitions_android

References
----------

.. [1] https://android.googlesource.com/platform/external/avb/+/master/README.md
.. [2] https://www.op-tee.org/
