// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2014 Freescale Semiconductor, Inc.
 * Synced from Linux v4.19
 */

#ifndef __LINUX_MTD_SPI_NOR_H
#define __LINUX_MTD_SPI_NOR_H

#include <mtd.h>
#include <linux/bitops.h>
#include <linux/mtd/cfi.h>
#include <linux/mtd/mtd.h>
#include <spi-mem.h>

/*
 * Manufacturer IDs
 *
 * The first byte returned from the flash after sending opcode SPINOR_OP_RDID.
 * Sometimes these are the same as CFI IDs, but sometimes they aren't.
 */
#define SNOR_MFR_ATMEL		CFI_MFR_ATMEL
#define SNOR_MFR_GIGADEVICE	0xc8
#define SNOR_MFR_INTEL		CFI_MFR_INTEL
#define SNOR_MFR_ST		CFI_MFR_ST /* ST Micro <--> Micron */
#define SNOR_MFR_MICRON		CFI_MFR_MICRON /* ST Micro <--> Micron */
#define SNOR_MFR_ISSI		CFI_MFR_PMC
#define SNOR_MFR_MACRONIX	CFI_MFR_MACRONIX
#define SNOR_MFR_SPANSION	CFI_MFR_AMD
#define SNOR_MFR_SST		CFI_MFR_SST
#define SNOR_MFR_WINBOND	0xef /* Also used by some Spansion */
#define SNOR_MFR_CYPRESS	0x34

/*
 * Note on opcode nomenclature: some opcodes have a format like
 * SPINOR_OP_FUNCTION{4,}_x_y_z. The numbers x, y, and z stand for the number
 * of I/O lines used for the opcode, address, and data (respectively). The
 * FUNCTION has an optional suffix of '4', to represent an opcode which
 * requires a 4-byte (32-bit) address.
 */

/* Flash opcodes. */
#define SPINOR_OP_WREN		0x06	/* Write enable */
#define SPINOR_OP_RDSR		0x05	/* Read status register */
#define SPINOR_OP_WRSR		0x01	/* Write status register 1 byte */
#define SPINOR_OP_RDSR2		0x3f	/* Read status register 2 */
#define SPINOR_OP_WRSR2		0x3e	/* Write status register 2 */
#define SPINOR_OP_READ		0x03	/* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST	0x0b	/* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2	0x3b	/* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2	0xbb	/* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4	0x6b	/* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4	0xeb	/* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8	0x8b	/* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8	0xcb	/* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP		0x02	/* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4	0x32	/* Quad page program */
#define SPINOR_OP_PP_1_4_4	0x38	/* Quad page program */
#define SPINOR_OP_PP_1_1_8	0x82	/* Octal page program */
#define SPINOR_OP_PP_1_8_8	0xc2	/* Octal page program */
#define SPINOR_OP_BE_4K		0x20	/* Erase 4KiB block */
#define SPINOR_OP_BE_4K_PMC	0xd7	/* Erase 4KiB block on PMC chips */
#define SPINOR_OP_BE_32K	0x52	/* Erase 32KiB block */
#define SPINOR_OP_CHIP_ERASE	0xc7	/* Erase whole flash chip */
#define SPINOR_OP_SE		0xd8	/* Sector erase (usually 64KiB) */
#define SPINOR_OP_RDID		0x9f	/* Read JEDEC ID */
#define SPINOR_OP_RDSFDP	0x5a	/* Read SFDP */
#define SPINOR_OP_RDCR		0x35	/* Read configuration register */
#define SPINOR_OP_RDFSR		0x70	/* Read flag status register */
#define SPINOR_OP_CLFSR		0x50	/* Clear flag status register */
#define SPINOR_OP_RDEAR		0xc8	/* Read Extended Address Register */
#define SPINOR_OP_WREAR		0xc5	/* Write Extended Address Register */
#define SPINOR_OP_SRSTEN	0x66	/* Software Reset Enable */
#define SPINOR_OP_SRST		0x99	/* Software Reset */

