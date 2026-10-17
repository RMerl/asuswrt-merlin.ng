.. SPDX-License-Identifier: GPL-2.0+

Pre-Generated FIP File Repo
===========================

Pre-built Flattened Image Package (FIP) sources and Amlogic signing binaries for many
commercially available boards and some Android STB devices are collected for use with
distro build-systems here: https://github.com/LibreELEC/amlogic-boot-fip

Using the pre-built FIP sources to sign U-Boot is simple, e.g. for LePotato:

.. code-block:: bash

    $ git clone https://github.com/LibreELEC/amlogic-boot-fip --depth=1
    $ cd amlogic-boot-fip
    $ mkdir my-output-dir
    $ ./build-fip.sh lepotato /path/to/u-boot/u-boot.bin my-output-dir

Then write U-Boot to SD or eMMC with:

.. code-block:: bash

    $ DEV=/dev/boot_device
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=512 skip=1 seek=1
    $ dd if=fip/u-boot.bin.sd.bin of=$DEV conv=fsync,notrunc bs=1 count=440

Files Included
--------------

Amlogic ARMv8 SoCs use a vendor modified variant of the ARM Trusted Firmware-A boot
architecture. See documentation here: https://www.trustedfirmware.org/projects/tf-a/

Trusted Firmware-A uses the following boot elements (simplified):

- BL1: First boot step implemented in ROM on Amlogic SoCs

- BL2: Second boot step used to initialize the SoC main clocks & DDR interface. BL21
  and ACS board-specific binaries must be "inserted" into the BL2 binary before signing
  and packaging in order to be flashed on the platform

- BL30: Amlogic Secure Co-Processor (SCP) firmware used to handle all system management
  operations (DVFS, suspend/resume, ..)

- BL301: Amlogic Secure Co-Processor (SCP) board-specific firmware "plug-in" to handle
  custom DVFS & suspend-resume parameters

- BL31: Initializes the interrupt controller and the system management interface (PSCI)

- BL32 (Optional): Is the Trusted Environment Execution (TEE) Operating System used to
  run secure Trusted Apps, e.g. OP-TEE

- BL33: Is the last non-secure step, usually U-Boot which loads Linux

Amlogic sources provide the following binaries:

- bl2.bin
- bl30.bin
- bl30.bin
- bl31.img
- bl32.bin

For G12A/B and SM1 Amlogic also provides DDR drivers used by the BL2 binary:

- ddr4_1d.fw
- ddr4_2d.fw
- ddr3_1d.fw
- piei.fw
- lpddr4_1d.fw
- lpddr4_2d.fw
- diag_lpddr4.fw
- aml_ddr.fw

The following files are generated from the Amlogic U-Boot fork:

- acs.bin: Contains the PLL & DDR parameters for the board
- bl301.bin: Contains the DVFS & suspend-resume handling code for the board
- bl33.bin: U-boot binary image

The acs.bin and bl301.bin files use U-Boot GPL-2.0+ headers and U-Boot build system and
are thus considered to be issued from GPL-2.0+ source code.

Amlogic alo provides pre-compiled x86_64 and Python2 binaries:

- aml_encrypt_gxb
- aml_encrypt_gxl
- aml_encrypt_g12a
- aml_encrypt_g12b
- acs_tool.pyc

The repo replaces the pre-compiled acs_tool.pyc with a Python3 acs_tool.py that can be
used with modern build hosts.

The repo also provides the following files used with GXBB boards:

- bl1.bin.hardkernel
- aml_chksum

The repo also supports the open-source 'gxlimg' signing tool that can be used to sign
U-Boot binaries for GXL/GXM/G12A/G12B/SM1 boards: https://github.com/repk/gxlimg

Licensing
---------

The licence of Amlogic provided binaries was not historically clear but has now been
clarified. The current Amlogic distribution licence is below:

.. code-block:: C

    // Copyright (C) 2018 Amlogic, Inc. All rights reserved.
    //
    // All information contained herein is Amlogic confidential.
    //
    // This software is provided to you pursuant to Software License
    // Agreement (SLA) with Amlogic Inc ("Amlogic"). This software may be
    // used only in accordance with the terms of this agreement.
    //
    // Redistribution and use in source and binary forms, with or without
    // modification is strictly prohibited without prior written permission
    // from Amlogic.
    //
    // THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    // "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    // LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    // A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    // OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    // SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    // LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    // DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    // THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    // (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    // OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
