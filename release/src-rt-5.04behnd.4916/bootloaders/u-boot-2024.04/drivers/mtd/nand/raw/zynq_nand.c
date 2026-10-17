// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2016 Xilinx, Inc.
 *
 * Xilinx Zynq NAND Flash Controller Driver
 * This driver is based on plat_nand.c and mxc_nand.c drivers
 */

#include <common.h>
#include <log.h>
#include <malloc.h>
#include <asm/io.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <nand.h>
#include <linux/ioport.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/rawnand.h>
#include <linux/mtd/partitions.h>
#include <linux/mtd/nand_ecc.h>
#include <asm/arch/hardware.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>
#include <linux/printk.h>

/* The NAND flash driver defines */
#define ZYNQ_NAND_CMD_PHASE		1
#define ZYNQ_NAND_DATA_PHASE		2
#define ZYNQ_NAND_ECC_SIZE		512
#define ZYNQ_NAND_SET_OPMODE_8BIT	(0 << 0)
#define ZYNQ_NAND_SET_OPMODE_16BIT	(1 << 0)
#define ZYNQ_NAND_ECC_STATUS		(1 << 6)
#define ZYNQ_MEMC_CLRCR_INT_CLR1	(1 << 4)
#define ZYNQ_MEMC_SR_RAW_INT_ST1	(1 << 6)
#define ZYNQ_MEMC_SR_INT_ST1		(1 << 4)
#define ZYNQ_MEMC_NAND_ECC_MODE_MASK	0xC

/* Flash memory controller operating parameters */
#define ZYNQ_NAND_CLR_CONFIG	((0x1 << 1)  |	/* Disable interrupt */ \
				(0x1 << 4)   |	/* Clear interrupt */ \
				(0x1 << 6))	/* Disable ECC interrupt */

#ifndef CONFIG_NAND_ZYNQ_USE_BOOTLOADER1_TIMINGS

/* Assuming 50MHz clock (20ns cycle time) and 3V operation */
#define ZYNQ_NAND_SET_CYCLES	((0x2 << 20) |	/* t_rr from nand_cycles */ \
				(0x2 << 17)  |	/* t_ar from nand_cycles */ \
				(0x1 << 14)  |	/* t_clr from nand_cycles */ \
				(0x3 << 11)  |	/* t_wp from nand_cycles */ \
				(0x2 << 8)   |	/* t_rea from nand_cycles */ \
				(0x5 << 4)   |	/* t_wc from nand_cycles */ \
				(0x5 << 0))	/* t_rc from nand_cycles */
#endif


#define ZYNQ_NAND_DIRECT_CMD	((0x4 << 23) |	/* Chip 0 from interface 1 */ \
				(0x2 << 21))	/* UpdateRegs operation */

#define ZYNQ_NAND_ECC_CONFIG	((0x1 << 2)  |	/* ECC available on APB */ \
				(0x1 << 4)   |	/* ECC read at end of page */ \
				(0x0 << 5))	/* No Jumping */

#define ZYNQ_NAND_ECC_CMD1	((0x80)      |	/* Write command */ \
				(0x00 << 8)  |	/* Read command */ \
				(0x30 << 16) |	/* Read End command */ \
				(0x1 << 24))	/* Read End command calid */

#define ZYNQ_NAND_ECC_CMD2	((0x85)      |	/* Write col change cmd */ \
				(0x05 << 8)  |	/* Read col change cmd */ \
				(0xE0 << 16) |	/* Read col change end cmd */ \
				(0x1 << 24))	/* Read col change
							end cmd valid */
/* AXI Address definitions */
#define START_CMD_SHIFT			3
#define END_CMD_SHIFT			11
#define END_CMD_VALID_SHIFT		20
#define ADDR_CYCLES_SHIFT		21
#define CLEAR_CS_SHIFT			21
#define ECC_LAST_SHIFT			10
#define COMMAND_PHASE			(0 << 19)
#define DATA_PHASE			(1 << 19)
#define ONDIE_ECC_FEATURE_ADDR		0x90
#define ONDIE_ECC_FEATURE_ENABLE	0x08

#define ZYNQ_NAND_ECC_LAST	(1 << ECC_LAST_SHIFT)	/* Set ECC_Last */
#define ZYNQ_NAND_CLEAR_CS	(1 << CLEAR_CS_SHIFT)	/* Clear chip select */

/* ECC block registers bit position and bit mask */
#define ZYNQ_NAND_ECC_BUSY	(1 << 6)	/* ECC block is busy */
#define ZYNQ_NAND_ECC_MASK	0x00FFFFFF	/* ECC value mask */

#define ZYNQ_NAND_ROW_ADDR_CYCL_MASK	0x0F
#define ZYNQ_NAND_COL_ADDR_CYCL_MASK	0xF0

#define ZYNQ_NAND_MIO_NUM_NAND_8BIT	13
#define ZYNQ_NAND_MIO_NUM_NAND_16BIT	8

enum zynq_nand_bus_width {
	NAND_BW_UNKNOWN = -1,
	NAND_BW_8BIT,
	NAND_BW_16BIT,
};

#ifndef NAND_CMD_LOCK_TIGHT
#define NAND_CMD_LOCK_TIGHT 0x2c
#endif

#ifndef NAND_CMD_LOCK_STATUS
#define NAND_CMD_LOCK_STATUS 0x7a
#endif

/* SMC register set */
struct zynq_nand_smc_regs {
	u32 csr;		/* 0x00 */
	u32 reserved0[2];
	u32 cfr;		/* 0x0C */
	u32 dcr;		/* 0x10 */
	u32 scr;		/* 0x14 */
	u32 sor;		/* 0x18 */
	u32 reserved1[249];
	u32 esr;		/* 0x400 */
	u32 emcr;		/* 0x404 */
	u32 emcmd1r;		/* 0x408 */
	u32 emcmd2r;		/* 0x40C */
	u32 reserved2[2];
	u32 eval0r;		/* 0x418 */
};

/*
 * struct nand_config - Defines the NAND flash driver instance
 * @parts:		Pointer to the mtd_partition structure
 * @nand_base:		Virtual address of the NAND flash device
 * @end_cmd_pending:	End command is pending
 * @end_cmd:		End command
 */
