.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Manorit Chawdhry <m-chawdhry@ti.com>

J721S2 and AM68 Platforms
=========================

Introduction:
-------------

The J721S2 family of SoCs are part of K3 Multicore SoC architecture platform
targeting automotive applications. They are designed as a low power, high
performance and highly integrated device architecture, adding significant
enhancement on processing power, graphics capability, video and imaging
processing, virtualization and coherent memory support.

The AM68 Starter Kit/Evaluation Module (EVM) is based on the J721S2 family
of SoCs. They are designed for machine vision, traffic monitoring, retail
automation, and factory automation.

The device is partitioned into three functional domains, each containing
specific processing cores and peripherals:

1. Wake-up (WKUP) domain:
    * ARM Cortex-M4F processor, runs TI Foundational Security (TIFS)

2. Microcontroller (MCU) domain:
    * Dual core ARM Cortex-R5F processor, runs device management
      and SoC early boot

3. MAIN domain:
    * Dual core 64-bit ARM Cortex-A72, runs HLOS

More info can be found in TRM: https://www.ti.com/lit/pdf/spruj28

Platform information:

* https://www.ti.com/tool/J721S2XSOMXEVM
* https://www.ti.com/tool/SK-AM68

Boot Flow:
----------

Below is the pictorial representation of boot flow:

.. image:: img/boot_diagram_k3_current.svg

- On this platform, "TI Foundational Security" (TIFS) functions as the
  security enclave master while "Device Manager" (DM), also known as the
  "TISCI server" in TI terminology, offers all the essential services.

- As illustrated in the diagram above, R5 SPL manages power and clock
  services independently before handing over control to "DM". The A72 or
  the C7x (Aux core) software components request TIFS/DM to handle
  security or device management services.

Sources:
--------

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

Build procedure:
----------------

0. Setup the environment variables:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_desc
    :end-before: .. k3_rst_include_end_common_env_vars_desc

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_board_env_vars_desc
    :end-before: .. k3_rst_include_end_board_env_vars_desc

Set the variables corresponding to this platform:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_defn
    :end-before: .. k3_rst_include_end_common_env_vars_defn
.. prompt:: bash $

  export UBOOT_CFG_CORTEXR=j721s2_evm_r5_defconfig
  export UBOOT_CFG_CORTEXA=j721s2_evm_a72_defconfig
  export TFA_BOARD=generic
  export TFA_EXTRA_ARGS="K3_USART=0x8"
  # The following is not a typo, j784s4 is the OP-TEE platform for j721s2
  export OPTEE_PLATFORM=k3-j784s4
  export OPTEE_EXTRA_ARGS="CFG_CONSOLE_UART=0x8"

.. j721s2_evm_rst_include_start_build_steps

1. Trusted Firmware-A:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_tfa
    :end-before: .. k3_rst_include_end_build_steps_tfa


2. OP-TEE:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_optee
    :end-before: .. k3_rst_include_end_build_steps_optee

3. U-Boot:

.. _j721s2_evm_rst_u_boot_r5:

* 3.1 R5:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_spl_r5
    :end-before: .. k3_rst_include_end_build_steps_spl_r5

.. _j721s2_evm_rst_u_boot_a72:

* 3.2 A72:

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_build_steps_uboot
    :end-before: .. k3_rst_include_end_build_steps_uboot
.. j721s2_evm_rst_include_end_build_steps

Target Images
-------------

In order to boot we need tiboot3.bin, tispl.bin and u-boot.img. Each SoC
variant (GP, HS-FS, HS-SE) requires a different source for these files.

 - GP

    * tiboot3-j721s2-gp-evm.bin from :ref:`step 3.1 <j721s2_evm_rst_u_boot_r5>`
    * tispl.bin_unsigned, u-boot.img_unsigned from :ref:`step 3.2 <j721s2_evm_rst_u_boot_a72>`

 - HS-FS

    * tiboot3-j721s2-hs-fs-evm.bin from :ref:`step 3.1 <j721s2_evm_rst_u_boot_r5>`
    * tispl.bin, u-boot.img from :ref:`step 3.2 <j721s2_evm_rst_u_boot_a72>`

 - HS-SE

    * tiboot3-j721s2-hs-evm.bin from :ref:`step 3.1 <j721s2_evm_rst_u_boot_r5>`
    * tispl.bin, u-boot.img from :ref:`step 3.2 <j721s2_evm_rst_u_boot_a72>`

