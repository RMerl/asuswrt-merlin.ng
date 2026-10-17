.. SPDX-License-Identifier: GPL-2.0+

ACPI on QEMU
============

QEMU can provide ACPI tables on ARM, RISC-V (since QEMU v8.0.0), and x86.

The following U-Boot settings are needed for ACPI support::

    CONFIG_CMD_QFW=y
    CONFIG_ACPI=y
    CONFIG_GENERATE_ACPI_TABLE=y

On x86 these settings are already included in the defconfig files. ARM and
RISC-V default to use device-trees.

Instead of updating the configuration manually you can add the configuration
fragment `acpi.config` to the make command for initializing the configuration.
E.g.

.. code-block:: bash

    make qemu-riscv64_smode_defconfig acpi.config
