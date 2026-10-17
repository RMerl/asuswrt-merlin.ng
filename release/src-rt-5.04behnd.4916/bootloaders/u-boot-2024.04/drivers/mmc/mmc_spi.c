/*
 * generic mmc spi driver
 *
 * Copyright (C) 2010 Thomas Chou <thomas@wytron.com.tw>
 * Copyright 2019 Bhargav Shah <bhargavshah1988@gmail.com>
 *
 * Licensed under the GPL-2 or later.
 */
#include <common.h>
#include <errno.h>
#include <log.h>
#include <malloc.h>
#include <part.h>
#include <mmc.h>
#include <stdlib.h>
#include <linux/bitops.h>
#include <u-boot/crc.h>
#include <linux/crc7.h>
#include <asm/byteorder.h>
#include <dm.h>
#include <spi.h>

/* MMC/SD in SPI mode reports R1 status always */
#define R1_SPI_IDLE			BIT(0)
#define R1_SPI_ERASE_RESET		BIT(1)
#define R1_SPI_ILLEGAL_COMMAND		BIT(2)
#define R1_SPI_COM_CRC			BIT(3)
#define R1_SPI_ERASE_SEQ		BIT(4)
#define R1_SPI_ADDRESS			BIT(5)
#define R1_SPI_PARAMETER		BIT(6)
/* R1 bit 7 is always zero, reuse this bit for error */
#define R1_SPI_ERROR			BIT(7)

/* Response tokens used to ack each block written: */
#define SPI_MMC_RESPONSE_CODE(x)	((x) & 0x1f)
#define SPI_RESPONSE_ACCEPTED		((2 << 1)|1)
#define SPI_RESPONSE_CRC_ERR		((5 << 1)|1)
#define SPI_RESPONSE_WRITE_ERR		((6 << 1)|1)

/*
 * Read and write blocks start with these tokens and end with crc;
 * on error, read tokens act like a subset of R2_SPI_* values.
 */
/* single block write multiblock read */
#define SPI_TOKEN_SINGLE		0xfe
/* multiblock write */
#define SPI_TOKEN_MULTI_WRITE		0xfc
/* terminate multiblock write */
#define SPI_TOKEN_STOP_TRAN		0xfd

/* MMC SPI commands start with a start bit "0" and a transmit bit "1" */
#define MMC_SPI_CMD(x) (0x40 | (x))

/* bus capability */
#define MMC_SPI_VOLTAGE			(MMC_VDD_32_33 | MMC_VDD_33_34)
#define MMC_SPI_MIN_CLOCK		400000	/* 400KHz to meet MMC spec */
#define MMC_SPI_MAX_CLOCK		25000000 /* SD/MMC legacy speed */

/* timeout value */
#define CMD_TIMEOUT			8
#define READ_TIMEOUT			3000000 /* 1 sec */
#define WRITE_TIMEOUT			3000000 /* 1 sec */
#define R1B_TIMEOUT			3000000 /* 1 sec */

struct mmc_spi_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

struct mmc_spi_priv {
	struct spi_slave *spi;
};

/**
 * mmc_spi_sendcmd() - send a command to the SD card
 *
 * @dev:	mmc_spi device
 * @cmdidx:	command index
 * @cmdarg:	command argument
 * @resp_type:	card response type
 * @resp:	buffer to store the card response
 * @resp_size:	size of the card response
 * @resp_match:	if true, compare each of received bytes with @resp_match_value
 * @resp_match_value:	a value to be compared with each of received bytes
 * @r1b:	if true, receive additional bytes for busy signal token
 * Return: 0 if OK, -ETIMEDOUT if no card response is received, -ve on error
 */
