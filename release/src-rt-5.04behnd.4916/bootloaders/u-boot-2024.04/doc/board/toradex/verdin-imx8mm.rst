.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Marcel Ziswiler <marcel.ziswiler@toradex.com>

Verdin iMX8M Mini Module
========================

- SoM: https://www.toradex.com/computer-on-modules/verdin-arm-family/nxp-imx-8m-mini-nano
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
    $ make PLAT=imx8mm IMX_BOOT_UART_BASE=0x30860000 bl31
    $ cp build/imx8mm/release/bl31.bin ../

Get the DDR Firmware
--------------------

.. code-block:: bash

    $ cd ..
    $ wget https://www.nxp.com/lgfiles/NMG/MAD/YOCTO/firmware-imx-8.10.1.bin
    $ chmod +x firmware-imx-8.10.1.bin
    $ ./firmware-imx-8.10.1.bin
    $ cp firmware-imx-8.10.1/firmware/ddr/synopsys/lpddr4*.bin ./

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=aarch64-linux-gnu-
    $ make verdin-imx8mm_defconfig
    $ make

Flash to eMMC
-------------

.. code-block:: bash

    > tftpboot ${loadaddr} flash.bin
    > setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    > mmc dev 0 1 && mmc write ${loadaddr} 0x2 ${blkcnt}

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

  U-Boot SPL 2021.10-rc2-00028-gee010ba1129 (Aug 23 2021 - 16:56:02 +0200)
  Normal Boot
  WDT:   Started with servicing (60s timeout)
  Trying to boot from MMC1
  NOTICE:  BL31: v2.2(release):rel_imx_5.4.70_2.3.2_rc1-5-g835a8f67b
  NOTICE:  BL31: Built : 18:02:12, Aug 16 2021


  U-Boot 2021.10-rc2-00028-gee010ba1129 (Aug 23 2021 - 16:56:02 +0200)

  CPU:   Freescale i.MX8MMQ rev1.0 at 1200 MHz
  Reset cause: POR
  DRAM:  2 GiB
  WDT:   Started with servicing (60s timeout)
  MMC:   FSL_SDHC: 0, FSL_SDHC: 1, FSL_SDHC: 2
  Loading Environment from MMC... OK
  In:    serial
  Out:   serial
  Err:   serial
  Model: Toradex Verdin iMX8M Mini Quad 2GB Wi-Fi / BT IT V1.1A, Serial# 06760554
  Carrier: Toradex Verdin Development Board V1.1A, Serial# 10754333
  Setting variant to wifi
  Net:   eth0: ethernet@30be0000
  Hit any key to stop autoboot:  0
  Verdin iMX8MM #
