.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Marcel Ziswiler <marcel.ziswiler@toradex.com>

Verdin AM62 Module
==================

- SoM: https://www.toradex.com/computer-on-modules/verdin-arm-family/ti-am62
- Carrier board: https://www.toradex.com/products/carrier-board/verdin-development-board-kit

Quick Start
-----------

- Get the binary-only SYSFW
- Get binary-only TI Linux firmware
- Build the ARM trusted firmware binary
- Build the OPTEE binary
- Build U-Boot for the R5
- Build U-Boot for the A53
- Flash to eMMC
- Boot

For an overview of the TI AM62 SoC boot flow please head over to:
:doc:`../ti/am62x_sk`

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

 $ export UBOOT_CFG_CORTEXR=verdin-am62_r5_defconfig
 $ export UBOOT_CFG_CORTEXA=verdin-am62_a53_defconfig
 $ export TFA_BOARD=lite
 $ # we don't use any extra TFA parameters
 $ unset TFA_EXTRA_ARGS
 $ export OPTEE_PLATFORM=k3-am62x
 $ export OPTEE_EXTRA_ARGS="CFG_WITH_SOFTWARE_PRNG=y"

.. include::  ../ti/am62x_sk.rst
    :start-after: .. am62x_evm_rst_include_start_build_steps
    :end-before: .. am62x_evm_rst_include_end_build_steps

Flash to eMMC
-------------

.. code-block:: bash

    => mmc dev 0 1
    => fatload mmc 1 ${loadaddr} tiboot3.bin
    => mmc write ${loadaddr} 0x0 0x400
    => fatload mmc 1 ${loadaddr} tispl.bin
    => mmc write ${loadaddr} 0x400 0x1000
    => fatload mmc 1 ${loadaddr} u-boot.img
    => mmc write ${loadaddr} 0x1400 0x2000

As a convenience, instead of having to remember all those addresses and sizes,
one may also use the update U-Boot wrappers:

.. code-block:: bash

    > tftpboot ${loadaddr} tiboot3-am62x-gp-verdin.bin
    > run update_tiboot3

    > tftpboot ${loadaddr} tispl.bin
    > run update_tispl

    > tftpboot ${loadaddr} u-boot.img
    > run update_uboot

Boot
----

Output::

  U-Boot SPL 2023.10-rc1-00210-gb678170a34c (Aug 03 2023 - 00:09:14 +0200)
  SYSFW ABI: 3.1 (firmware rev 0x0009 '9.0.1--v09.00.01 (Kool Koala)')
  SPL initial stack usage: 13368 bytes
  Trying to boot from MMC1
  Authentication passed
  Authentication passed
  Authentication passed
  Authentication passed
  Authentication passed
  Starting ATF on ARM64 core...

  NOTICE:  BL31: v2.9(release):v2.9.0-73-g463655cc8
  NOTICE:  BL31: Built : 14:51:42, Jun  5 2023
  I/TC:
  I/TC: OP-TEE version: 3.21.0-168-g322cf9e33 (gcc version 12.2.1 20221205 (Arm GNU Toolchain 12.2.Rel1 (Build arm-12.24))) #2 Mon Jun  5 13:04:15 UTC 2023 aarch64
  I/TC: WARNING: This OP-TEE configuration might be insecure!
  I/TC: WARNING: Please check https://optee.readthedocs.io/en/latest/architecture/porting_guidelines.html
  I/TC: Primary CPU initializing
  I/TC: SYSFW ABI: 3.1 (firmware rev 0x0009 '9.0.1--v09.00.01 (Kool Koala)')
  I/TC: HUK Initialized
  I/TC: Primary CPU switching to normal world boot

  U-Boot SPL 2023.10-rc1-00210-gb678170a34c (Aug 03 2023 - 00:09:41 +0200)
  SYSFW ABI: 3.1 (firmware rev 0x0009 '9.0.1--v09.00.01 (Kool Koala)')
  SPL initial stack usage: 1840 bytes
  Trying to boot from MMC1
  Authentication passed
  Authentication passed


  U-Boot 2023.10-rc1-00210-gb678170a34c (Aug 03 2023 - 00:09:41 +0200)

  SoC:   AM62X SR1.0 HS-FS
  DRAM:  2 GiB
  Core:  136 devices, 28 uclasses, devicetree: separate
  MMC:   mmc@fa10000: 0, mmc@fa00000: 1
  Loading Environment from MMC... OK
  In:    serial@2800000
  Out:   serial@2800000
  Err:   serial@2800000
  Model: Toradex 0076 Verdin AM62 Quad 2GB WB IT V1.0A
  Serial#: 15037380
  Carrier: Toradex Verdin Development Board V1.1A, Serial# 10754333
  am65_cpsw_nuss ethernet@8000000: K3 CPSW: nuss_ver: 0x6BA01103 cpsw_ver: 0x6BA81103 ale_ver: 0x00290105 Ports:2 mdio_freq:1000000
  Setting variant to wifi
  Net:
  Warning: ethernet@8000000port@1 MAC addresses don't match:
  Address in ROM is		1c:63:49:22:5f:f9
  Address in environment is	00:14:2d:e5:73:c4
  eth0: ethernet@8000000port@1 [PRIME], eth1: ethernet@8000000port@2
  Hit any key to stop autoboot:  0
  Verdin AM62 #