static int mmc_spi_sendcmd(struct udevice *dev,
			   ushort cmdidx, u32 cmdarg, u32 resp_type,
			   u8 *resp, u32 resp_size,
			   bool resp_match, u8 resp_match_value, bool r1b)
{
	int i, rpos = 0, ret = 0;
	u8 cmdo[7], r;

	if (!resp || !resp_size)
		return 0;

	debug("%s: cmd%d cmdarg=0x%x resp_type=0x%x "
	      "resp_size=%d resp_match=%d resp_match_value=0x%x\n",
	      __func__, cmdidx, cmdarg, resp_type,
	      resp_size, resp_match, resp_match_value);

	cmdo[0] = 0xff;
	cmdo[1] = MMC_SPI_CMD(cmdidx);
	cmdo[2] = cmdarg >> 24;
	cmdo[3] = cmdarg >> 16;
	cmdo[4] = cmdarg >> 8;
	cmdo[5] = cmdarg;
	cmdo[6] = (crc7(0, &cmdo[1], 5) << 1) | 0x01;
	ret = dm_spi_xfer(dev, sizeof(cmdo) * 8, cmdo, NULL, SPI_XFER_BEGIN);
	if (ret)
		return ret;

	ret = dm_spi_xfer(dev, 1 * 8, NULL, &r, 0);
	if (ret)
		return ret;

	debug("%s: cmd%d", __func__, cmdidx);

	if (resp_match)
		r = ~resp_match_value;
	i = CMD_TIMEOUT;
	while (i) {
		ret = dm_spi_xfer(dev, 1 * 8, NULL, &r, 0);
		if (ret)
			return ret;
		debug(" resp%d=0x%x", rpos, r);
		rpos++;
		i--;

		if (resp_match) {
			if (r == resp_match_value)
				break;
		} else {
			if (!(r & 0x80))
				break;
		}

		if (!i)
			return -ETIMEDOUT;
	}

	resp[0] = r;
	for (i = 1; i < resp_size; i++) {
		ret = dm_spi_xfer(dev, 1 * 8, NULL, &r, 0);
		if (ret)
			return ret;
		debug(" resp%d=0x%x", rpos, r);
		rpos++;
		resp[i] = r;
	}

	if (r1b == true) {
		i = R1B_TIMEOUT;
		while (i) {
			ret = dm_spi_xfer(dev, 1 * 8, NULL, &r, 0);
			if (ret)
				return ret;

			debug(" resp%d=0x%x", rpos, r);
			rpos++;
			i--;

			if (r)
				break;
		}
		if (!i)
			return -ETIMEDOUT;
	}

	debug("\n");

	return 0;
}

/**
 * mmc_spi_readdata() - read data block(s) from the SD card
 *
 * @dev:	mmc_spi device
 * @xbuf:	buffer of the actual data (excluding token and crc) to read
 * @bcnt:	number of data blocks to transfer
 * @bsize:	size of the actual data (excluding token and crc) in bytes
 * Return: 0 if OK, -ECOMM if crc error, -ETIMEDOUT on other errors
 */
static int mmc_spi_readdata(struct udevice *dev,
			    void *xbuf, u32 bcnt, u32 bsize)
{
	u16 crc;
	u8 *buf = xbuf, r1;
	int i, ret = 0;

	while (bcnt--) {
		for (i = 0; i < READ_TIMEOUT; i++) {
			ret = dm_spi_xfer(dev, 1 * 8, NULL, &r1, 0);
			if (ret)
				return ret;
			if (r1 == SPI_TOKEN_SINGLE)
				break;
		}
		debug("%s: data tok%d 0x%x\n", __func__, i, r1);
		if (r1 == SPI_TOKEN_SINGLE) {
			ret = dm_spi_xfer(dev, bsize * 8, NULL, buf, 0);
			if (ret)
				return ret;
			ret = dm_spi_xfer(dev, 2 * 8, NULL, &crc, 0);
			if (ret)
				return ret;
#ifdef CONFIG_MMC_SPI_CRC_ON
			u16 crc_ok = be16_to_cpu(crc16_ccitt(0, buf, bsize));
			if (crc_ok != crc) {
				debug("%s: data crc error, expected %04x got %04x\n",
				      __func__, crc_ok, crc);
				r1 = R1_SPI_COM_CRC;
				break;
			}
#endif
			r1 = 0;
		} else {
			r1 = R1_SPI_ERROR;
			break;
		}
		buf += bsize;
	}

	if (r1 & R1_SPI_COM_CRC)
		ret = -ECOMM;
	else if (r1) /* other errors */
		ret = -ETIMEDOUT;

	return ret;
}

/**
 * mmc_spi_writedata() - write data block(s) to the SD card
 *
 * @dev:	mmc_spi device
 * @xbuf:	buffer of the actual data (excluding token and crc) to write
 * @bcnt:	number of data blocks to transfer
 * @bsize:	size of actual data (excluding token and crc) in bytes
 * @multi:	indicate a transfer by multiple block write command (CMD25)
 * Return: 0 if OK, -ECOMM if crc error, -ETIMEDOUT on other errors
 */