struct nand_config {
	void __iomem	*nand_base;
	u8		end_cmd_pending;
	u8		end_cmd;
};

struct nand_drv {
	struct zynq_nand_smc_regs *reg;
	struct nand_config config;
};

struct zynq_nand_info {
	struct udevice *dev;
	struct nand_drv nand_ctrl;
	struct nand_chip nand_chip;
};

/*
 * struct zynq_nand_command_format - Defines NAND flash command format
 * @start_cmd:		First cycle command (Start command)
 * @end_cmd:		Second cycle command (Last command)
 * @addr_cycles:	Number of address cycles required to send the address
 * @end_cmd_valid:	The second cycle command is valid for cmd or data phase
 */
struct zynq_nand_command_format {
	u8 start_cmd;
	u8 end_cmd;
	u8 addr_cycles;
	u8 end_cmd_valid;
};

/*  The NAND flash operations command format */
static const struct zynq_nand_command_format zynq_nand_commands[] = {
	{NAND_CMD_READ0, NAND_CMD_READSTART, 5, ZYNQ_NAND_CMD_PHASE},
	{NAND_CMD_RNDOUT, NAND_CMD_RNDOUTSTART, 2, ZYNQ_NAND_CMD_PHASE},
	{NAND_CMD_READID, NAND_CMD_NONE, 1, 0},
	{NAND_CMD_STATUS, NAND_CMD_NONE, 0, 0},
	{NAND_CMD_SEQIN, NAND_CMD_PAGEPROG, 5, ZYNQ_NAND_DATA_PHASE},
	{NAND_CMD_RNDIN, NAND_CMD_NONE, 2, 0},
	{NAND_CMD_ERASE1, NAND_CMD_ERASE2, 3, ZYNQ_NAND_CMD_PHASE},
	{NAND_CMD_RESET, NAND_CMD_NONE, 0, 0},
	{NAND_CMD_PARAM, NAND_CMD_NONE, 1, 0},
	{NAND_CMD_GET_FEATURES, NAND_CMD_NONE, 1, 0},
	{NAND_CMD_SET_FEATURES, NAND_CMD_NONE, 1, 0},
	{NAND_CMD_LOCK, NAND_CMD_NONE, 0, 0},
	{NAND_CMD_LOCK_TIGHT, NAND_CMD_NONE, 0, 0},
	{NAND_CMD_UNLOCK1, NAND_CMD_NONE, 3, 0},
	{NAND_CMD_UNLOCK2, NAND_CMD_NONE, 3, 0},
	{NAND_CMD_LOCK_STATUS, NAND_CMD_NONE, 3, 0},
	{NAND_CMD_NONE, NAND_CMD_NONE, 0, 0},
	/* Add all the flash commands supported by the flash device */
};

/* Define default oob placement schemes for large and small page devices */
static struct nand_ecclayout nand_oob_16 = {
	.eccbytes = 3,
	.eccpos = {0, 1, 2},
	.oobfree = {
		{ .offset = 8, .length = 8 }
	}
};

static struct nand_ecclayout nand_oob_64 = {
	.eccbytes = 12,
	.eccpos = {
		   52, 53, 54, 55, 56, 57,
		   58, 59, 60, 61, 62, 63},
	.oobfree = {
		{ .offset = 2, .length = 50 }
	}
};

static struct nand_ecclayout ondie_nand_oob_64 = {
	.eccbytes = 32,

	.eccpos = {
		8, 9, 10, 11, 12, 13, 14, 15,
		24, 25, 26, 27, 28, 29, 30, 31,
		40, 41, 42, 43, 44, 45, 46, 47,
		56, 57, 58, 59, 60, 61, 62, 63
	},

	.oobfree = {
		{ .offset = 4, .length = 4 },
		{ .offset = 20, .length = 4 },
		{ .offset = 36, .length = 4 },
		{ .offset = 52, .length = 4 }
	}
};

/* bbt decriptors for chips with on-die ECC and
   chips with 64-byte OOB */
static u8 bbt_pattern[] = {'B', 'b', 't', '0' };
static u8 mirror_pattern[] = {'1', 't', 'b', 'B' };

static struct nand_bbt_descr bbt_main_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 4,
	.len = 4,
	.veroffs = 20,
	.maxblocks = 4,
	.pattern = bbt_pattern
};

static struct nand_bbt_descr bbt_mirror_descr = {
	.options = NAND_BBT_LASTBLOCK | NAND_BBT_CREATE | NAND_BBT_WRITE |
		NAND_BBT_2BIT | NAND_BBT_VERSION | NAND_BBT_PERCHIP,
	.offs = 4,
	.len = 4,
	.veroffs = 20,
	.maxblocks = 4,
	.pattern = mirror_pattern
};

/*
 * zynq_nand_waitfor_ecc_completion - Wait for ECC completion
 *
 * returns: status for command completion, -1 for Timeout
 */
static int zynq_nand_waitfor_ecc_completion(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd_to_nand(mtd);
	struct nand_drv *smc = nand_get_controller_data(nand_chip);
	unsigned long timeout;
	u32 status;

	/* Wait max 10us */
	timeout = 10;
	status = readl(&smc->reg->esr);
	while (status & ZYNQ_NAND_ECC_BUSY) {
		status = readl(&smc->reg->esr);
		if (timeout == 0)
			return -1;
		timeout--;
		udelay(1);
	}

	return status;
}

/*
 * zynq_nand_init_nand_flash - Initialize NAND controller
 * @option:	Device property flags
 *
 * This function initializes the NAND flash interface on the NAND controller.
 *
 * returns:	0 on success or error value on failure
 */