/* 4-byte address opcodes - used on Spansion and some Macronix flashes. */
#define SPINOR_OP_READ_4B	0x13	/* Read data bytes (low frequency) */
#define SPINOR_OP_READ_FAST_4B	0x0c	/* Read data bytes (high frequency) */
#define SPINOR_OP_READ_1_1_2_4B	0x3c	/* Read data bytes (Dual Output SPI) */
#define SPINOR_OP_READ_1_2_2_4B	0xbc	/* Read data bytes (Dual I/O SPI) */
#define SPINOR_OP_READ_1_1_4_4B	0x6c	/* Read data bytes (Quad Output SPI) */
#define SPINOR_OP_READ_1_4_4_4B	0xec	/* Read data bytes (Quad I/O SPI) */
#define SPINOR_OP_READ_1_1_8_4B	0x7c	/* Read data bytes (Octal Output SPI) */
#define SPINOR_OP_READ_1_8_8_4B	0xcc	/* Read data bytes (Octal I/O SPI) */
#define SPINOR_OP_PP_4B		0x12	/* Page program (up to 256 bytes) */
#define SPINOR_OP_PP_1_1_4_4B	0x34	/* Quad page program */
#define SPINOR_OP_PP_1_4_4_4B	0x3e	/* Quad page program */
#define SPINOR_OP_PP_1_1_8_4B	0x84	/* Octal page program */
#define SPINOR_OP_PP_1_8_8_4B	0x8e	/* Octal page program */
#define SPINOR_OP_BE_4K_4B	0x21	/* Erase 4KiB block */
#define SPINOR_OP_BE_32K_4B	0x5c	/* Erase 32KiB block */
#define SPINOR_OP_SE_4B		0xdc	/* Sector erase (usually 64KiB) */

/* Double Transfer Rate opcodes - defined in JEDEC JESD216B. */
#define SPINOR_OP_READ_1_1_1_DTR	0x0d
#define SPINOR_OP_READ_1_2_2_DTR	0xbd
#define SPINOR_OP_READ_1_4_4_DTR	0xed

#define SPINOR_OP_READ_1_1_1_DTR_4B	0x0e
#define SPINOR_OP_READ_1_2_2_DTR_4B	0xbe
#define SPINOR_OP_READ_1_4_4_DTR_4B	0xee

/* Used for SST flashes only. */
#define SPINOR_OP_BP		0x02	/* Byte program */
#define SPINOR_OP_WRDI		0x04	/* Write disable */
#define SPINOR_OP_AAI_WP	0xad	/* Auto address increment word program */

/* Used for SST26* flashes only. */
#define SPINOR_OP_READ_BPR	0x72	/* Read block protection register */
#define SPINOR_OP_WRITE_BPR	0x42	/* Write block protection register */

/* Used for S3AN flashes only */
#define SPINOR_OP_XSE		0x50	/* Sector erase */
#define SPINOR_OP_XPP		0x82	/* Page program */
#define SPINOR_OP_XRDSR		0xd7	/* Read status register */

#define XSR_PAGESIZE		BIT(0)	/* Page size in Po2 or Linear */
#define XSR_RDY			BIT(7)	/* Ready */

/* Used for Macronix and Winbond flashes. */
#define SPINOR_OP_EN4B		0xb7	/* Enter 4-byte mode */
#define SPINOR_OP_EX4B		0xe9	/* Exit 4-byte mode */
#define SPINOR_OP_EN4B			0xb7		/* Enter 4-byte mode */
#define SPINOR_OP_EX4B			0xe9		/* Exit 4-byte mode */
#define SPINOR_OP_RD_CR2		0x71		/* Read configuration register 2 */
#define SPINOR_OP_WR_CR2		0x72		/* Write configuration register 2 */
#define SPINOR_OP_MXIC_DTR_RD		0xee		/* Fast Read opcode in DTR mode */
#define SPINOR_REG_MXIC_CR2_MODE	0x00000000	/* For setting octal DTR mode */
#define SPINOR_REG_MXIC_OPI_DTR_EN	0x2		/* Enable Octal DTR */
#define SPINOR_REG_MXIC_CR2_DC		0x00000300	/* For setting dummy cycles */
#define SPINOR_REG_MXIC_DC_20		0x0		/* Setting dummy cycles to 20 */
#define MXIC_MAX_DC			20		/* Maximum value of dummy cycles */