static int mmc_spi_writedata(struct udevice *dev, const void *xbuf,
			     u32 bcnt, u32 bsize, int multi)
{
	const u8 *buf = xbuf;
	u8 r1, tok[2];
	u16 crc;
	int i, ret = 0;

	tok[0] = 0xff;
	tok[1] = multi ? SPI_TOKEN_MULTI_WRITE : SPI_TOKEN_SINGLE;

	while (bcnt--) {
#ifdef CONFIG_MMC_SPI_CRC_ON
		crc = cpu_to_be16(crc16_ccitt(0, (u8 *)buf, bsize));
#endif
		dm_spi_xfer(dev, 2 * 8, tok, NULL, 0);
		dm_spi_xfer(dev, bsize * 8, buf, NULL, 0);
		dm_spi_xfer(dev, 2 * 8, &crc, NULL, 0);
		for (i = 0; i < CMD_TIMEOUT; i++) {
			dm_spi_xfer(dev, 1 * 8, NULL, &r1, 0);
			if ((r1 & 0x10) == 0) /* response token */
				break;
		}
		debug("%s: data tok%d 0x%x\n", __func__, i, r1);
		if (SPI_MMC_RESPONSE_CODE(r1) == SPI_RESPONSE_ACCEPTED) {
			debug("%s: data accepted\n", __func__);
			for (i = 0; i < WRITE_TIMEOUT; i++) { /* wait busy */
				dm_spi_xfer(dev, 1 * 8, NULL, &r1, 0);
				if (i && r1 == 0xff) {
					r1 = 0;
					break;
				}
			}
			if (i == WRITE_TIMEOUT) {
				debug("%s: data write timeout 0x%x\n",
				      __func__, r1);
				r1 = R1_SPI_ERROR;
				break;
			}
		} else {
			debug("%s: data error 0x%x\n", __func__, r1);
			r1 = R1_SPI_COM_CRC;
			break;
		}
		buf += bsize;
	}
	if (multi && bcnt == -1) { /* stop multi write */
		tok[1] = SPI_TOKEN_STOP_TRAN;
		dm_spi_xfer(dev, 2 * 8, tok, NULL, 0);
		for (i = 0; i < WRITE_TIMEOUT; i++) { /* wait busy */
			dm_spi_xfer(dev, 1 * 8, NULL, &r1, 0);
			if (i && r1 == 0xff) {
				r1 = 0;
				break;
			}
		}
		if (i == WRITE_TIMEOUT) {
			debug("%s: data write timeout 0x%x\n", __func__, r1);
			r1 = R1_SPI_ERROR;
		}
	}

	if (r1 & R1_SPI_COM_CRC)
		ret = -ECOMM;
	else if (r1) /* other errors */
		ret = -ETIMEDOUT;

	return ret;
}

static int dm_mmc_spi_set_ios(struct udevice *dev)
{
	return 0;
}

static int dm_mmc_spi_request(struct udevice *dev, struct mmc_cmd *cmd,
			      struct mmc_data *data)
{
	int i, multi, ret = 0;
	u8 *resp = NULL;
	u32 resp_size = 0;
	bool resp_match = false, r1b = false;
	u8 resp8 = 0, resp16[2] = { 0 }, resp40[5] = { 0 }, resp_match_value = 0;

	dm_spi_claim_bus(dev);

	for (i = 0; i < 4; i++)
		cmd->response[i] = 0;

	switch (cmd->cmdidx) {
	case SD_CMD_APP_SEND_OP_COND:
	case MMC_CMD_SEND_OP_COND:
		resp = &resp8;
		resp_size = sizeof(resp8);
		cmd->cmdarg = 0x40000000;
		break;
	case SD_CMD_SEND_IF_COND:
		resp = (u8 *)&resp40[0];
		resp_size = sizeof(resp40);
		resp_match = true;
		resp_match_value = R1_SPI_IDLE;
		break;
	case MMC_CMD_SPI_READ_OCR:
		resp = (u8 *)&resp40[0];
		resp_size = sizeof(resp40);
		break;
	case MMC_CMD_SEND_STATUS:
		resp = (u8 *)&resp16[0];
		resp_size = sizeof(resp16);
		break;
	case MMC_CMD_SET_BLOCKLEN:
	case MMC_CMD_SPI_CRC_ON_OFF:
		resp = &resp8;
		resp_size = sizeof(resp8);
		resp_match = true;
		resp_match_value = 0x0;
		break;
	case MMC_CMD_STOP_TRANSMISSION:
	case MMC_CMD_ERASE:
		resp = &resp8;
		resp_size = sizeof(resp8);
		r1b = true;
		break;
	case MMC_CMD_SEND_CSD:
	case MMC_CMD_SEND_CID:
	case MMC_CMD_READ_SINGLE_BLOCK:
	case MMC_CMD_READ_MULTIPLE_BLOCK:
	case MMC_CMD_WRITE_SINGLE_BLOCK:
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
	case MMC_CMD_APP_CMD:
	case SD_CMD_ERASE_WR_BLK_START:
	case SD_CMD_ERASE_WR_BLK_END:
		resp = &resp8;
		resp_size = sizeof(resp8);
		break;
	default:
		resp = &resp8;
		resp_size = sizeof(resp8);
		resp_match = true;
		resp_match_value = R1_SPI_IDLE;
		break;
	};