static int zynq_nand_init_nand_flash(struct mtd_info *mtd, int option)
{
	struct nand_chip *nand_chip = mtd_to_nand(mtd);
	struct nand_drv *smc = nand_get_controller_data(nand_chip);
	int status;

	/* disable interrupts */
	writel(ZYNQ_NAND_CLR_CONFIG, &smc->reg->cfr);
#ifndef CONFIG_NAND_ZYNQ_USE_BOOTLOADER1_TIMINGS
	/* Initialize the NAND interface by setting cycles and operation mode */
	writel(ZYNQ_NAND_SET_CYCLES, &smc->reg->scr);
#endif
	if (option & NAND_BUSWIDTH_16)
		writel(ZYNQ_NAND_SET_OPMODE_16BIT, &smc->reg->sor);
	else
		writel(ZYNQ_NAND_SET_OPMODE_8BIT, &smc->reg->sor);

	writel(ZYNQ_NAND_DIRECT_CMD, &smc->reg->dcr);

	/* Wait till the ECC operation is complete */
	status = zynq_nand_waitfor_ecc_completion(mtd);
	if (status < 0) {
		printf("%s: Timeout\n", __func__);
		return status;
	}

	/* Set the command1 and command2 register */
	writel(ZYNQ_NAND_ECC_CMD1, &smc->reg->emcmd1r);
	writel(ZYNQ_NAND_ECC_CMD2, &smc->reg->emcmd2r);

	return 0;
}

/*
 * zynq_nand_calculate_hwecc - Calculate Hardware ECC
 * @mtd:	Pointer to the mtd_info structure
 * @data:	Pointer to the page data
 * @ecc_code:	Pointer to the ECC buffer where ECC data needs to be stored
 *
 * This function retrieves the Hardware ECC data from the controller and returns
 * ECC data back to the MTD subsystem.
 *
 * returns:	0 on success or error value on failure
 */
static int zynq_nand_calculate_hwecc(struct mtd_info *mtd, const u8 *data,
		u8 *ecc_code)
{
	struct nand_chip *nand_chip = mtd_to_nand(mtd);
	struct nand_drv *smc = nand_get_controller_data(nand_chip);
	u32 ecc_value = 0;
	u8 ecc_reg, ecc_byte;
	int ecc_status;

	/* Wait till the ECC operation is complete */
	ecc_status = zynq_nand_waitfor_ecc_completion(mtd);
	if (ecc_status < 0) {
		printf("%s: Timeout\n", __func__);
		return ecc_status;
	}

	for (ecc_reg = 0; ecc_reg < 4; ecc_reg++) {
		/* Read ECC value for each block */
		ecc_value = readl(&smc->reg->eval0r + ecc_reg);

		/* Get the ecc status from ecc read value */
		ecc_status = (ecc_value >> 24) & 0xFF;

		/* ECC value valid */
		if (ecc_status & ZYNQ_NAND_ECC_STATUS) {
			for (ecc_byte = 0; ecc_byte < 3; ecc_byte++) {
				/* Copy ECC bytes to MTD buffer */
				*ecc_code = ecc_value & 0xFF;
				ecc_value = ecc_value >> 8;
				ecc_code++;
			}
		} else {
			debug("%s: ecc status failed\n", __func__);
		}
	}

	return 0;
}

/*
 * onehot - onehot function
 * @value:	value to check for onehot
 *
 * This function checks whether a value is onehot or not.
 * onehot is if and only if one bit is set.
 *
 * FIXME: Try to move this in common.h
 */
static bool onehot(unsigned short value)
{
	bool onehot;

	onehot = value && !(value & (value - 1));
	return onehot;
}

/*
 * zynq_nand_correct_data - ECC correction function
 * @mtd:	Pointer to the mtd_info structure
 * @buf:	Pointer to the page data
 * @read_ecc:	Pointer to the ECC value read from spare data area
 * @calc_ecc:	Pointer to the calculated ECC value
 *
 * This function corrects the ECC single bit errors & detects 2-bit errors.
 *
 * returns:	0 if no ECC errors found
 *		1 if single bit error found and corrected.
 *		-1 if multiple ECC errors found.
 */
static int zynq_nand_correct_data(struct mtd_info *mtd, unsigned char *buf,
			unsigned char *read_ecc, unsigned char *calc_ecc)
{
	unsigned char bit_addr;
	unsigned int byte_addr;
	unsigned short ecc_odd, ecc_even;
	unsigned short read_ecc_lower, read_ecc_upper;
	unsigned short calc_ecc_lower, calc_ecc_upper;

	read_ecc_lower = (read_ecc[0] | (read_ecc[1] << 8)) & 0xfff;
	read_ecc_upper = ((read_ecc[1] >> 4) | (read_ecc[2] << 4)) & 0xfff;

	calc_ecc_lower = (calc_ecc[0] | (calc_ecc[1] << 8)) & 0xfff;
	calc_ecc_upper = ((calc_ecc[1] >> 4) | (calc_ecc[2] << 4)) & 0xfff;

	ecc_odd = read_ecc_lower ^ calc_ecc_lower;
	ecc_even = read_ecc_upper ^ calc_ecc_upper;

	if ((ecc_odd == 0) && (ecc_even == 0))
		return 0;       /* no error */

	if (ecc_odd == (~ecc_even & 0xfff)) {
		/* bits [11:3] of error code is byte offset */
		byte_addr = (ecc_odd >> 3) & 0x1ff;
		/* bits [2:0] of error code is bit offset */
		bit_addr = ecc_odd & 0x7;
		/* Toggling error bit */
		buf[byte_addr] ^= (1 << bit_addr);
		return 1;
	}

	if (onehot(ecc_odd | ecc_even))
		return 1; /* one error in parity */

	return -1; /* Uncorrectable error */
}

/*
 * zynq_nand_read_oob - [REPLACABLE] the most common OOB data read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to read
 * @sndcmd:	flag whether to issue read command or not
 */
static int zynq_nand_read_oob(struct mtd_info *mtd, struct nand_chip *chip,
			int page)
{
	unsigned long data_phase_addr = 0;
	int data_width = 4;
	u8 *p;

	chip->cmdfunc(mtd, NAND_CMD_READOOB, 0, page);

	p = chip->oob_poi;
	chip->read_buf(mtd, p, (mtd->oobsize - data_width));
	p += mtd->oobsize - data_width;

	data_phase_addr = (unsigned long)chip->IO_ADDR_R;
	data_phase_addr |= ZYNQ_NAND_CLEAR_CS;
	chip->IO_ADDR_R = (void __iomem *)data_phase_addr;
	chip->read_buf(mtd, p, data_width);

	return 0;
}

/*
 * zynq_nand_write_oob - [REPLACABLE] the most common OOB data write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @page:	page number to write
 */
