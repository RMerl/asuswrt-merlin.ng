.. SPDX-License-Identifier: GPL-2.0+

U-Boot for the ASUS Transformer device family
=============================================

``DISCLAMER!`` Moving your ASUS Transformer to use U-Boot assumes replacement
of the vendor ASUS bootloader. Vendor Android firmwares will no longer be
able to run on the device. This replacement IS reversible.

Quick Start
-----------

- Build U-Boot
- Process U-Boot
- Flashing U-Boot into the eMMC
- Flashing U-Boot into the SPI flash
- Boot
- Self Upgrading

Build U-Boot
------------

Device support is implemented by applying a config fragment to a generic board
defconfig. Valid fragments are ``tf201.config``, ``tf300t.config``,
``tf300tg.config``, ``tf300tl.config``, ``tf700t.config``, ``tf600t.config`` and
``p1801-t.config``.

.. code-block:: bash

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make transformer_t30_defconfig tf201.config # For TF201
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb-tegra.bin``
image, ready for further processing.

Process U-Boot
--------------

``DISCLAMER!`` All questions related to the re-crypt work should be asked
in re-crypt repo issues. NOT HERE!

re-crypt is a tool that processes the ``u-boot-dtb-tegra.bin`` binary into form
usable by device. This process is required only on the first installation or
to recover the device in case of a failed update. You need to know your
tablet's individual SBK to continue.

Permanent installation can be performed either by using the nv3p protocol or by
pre-loading just built U-Boot into RAM.

Processing for the NV3P protocol
********************************

.. code-block:: bash

    $ git clone https://gitlab.com/grate-driver/re-crypt.git
    $ cd re-crypt # place your u-boot-dtb-tegra.bin here
    $ ./re-crypt.py --dev tf201 --sbk <your sbk>

where SBK has next form ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX`` ``0xXXXXXXXX``

The script will produce a ``repart-block.bin`` ready to flash.

Processing for pre-loaded U-Boot
********************************

The procedure is the same, but the ``--split`` argument is used with the
``re-crypt.py``. The script will produce ``bct.img`` and ``ebt.img`` ready
to flash.

Flashing U-Boot into the eMMC
-----------------------------

``DISCLAMER!`` All questions related to NvFlash should be asked in the proper
place. NOT HERE! Flashing U-Boot will erase all eMMC, so make a backup before!

Permanent installation can be performed either by using the nv3p protocol or by
pre-loading just built U-Boot into RAM.

Flashing with the NV3P protocol
*******************************

Nv3p is a custom Nvidia protocol used to recover bricked devices. Devices can
enter it either by using ``wheelie`` with the correct ``blob.bin`` file or by
pre-loading vendor bootloader with the Fusée Gelée.

With nv3p, ``repart-block.bin`` is used. It contains BCT and a bootloader in
encrypted state in form, which can just be written RAW at the start of eMMC.

.. code-block:: bash

    $ wheelie --blob blob.bin
    $ nvflash --resume --rawdevicewrite 0 1024 repart-block.bin

When flashing is done, reboot the device.

Flashing with a pre-loaded U-Boot
*********************************

U-Boot pre-loaded into RAM acts the same as when it was booted "cold". Currently
U-Boot supports bootmenu entry fastboot, which allows to write a processed copy
of U-Boot permanently into eMMC.

While pre-loading U-Boot, hold the ``volume down`` button which will trigger
the bootmenu. There, select ``fastboot`` using the volume and power buttons.
After, on host PC, do:

.. code-block:: bash

    $ fastboot flash 0.1 bct.img
    $ fastboot flash 0.2 ebt.img
    $ fastboot reboot

Device will reboot.

Flashing U-Boot into the SPI Flash
----------------------------------

Some of Transformers use a separate 4 MB SPI flash, which contains all data
required for boot. It is flashed from within U-Boot itself, preloaded into RAM
using Fusée Gelée.

After creating your ``repart-block.bin`` you have to place it on a 1st partition
of microSD card formated in fat. Then insert this microSD card into your tablet
and boot it using Fusée Gelée and U-Boot, which was included into
``repart-block.bin``, while booting you must hold the ``volume down`` button.

The process should take less than a minute, if everything goes correctly,
on microSD will appear ``spi-flash-backup.bin`` file, which is the dump of your
SPI Flash content and can be used to restore UEFI, do not lose it, tablet will
power itself off.

Self-updating of U-Boot is performed by placing ``u-boot-dtb-tegra.bin`` on 1st
partition of microSD, inserting it into the tablet and booting with a pressed
``volume down`` button.

Boot
----

To boot Linux, U-Boot will look for an ``extlinux.conf`` on MicroSD and then on
eMMC. Additionally, if the Volume Down button is pressed while booting, the
device will enter bootmenu. Bootmenu contains entries to mount MicroSD and eMMC
as mass storage, fastboot, reboot, reboot RCM, poweroff, enter U-Boot console
and update bootloader (check the next chapter).

Flashing ``repart-block.bin`` eliminates vendor restrictions on eMMC and allows
the user to use/partition it in any way the user desires.

Self Upgrading
--------------

Place your ``u-boot-dtb-tegra.bin`` on the first partition of the MicroSD card
and insert it into the tablet. Enter bootmenu, choose update the bootloader
option with the Power button and U-Boot should update itself. Once the process
is completed, U-Boot will ask to press any button to reboot.
