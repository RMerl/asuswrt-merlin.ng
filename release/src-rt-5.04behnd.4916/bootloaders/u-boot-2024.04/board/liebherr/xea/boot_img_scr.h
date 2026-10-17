/* SPDX-License-Identifier:     GPL-2.0+ */
/*
 * Struct for boot image source description for placing in last
 * two SPI NOR flash sectors on legcom.
 */

struct boot_img_src {
	u8 magic;	/* Must be 'B' = 0x42 */
	u8 flags;	/* flags to specify mmcblk[0|1] boot[0|1] */
	u8 crc8;	/* CRC-8 over above two bytes */
} __packed;

/*
 * Bit definition in boot_img_src.flags:
 *  Bit 0: mmcblk device 0 or 1 (1 - if this bit set)
 *  Bit 1: mmcblk boot partition 0 or 1.
 *         for eMMC: boot0 if this bit is cleared, boot1 - if set
 *         for SD-card the boot partition value will always be 0
 *         (independent of the value of this bit)
 *
 */
#define BOOT_SRC_MMC1	BIT(0)
#define BOOT_SRC_PART1	BIT(1)

/* Offset of the first boot image source descriptor in SPI NOR */
#define SPI_FLASH_BOOT_SRC_OFFS	0xFE0000
#define SPI_FLASH_SECTOR_SIZE	0x10000
