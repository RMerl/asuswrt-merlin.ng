.. SPDX-License-Identifier: GPL-2.0+ OR BSD-3-Clause
.. sectionauthor:: Nishanth Menon <nm@ti.com>

J721E/TDA4VM Beagleboard.org BeagleBone AI-64
=============================================

Introduction:
-------------

BeagleBoard.org BeagleBone AI-64 is an open source hardware single
board computer based on the Texas Instruments TDA4VM SoC featuring
dual-core 2.0GHz Arm Cortex-A72 processor, C7x+MMA and 2 C66x
floating-point VLIW DSPs, 3x dual ARM Cortex-R5 co-processors,
2x 6-core Programmable Real-Time Unit and Industrial Communication
SubSystem, PowerVR Rogue 8XE GE8430 3D GPU. The board features 4GB
DDR4, USB3.0 Type-C, 2x USB SS Type-A, miniDisplayPort, 2x 4-lane
CSI, DSI, 16GB eMMC flash, 1G Ethernet, M.2 E-key for WiFi/BT, and
BeagleBone expansion headers.

Further information can be found at:

* Product Page: https://beagleboard.org/ai-64
* Hardware documentation: https://git.beagleboard.org/beagleboard/beaglebone-ai-64

Boot Flow:
----------
Below is the pictorial representation of boot flow:

.. image:: ../ti/img/boot_diagram_j721e.svg
  :alt: Boot flow diagram

- On this platform, DMSC runs 'TI Foundational Security' (TIFS) which
  functions as the security enclave master. The 'Device Manager' (DM),
  also known as the 'TISCI server' in "TI terminology", running on boot
  R5F, offers all the essential services required for device management.
  The A72, C7x, C6x or R5F (Aux cores) sends requests to TIFS/DM to
  accomplish the needed services, as illustrated in the diagram above.

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
.. prompt:: bash $

  export UBOOT_CFG_CORTEXR=j721e_beagleboneai64_r5_defconfig
  export UBOOT_CFG_CORTEXA=j721e_beagleboneai64_a72_defconfig
  export TFA_BOARD=generic
  # we dont use any extra TFA parameters
  unset TFA_EXTRA_ARGS
  export OPTEE_PLATFORM=k3-j721e
  # we dont use any extra OP-TEE parameters
  unset OPTEE_EXTRA_ARGS

.. include::  ../ti/j721e_evm.rst
    :start-after: .. j721e_evm_rst_include_start_build_steps
    :end-before: .. j721e_evm_rst_include_end_build_steps

Target Images
--------------
Copy the below images to an SD card and boot:

* tiboot3-j721e-gp-evm.bin from R5 build as tiboot3.bin
* tispl.bin_unsigned from Cortex-A build as tispl.bin
* u-boot.img_unsigned from Cortex-A build as u-boot.img

Image formats
-------------

- tiboot3.bin

.. image:: ../ti/img/no_multi_cert_tiboot3.bin.svg
  :alt: tiboot3.bin image format

- tispl.bin

.. image:: ../ti/img/dm_tispl.bin.svg
  :alt: tispl.bin image format

- sysfw.itb

.. image:: ../ti/img/sysfw.itb.svg
  :alt: sysfw.itb image format

Additional hardware for U-Boot development
------------------------------------------

* Serial Console is critical for U-Boot development on BeagleBone AI-64. See
  `BeagleBone AI-64 connector documentation
  <https://docs.beagleboard.org/latest/boards/beaglebone/ai-64/ch07.html>`_.
* uSD is preferred option over eMMC, and a SD/MMC reader will be needed.
* (optionally) JTAG is useful when working with very early stages of boot.

Default storage options
-----------------------

There are multiple storage media options on BeagleBone AI-64, but primarily:

* Onboard eMMC (default) - reliable, fast and meant for deployment use.
* SD/MMC card interface (hold 'BOOT' switch and power on) - Entirely
  depends on the SD card quality.

Flash to uSD card or how to deal with "bricked" Board
--------------------------------------------------------

When deploying or working on Linux, it's common to use the onboard
eMMC. However, avoiding the eMMC and using the uSD card is safer when
working with U-Boot.

If you choose to  hand format your own bootable uSD card, be
aware that it can be difficult. The following information
may be helpful, but remember that it is only sometimes
reliable, and partition options can cause issues. These
can potentially help:

* https://git.ti.com/cgit/arago-project/tisdk-setup-scripts/tree/create-sdcard.sh
* https://elinux.org/Beagleboard:Expanding_File_System_Partition_On_A_microSD

The simplest option is to start with a standard distribution
image like those in `BeagleBoard.org Distros Page
<https://www.beagleboard.org/distros>`_ and download a disk image for
BeagleBone AI-64. Pick a 16GB+ uSD card to be on the safer side.

With an SD/MMC Card reader and `Balena Etcher
<https://etcher.balena.io/>`_, having a functional setup in minutes is
a trivial matter, and it works on almost all Host Operating Systems.
Yes Windows users, Windows Subsystem for Linux(WSL) based development
with U-Boot and update uSD card is practical.