static int zynq_nand_write_oob(struct mtd_info *mtd, struct nand_chip *chip,
			     int page)
{
	int status = 0, data_width = 4;
	const u8 *buf = chip->oob_poi;
	unsigned long data_phase_addr = 0;

	chip->cmdfunc(mtd, NAND_CMD_SEQIN, mtd->writesize, page);

	chip->write_buf(mtd, buf, (mtd->oobsize - data_width));
	buf += mtd->oobsize - data_width;

	data_phase_addr = (unsigned long)chip->IO_ADDR_W;
	data_phase_addr |= ZYNQ_NAND_CLEAR_CS;
	data_phase_addr |= (1 << END_CMD_VALID_SHIFT);
	chip->IO_ADDR_W = (void __iomem *)data_phase_addr;
	chip->write_buf(mtd, buf, data_width);

	/* Send command to program the OOB data */
	chip->cmdfunc(mtd, NAND_CMD_PAGEPROG, -1, -1);
	status = chip->waitfunc(mtd, chip);

	return status & NAND_STATUS_FAIL ? -EIO : 0;
}

/*
 * zynq_nand_read_page_raw - [Intern] read raw page data without ecc
 * @mtd:        mtd info structure
 * @chip:       nand chip info structure
 * @buf:        buffer to store read data
 * @oob_required: must write chip->oob_poi to OOB
 * @page:       page number to read
 */
static int zynq_nand_read_page_raw(struct mtd_info *mtd, struct nand_chip *chip,
				   u8 *buf,  int oob_required, int page)
{
	unsigned long data_width = 4;
	unsigned long data_phase_addr = 0;
	u8 *p;

	chip->read_buf(mtd, buf, mtd->writesize);

	p = chip->oob_poi;
	chip->read_buf(mtd, p, (mtd->oobsize - data_width));
	p += (mtd->oobsize - data_width);

	data_phase_addr = (unsigned long)chip->IO_ADDR_R;
	data_phase_addr |= ZYNQ_NAND_CLEAR_CS;
	chip->IO_ADDR_R = (void __iomem *)data_phase_addr;

	chip->read_buf(mtd, p, data_width);
	return 0;
}

static int zynq_nand_read_page_raw_nooob(struct mtd_info *mtd,
		struct nand_chip *chip, u8 *buf, int oob_required, int page)
{
	chip->read_buf(mtd, buf, mtd->writesize);
	return 0;
}

static int zynq_nand_read_subpage_raw(struct mtd_info *mtd,
				    struct nand_chip *chip, u32 data_offs,
				    u32 readlen, u8 *buf, int page)
{
	if (data_offs != 0) {
		chip->cmdfunc(mtd, NAND_CMD_RNDOUT, data_offs, -1);
		buf += data_offs;
	}
	chip->read_buf(mtd, buf, readlen);

	return 0;
}

/*
 * zynq_nand_write_page_raw - [Intern] raw page write function
 * @mtd:        mtd info structure
 * @chip:       nand chip info structure
 * @buf:        data buffer
 * @oob_required: must write chip->oob_poi to OOB
 */
static int zynq_nand_write_page_raw(struct mtd_info *mtd,
	struct nand_chip *chip, const u8 *buf, int oob_required, int page)
{
	unsigned long data_width = 4;
	unsigned long data_phase_addr = 0;
	u8 *p;

	chip->write_buf(mtd, buf, mtd->writesize);

	p = chip->oob_poi;
	chip->write_buf(mtd, p, (mtd->oobsize - data_width));
	p += (mtd->oobsize - data_width);

	data_phase_addr = (unsigned long)chip->IO_ADDR_W;
	data_phase_addr |= ZYNQ_NAND_CLEAR_CS;
	data_phase_addr |= (1 << END_CMD_VALID_SHIFT);
	chip->IO_ADDR_W = (void __iomem *)data_phase_addr;

	chip->write_buf(mtd, p, data_width);

	return 0;
}

/*
 * nand_write_page_hwecc - Hardware ECC based page write function
 * @mtd:	Pointer to the mtd info structure
 * @chip:	Pointer to the NAND chip info structure
 * @buf:	Pointer to the data buffer
 * @oob_required: must write chip->oob_poi to OOB
 *
 * This functions writes data and hardware generated ECC values in to the page.
 */
static int zynq_nand_write_page_hwecc(struct mtd_info *mtd,
	struct nand_chip *chip, const u8 *buf, int oob_required, int page)
{
	int i, eccsteps, eccsize = chip->ecc.size;
	u8 *ecc_calc = chip->buffers->ecccalc;
	const u8 *p = buf;
	u32 *eccpos = chip->ecc.layout->eccpos;
	unsigned long data_phase_addr = 0;
	unsigned long data_width = 4;
	u8 *oob_ptr;

	for (eccsteps = chip->ecc.steps; (eccsteps - 1); eccsteps--) {
		chip->write_buf(mtd, p, eccsize);
		p += eccsize;
	}
	chip->write_buf(mtd, p, (eccsize - data_width));
	p += eccsize - data_width;

	/* Set ECC Last bit to 1 */
	data_phase_addr = (unsigned long) chip->IO_ADDR_W;
	data_phase_addr |= ZYNQ_NAND_ECC_LAST;
	chip->IO_ADDR_W = (void __iomem *)data_phase_addr;
	chip->write_buf(mtd, p, data_width);

	/* Wait for ECC to be calculated and read the error values */
	p = buf;
	chip->ecc.calculate(mtd, p, &ecc_calc[0]);

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ~(ecc_calc[i]);

	/* Clear ECC last bit */
	data_phase_addr = (unsigned long)chip->IO_ADDR_W;
	data_phase_addr &= ~ZYNQ_NAND_ECC_LAST;
	chip->IO_ADDR_W = (void __iomem *)data_phase_addr;

	/* Write the spare area with ECC bytes */
	oob_ptr = chip->oob_poi;
	chip->write_buf(mtd, oob_ptr, (mtd->oobsize - data_width));

	data_phase_addr = (unsigned long)chip->IO_ADDR_W;
	data_phase_addr |= ZYNQ_NAND_CLEAR_CS;
	data_phase_addr |= (1 << END_CMD_VALID_SHIFT);
	chip->IO_ADDR_W = (void __iomem *)data_phase_addr;
	oob_ptr += (mtd->oobsize - data_width);
	chip->write_buf(mtd, oob_ptr, data_width);