/* Used for Spansion flashes only. */
#define SPINOR_OP_BRWR		0x17	/* Bank register write */
#define SPINOR_OP_BRRD		0x16	/* Bank register read */
#define SPINOR_OP_CLSR		0x30	/* Clear status register 1 */
#define SPINOR_OP_EX4B_CYPRESS	0xB8	/* Exit 4-byte mode */

/* Used for Micron flashes only. */
#define SPINOR_OP_RD_EVCR	0x65	/* Read EVCR register */
#define SPINOR_OP_WD_EVCR	0x61	/* Write EVCR register */
#define SPINOR_OP_MT_DTR_RD	0xfd	/* Fast Read opcode in DTR mode */
#define SPINOR_OP_MT_RD_ANY_REG	0x85	/* Read volatile register */
#define SPINOR_OP_MT_WR_ANY_REG	0x81	/* Write volatile register */
#define SPINOR_REG_MT_CFR0V	0x00	/* For setting octal DTR mode */
#define SPINOR_REG_MT_CFR1V	0x01	/* For setting dummy cycles */
#define SPINOR_MT_OCT_DTR	0xe7	/* Enable Octal DTR with DQS. */

/* Status Register bits. */
#define SR_WIP			BIT(0)	/* Write in progress */
#define SR_WEL			BIT(1)	/* Write enable latch */
/* meaning of other SR_* bits may differ between vendors */
#define SR_BP0			BIT(2)	/* Block protect 0 */
#define SR_BP1			BIT(3)	/* Block protect 1 */
#define SR_BP2			BIT(4)	/* Block protect 2 */
#define SR_TB			BIT(5)	/* Top/Bottom protect */
#define SR_SRWD			BIT(7)	/* SR write protect */
/* Spansion/Cypress specific status bits */
#define SR_E_ERR		BIT(5)
#define SR_P_ERR		BIT(6)

#define SR_QUAD_EN_MX		BIT(6)	/* Macronix Quad I/O */

/* Enhanced Volatile Configuration Register bits */
#define EVCR_QUAD_EN_MICRON	BIT(7)	/* Micron Quad I/O */

/* Flag Status Register bits */
#define FSR_READY		BIT(7)	/* Device status, 0 = Busy, 1 = Ready */
#define FSR_E_ERR		BIT(5)	/* Erase operation status */
#define FSR_P_ERR		BIT(4)	/* Program operation status */
#define FSR_PT_ERR		BIT(1)	/* Protection error bit */

/* Configuration Register bits. */
#define CR_QUAD_EN_SPAN		BIT(1)	/* Spansion Quad I/O */

/* Status Register 2 bits. */
#define SR2_QUAD_EN_BIT7	BIT(7)

/* For Cypress flash. */
#define SPINOR_OP_RD_ANY_REG			0x65	/* Read any register */
#define SPINOR_OP_WR_ANY_REG			0x71	/* Write any register */
#define SPINOR_OP_CYPRESS_CLPEF			0x82	/* Clear P/E err flag */
#define SPINOR_REG_CYPRESS_ARCFN		0x00000006
#define SPINOR_REG_CYPRESS_STR1V		0x00800000
#define SPINOR_REG_CYPRESS_CFR1V		0x00800002
#define SPINOR_REG_CYPRESS_CFR2V		0x00800003
#define SPINOR_REG_CYPRESS_CFR2_MEMLAT_MASK	GENMASK(3, 0)
#define SPINOR_REG_CYPRESS_CFR2_MEMLAT_11_24	0xb
#define SPINOR_REG_CYPRESS_CFR3V		0x00800004
#define SPINOR_REG_CYPRESS_CFR3_PGSZ		BIT(4) /* Page size. */
#define SPINOR_REG_CYPRESS_CFR3_UNISECT		BIT(3) /* Uniform sector mode */
#define SPINOR_REG_CYPRESS_CFR5V		0x00800006
#define SPINOR_REG_CYPRESS_CFR5_BIT6		BIT(6)
#define SPINOR_REG_CYPRESS_CFR5_DDR		BIT(1)
#define SPINOR_REG_CYPRESS_CFR5_OPI		BIT(0)
#define SPINOR_REG_CYPRESS_CFR5_OCT_DTR_EN				\
	(SPINOR_REG_CYPRESS_CFR5_BIT6 |	SPINOR_REG_CYPRESS_CFR5_DDR |	\
	 SPINOR_REG_CYPRESS_CFR5_OPI)
