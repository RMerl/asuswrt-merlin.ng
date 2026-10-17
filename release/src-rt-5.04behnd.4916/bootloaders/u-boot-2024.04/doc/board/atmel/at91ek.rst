.. SPDX-License-Identifier: GPL-2.0+

AT91 Evaluation kits
====================

Board mapping & boot media
--------------------------

AT91SAM9260EK, AT91SAM9G20EK & AT91SAM9XEEK
^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Memory map::

	0x20000000 - 23FFFFFF	SDRAM (64 MB)
	0xC0000000 - Cxxxxxxx	Atmel Dataflash card (J13)
	0xD0000000 - D07FFFFF	Soldered Atmel Dataflash (AT45DB642)

Environment variables

U-Boot environment variables can be stored at different places:

	- Dataflash on SPI chip select 1 (default)
	- Dataflash on SPI chip select 0 (dataflash card)
	- Nand flash

You can choose your storage location at config step (here for at91sam9260ek)::

	make at91sam9260ek_nandflash_config	- use nand flash
	make at91sam9260ek_dataflash_cs0_config	- use data flash (spi cs0)
	make at91sam9260ek_dataflash_cs1_config	- use data flash (spi cs1)


AT91SAM9261EK, AT91SAM9G10EK
^^^^^^^^^^^^^^^^^^^^^^^^^^^^

Memory map::

	0x20000000 - 23FFFFFF	SDRAM (64 MB)
	0xC0000000 - C07FFFFF	Soldered Atmel Dataflash (AT45DB642)
	0xD0000000 - Dxxxxxxx	Atmel Dataflash card (J22)

Environment variables

U-Boot environment variables can be stored at different places:

	- Dataflash on SPI chip select 0 (default)
	- Dataflash on SPI chip select 3 (dataflash card)
	- Nand flash

You can choose your storage location at config step (here for at91sam9260ek)::

	make at91sam9261ek_nandflash_config	- use nand flash
	make at91sam9261ek_dataflash_cs0_config	- use data flash (spi cs0)
	make at91sam9261ek_dataflash_cs3_config	- use data flash (spi cs3)


AT91SAM9263EK
^^^^^^^^^^^^^

Memory map::

	0x20000000 - 23FFFFFF	SDRAM (64 MB)
	0xC0000000 - Cxxxxxxx	Atmel Dataflash card (J9)

Environment variables

U-Boot environment variables can be stored at different places:

	- Dataflash on SPI chip select 0 (dataflash card)
	- Nand flash
	- Nor flash (not populate by default)

You can choose your storage location at config step (here for at91sam9260ek)::

	make at91sam9263ek_nandflash_config	- use nand flash
	make at91sam9263ek_dataflash_cs0_config	- use data flash (spi cs0)
	make at91sam9263ek_norflash_config	- use nor flash

You can choose to boot directly from U-Boot at config step::

	make at91sam9263ek_norflash_boot_config	- boot from nor flash


AT91SAM9M10G45EK
^^^^^^^^^^^^^^^^

Memory map::

	0x70000000 - 77FFFFFF	SDRAM (128 MB)

Environment variables

U-Boot environment variables can be stored at different places:

	- Nand flash

You can choose your storage location at config step (here for at91sam9m10g45ek)::

	make at91sam9m10g45ek_nandflash_config		- use nand flash


AT91SAM9RLEK
^^^^^^^^^^^^

Memory map::

	0x20000000 - 23FFFFFF	SDRAM (64 MB)
	0xC0000000 - C07FFFFF   Soldered Atmel Dataflash (AT45DB642)

Environment variables

U-Boot environment variables can be stored at different places:

	- Dataflash on SPI chip select 0
	- Nand flash.

You can choose your storage location at config step (here for at91sam9rlek)::

	make at91sam9rlek_nandflash_config	- use nand flash


AT91SAM9N12EK, AT91SAM9X5EK
^^^^^^^^^^^^^^^^^^^^^^^^^^^

Memory map::

	0x20000000 - 27FFFFFF	SDRAM (128 MB)

Environment variables

U-Boot environment variables can be stored at different places:

	- Nand flash
	- SD/MMC card
	- Serialflash/Dataflash on SPI chip select 0

You can choose your storage location at config step (here for at91sam9x5ek)::

	make at91sam9x5ek_dataflash_config	- use data flash
	make at91sam9x5ek_mmc_config		- use sd/mmc card
	make at91sam9x5ek_nandflash_config	- use nand flash
	make at91sam9x5ek_spiflash_config	- use serial flash


SAMA5D3XEK
^^^^^^^^^^

Memory map::

	0x20000000 - 3FFFFFFF	SDRAM (512 MB)

Environment variables

U-Boot environment variables can be stored at different places:

	- Nand flash
	- SD/MMC card
	- Serialflash on SPI chip select 0

You can choose your storage location at config step (here for sama5d3xek)::

	make sama5d3xek_mmc_config		- use SD/MMC card
	make sama5d3xek_nandflash_config	- use nand flash
	make sama5d3xek_serialflash_config	- use serial flash


NAND partition table
--------------------

All the board support boot from NAND flash will use the following NAND
partition table::

	0x00000000 - 0x0003FFFF	bootstrap	(256 KiB)
	0x00040000 - 0x000BFFFF u-boot		(512 KiB)
	0x000C0000 - 0x000FFFFF env		(256 KiB)
	0x00100000 - 0x0013FFFF env_redundant	(256 KiB)
	0x00140000 - 0x0017FFFF spare		(256 KiB)
	0x00180000 - 0x001FFFFF dtb		(512 KiB)
	0x00200000 - 0x007FFFFF kernel		(6 MiB)
	0x00800000 - 0xxxxxxxxx rootfs		(All left)


Watchdog support
----------------

For security reasons, the at91 watchdog is running at boot time and,
if deactivated, cannot be used anymore.
If you want to use the watchdog, you will need to keep it running in
your code (make sure not to disable it in AT91Bootstrap for instance).

In the U-Boot configuration, the AT91 watchdog support is enabled using
the CONFIG_WDT and CONFIG_WDT_AT91 options.
