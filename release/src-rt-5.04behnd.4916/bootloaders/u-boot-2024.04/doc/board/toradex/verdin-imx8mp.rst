.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Marcel Ziswiler <marcel.ziswiler@toradex.com>

Verdin iMX8M Plus Module
========================

- SoM: https://www.toradex.com/computer-on-modules/verdin-arm-family/nxp-imx-8m-plus
- Carrier board: https://www.toradex.com/products/carrier-board/verdin-development-board-kit

Quick Start
-----------

- Build the ARM trusted firmware binary
- Get the DDR firmware
- Build U-Boot
- Flash to eMMC
- Boot

Get and Build the ARM Trusted Firmware (Trusted Firmware A)
-----------------------------------------------------------

.. code-block:: bash

    $ echo "Downloading and building TF-A..."
    $ git clone https://git.trustedfirmware.org/TF-A/trusted-firmware-a.git
    $ cd trusted-firmware-a

Then build ATF (TF-A):

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make PLAT=imx8mp IMX_BOOT_UART_BASE=0x30880000 bl31
    $ cp build/imx8mp/release/bl31.bin ../

Get the DDR Firmware
--------------------

.. code-block:: bash

    $ cd ..
    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.10.1.bin
    $ chmod +x firmware-imx-8.10.1.bin
    $ ./firmware-imx-8.10.1.bin
    $ cp firmware-imx-8.10.1/firmware/ddr/synopsys/lpddr4*_202006.bin ./

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make verdin-imx8mp_defconfig
    $ make

Flash to eMMC
-------------

.. code-block:: bash

    > tftpboot ${loadaddr} flash.bin
    > setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    > mmc dev 2 1 && mmc write ${loadaddr} 0x0 ${blkcnt}

As a convenience, instead of the last two commands, one may also use the update
U-Boot wrapper:

.. code-block:: bash

    > run update_uboot

Boot
----

ATF, U-Boot proper and u-boot.dtb images are packed into a FIT image,
which is loaded and parsed by SPL.

Boot sequence is:

* SPL ---> ATF (TF-A) ---> U-Boot proper

Output::

  U-Boot SPL 2022.04-rc1-00164-g21a0312611-dirty (Feb 07 2022 - 11:34:04 +0100)
  Quad die, dual rank failed, attempting dual die, single rank configuration.
  Normal Boot
  WDT:   Started watchdog@30280000 with servicing (60s timeout)
  Trying to boot from BOOTROM
  Find img info 0x&48025a00, size 872
  Need continue download 1024
  Download 779264, Total size 780424
  NOTICE:  BL31: v2.2(release):rel_imx_5.4.70_2.3.2_rc1-5-g835a8f67b
  NOTICE:  BL31: Built : 16:52:37, Aug 26 2021


  U-Boot 2022.04-rc1-00164-g21a0312611-dirty (Feb 07 2022 - 11:34:04 +0100)

  CPU:   Freescale i.MX8MP[8] rev1.1 at 1200 MHz
  Reset cause: POR
  DRAM:  8 GiB
  Core:  78 devices, 18 uclasses, devicetree: separate
  WDT:   Started watchdog@30280000 with servicing (60s timeout)
  MMC:   FSL_SDHC: 1, FSL_SDHC: 2
  Loading Environment from MMC... OK
  In:    serial
  Out:   serial
  Err:   serial
  Model: Toradex Verdin iMX8M Plus Quad 4GB Wi-Fi / BT IT V1.0B, Serial# 06817281
  Carrier: Toradex Verdin Development Board V1.1A, Serial# 10807609
  Setting variant to wifi
  Net:   Hard-coding pdata->enetaddr
  eth1: ethernet@30be0000, eth0: ethernet@30bf0000 [PRIME]
  Hit any key to stop autoboot:  0
  Verdin iMX8MP #