#define SPINOR_OP_CYPRESS_RD_FAST		0xee

/* Supported SPI protocols */
#define SNOR_PROTO_INST_MASK	GENMASK(23, 16)
#define SNOR_PROTO_INST_SHIFT	16
#define SNOR_PROTO_INST(_nbits)	\
	((((unsigned long)(_nbits)) << SNOR_PROTO_INST_SHIFT) & \
	 SNOR_PROTO_INST_MASK)

#define SNOR_PROTO_ADDR_MASK	GENMASK(15, 8)
#define SNOR_PROTO_ADDR_SHIFT	8
#define SNOR_PROTO_ADDR(_nbits)	\
	((((unsigned long)(_nbits)) << SNOR_PROTO_ADDR_SHIFT) & \
	 SNOR_PROTO_ADDR_MASK)

#define SNOR_PROTO_DATA_MASK	GENMASK(7, 0)
#define SNOR_PROTO_DATA_SHIFT	0
#define SNOR_PROTO_DATA(_nbits)	\
	((((unsigned long)(_nbits)) << SNOR_PROTO_DATA_SHIFT) & \
	 SNOR_PROTO_DATA_MASK)

#define SNOR_PROTO_IS_DTR	BIT(24)	/* Double Transfer Rate */

#define SNOR_PROTO_STR(_inst_nbits, _addr_nbits, _data_nbits)	\
	(SNOR_PROTO_INST(_inst_nbits) |				\
	 SNOR_PROTO_ADDR(_addr_nbits) |				\
	 SNOR_PROTO_DATA(_data_nbits))
#define SNOR_PROTO_DTR(_inst_nbits, _addr_nbits, _data_nbits)	\
	(SNOR_PROTO_IS_DTR |					\
	 SNOR_PROTO_STR(_inst_nbits, _addr_nbits, _data_nbits))

enum spi_nor_protocol {
	SNOR_PROTO_1_1_1 = SNOR_PROTO_STR(1, 1, 1),
	SNOR_PROTO_1_1_2 = SNOR_PROTO_STR(1, 1, 2),
	SNOR_PROTO_1_1_4 = SNOR_PROTO_STR(1, 1, 4),
	SNOR_PROTO_1_1_8 = SNOR_PROTO_STR(1, 1, 8),
	SNOR_PROTO_1_2_2 = SNOR_PROTO_STR(1, 2, 2),
	SNOR_PROTO_1_4_4 = SNOR_PROTO_STR(1, 4, 4),
	SNOR_PROTO_1_8_8 = SNOR_PROTO_STR(1, 8, 8),
	SNOR_PROTO_2_2_2 = SNOR_PROTO_STR(2, 2, 2),
	SNOR_PROTO_4_4_4 = SNOR_PROTO_STR(4, 4, 4),
	SNOR_PROTO_8_8_8 = SNOR_PROTO_STR(8, 8, 8),

	SNOR_PROTO_1_1_1_DTR = SNOR_PROTO_DTR(1, 1, 1),
	SNOR_PROTO_1_2_2_DTR = SNOR_PROTO_DTR(1, 2, 2),
	SNOR_PROTO_1_4_4_DTR = SNOR_PROTO_DTR(1, 4, 4),
	SNOR_PROTO_1_8_8_DTR = SNOR_PROTO_DTR(1, 8, 8),
	SNOR_PROTO_8_8_8_DTR = SNOR_PROTO_DTR(8, 8, 8),
};

static inline bool spi_nor_protocol_is_dtr(enum spi_nor_protocol proto)
{
	return !!(proto & SNOR_PROTO_IS_DTR);
}

static inline u8 spi_nor_get_protocol_inst_nbits(enum spi_nor_protocol proto)
{
	return ((unsigned long)(proto & SNOR_PROTO_INST_MASK)) >>
		SNOR_PROTO_INST_SHIFT;
}

static inline u8 spi_nor_get_protocol_addr_nbits(enum spi_nor_protocol proto)
{
	return ((unsigned long)(proto & SNOR_PROTO_ADDR_MASK)) >>
		SNOR_PROTO_ADDR_SHIFT;
}

