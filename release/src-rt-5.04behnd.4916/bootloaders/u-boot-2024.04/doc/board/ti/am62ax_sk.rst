.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Jai Luthra <j-luthra@ti.com>

AM62A Platforms
===============

Introduction:
-------------
The AM62A SoC family is built on the K3 Multicore SoC architecture platform,
providing a deep learning accelerator, multi-camera support with ISP, video
transcoder and other BOM-saving integrations.
The AM62A SoC enables cost-sensitive automotive applications including driver
and in-cabin monitoring systems, next generation of eMirror system, as well as
a broad set of industrial applications in Factory Automation, Building
Automation, Robotics and more.

Some highlights of this SoC are:

* Quad-Cortex-A53s (running up to 1.4GHz) in a single cluster.
* Cortex-R5F for general-purpose or safety usage.
* Deep Learning Accelerator with Single-core C7x Vector DSP with MMA (up to
  1.0GHz).
* Vision Processing Accelerator (VPAC) with a 315MPixel/s ISP (up to 5MP @
  60fps) supporting 16-bit RAW input with RGB-IR separation.
* 4K Video encoder and decoder for HEVC (Level 5.1 High-tier) and H.264 (Level
  5.2) supporting upto 240MPixels/s and MJPEG encoder at 416MPixels/s
* Single display with 24-bit RGB parallel (DPI) interface supporting upto
  165Mhz pixel clock for 2K resolution.
* Integrated Giga-bit Ethernet switch supporting up to a total of two
  external ports (TSN capable).
* 9xUARTs, 5xSPI, 6xI2C, 2xUSB2, 3xCAN-FD, 3x eMMC and SD, GPMC for
  NAND/FPGA connection, OSPI memory controller, 3xMcASP for audio,
  1x CSI-RX-4L for Camera, eCAP/eQEP, ePWM, among other peripherals.
* Dedicated Centralized System Controller for Security, Power, and
  Resource Management.
* Multiple low power modes support, ex: Deep sleep, Standby, MCU-only,
  enabling battery powered system design.

More details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/spruj16

Platform information:

* https://www.ti.com/tool/SK-AM62A-LP

Boot Flow:
----------
Below is the pictorial representation of boot flow:

.. image:: img/boot_diagram_k3_current.svg
  :alt: Boot flow diagram

- Here TIFS acts as master and provides all the critical services. R5/A53
  requests TIFS to get these services done as shown in the above diagram.

Sources:
--------

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_boot_sources
    :end-before: .. k3_rst_include_end_boot_sources

Build procedure:
----------------
0. Setup the environment variables:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_desc
    :end-before: .. k3_rst_include_end_common_env_vars_desc

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_board_env_vars_desc
    :end-before: .. k3_rst_include_end_board_env_vars_desc

Set the variables corresponding to this platform:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_common_env_vars_defn
    :end-before: .. k3_rst_include_end_common_env_vars_defn
.. code-block:: bash

 $ export UBOOT_CFG_CORTEXR=am62ax_evm_r5_defconfig
 $ export UBOOT_CFG_CORTEXA=am62ax_evm_a53_defconfig
 $ export TFA_BOARD=lite
 $ # we dont use any extra TFA parameters
 $ unset TFA_EXTRA_ARGS
 $ export OPTEE_PLATFORM=k3-am62ax
 $ # we dont use any extra OPTEE parameters
 $ unset OPTEE_EXTRA_ARGS

1. Trusted Firmware-A:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_tfa
    :end-before: .. k3_rst_include_end_build_steps_tfa

2. OP-TEE:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_optee
    :end-before: .. k3_rst_include_end_build_steps_optee

3. U-Boot:

* 3.1 R5:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_spl_r5
    :end-before: .. k3_rst_include_end_build_steps_spl_r5

* 3.2 A53:

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_build_steps_uboot
    :end-before: .. k3_rst_include_end_build_steps_uboot

Target Images
--------------
In order to boot we need tiboot3.bin, tispl.bin and u-boot.img.  Each SoC
variant (GP, HS-FS, HS-SE) requires a different source for these files.

 - GP

        * tiboot3-am62ax-gp-evm.bin from step 3.1
        * tispl.bin_unsigned, u-boot.img_unsigned from step 3.2

 - HS-FS

        * tiboot3-am62ax-hs-fs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

 - HS-SE

        * tiboot3-am62ax-hs-evm.bin from step 3.1
        * tispl.bin, u-boot.img from step 3.2

Image formats:
--------------

- tiboot3.bin

.. image:: img/multi_cert_tiboot3.bin.svg
  :alt: tiboot3.bin image format

- tispl.bin

.. image:: img/dm_tispl.bin.svg
  :alt: tispl.bin image format

Switch Setting for Boot Mode
----------------------------

Boot Mode pins provide means to select the boot mode and options before the
device is powered up. After every POR, they are the main source to populate
the Boot Parameter Tables.

The following table shows some common boot modes used on AM62 platform. More
details can be found in the Technical Reference Manual:
https://www.ti.com/lit/pdf/spruj16 under the `Boot Mode Pins` section.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - Switch Label
     - SW2: 12345678
     - SW3: 12345678

   * - SD
     - 01000000
     - 11000010

   * - OSPI
     - 00000000
     - 11001110

   * - EMMC
     - 00000000
     - 11010010

   * - UART
     - 00000000
     - 11011100

   * - USB DFU
     - 00000000
     - 11001010

For SW2 and SW1, the switch state in the "ON" position = 1.

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup information.

.. warning::

  **OpenOCD support since**: August 2023 (git master)

  Until the next stable release of OpenOCD is available in your development
  environment's distribution, it might be necessary to build OpenOCD `from the
  source <https://github.com/openocd-org/openocd>`_.

.. include::  k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_XDS110
    :end-before: .. k3_rst_include_end_openocd_connect_XDS110

To start OpenOCD and connect to the board

.. code-block:: bash

  openocd -f board/ti_am62a7evm.cfg