	return 0;
}

/*
 * zynq_nand_write_page_swecc - [REPLACABLE] software ecc based page
 * write function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	data buffer
 * @oob_required: must write chip->oob_poi to OOB
 */
static int zynq_nand_write_page_swecc(struct mtd_info *mtd,
	struct nand_chip *chip, const u8 *buf, int oob_required, int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	u8 *ecc_calc = chip->buffers->ecccalc;
	const u8 *p = buf;
	u32 *eccpos = chip->ecc.layout->eccpos;

	/* Software ecc calculation */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		chip->oob_poi[eccpos[i]] = ecc_calc[i];

	return chip->ecc.write_page_raw(mtd, chip, buf, 1, page);
}

/*
 * nand_read_page_hwecc - Hardware ECC based page read function
 * @mtd:	Pointer to the mtd info structure
 * @chip:	Pointer to the NAND chip info structure
 * @buf:	Pointer to the buffer to store read data
 * @oob_required: must write chip->oob_poi to OOB
 * @page:	page number to read
 *
 * This functions reads data and checks the data integrity by comparing hardware
 * generated ECC values and read ECC values from spare area.
 *
 * returns:	0 always and updates ECC operation status in to MTD structure
 */
static int zynq_nand_read_page_hwecc(struct mtd_info *mtd,
	struct nand_chip *chip, u8 *buf, int oob_required, int page)
{
	int i, stat, eccsteps, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	u8 *p = buf;
	u8 *ecc_calc = chip->buffers->ecccalc;
	u8 *ecc_code = chip->buffers->ecccode;
	u32 *eccpos = chip->ecc.layout->eccpos;
	unsigned long data_phase_addr = 0;
	unsigned long data_width = 4;
	u8 *oob_ptr;

	for (eccsteps = chip->ecc.steps; (eccsteps - 1); eccsteps--) {
		chip->read_buf(mtd, p, eccsize);
		p += eccsize;
	}
	chip->read_buf(mtd, p, (eccsize - data_width));
	p += eccsize - data_width;

	/* Set ECC Last bit to 1 */
	data_phase_addr = (unsigned long)chip->IO_ADDR_R;
	data_phase_addr |= ZYNQ_NAND_ECC_LAST;
	chip->IO_ADDR_R = (void __iomem *)data_phase_addr;
	chip->read_buf(mtd, p, data_width);

	/* Read the calculated ECC value */
	p = buf;
	chip->ecc.calculate(mtd, p, &ecc_calc[0]);

	/* Clear ECC last bit */
	data_phase_addr = (unsigned long)chip->IO_ADDR_R;
	data_phase_addr &= ~ZYNQ_NAND_ECC_LAST;
	chip->IO_ADDR_R = (void __iomem *)data_phase_addr;

	/* Read the stored ECC value */
	oob_ptr = chip->oob_poi;
	chip->read_buf(mtd, oob_ptr, (mtd->oobsize - data_width));

	/* de-assert chip select */
	data_phase_addr = (unsigned long)chip->IO_ADDR_R;
	data_phase_addr |= ZYNQ_NAND_CLEAR_CS;
	chip->IO_ADDR_R = (void __iomem *)data_phase_addr;

	oob_ptr += (mtd->oobsize - data_width);
	chip->read_buf(mtd, oob_ptr, data_width);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = ~(chip->oob_poi[eccpos[i]]);

	eccsteps = chip->ecc.steps;
	p = buf;

	/* Check ECC error for all blocks and correct if it is correctable */
	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

/*
 * zynq_nand_read_page_swecc - [REPLACABLE] software ecc based page
 * read function
 * @mtd:	mtd info structure
 * @chip:	nand chip info structure
 * @buf:	buffer to store read data
 * @page:	page number to read
 */
static int zynq_nand_read_page_swecc(struct mtd_info *mtd,
	struct nand_chip *chip, u8 *buf, int oob_required,  int page)
{
	int i, eccsize = chip->ecc.size;
	int eccbytes = chip->ecc.bytes;
	int eccsteps = chip->ecc.steps;
	u8 *p = buf;
	u8 *ecc_calc = chip->buffers->ecccalc;
	u8 *ecc_code = chip->buffers->ecccode;
	u32 *eccpos = chip->ecc.layout->eccpos;

	chip->ecc.read_page_raw(mtd, chip, buf, 1, page);

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize)
		chip->ecc.calculate(mtd, p, &ecc_calc[i]);

	for (i = 0; i < chip->ecc.total; i++)
		ecc_code[i] = chip->oob_poi[eccpos[i]];

	eccsteps = chip->ecc.steps;
	p = buf;

	for (i = 0; eccsteps; eccsteps--, i += eccbytes, p += eccsize) {
		int stat;

		stat = chip->ecc.correct(mtd, p, &ecc_code[i], &ecc_calc[i]);
		if (stat < 0)
			mtd->ecc_stats.failed++;
		else
			mtd->ecc_stats.corrected += stat;
	}
	return 0;
}

/*
 * zynq_nand_select_chip - Select the flash device
 * @mtd:	Pointer to the mtd_info structure
 * @chip:	Chip number to be selected
 *
 * This function is empty as the NAND controller handles chip select line
 * internally based on the chip address passed in command and data phase.
 */
static void zynq_nand_select_chip(struct mtd_info *mtd, int chip)
{
	/* Not support multiple chips yet */
}

/*
 * zynq_nand_cmd_function - Send command to NAND device
 * @mtd:	Pointer to the mtd_info structure
 * @command:	The command to be sent to the flash device
 * @column:	The column address for this command, -1 if none
 * @page_addr:	The page address for this command, -1 if none
 */