static inline u8 spi_nor_get_protocol_data_nbits(enum spi_nor_protocol proto)
{
	return ((unsigned long)(proto & SNOR_PROTO_DATA_MASK)) >>
		SNOR_PROTO_DATA_SHIFT;
}

static inline u8 spi_nor_get_protocol_width(enum spi_nor_protocol proto)
{
	return spi_nor_get_protocol_data_nbits(proto);
}

#define SPI_NOR_MAX_CMD_SIZE	8
enum spi_nor_ops {
	SPI_NOR_OPS_READ = 0,
	SPI_NOR_OPS_WRITE,
	SPI_NOR_OPS_ERASE,
	SPI_NOR_OPS_LOCK,
	SPI_NOR_OPS_UNLOCK,
};

enum spi_nor_option_flags {
	SNOR_F_USE_FSR		= BIT(0),
	SNOR_F_HAS_SR_TB	= BIT(1),
	SNOR_F_NO_OP_CHIP_ERASE	= BIT(2),
	SNOR_F_S3AN_ADDR_DEFAULT = BIT(3),
	SNOR_F_READY_XSR_RDY	= BIT(4),
	SNOR_F_USE_CLSR		= BIT(5),
	SNOR_F_BROKEN_RESET	= BIT(6),
	SNOR_F_SOFT_RESET	= BIT(7),
	SNOR_F_IO_MODE_EN_VOLATILE = BIT(8),
};

struct spi_nor;

/**
 * struct spi_nor_hwcaps - Structure for describing the hardware capabilies
 * supported by the SPI controller (bus master).
 * @mask:		the bitmask listing all the supported hw capabilies
 */
struct spi_nor_hwcaps {
	u32	mask;
};

/*
 *(Fast) Read capabilities.
 * MUST be ordered by priority: the higher bit position, the higher priority.
 * As a matter of performances, it is relevant to use Octo SPI protocols first,
 * then Quad SPI protocols before Dual SPI protocols, Fast Read and lastly
 * (Slow) Read.
 */
#define SNOR_HWCAPS_READ_MASK		GENMASK(15, 0)
#define SNOR_HWCAPS_READ		BIT(0)
#define SNOR_HWCAPS_READ_FAST		BIT(1)
#define SNOR_HWCAPS_READ_1_1_1_DTR	BIT(2)

#define SNOR_HWCAPS_READ_DUAL		GENMASK(6, 3)
#define SNOR_HWCAPS_READ_1_1_2		BIT(3)
#define SNOR_HWCAPS_READ_1_2_2		BIT(4)
#define SNOR_HWCAPS_READ_2_2_2		BIT(5)
#define SNOR_HWCAPS_READ_1_2_2_DTR	BIT(6)

#define SNOR_HWCAPS_READ_QUAD		GENMASK(10, 7)
#define SNOR_HWCAPS_READ_1_1_4		BIT(7)
#define SNOR_HWCAPS_READ_1_4_4		BIT(8)
#define SNOR_HWCAPS_READ_4_4_4		BIT(9)
#define SNOR_HWCAPS_READ_1_4_4_DTR	BIT(10)

#define SNOR_HWCPAS_READ_OCTO		GENMASK(15, 11)
#define SNOR_HWCAPS_READ_1_1_8		BIT(11)
#define SNOR_HWCAPS_READ_1_8_8		BIT(12)
#define SNOR_HWCAPS_READ_8_8_8		BIT(13)
#define SNOR_HWCAPS_READ_1_8_8_DTR	BIT(14)
#define SNOR_HWCAPS_READ_8_8_8_DTR	BIT(15)

/*
 * Page Program capabilities.
 * MUST be ordered by priority: the higher bit position, the higher priority.
 * Like (Fast) Read capabilities, Octo/Quad SPI protocols are preferred to the
 * legacy SPI 1-1-1 protocol.
 * Note that Dual Page Programs are not supported because there is no existing
 * JEDEC/SFDP standard to define them. Also at this moment no SPI flash memory
 * implements such commands.
 */
#define SNOR_HWCAPS_PP_MASK		GENMASK(23, 16)
#define SNOR_HWCAPS_PP			BIT(16)

