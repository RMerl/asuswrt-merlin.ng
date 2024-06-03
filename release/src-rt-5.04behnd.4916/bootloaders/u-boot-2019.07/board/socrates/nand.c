// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2008
 * Sergei Poselenov, Emcraft Systems, sposelenov@emcraft.com.
 */

#include <common.h>

#if defined(CONFIG_SYS_NAND_BASE)
#include <nand.h>
#include <linux/errno.h>
#include <asm/io.h>

static int state;
static void sc_nand_write_byte(struct mtd_info *mtd, u_char byte);
static void sc_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len);
static u_char sc_nand_read_byte(struct mtd_info *mtd);
static u16 sc_nand_read_word(struct mtd_info *mtd);
static void sc_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len);
static int sc_nand_device_ready(struct mtd_info *mtdinfo);

#define FPGA_NAND_CMD_MASK		(0x7 << 28)
#define FPGA_NAND_CMD_COMMAND		(0x0 << 28)
#define FPGA_NAND_CMD_ADDR		(0x1 << 28)
#define FPGA_NAND_CMD_READ		(0x2 << 28)
#define FPGA_NAND_CMD_WRITE		(0x3 << 28)
#define FPGA_NAND_BUSY			(0x1 << 15)
#define FPGA_NAND_ENABLE		(0x1 << 31)
#define FPGA_NAND_DATA_SHIFT		16

/**
 * sc_nand_write_byte -  write one byte to the chip
 * @mtd:	MTD device structure
 * @byte:	pointer to data byte to write
 */
static void sc_nand_write_byte(struct mtd_info *mtd, u_char byte)
{
	sc_nand_write_buf(mtd, (const uchar *)&byte, sizeof(byte));
}

/**
 * sc_nand_write_buf -  write buffer to chip
 * @mtd:	MTD device structure
 * @buf:	data buffer
 * @len:	number of bytes to write
 */
static void sc_nand_write_buf(struct mtd_info *mtd, const u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd_to_nand(mtd);

	for (i = 0; i < len; i++) {
		out_be32(this->IO_ADDR_W,
			 state | (buf[i] << FPGA_NAND_DATA_SHIFT));
	}
}


/**
 * sc_nand_read_byte -  read one byte from the chip
 * @mtd:	MTD device structure
 */
static u_char sc_nand_read_byte(struct mtd_info *mtd)
{
	u8 byte;
	sc_nand_read_buf(mtd, (uchar *)&byte, sizeof(byte));
	return byte;
}

/**
 * sc_nand_read_word -  read one word from the chip
 * @mtd:	MTD device structure
 */
static u16 sc_nand_read_word(struct mtd_info *mtd)
{
	u16 word;
	sc_nand_read_buf(mtd, (uchar *)&word, sizeof(word));
	return word;
}

/**
 * sc_nand_read_buf -  read chip data into buffer
 * @mtd:	MTD device structure
 * @buf:	buffer to store date
 * @len:	number of bytes to read
 */
static void sc_nand_read_buf(struct mtd_info *mtd, u_char *buf, int len)
{
	int i;
	struct nand_chip *this = mtd_to_nand(mtd);
	int val;

	val = (state & FPGA_NAND_ENABLE) | FPGA_NAND_CMD_READ;

	out_be32(this->IO_ADDR_W, val);
	for (i = 0; i < len; i++) {
		buf[i] = (in_be32(this->IO_ADDR_R) >> FPGA_NAND_DATA_SHIFT) & 0xff;
	}
}

/**
 * sc_nand_device_ready - Check the NAND device is ready for next command.
 * @mtd:	MTD device structure
 */
static int sc_nand_device_ready(struct mtd_info *mtdinfo)
{
	struct nand_chip *this = mtd_to_nand(mtdinfo);

	if (in_be32(this->IO_ADDR_W) & FPGA_NAND_BUSY)
		return 0; /* busy */
	return 1;
}

/**
 * sc_nand_hwcontrol - NAND control functions wrapper.
 * @mtd:	MTD device structure
 * @cmd:	Command
 */
static void sc_nand_hwcontrol(struct mtd_info *mtdinfo, int cmd, unsigned int ctrl)
{
	if (ctrl & NAND_CTRL_CHANGE) {
		state &= ~(FPGA_NAND_CMD_MASK | FPGA_NAND_ENABLE);

		switch (ctrl & (NAND_ALE | NAND_CLE)) {
		case 0:
			state |= FPGA_NAND_CMD_WRITE;
			break;

		case NAND_ALE:
			state |= FPGA_NAND_CMD_ADDR;
			break;

		case NAND_CLE:
			state |= FPGA_NAND_CMD_COMMAND;
			break;

		default:
			printf("%s: unknown ctrl %#x\n", __FUNCTION__, ctrl);
		}

		if (ctrl & NAND_NCE)
			state |= FPGA_NAND_ENABLE;
	}

	if (cmd != NAND_CMD_NONE)
		sc_nand_write_byte(mtdinfo, cmd);
}

int board_nand_init(struct nand_chip *nand)
{
	nand->cmd_ctrl = sc_nand_hwcontrol;
	nand->ecc.mode = NAND_ECC_SOFT;
	nand->dev_ready = sc_nand_device_ready;
	nand->read_byte = sc_nand_read_byte;
	nand->read_word = sc_nand_read_word;
	nand->write_buf = sc_nand_write_buf;
	nand->read_buf = sc_nand_read_buf;

	return 0;
}

#endif