static void zynq_nand_cmd_function(struct mtd_info *mtd, unsigned int command,
				 int column, int page_addr)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	struct nand_drv *smc = nand_get_controller_data(chip);
	const struct zynq_nand_command_format *curr_cmd = NULL;
	u8 addr_cycles = 0;
	struct nand_config *xnand = &smc->config;
	void *cmd_addr;
	unsigned long cmd_data = 0;
	unsigned long cmd_phase_addr = 0;
	unsigned long data_phase_addr = 0;
	u8 end_cmd = 0;
	u8 end_cmd_valid = 0;
	u32 index;

	if (xnand->end_cmd_pending) {
		/* Check for end command if this command request is same as the
		 * pending command then return
		 */
		if (xnand->end_cmd == command) {
			xnand->end_cmd = 0;
			xnand->end_cmd_pending = 0;
			return;
		}
	}

	/* Emulate NAND_CMD_READOOB for large page device */
	if ((mtd->writesize > ZYNQ_NAND_ECC_SIZE) &&
	    (command == NAND_CMD_READOOB)) {
		column += mtd->writesize;
		command = NAND_CMD_READ0;
	}

	/* Get the command format */
	for (index = 0; index < ARRAY_SIZE(zynq_nand_commands); index++)
		if (command == zynq_nand_commands[index].start_cmd)
			break;

	if (index == ARRAY_SIZE(zynq_nand_commands)) {
		printf("%s: Unsupported start cmd %02x\n", __func__, command);
		return;
	}
	curr_cmd = &zynq_nand_commands[index];

	/* Clear interrupt */
	writel(ZYNQ_MEMC_CLRCR_INT_CLR1, &smc->reg->cfr);

	/* Get the command phase address */
	if (curr_cmd->end_cmd_valid == ZYNQ_NAND_CMD_PHASE)
		end_cmd_valid = 1;

	if (curr_cmd->end_cmd == (u8)NAND_CMD_NONE)
		end_cmd = 0x0;
	else
		end_cmd = curr_cmd->end_cmd;

	if (command == NAND_CMD_READ0 ||
	    command == NAND_CMD_SEQIN) {
		addr_cycles = chip->onfi_params.addr_cycles &
				ZYNQ_NAND_ROW_ADDR_CYCL_MASK;
		addr_cycles += ((chip->onfi_params.addr_cycles &
				ZYNQ_NAND_COL_ADDR_CYCL_MASK) >> 4);
	} else {
		addr_cycles = curr_cmd->addr_cycles;
	}

	cmd_phase_addr = (unsigned long)xnand->nand_base	|
			(addr_cycles << ADDR_CYCLES_SHIFT)	|
			(end_cmd_valid << END_CMD_VALID_SHIFT)		|
			(COMMAND_PHASE)					|
			(end_cmd << END_CMD_SHIFT)			|
			(curr_cmd->start_cmd << START_CMD_SHIFT);

	cmd_addr = (void __iomem *)cmd_phase_addr;

	/* Get the data phase address */
	end_cmd_valid = 0;

	data_phase_addr = (unsigned long)xnand->nand_base	|
			(0x0 << CLEAR_CS_SHIFT)				|
			(end_cmd_valid << END_CMD_VALID_SHIFT)		|
			(DATA_PHASE)					|
			(end_cmd << END_CMD_SHIFT)			|
			(0x0 << ECC_LAST_SHIFT);

	chip->IO_ADDR_R = (void  __iomem *)data_phase_addr;
	chip->IO_ADDR_W = chip->IO_ADDR_R;

	/* Command phase AXI Read & Write */
	if (column != -1 && page_addr != -1) {
		/* Adjust columns for 16 bit bus width */
		if (chip->options & NAND_BUSWIDTH_16)
			column >>= 1;
		cmd_data = column;
		if (mtd->writesize > ZYNQ_NAND_ECC_SIZE) {
			cmd_data |= page_addr << 16;
			/* Another address cycle for devices > 128MiB */
			if (chip->chipsize > (128 << 20)) {
				writel(cmd_data, cmd_addr);
				cmd_data = (page_addr >> 16);
			}
		} else {
			cmd_data |= page_addr << 8;
		}
	} else if (page_addr != -1)  { /* Erase */
		cmd_data = page_addr;
	} else if (column != -1) { /* Change read/write column, read id etc */
		/* Adjust columns for 16 bit bus width */
		if ((chip->options & NAND_BUSWIDTH_16) &&
		    ((command == NAND_CMD_READ0) ||
		     (command == NAND_CMD_SEQIN) ||
		     (command == NAND_CMD_RNDOUT) ||
		     (command == NAND_CMD_RNDIN)))
			column >>= 1;
		cmd_data = column;
	}

	writel(cmd_data, cmd_addr);

	if (curr_cmd->end_cmd_valid) {
		xnand->end_cmd = curr_cmd->end_cmd;
		xnand->end_cmd_pending = 1;
	}

	ndelay(100);

	if ((command == NAND_CMD_READ0) ||
	    (command == NAND_CMD_RESET) ||
	    (command == NAND_CMD_PARAM) ||
	    (command == NAND_CMD_GET_FEATURES))
		/* wait until command is processed */
		nand_wait_ready(mtd);
}

/*
 * zynq_nand_read_buf - read chip data into buffer
 * @mtd:        MTD device structure
 * @buf:        buffer to store date
 * @len:        number of bytes to read
 */
static void zynq_nand_read_buf(struct mtd_info *mtd, u8 *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);

	/* Make sure that buf is 32 bit aligned */
	if (((unsigned long)buf & 0x3) != 0) {
		if (((unsigned long)buf & 0x1) != 0) {
			if (len) {
				*buf = readb(chip->IO_ADDR_R);
				buf += 1;
				len--;
			}
		}

		if (((unsigned long)buf & 0x3) != 0) {
			if (len >= 2) {
				*(u16 *)buf = readw(chip->IO_ADDR_R);
				buf += 2;
				len -= 2;
			}
		}
	}

	/* copy aligned data */
	while (len >= 4) {
		*(u32 *)buf = readl(chip->IO_ADDR_R);
		buf += 4;
		len -= 4;
	}

	/* mop up any remaining bytes */
	if (len) {
		if (len >= 2) {
			*(u16 *)buf = readw(chip->IO_ADDR_R);
			buf += 2;
			len -= 2;
		}
		if (len)
			*buf = readb(chip->IO_ADDR_R);
	}
}

/*
 * zynq_nand_write_buf - write buffer to chip
 * @mtd:        MTD device structure
 * @buf:        data buffer
 * @len:        number of bytes to write
 */