#define SNOR_HWCAPS_PP_QUAD		GENMASK(19, 17)
#define SNOR_HWCAPS_PP_1_1_4		BIT(17)
#define SNOR_HWCAPS_PP_1_4_4		BIT(18)
#define SNOR_HWCAPS_PP_4_4_4		BIT(19)

#define SNOR_HWCAPS_PP_OCTO		GENMASK(23, 20)
#define SNOR_HWCAPS_PP_1_1_8		BIT(20)
#define SNOR_HWCAPS_PP_1_8_8		BIT(21)
#define SNOR_HWCAPS_PP_8_8_8		BIT(22)
#define SNOR_HWCAPS_PP_8_8_8_DTR	BIT(23)

#define SNOR_HWCAPS_X_X_X	(SNOR_HWCAPS_READ_2_2_2 |	\
				 SNOR_HWCAPS_READ_4_4_4 |	\
				 SNOR_HWCAPS_READ_8_8_8 |	\
				 SNOR_HWCAPS_PP_4_4_4 |		\
				 SNOR_HWCAPS_PP_8_8_8)

#define SNOR_HWCAPS_X_X_X_DTR	(SNOR_HWCAPS_READ_8_8_8_DTR |	\
				 SNOR_HWCAPS_PP_8_8_8_DTR)

#define SNOR_HWCAPS_DTR		(SNOR_HWCAPS_READ_1_1_1_DTR |	\
				 SNOR_HWCAPS_READ_1_2_2_DTR |	\
				 SNOR_HWCAPS_READ_1_4_4_DTR |	\
				 SNOR_HWCAPS_READ_1_8_8_DTR)

#define SNOR_HWCAPS_ALL		(SNOR_HWCAPS_READ_MASK |	\
				 SNOR_HWCAPS_PP_MASK)

struct spi_nor_read_command {
	u8			num_mode_clocks;
	u8			num_wait_states;
	u8			opcode;
	enum spi_nor_protocol	proto;
};

struct spi_nor_pp_command {
	u8			opcode;
	enum spi_nor_protocol	proto;
};

enum spi_nor_read_command_index {
	SNOR_CMD_READ,
	SNOR_CMD_READ_FAST,
	SNOR_CMD_READ_1_1_1_DTR,

	/* Dual SPI */
	SNOR_CMD_READ_1_1_2,
	SNOR_CMD_READ_1_2_2,
	SNOR_CMD_READ_2_2_2,
	SNOR_CMD_READ_1_2_2_DTR,

	/* Quad SPI */
	SNOR_CMD_READ_1_1_4,
	SNOR_CMD_READ_1_4_4,
	SNOR_CMD_READ_4_4_4,
	SNOR_CMD_READ_1_4_4_DTR,

	/* Octo SPI */
	SNOR_CMD_READ_1_1_8,
	SNOR_CMD_READ_1_8_8,
	SNOR_CMD_READ_8_8_8,
	SNOR_CMD_READ_1_8_8_DTR,
	SNOR_CMD_READ_8_8_8_DTR,

	SNOR_CMD_READ_MAX
};

enum spi_nor_pp_command_index {
	SNOR_CMD_PP,

	/* Quad SPI */
	SNOR_CMD_PP_1_1_4,
	SNOR_CMD_PP_1_4_4,
	SNOR_CMD_PP_4_4_4,

	/* Octo SPI */
	SNOR_CMD_PP_1_1_8,
	SNOR_CMD_PP_1_8_8,
	SNOR_CMD_PP_8_8_8,
	SNOR_CMD_PP_8_8_8_DTR,

	SNOR_CMD_PP_MAX
};

struct spi_nor_flash_parameter {
	u64				size;
	u32				page_size;
	u8				rdsr_dummy;
	u8				rdsr_addr_nbytes;

	struct spi_nor_hwcaps		hwcaps;
	struct spi_nor_read_command	reads[SNOR_CMD_READ_MAX];
	struct spi_nor_pp_command	page_programs[SNOR_CMD_PP_MAX];

	int (*quad_enable)(struct spi_nor *nor);
};

/**
 * enum spi_nor_cmd_ext - describes the command opcode extension in DTR mode
 * @SPI_MEM_NOR_NONE: no extension. This is the default, and is used in Legacy
 *		      SPI mode
 * @SPI_MEM_NOR_REPEAT: the extension is same as the opcode
 * @SPI_MEM_NOR_INVERT: the extension is the bitwise inverse of the opcode
 * @SPI_MEM_NOR_HEX: the extension is any hex value. The command and opcode
 *		     combine to form a 16-bit opcode.
 */
