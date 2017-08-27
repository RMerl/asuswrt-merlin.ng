/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License v2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 021110-1307, USA.
 */

#include <asm-generic/int-ll64.h>
#include <linux/mmc/ioctl.h>
#include <stdio.h>

#define CHECK(expr, msg, err_stmt) { if (expr) { fprintf(stderr, msg); err_stmt; } }

#ifndef offsetof
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif

/* From kernel linux/major.h */
#define MMC_BLOCK_MAJOR			179

/* From kernel linux/mmc/mmc.h */
#define MMC_SWITCH		6	/* ac	[31:0] See below	R1b */
#define MMC_SEND_EXT_CSD	8	/* adtc				R1  */
#define MMC_SEND_STATUS		13	/* ac   [31:16] RCA        R1  */
#define R1_SWITCH_ERROR   (1 << 7)  /* sx, c */
#define MMC_SWITCH_MODE_WRITE_BYTE	0x03	/* Set target to value */
#define MMC_READ_MULTIPLE_BLOCK  18   /* adtc [31:0] data addr   R1  */
#define MMC_WRITE_MULTIPLE_BLOCK 25   /* adtc                    R1  */

/*
 * EXT_CSD fields
 */
#define EXT_CSD_S_CMD_SET		504
#define EXT_CSD_HPI_FEATURE		503
#define EXT_CSD_BKOPS_SUPPORT		502	/* RO */
#define EXT_CSD_BOOT_INFO		228	/* R/W */
#define EXT_CSD_SEC_COUNT_3		215
#define EXT_CSD_SEC_COUNT_2		214
#define EXT_CSD_SEC_COUNT_1		213
#define EXT_CSD_SEC_COUNT_0		212
#define EXT_CSD_PART_SWITCH_TIME	199
#define EXT_CSD_BOOT_CFG		179
#define EXT_CSD_PART_CONFIG		179
#define EXT_CSD_BOOT_BUS_CONDITIONS 	177
#define EXT_CSD_ERASE_GROUP_DEF		175
#define EXT_CSD_BOOT_WP			173
#define EXT_CSD_WR_REL_SET			167
#define EXT_CSD_WR_REL_PARAM		166
#define EXT_CSD_SANITIZE_START		165
#define EXT_CSD_BKOPS_EN		163	/* R/W */
#define EXT_CSD_RST_N_FUNCTION		162	/* R/W */
#define EXT_CSD_PARTITIONING_SUPPORT	160	/* RO */
#define EXT_CSD_MAX_ENH_SIZE_MULT_2	159
#define EXT_CSD_MAX_ENH_SIZE_MULT_1	158
#define EXT_CSD_MAX_ENH_SIZE_MULT_0	157
#define EXT_CSD_PARTITIONS_ATTRIBUTE	156	/* R/W */
#define EXT_CSD_PARTITION_SETTING_COMPLETED	155	/* R/W */
#define EXT_CSD_ENH_SIZE_MULT_2		142
#define EXT_CSD_ENH_SIZE_MULT_1		141
#define EXT_CSD_ENH_SIZE_MULT_0		140
#define EXT_CSD_ENH_START_ADDR_3	139
#define EXT_CSD_ENH_START_ADDR_2	138
#define EXT_CSD_ENH_START_ADDR_1	137
#define EXT_CSD_ENH_START_ADDR_0	136
#define EXT_CSD_NATIVE_SECTOR_SIZE	63 /* R */
#define EXT_CSD_USE_NATIVE_SECTOR	62 /* R/W */
#define EXT_CSD_DATA_SECTOR_SIZE	61 /* R */

/*
 * WR_REL_PARAM field definitions
 */
#define HS_CTRL_REL	(1<<0)
#define EN_REL_WR	(1<<2)

/*
 * BKOPS_EN field definition
 */
#define BKOPS_ENABLE	(1<<0)

/*
 * EXT_CSD field definitions
 */
#define EXT_CSD_HPI_SUPP		(1<<0)
#define EXT_CSD_HPI_IMPL		(1<<1)
#define EXT_CSD_CMD_SET_NORMAL		(1<<0)
#define EXT_CSD_BOOT_WP_B_PWR_WP_DIS	(0x40)
#define EXT_CSD_BOOT_WP_B_PERM_WP_DIS	(0x10)
#define EXT_CSD_BOOT_WP_B_PERM_WP_EN	(0x04)
#define EXT_CSD_BOOT_WP_B_PWR_WP_EN	(0x01)
#define EXT_CSD_BOOT_INFO_HS_MODE	(1<<2)
#define EXT_CSD_BOOT_INFO_DDR_DDR	(1<<1)
#define EXT_CSD_BOOT_INFO_ALT		(1<<0)
#define EXT_CSD_BOOT_CFG_ACK		(1<<6)
#define EXT_CSD_BOOT_CFG_EN		(0x38)
#define EXT_CSD_BOOT_CFG_ACC		(0x07)
#define EXT_CSD_RST_N_EN_MASK		(0x03)
#define EXT_CSD_HW_RESET_EN		(0x01)
#define EXT_CSD_HW_RESET_DIS		(0x02)
#define EXT_CSD_PART_CONFIG_ACC_MASK	  (0x7)
#define EXT_CSD_PART_CONFIG_ACC_BOOT0	  (0x1)
#define EXT_CSD_PART_CONFIG_ACC_BOOT1	  (0x2)
#define EXT_CSD_PART_CONFIG_ACC_USER_AREA (0x7)
#define EXT_CSD_PART_CONFIG_ACC_ACK	  (0x40)
#define EXT_CSD_PARTITIONING_EN		(1<<0)
#define EXT_CSD_ENH_ATTRIBUTE_EN	(1<<1)
#define EXT_CSD_ENH_USR			(1<<0)

/* From kernel linux/mmc/core.h */
#define MMC_RSP_PRESENT	(1 << 0)
#define MMC_RSP_136	(1 << 1)		/* 136 bit response */
#define MMC_RSP_CRC	(1 << 2)		/* expect valid crc */
#define MMC_RSP_BUSY	(1 << 3)		/* card may send busy */
#define MMC_RSP_OPCODE	(1 << 4)		/* response contains opcode */

#define MMC_CMD_AC	(0 << 5)
#define MMC_CMD_ADTC	(1 << 5)

#define MMC_RSP_SPI_S1	(1 << 7)		/* one status byte */
#define MMC_RSP_SPI_BUSY (1 << 10)		/* card may send busy */

#define MMC_RSP_SPI_R1	(MMC_RSP_SPI_S1)
#define MMC_RSP_SPI_R1B	(MMC_RSP_SPI_S1|MMC_RSP_SPI_BUSY)

#define MMC_RSP_R1	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE)
#define MMC_RSP_R1B	(MMC_RSP_PRESENT|MMC_RSP_CRC|MMC_RSP_OPCODE|MMC_RSP_BUSY)