static void zynq_nand_write_buf(struct mtd_info *mtd, const u8 *buf, int len)
{
	struct nand_chip *chip = mtd_to_nand(mtd);
	const u32 *nand = chip->IO_ADDR_W;

	/* Make sure that buf is 32 bit aligned */
	if (((unsigned long)buf & 0x3) != 0) {
		if (((unsigned long)buf & 0x1) != 0) {
			if (len) {
				writeb(*buf, nand);
				buf += 1;
				len--;
			}
		}

		if (((unsigned long)buf & 0x3) != 0) {
			if (len >= 2) {
				writew(*(u16 *)buf, nand);
				buf += 2;
				len -= 2;
			}
		}
	}

	/* copy aligned data */
	while (len >= 4) {
		writel(*(u32 *)buf, nand);
		buf += 4;
		len -= 4;
	}

	/* mop up any remaining bytes */
	if (len) {
		if (len >= 2) {
			writew(*(u16 *)buf, nand);
			buf += 2;
			len -= 2;
		}

		if (len)
			writeb(*buf, nand);
	}
}

/*
 * zynq_nand_device_ready - Check device ready/busy line
 * @mtd:	Pointer to the mtd_info structure
 *
 * returns:	0 on busy or 1 on ready state
 */
static int zynq_nand_device_ready(struct mtd_info *mtd)
{
	struct nand_chip *nand_chip = mtd_to_nand(mtd);
	struct nand_drv *smc = nand_get_controller_data(nand_chip);
	u32 csr_val;

	csr_val = readl(&smc->reg->csr);
	/* Check the raw_int_status1 bit */
	if (csr_val & ZYNQ_MEMC_SR_RAW_INT_ST1) {
		/* Clear the interrupt condition */
		writel(ZYNQ_MEMC_SR_INT_ST1, &smc->reg->cfr);
		return 1;
	}

	return 0;
}

static int zynq_nand_check_is_16bit_bw_flash(void)
{
	int is_16bit_bw = NAND_BW_UNKNOWN;
	int mio_num_8bit = 0, mio_num_16bit = 0;

	mio_num_8bit = zynq_slcr_get_mio_pin_status("nand8");
	if (mio_num_8bit == ZYNQ_NAND_MIO_NUM_NAND_8BIT)
		is_16bit_bw = NAND_BW_8BIT;

	mio_num_16bit = zynq_slcr_get_mio_pin_status("nand16");
	if (mio_num_8bit == ZYNQ_NAND_MIO_NUM_NAND_8BIT &&
	    mio_num_16bit == ZYNQ_NAND_MIO_NUM_NAND_16BIT)
		is_16bit_bw = NAND_BW_16BIT;

	return is_16bit_bw;
}