enum spi_nor_cmd_ext {
	SPI_NOR_EXT_NONE = 0,
	SPI_NOR_EXT_REPEAT,
	SPI_NOR_EXT_INVERT,
	SPI_NOR_EXT_HEX,
};

/**
 * struct flash_info - Forward declaration of a structure used internally by
 *		       spi_nor_scan()
 */
struct flash_info;

/*
 * TODO: Remove, once all users of spi_flash interface are moved to MTD
 *
struct spi_flash {
 *	Defined below (keep this text to enable searching for spi_flash decl)
 * }
 */
#ifndef DT_PLAT_C
#define spi_flash spi_nor
#endif

/**
 * struct spi_nor - Structure for defining a the SPI NOR layer
 * @mtd:		point to a mtd_info structure
 * @lock:		the lock for the read/write/erase/lock/unlock operations
 * @dev:		point to a spi device, or a spi nor controller device.
 * @info:		spi-nor part JDEC MFR id and other info
 * @manufacturer_sfdp:	manufacturer specific SFDP table
 * @page_size:		the page size of the SPI NOR
 * @addr_width:		number of address bytes
 * @erase_opcode:	the opcode for erasing a sector
 * @read_opcode:	the read opcode
 * @read_dummy:		the dummy needed by the read operation
 * @program_opcode:	the program opcode
 * @rdsr_dummy		dummy cycles needed for Read Status Register command.
 * @rdsr_addr_nbytes:	dummy address bytes needed for Read Status Register
 *			command.
 * @addr_mode_nbytes:	number of address bytes of current address mode. Useful
 *			when the flash operates with 4B opcodes but needs the
 *			internal address mode for opcodes that don't have a 4B
 *			opcode correspondent.
 * @bank_read_cmd:	Bank read cmd
 * @bank_write_cmd:	Bank write cmd
 * @bank_curr:		Current flash bank
 * @sst_write_second:	used by the SST write operation
 * @flags:		flag options for the current SPI-NOR (SNOR_F_*)
 * @read_proto:		the SPI protocol for read operations
 * @write_proto:	the SPI protocol for write operations
 * @reg_proto		the SPI protocol for read_reg/write_reg/erase operations
 * @cmd_buf:		used by the write_reg
 * @cmd_ext_type:	the command opcode extension for DTR mode.
 * @fixups:		flash-specific fixup hooks.
 * @prepare:		[OPTIONAL] do some preparations for the
 *			read/write/erase/lock/unlock operations
 * @unprepare:		[OPTIONAL] do some post work after the
 *			read/write/erase/lock/unlock operations
 * @read_reg:		[DRIVER-SPECIFIC] read out the register
 * @write_reg:		[DRIVER-SPECIFIC] write data to the register
 * @read:		[DRIVER-SPECIFIC] read data from the SPI NOR
 * @write:		[DRIVER-SPECIFIC] write data to the SPI NOR
 * @erase:		[DRIVER-SPECIFIC] erase a sector of the SPI NOR
 *			at the offset @offs; if not provided by the driver,
 *			spi-nor will send the erase opcode via write_reg()
 * @flash_lock:		[FLASH-SPECIFIC] lock a region of the SPI NOR
 * @flash_unlock:	[FLASH-SPECIFIC] unlock a region of the SPI NOR
 * @flash_is_unlocked:	[FLASH-SPECIFIC] check if a region of the SPI NOR is
 *			completely unlocked
 * @quad_enable:	[FLASH-SPECIFIC] enables SPI NOR quad mode
 * @octal_dtr_enable:	[FLASH-SPECIFIC] enables SPI NOR octal DTR mode.
 * @ready:		[FLASH-SPECIFIC] check if the flash is ready
 * @dirmap:		pointers to struct spi_mem_dirmap_desc for reads/writes.
 * @priv:		the private data
 */