Updating U-Boot is a matter of copying the tiboot3.bin, tispl.bin and
u-boot.img to the "BOOT" partition of the uSD card. Remember to sync
and unmount (or Eject - depending on the Operating System) the uSD
card prior to physically removing from SD card reader.

Also see following section on switch setting used for booting using
uSD card.

.. note::
  Great news! If the board has not been damaged physically, there's no
  need to worry about it being "bricked" on this platform. You only have
  to flash an uSD card, plug it in, and reinstall the image on eMMC. This
  means that even if you make a mistake, you can quickly fix it and rest
  easy.

  If you are frequently working with uSD cards, you might find the
  following useful:

  * `USB-SD-Mux <https://www.linux-automation.com/en/products/usb-sd-mux.html>`_
  * `SD-Wire <https://wiki.tizen.org/SDWire>`_

Flash to eMMC
-------------

The eMMC layout selected is user-friendly for developers. The
boot hardware partition of the eMMC only contains the fixed-size
tiboot3.bin image. This is because the contents of the boot partitions
need to run from the SoC's internal SRAM, which remains a fixed size
constant. The other components of the boot sequence, such as tispl.bin
and u-boot.img, are located in the /BOOT partition in the User Defined
Area (UDA) hardware partition of the eMMC. These components can vary
significantly in size. The choice of keeping tiboot3.bin in boot0 or
boot1 partition depends on A/B update requirements.

.. image:: img/beagleplay_emmc.svg
  :alt: eMMC partitions and boot file organization for BeagleBone AI-64

The following are the steps from Linux shell to program eMMC:

.. prompt:: bash #

  # Enable Boot0 boot
  mmc bootpart enable 1 2 /dev/mmcblk0
  mmc bootbus set single_backward x1 x8 /dev/mmcblk0
  mmc hwreset enable /dev/mmcblk0

  # Clear eMMC boot0
  echo '0' >> /sys/class/block/mmcblk0boot0/force_ro
  dd if=/dev/zero of=/dev/mmcblk0boot0 count=32 bs=128k
  # Write tiboot3.bin
  dd if=tiboot3.bin of=/dev/mmcblk0boot0 bs=128k

  # Copy the rest of the boot binaries
  mount /dev/mmcblk0p1 /boot/firmware
  cp tispl.bin /boot/firmware
  cp u-boot.img /boot/firmware
  sync

.. warning ::

  U-Boot is configured to prioritize booting from an SD card if it
  detects a valid boot partition and boot files on it, even if the
  system initially booted from eMMC. The boot order is set as follows:

  * SD/MMC
  * eMMC
  * USB
  * PXE

LED patterns during boot
------------------------

.. list-table:: USR LED status indication
   :widths: 16 16
   :header-rows: 1

   * - USR LEDs (012345)
     - Indicates

   * - 00000
     - Boot failure or R5 image not started up

   * - 11111
     - A53 SPL/U-boot has started up

   * - 10101
     - OS boot process has been initiated

   * - 01010
     - OS boot process failed and drops to U-Boot shell

.. note ::

  In the table above, 0 indicates LED switched off and 1 indicates LED
  switched ON.

.. warning ::

  The green LED very next to the serial connector labelled "WKUP UART0"
  is the power LED (LED6). This is the same color as the rest of the USR
  LEDs. If the "green" LED6 power LED is not glowing, the system power
  supply is not functional. Please refer to `BeagleBone AI-64 documentation
  <https://beagleboard.org/ai-64/>`_ for further information.

Switch Setting for Boot Mode
----------------------------

The boot time option is configured via "BOOT" button on the board.
See `BeagleBone AI-64 Schematics <https://git.beagleboard.org/beagleboard/beaglebone-ai-64/-/blob/main/BeagleBone_AI-64_SCH.pdf>`_
for details.

.. list-table:: Boot Modes
   :widths: 16 16 16
   :header-rows: 1

   * - BOOT Switch Position
     - Primary Boot
     - Secondary Boot

   * - Not Pressed
     - eMMC
     - SD Card

   * - Pressed
     - SD Card
     - SD Card

To switch to SD card boot mode, hold the BOOT button while powering on
with Type-C power supply, then release when power LED lights up.

Debugging U-Boot
----------------

See :ref:`Common Debugging environment - OpenOCD<k3_rst_refer_openocd>`: for
detailed setup and debugging information.

.. warning::

  **OpenOCD support since**: v0.12.0

  If the default package version of OpenOCD in your development
  environment's distribution needs to be updated, it might be necessary to
  build OpenOCD from the source.

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_openocd_connect_tag_connect
    :end-before: .. k3_rst_include_end_openocd_connect_tag_connect

.. include::  ../ti/k3.rst
    :start-after: .. k3_rst_include_start_openocd_cfg_external_intro
    :end-before: .. k3_rst_include_end_openocd_cfg_external_intro

For example, with BeagleBone AI-64 (J721e platform), the openocd_connect.cfg:

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
    set SOC j721e
  }

  source [find target/ti_k3.cfg]

  ftdi tdo_sample_edge falling

  # Speeds for FT2232H are in multiples of 2, and 32MHz is tops
  # max speed we seem to achieve is ~20MHz.. so we pick 16MHz
  adapter speed 16000