static int zynq_nand_probe(struct udevice *dev)
{
	struct zynq_nand_info *zynq = dev_get_priv(dev);
	struct nand_chip *nand_chip = &zynq->nand_chip;
	struct nand_drv *smc = &zynq->nand_ctrl;
	struct nand_config *xnand = &smc->config;
	struct mtd_info *mtd;
	struct resource res;
	ofnode of_nand;
	unsigned long ecc_page_size;
	u8 maf_id, dev_id, i;
	u8 get_feature[4];
	u8 set_feature[4] = {ONDIE_ECC_FEATURE_ENABLE, 0x00, 0x00, 0x00};
	unsigned long ecc_cfg;
	int ondie_ecc_enabled = 0;
	int is_16bit_bw;

	smc->reg = dev_read_addr_ptr(dev);
	of_nand = dev_read_subnode(dev, "nand-controller@0,0");
	if (!ofnode_valid(of_nand)) {
		of_nand = dev_read_subnode(dev, "flash@e1000000");
		if (!ofnode_valid(of_nand)) {
			printf("Failed to find nand node in dt\n");
			return -ENODEV;
		}
	}

	if (!ofnode_is_enabled(of_nand)) {
		debug("Nand node in dt disabled\n");
		return dm_scan_fdt_dev(dev);
	}

	if (ofnode_read_resource(of_nand, 0, &res)) {
		printf("Failed to get nand resource\n");
		return -ENODEV;
	}

	xnand->nand_base = (void __iomem *)res.start;
	mtd = nand_to_mtd(nand_chip);
	nand_set_controller_data(nand_chip, &zynq->nand_ctrl);

	/* Set address of NAND IO lines */
	nand_chip->IO_ADDR_R = xnand->nand_base;
	nand_chip->IO_ADDR_W = xnand->nand_base;

	/* Set the driver entry points for MTD */
	nand_chip->cmdfunc = zynq_nand_cmd_function;
	nand_chip->dev_ready = zynq_nand_device_ready;
	nand_chip->select_chip = zynq_nand_select_chip;

	/* If we don't set this delay driver sets 20us by default */
	nand_chip->chip_delay = 30;

	/* Buffer read/write routines */
	nand_chip->read_buf = zynq_nand_read_buf;
	nand_chip->write_buf = zynq_nand_write_buf;

	is_16bit_bw = zynq_nand_check_is_16bit_bw_flash();
	if (is_16bit_bw == NAND_BW_UNKNOWN) {
		printf("%s: Unable detect NAND based on MIO settings\n",
		       __func__);
		return -EINVAL;
	}

	if (is_16bit_bw == NAND_BW_16BIT)
		nand_chip->options = NAND_BUSWIDTH_16;

	nand_chip->bbt_options = NAND_BBT_USE_FLASH;

	/* Initialize the NAND flash interface on NAND controller */
	if (zynq_nand_init_nand_flash(mtd, nand_chip->options) < 0) {
		printf("%s: nand flash init failed\n", __func__);
		return -EINVAL;
	}

	/* first scan to find the device and get the page size */
	if (nand_scan_ident(mtd, 1, NULL)) {
		printf("%s: nand_scan_ident failed\n", __func__);
		return -EINVAL;
	}
	/* Send the command for reading device ID */
	nand_chip->cmdfunc(mtd, NAND_CMD_RESET, -1, -1);
	nand_chip->cmdfunc(mtd, NAND_CMD_READID, 0x00, -1);

	/* Read manufacturer and device IDs */
	maf_id = nand_chip->read_byte(mtd);
	dev_id = nand_chip->read_byte(mtd);

	if ((maf_id == 0x2c) && ((dev_id == 0xf1) ||
				 (dev_id == 0xa1) || (dev_id == 0xb1) ||
				 (dev_id == 0xaa) || (dev_id == 0xba) ||
				 (dev_id == 0xda) || (dev_id == 0xca) ||
				 (dev_id == 0xac) || (dev_id == 0xbc) ||
				 (dev_id == 0xdc) || (dev_id == 0xcc) ||
				 (dev_id == 0xa3) || (dev_id == 0xb3) ||
				 (dev_id == 0xd3) || (dev_id == 0xc3))) {
		nand_chip->cmdfunc(mtd, NAND_CMD_SET_FEATURES,
						ONDIE_ECC_FEATURE_ADDR, -1);
		for (i = 0; i < 4; i++)
			writeb(set_feature[i], nand_chip->IO_ADDR_W);

		/* Wait for 1us after writing data with SET_FEATURES command */
		ndelay(1000);

		nand_chip->cmdfunc(mtd, NAND_CMD_GET_FEATURES,
						ONDIE_ECC_FEATURE_ADDR, -1);
		nand_chip->read_buf(mtd, get_feature, 4);

		if (get_feature[0] & ONDIE_ECC_FEATURE_ENABLE) {
			debug("%s: OnDie ECC flash\n", __func__);
			ondie_ecc_enabled = 1;
		} else {
			printf("%s: Unable to detect OnDie ECC\n", __func__);
		}
	}

	if (ondie_ecc_enabled) {
		/* Bypass the controller ECC block */
		ecc_cfg = readl(&smc->reg->emcr);
		ecc_cfg &= ~ZYNQ_MEMC_NAND_ECC_MODE_MASK;
		writel(ecc_cfg, &smc->reg->emcr);

		/* The software ECC routines won't work
		 * with the SMC controller
		 */
		nand_chip->ecc.mode = NAND_ECC_HW;
		nand_chip->ecc.strength = 1;
		nand_chip->ecc.read_page = zynq_nand_read_page_raw_nooob;
		nand_chip->ecc.read_subpage = zynq_nand_read_subpage_raw;
		nand_chip->ecc.write_page = zynq_nand_write_page_raw;
		nand_chip->ecc.read_page_raw = zynq_nand_read_page_raw;
		nand_chip->ecc.write_page_raw = zynq_nand_write_page_raw;
		nand_chip->ecc.read_oob = zynq_nand_read_oob;
		nand_chip->ecc.write_oob = zynq_nand_write_oob;
		nand_chip->ecc.size = mtd->writesize;
		nand_chip->ecc.bytes = 0;

		/* NAND with on-die ECC supports subpage reads */
		nand_chip->options |= NAND_SUBPAGE_READ;

		/* On-Die ECC spare bytes offset 8 is used for ECC codes */
		nand_chip->ecc.layout = &ondie_nand_oob_64;
		/* Use the BBT pattern descriptors */
		nand_chip->bbt_td = &bbt_main_descr;
		nand_chip->bbt_md = &bbt_mirror_descr;
	} else {
		/* Hardware ECC generates 3 bytes ECC code for each 512 bytes */
		nand_chip->ecc.mode = NAND_ECC_HW;
		nand_chip->ecc.strength = 1;
		nand_chip->ecc.size = ZYNQ_NAND_ECC_SIZE;
		nand_chip->ecc.bytes = 3;
		nand_chip->ecc.calculate = zynq_nand_calculate_hwecc;
		nand_chip->ecc.correct = zynq_nand_correct_data;
		nand_chip->ecc.hwctl = NULL;
		nand_chip->ecc.read_page = zynq_nand_read_page_hwecc;
		nand_chip->ecc.write_page = zynq_nand_write_page_hwecc;
		nand_chip->ecc.read_page_raw = zynq_nand_read_page_raw;
		nand_chip->ecc.write_page_raw = zynq_nand_write_page_raw;
		nand_chip->ecc.read_oob = zynq_nand_read_oob;
		nand_chip->ecc.write_oob = zynq_nand_write_oob;

		switch (mtd->writesize) {
		case 512:
			ecc_page_size = 0x1;
			/* Set the ECC memory config register */
			writel((ZYNQ_NAND_ECC_CONFIG | ecc_page_size),
			       &smc->reg->emcr);
			break;
		case 1024:
			ecc_page_size = 0x2;
			/* Set the ECC memory config register */
			writel((ZYNQ_NAND_ECC_CONFIG | ecc_page_size),
			       &smc->reg->emcr);
			break;
		case 2048:
			ecc_page_size = 0x3;
			/* Set the ECC memory config register */
			writel((ZYNQ_NAND_ECC_CONFIG | ecc_page_size),
			       &smc->reg->emcr);
			break;
		default:
			nand_chip->ecc.mode = NAND_ECC_SOFT;
			nand_chip->ecc.calculate = nand_calculate_ecc;
			nand_chip->ecc.correct = nand_correct_data;
			nand_chip->ecc.read_page = zynq_nand_read_page_swecc;
			nand_chip->ecc.write_page = zynq_nand_write_page_swecc;
			nand_chip->ecc.size = 256;
			break;
		}

		if (mtd->oobsize == 16)
			nand_chip->ecc.layout = &nand_oob_16;
		else if (mtd->oobsize == 64)
			nand_chip->ecc.layout = &nand_oob_64;
		else
			printf("%s: No oob layout found\n", __func__);
	}

	/* Second phase scan */
	if (nand_scan_tail(mtd)) {
		printf("%s: nand_scan_tail failed\n", __func__);
		return -EINVAL;
	}
	if (nand_register(0, mtd))
		return -EINVAL;

	return 0;
}

static const struct udevice_id zynq_nand_dt_ids[] = {
	{.compatible = "arm,pl353-smc-r2p1",},
	{ /* sentinel */ }
};

U_BOOT_DRIVER(zynq_nand) = {
	.name = "zynq_nand",
	.id = UCLASS_MTD,
	.of_match = zynq_nand_dt_ids,
	.probe = zynq_nand_probe,
	.priv_auto	= sizeof(struct zynq_nand_info),
};

void board_nand_init(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_get_device_by_driver(UCLASS_MTD,
					  DM_DRIVER_GET(zynq_nand), &dev);
	if (ret && ret != -ENODEV)
		pr_err("Failed to initialize %s. (error %d)\n", dev->name, ret);
}