Image formats:
--------------

- tiboot3.bin

.. image:: img/multi_cert_tiboot3.bin.svg

- tispl.bin

.. image:: img/dm_tispl.bin.svg

R5 Memory Map:
--------------

.. list-table::
   :widths: 16 16 16
   :header-rows: 1

   * - Region
     - Start Address
     - End Address

   * - SPL
     - 0x41c00000
     - 0x41c40000

   * - EMPTY
     - 0x41c40000
     - 0x41c61f20

   * - STACK
     - 0x41c65f20
     - 0x41c61f20

   * - Global data
     - 0x41c65f20
     - 0x41c66000

   * - Heap
     - 0x41c66000
     - 0x41c76000

   * - BSS
     - 0x41c76000
     - 0x41c80000

   * - DM DATA
     - 0x41c80000
     - 0x41c84130

   * - EMPTY
     - 0x41c84130
     - 0x41cff9fc

   * - MCU Scratchpad
     - 0x41cff9fc
     - 0x41cffbfc

   * - ROM DATA
     - 0x41cffbfc
     - 0x41cfffff

Switch Setting for Boot Mode
----------------------------

Boot Mode pins provide means to select the boot mode and options before the
device is powered up. After every POR, they are the main source to populate
the Boot Parameter Tables.

Boot Mode Pins for J721S2-EVM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

The following table shows some common boot modes used on J721S2 platform.
More details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/spruj28 under the `Boot Mode Pins` section.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW9: 12345678
     - SW8: 12345678

   * - SD
     - 00000000
     - 10000010

   * - EMMC
     - 01000000
     - 10000000

   * - OSPI
     - 01000000
     - 00000110

   * - UART
     - 01110000
     - 00000000

   * - USB DFU
     - 00100000
     - 10000000

For SW8 and SW9, the switch state in the "ON" position = 1.

Boot Mode Pins for SK-AM68
^^^^^^^^^^^^^^^^^^^^^^^^^^

The following table shows some common boot modes used on AM68-SK platform.
More details can be found in the User Guide for AM68-SK:
https://www.ti.com/lit/pdf/spruj68 under the `Bootmode Settings` section.

.. list-table:: Boot Modes
   :widths: 16 16
   :header-rows: 1

   * - Switch Label
     - SW1: 1234

   * - SD
     - 0000

   * - xSPI
     - 0010

   * - UART
     - 1010

   * - Ethernet
     - 0100

For SW1, the switch state in the "ON" position = 1.

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup information.

.. warning::

  **OpenOCD support since**: v0.12.0

  If the default package version of OpenOCD in your development
  environment's distribution needs to be updated, it might be necessary to
  build OpenOCD from the source.

Debugging U-Boot on J721S2-EVM
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_XDS110
    :end-before: .. k3_rst_include_end_openocd_connect_XDS110

To start OpenOCD and connect to the board

.. prompt:: bash $

  openocd -f board/ti_j721s2evm.cfg

Debugging U-Boot on SK-AM68
^^^^^^^^^^^^^^^^^^^^^^^^^^^

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_cti20
    :end-before: .. k3_rst_include_end_openocd_connect_cti20

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_cfg_external_intro
    :end-before: .. k3_rst_include_end_openocd_cfg_external_intro

For SK-AM68, the openocd_connect.cfg is as follows:

.. code-block:: tcl

  # TUMPA example:
  # http://www.tiaowiki.com/w/TIAO_USB_Multi_Protocol_Adapter_User's_Manual
  source [find interface/ftdi/tumpa.cfg]

  transport select jtag

  # default JTAG configuration has only SRST and no TRST
  reset_config srst_only srst_push_pull

  # delay after SRST goes inactive
  adapter srst delay 20

  if { ![info exists SOC] } {
    # Set the SoC of interest
    set SOC j721s2
  }

  source [find target/ti_k3.cfg]

  ftdi tdo_sample_edge falling

  # Speeds for FT2232H are in multiples of 2, and 32MHz is tops
  # max speed we seem to achieve is ~20MHz.. so we pick 16MHz
  adapter speed 16000
