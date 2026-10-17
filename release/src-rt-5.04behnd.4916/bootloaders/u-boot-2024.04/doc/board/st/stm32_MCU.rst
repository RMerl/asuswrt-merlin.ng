.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Patrice Chotard <patrice.chotardy@foss.st.com>

STM32 MCU boards
================

This is a quick instruction for setup STM32 MCU boards.

Supported devices
-----------------

U-Boot supports the following STMP32 MCU SoCs:

 - STM32F429
 - STM32F469
 - STM32F746
 - STM32F769
 - STM32H743
 - STM32H750

SoCs information:
-----------------
STM32F4 series are Cortex-M4 MCU.
STM32F7 and STM32H7 series are Cortex-M7 MCU.

 + STM32F4 series: https://www.st.com/en/microcontrollers-microprocessors/stm32f4-series.html
 + STM32F7 series: https://www.st.com/en/microcontrollers-microprocessors/stm32f7-series.html
 + STM32H7 series: https://www.st.com/en/microcontrollers-microprocessors/stm32h7-series.html

Currently the following boards are supported:

 + stm32f429-discovery
 + stm32f469-discovery
 + stm32746g-evaluation
 + stm32f746-discovery
 + stm32f769-discovery
 + stm32h743i-discovery
 + stm32h743i-evaluation
 + stm32h750i-art-pi

Boot Sequences
--------------

For STM32F7 series, 2 boot configurations are supported with and without SPL

+------------------------+-------------------------+--------------+
| **FSBL**               | **SSBL**                | **OS**       |
+------------------------+-------------------------+--------------+
| First Stage Bootloader | Second Stage Bootloader | Linux Kernel |
+------------------------+-------------------------+--------------+
| embedded Flash         | DDR                                    |
+------------------------+-------------------------+--------------+

The boot chain with SPL
```````````````````````

defconfig_file :
   + **stm32746g-eval_spl_defconfig**
   + **stm32f746-disco_spl_defconfig**
   + **stm32f769-disco_spl_defconfig**

+------------+------------+-------+
| FSBL       | SSBL       | OS    |
+------------+------------+-------+
|U-Boot SPL  | U-Boot     | Linux |
+------------+------------+-------+

The boot chain without SPL
``````````````````````````

defconfig_file :
   + **stm32f429-discovery_defconfig**
   + **stm32f429-evaluation_defconfig**
   + **stm32f469-discovery_defconfig**
   + **stm32746g-eval_defconfig**
   + **stm32f746-disco_defconfig**
   + **stm32f769-disco_defconfig**
   + **stm32h743-disco_defconfig**
   + **stm32h743-eval_defconfig**
   + **stm32h750-art-pi_defconfig**

+-----------+-------+
| FSBL      | OS    |
+-----------+-------+
|U-Boot     | Linux |
+-----------+-------+

Build Procedure
---------------

1. Install the required tools for U-Boot

   * install package needed in U-Boot makefile
     (libssl-dev, swig, libpython-dev...)

   * install ARMv7 toolchain for 32bit Cortex-A (from Linaro,
     from SDK for STM32MP15x, or any crosstoolchains from your distribution)
     (you can use any gcc cross compiler compatible with U-Boot)

2. Set the cross compiler::

   # export CROSS_COMPILE=/path/to/toolchain/arm-linux-gnueabi-

3. Select the output directory (optional)::

   # export KBUILD_OUTPUT=/path/to/output

   for example: use one output directory for each configuration::

   # export KBUILD_OUTPUT=stm32f4
   # export KBUILD_OUTPUT=stm32f7
   # export KBUILD_OUTPUT=stm32h7

   you can build outside of code directory::

   # export KBUILD_OUTPUT=../build/stm32f4

4. Configure U-Boot::

   # make <defconfig_file>

   For example with <defconfig_file>:

   - For **stm32f429 discovery** board : **stm32f429-discovery_defconfig**
   - For **stm32f769 discovery** board with SPL: **stm32f769-disco_spl_defconfig**
   - For **stm32f769 discovery** board without SPL: **stm32f769-disco_defconfig**

5. Configure the device-tree and build the U-Boot image::

   # make DEVICE_TREE=<name> all

   Examples:

  a) boot with SPL on stm32f746 discovery board::

     # export KBUILD_OUTPUT=stm32f746-disco
     # make stm32f746-disco_spl_defconfig
     # make all

  b) boot without SPL on stm32f746 discovery board::

     # export KBUILD_OUTPUT=stm32f746-disco
     # make stm32f746-disco_defconfig
     # make all

  c) boot on stm32h743 discovery board::

     # export KBUILD_OUTPUT=stm32h743-disco
     # make stm32h743-disco_defconfig
     # make all

  d) boot on stm32h743 evaluation board::

     # export KBUILD_OUTPUT=stm32h743-disco
     # make stm32h743-eval_defconfig
     # make all

6. U-Boot Output files

   So in the output directory (selected by KBUILD_OUTPUT),
   you can found the needed U-Boot files, for example::

     - stm32f746-disco_defconfig = **u-boot-dtb.bin** and **u-boot.dtb**

       - FSBL = u-boot-dtb.bin

     - stm32f746-disco_spl_defconfig = **u-boot-dtb.bin**, **u-boot.dtb** and **u-boot-with-spl.bin**

       - FSBL + SSBL = u-boot-with-spl.bin
       - SSBL = u-boot-dtb.bin

7. Flash U-Boot files

Plug STM32 MCUs board using the USB ST-Link connector, hence it will expose
the flash area as a mass-storage. In this mass-storage you will find the
following files:

- DETAILS.TXT: give the bootrom version and build
- MBED.HTM: shortcul to the hardware board description web page from st.com.

Copy/paste the u-boot.bin or u-boot-with-spl.bin (in case of bootchain with SPL)
to this mass-storage. The "COM" LED will blink alternatively red and green during
the flash process. When done the board will reboot automatically.

In case of boot with SPL, by default SPL will try to load either a Linux
kernel (falcon mode) or, if the key "c" is maintained pressed, the main U-Boot.