struct spi_nor {
	struct mtd_info		mtd;
	struct udevice		*dev;
	struct spi_slave	*spi;
	const struct flash_info	*info;
	u8			*manufacturer_sfdp;
	u32			page_size;
	u8			addr_width;
	u8			erase_opcode;
	u8			read_opcode;
	u8			read_dummy;
	u8			program_opcode;
	u8			rdsr_dummy;
	u8			rdsr_addr_nbytes;
	u8			addr_mode_nbytes;
#ifdef CONFIG_SPI_FLASH_BAR
	u8			bank_read_cmd;
	u8			bank_write_cmd;
	u8			bank_curr;
#endif
	enum spi_nor_protocol	read_proto;
	enum spi_nor_protocol	write_proto;
	enum spi_nor_protocol	reg_proto;
	bool			sst_write_second;
	u32			flags;
	u8			cmd_buf[SPI_NOR_MAX_CMD_SIZE];
	enum spi_nor_cmd_ext	cmd_ext_type;
	struct spi_nor_fixups	*fixups;

	int (*setup)(struct spi_nor *nor, const struct flash_info *info,
		     const struct spi_nor_flash_parameter *params);
	int (*prepare)(struct spi_nor *nor, enum spi_nor_ops ops);
	void (*unprepare)(struct spi_nor *nor, enum spi_nor_ops ops);
	int (*read_reg)(struct spi_nor *nor, u8 opcode, u8 *buf, int len);
	int (*write_reg)(struct spi_nor *nor, u8 opcode, u8 *buf, int len);

	ssize_t (*read)(struct spi_nor *nor, loff_t from,
			size_t len, u_char *read_buf);
	ssize_t (*write)(struct spi_nor *nor, loff_t to,
			 size_t len, const u_char *write_buf);
	int (*erase)(struct spi_nor *nor, loff_t offs);

	int (*flash_lock)(struct spi_nor *nor, loff_t ofs, uint64_t len);
	int (*flash_unlock)(struct spi_nor *nor, loff_t ofs, uint64_t len);
	int (*flash_is_unlocked)(struct spi_nor *nor, loff_t ofs, uint64_t len);
	int (*quad_enable)(struct spi_nor *nor);
	int (*octal_dtr_enable)(struct spi_nor *nor);
	int (*ready)(struct spi_nor *nor);

	struct {
		struct spi_mem_dirmap_desc *rdesc;
		struct spi_mem_dirmap_desc *wdesc;
	} dirmap;

	void *priv;
	char mtd_name[MTD_NAME_SIZE(MTD_DEV_TYPE_NOR)];
/* Compatibility for spi_flash, remove once sf layer is merged with mtd */
	const char *name;
	u32 size;
	u32 sector_size;
	u32 erase_size;
};

#ifndef __UBOOT__
static inline void spi_nor_set_flash_node(struct spi_nor *nor,
					  const struct device_node *np)
{
	mtd_set_of_node(&nor->mtd, np);
}

static inline const struct
device_node *spi_nor_get_flash_node(struct spi_nor *nor)
{
	return mtd_get_of_node(&nor->mtd);
}
#endif /* __UBOOT__ */

/**
 * spi_nor_setup_op() - Set up common properties of a spi-mem op.
 * @nor:		pointer to a 'struct spi_nor'
 * @op:			pointer to the 'struct spi_mem_op' whose properties
 *			need to be initialized.
 * @proto:		the protocol from which the properties need to be set.
 */
void spi_nor_setup_op(const struct spi_nor *nor,
		      struct spi_mem_op *op,
		      const enum spi_nor_protocol proto);

/**
 * spi_nor_scan() - scan the SPI NOR
 * @nor:	the spi_nor structure
 *
 * The drivers can use this function to scan the SPI NOR.
 * In the scanning, it will try to get all the necessary information to
 * fill the mtd_info{} and the spi_nor{}.
 *
 * Return: 0 for success, others for failure.
 */
int spi_nor_scan(struct spi_nor *nor);

#if CONFIG_IS_ENABLED(SPI_FLASH_TINY)
static inline int spi_nor_remove(struct spi_nor *nor)
{
	return 0;
}
#else
/**
 * spi_nor_remove() - perform cleanup before booting to the next stage
 * @nor:	the spi_nor structure
 *
 * Return: 0 for success, -errno for failure.
 */
int spi_nor_remove(struct spi_nor *nor);
#endif

#endif