	ret = mmc_spi_sendcmd(dev, cmd->cmdidx, cmd->cmdarg, cmd->resp_type,
			      resp, resp_size, resp_match, resp_match_value, r1b);
	if (ret)
		goto done;

	switch (cmd->cmdidx) {
	case SD_CMD_APP_SEND_OP_COND:
	case MMC_CMD_SEND_OP_COND:
		cmd->response[0] = (resp8 & R1_SPI_IDLE) ? 0 : OCR_BUSY;
		break;
	case SD_CMD_SEND_IF_COND:
	case MMC_CMD_SPI_READ_OCR:
		cmd->response[0] = resp40[4];
		cmd->response[0] |= (uint)resp40[3] << 8;
		cmd->response[0] |= (uint)resp40[2] << 16;
		cmd->response[0] |= (uint)resp40[1] << 24;
		break;
	case MMC_CMD_SEND_STATUS:
		if (resp16[0] || resp16[1])
			cmd->response[0] = MMC_STATUS_ERROR;
		else
			cmd->response[0] = MMC_STATUS_RDY_FOR_DATA;
		break;
	case MMC_CMD_SEND_CID:
	case MMC_CMD_SEND_CSD:
		ret = mmc_spi_readdata(dev, cmd->response, 1, 16);
		if (ret)
			return ret;
		for (i = 0; i < 4; i++)
			cmd->response[i] =
				cpu_to_be32(cmd->response[i]);
		break;
	default:
		cmd->response[0] = resp8;
		break;
	}

	debug("%s: cmd%d resp0=0x%x resp1=0x%x resp2=0x%x resp3=0x%x\n",
	      __func__, cmd->cmdidx, cmd->response[0], cmd->response[1],
	      cmd->response[2], cmd->response[3]);

	if (data) {
		debug("%s: data flags=0x%x blocks=%d block_size=%d\n",
		      __func__, data->flags, data->blocks, data->blocksize);
		multi = (cmd->cmdidx == MMC_CMD_WRITE_MULTIPLE_BLOCK);
		if (data->flags == MMC_DATA_READ)
			ret = mmc_spi_readdata(dev, data->dest,
					       data->blocks, data->blocksize);
		else if  (data->flags == MMC_DATA_WRITE)
			ret = mmc_spi_writedata(dev, data->src,
						data->blocks, data->blocksize,
						multi);
	}

done:
	dm_spi_xfer(dev, 0, NULL, NULL, SPI_XFER_END);

	dm_spi_release_bus(dev);

	return ret;
}

static int mmc_spi_probe(struct udevice *dev)
{
	struct mmc_spi_priv *priv = dev_get_priv(dev);
	struct mmc_spi_plat *plat = dev_get_plat(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	char *name;

	priv->spi = dev_get_parent_priv(dev);
	if (!priv->spi->max_hz)
		priv->spi->max_hz = MMC_SPI_MAX_CLOCK;
	priv->spi->mode = SPI_MODE_0;
	priv->spi->wordlen = 8;

	name = malloc(strlen(dev->parent->name) + strlen(dev->name) + 4);
	if (!name)
		return -ENOMEM;
	sprintf(name, "%s:%s", dev->parent->name, dev->name);

	plat->cfg.name = name;
	plat->cfg.host_caps = MMC_MODE_SPI;
	plat->cfg.voltages = MMC_SPI_VOLTAGE;
	plat->cfg.f_min = MMC_SPI_MIN_CLOCK;
	plat->cfg.f_max = priv->spi->max_hz;
	plat->cfg.part_type = PART_TYPE_DOS;
	plat->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	plat->mmc.cfg = &plat->cfg;
	plat->mmc.priv = priv;
	plat->mmc.dev = dev;

	upriv->mmc = &plat->mmc;

	return 0;
}

static int mmc_spi_bind(struct udevice *dev)
{
	struct mmc_spi_plat *plat = dev_get_plat(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static const struct dm_mmc_ops mmc_spi_ops = {
	.send_cmd	= dm_mmc_spi_request,
	.set_ios	= dm_mmc_spi_set_ios,
};

static const struct udevice_id dm_mmc_spi_match[] = {
	{ .compatible = "mmc-spi-slot" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(mmc_spi) = {
	.name = "mmc_spi",
	.id = UCLASS_MMC,
	.of_match = dm_mmc_spi_match,
	.ops = &mmc_spi_ops,
	.probe = mmc_spi_probe,
	.bind = mmc_spi_bind,
	.plat_auto	= sizeof(struct mmc_spi_plat),
	.priv_auto	= sizeof(struct mmc_spi_priv),
};
