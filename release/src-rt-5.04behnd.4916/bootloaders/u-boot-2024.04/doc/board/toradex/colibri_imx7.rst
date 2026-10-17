.. SPDX-License-Identifier: GPL-2.0-or-later
.. sectionauthor:: Igor Opaniuk <igor.opaniuk@toradex.com>

Colibri iMX7 Modules
====================

- SoM: https://www.toradex.com/computer-on-modules/colibri-arm-family/nxp-freescale-imx7
- Carrier board: https://www.toradex.com/products/carrier-board/colibri-evaluation-board

Quick Start
-----------

- Build U-Boot
- NAND IMX image adjustments before flashing
- Flashing manually U-Boot to eMMC
- Flashing manually U-Boot to NAND
- Using ``update_uboot`` script

Build U-Boot
------------

.. code-block:: bash

    $ export CROSS_COMPILE=arm-linux-gnueabi-
    $ make colibri_imx7_emmc_defconfig # For NAND: colibri_imx7_defconfig
    $ make

After the build succeeds, you will obtain the final ``u-boot-dtb.imx`` IMX
specific image, ready for flashing (but check next section for additional
adjustments).

The final IMX program image includes (section ``6.6.7`` from `IMX7DRM
<https://www.nxp.com/webapp/Download?colCode=IMX7DRM>`_):

* **Image vector table** (IVT) for BootROM
* **Boot data** -indicates the program image location, program image size
  in bytes, and the plugin flag.
* **Device configuration data**
* **User image**: U-Boot image (``u-boot-dtb.bin``)

IMX image adjustments prior to flashing
---------------------------------------

1. U-Boot for both Colibri iMX7 NAND and eMMC versions
is built with HABv4 support (`AN4581.pdf
<https://www.nxp.com/docs/en/application-note/AN4581.pdf>`_)
enabled by default, which requires generating a proper
Command Sequence File (CSF) by srktool from NXP (not included in the
U-Boot tree, check additional details in introduction_habv4.txt)
and concatenate it to the final ``u-boot-dtb.imx``.

2. In case you don't want to generate a proper ``CSF`` (for any reason),
you still need to pad the IMX image so it has the same size as specified in
the **Boot Data** section of the IMX image.
To obtain this value, run:

.. code-block:: bash

    $ od -X -N 0x30 u-boot-dtb.imx
    0000000    402000d1 87800000 00000000 877ff42c
    0000020    877ff420 877ff400 878a5000 00000000
                        ^^^^^^^^
    0000040    877ff000 000a8060 00000000 40b401d2
               ^^^^^^^^ ^^^^^^^^

Where:

* ``877ff400`` - IVT self address
* ``877ff000`` - Program image address
* ``000a8060`` - Program image size

To calculate the padding:

* IVT offset = ``0x877ff400`` - ``0x877ff000`` = ``0x400``
* Program image size = ``0xa8060`` - ``0x400`` = ``0xa7c60``

and then pad the image:

.. code-block:: bash

    $ objcopy -I binary -O binary --pad-to 0xa7c60 --gap-fill=0x00 \
        u-boot-dtb.imx u-boot-dtb.imx.zero-padded

3. Also, according to the requirement from ``6.6.7.1``, the final image
should have ``0x400`` offset for the initial IVT table.

For eMMC setup we handle this by flashing it to ``0x400``, however
for NAND setup we adjust the image prior to flashing, adding padding at the
beginning of the image.

.. code-block:: bash

    $ dd if=u-boot-dtb.imx.zero-padded of=u-boot-dtb.imx.ready bs=1024 seek=1

Flash U-Boot IMX image to eMMC
------------------------------

Flash the ``u-boot-dtb.imx.zero-padded`` binary to the primary eMMC hardware
boot area partition:

.. code-block:: bash

    => load mmc 1:1 $loadaddr u-boot-dtb.imx.zero-padded
    => setexpr blkcnt ${filesize} + 0x1ff && setexpr blkcnt ${blkcnt} / 0x200
    => mmc dev 0 1
    => mmc write ${loadaddr} 0x2 ${blkcnt}

Flash U-Boot IMX image to NAND
------------------------------

.. code-block:: bash

    => load mmc 1:1 $loadaddr u-boot-dtb.imx.ready
    => nand erase.part u-boot1
    => nand write ${loadaddr} u-boot1 ${filesize}
    => nand erase.part u-boot2
    => nand write ${loadaddr} u-boot2 ${filesize}

Using update_uboot script
-------------------------

You can also use U-Boot env update_uboot script,
which wraps all eMMC/NAND specific command invocations:

.. code-block:: bash

    => load mmc 1:1 $loadaddr u-boot-dtb.imx.ready
    => run update_uboot
