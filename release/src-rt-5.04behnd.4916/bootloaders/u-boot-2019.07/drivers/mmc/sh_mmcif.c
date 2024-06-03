// SPDX-License-Identifier: GPL-2.0
/*
 * MMCIF driver.
 *
 * Copyright (C)  2011 Renesas Solutions Corp.
 */

#include <config.h>
#include <common.h>
#include <watchdog.h>
#include <command.h>
#include <mmc.h>
#include <clk.h>
#include <dm.h>
#include <malloc.h>
#include <linux/errno.h>
#include <linux/compat.h>
#include <linux/io.h>
#include <linux/sizes.h>
#include "sh_mmcif.h"

#define DRIVER_NAME	"sh_mmcif"

static int sh_mmcif_intr(void *dev_id)
{
	struct sh_mmcif_host *host = dev_id;
	u32 state = 0;

	state = sh_mmcif_read(&host->regs->ce_int);
	state &= sh_mmcif_read(&host->regs->ce_int_mask);

	if (state & INT_RBSYE) {
		sh_mmcif_write(~(INT_RBSYE | INT_CRSPE), &host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MRBSYE, &host->regs->ce_int_mask);
		goto end;
	} else if (state & INT_CRSPE) {
		sh_mmcif_write(~INT_CRSPE, &host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MCRSPE, &host->regs->ce_int_mask);
		/* one more interrupt (INT_RBSYE) */
		if (sh_mmcif_read(&host->regs->ce_cmd_set) & CMD_SET_RBSY)
			return -EAGAIN;
		goto end;
	} else if (state & INT_BUFREN) {
		sh_mmcif_write(~INT_BUFREN, &host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MBUFREN, &host->regs->ce_int_mask);
		goto end;
	} else if (state & INT_BUFWEN) {
		sh_mmcif_write(~INT_BUFWEN, &host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MBUFWEN, &host->regs->ce_int_mask);
		goto end;
	} else if (state & INT_CMD12DRE) {
		sh_mmcif_write(~(INT_CMD12DRE | INT_CMD12RBE | INT_CMD12CRE |
				  INT_BUFRE), &host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MCMD12DRE, &host->regs->ce_int_mask);
		goto end;
	} else if (state & INT_BUFRE) {
		sh_mmcif_write(~INT_BUFRE, &host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MBUFRE, &host->regs->ce_int_mask);
		goto end;
	} else if (state & INT_DTRANE) {
		sh_mmcif_write(~INT_DTRANE, &host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MDTRANE, &host->regs->ce_int_mask);
		goto end;
	} else if (state & INT_CMD12RBE) {
		sh_mmcif_write(~(INT_CMD12RBE | INT_CMD12CRE),
				&host->regs->ce_int);
		sh_mmcif_bitclr(MASK_MCMD12RBE, &host->regs->ce_int_mask);
		goto end;
	} else if (state & INT_ERR_STS) {
		/* err interrupts */
		sh_mmcif_write(~state, &host->regs->ce_int);
		sh_mmcif_bitclr(state, &host->regs->ce_int_mask);
		goto err;
	} else
		return -EAGAIN;

err:
	host->sd_error = 1;
	debug("%s: int err state = %08x\n", DRIVER_NAME, state);
end:
	host->wait_int = 1;
	return 0;
}

static int mmcif_wait_interrupt_flag(struct sh_mmcif_host *host)
{
	int timeout = 10000000;

	while (1) {
		timeout--;
		if (timeout < 0) {
			printf("timeout\n");
			return 0;
		}

		if (!sh_mmcif_intr(host))
			break;

		udelay(1);	/* 1 usec */
	}

	return 1;	/* Return value: NOT 0 = complete waiting */
}

static void sh_mmcif_clock_control(struct sh_mmcif_host *host, unsigned int clk)
{
	sh_mmcif_bitclr(CLK_ENABLE, &host->regs->ce_clk_ctrl);
	sh_mmcif_bitclr(CLK_CLEAR, &host->regs->ce_clk_ctrl);

	if (!clk)
		return;

	if (clk == CLKDEV_EMMC_DATA)
		sh_mmcif_bitset(CLK_PCLK, &host->regs->ce_clk_ctrl);
	else
		sh_mmcif_bitset((fls(DIV_ROUND_UP(host->clk,
						  clk) - 1) - 1) << 16,
				&host->regs->ce_clk_ctrl);
	sh_mmcif_bitset(CLK_ENABLE, &host->regs->ce_clk_ctrl);
}

static void sh_mmcif_sync_reset(struct sh_mmcif_host *host)
{
	u32 tmp;

	tmp = sh_mmcif_read(&host->regs->ce_clk_ctrl) & (CLK_ENABLE |
							 CLK_CLEAR);

	sh_mmcif_write(SOFT_RST_ON, &host->regs->ce_version);
	sh_mmcif_write(SOFT_RST_OFF, &host->regs->ce_version);
	sh_mmcif_bitset(tmp | SRSPTO_256 | SRBSYTO_29 | SRWDTO_29 | SCCSTO_29,
			&host->regs->ce_clk_ctrl);
	/* byte swap on */
	sh_mmcif_bitset(BUF_ACC_ATYP, &host->regs->ce_buf_acc);
}

static int sh_mmcif_error_manage(struct sh_mmcif_host *host)
{
	u32 state1, state2;
	int ret, timeout = 10000000;

	host->sd_error = 0;
	host->wait_int = 0;

	state1 = sh_mmcif_read(&host->regs->ce_host_sts1);
	state2 = sh_mmcif_read(&host->regs->ce_host_sts2);
	debug("%s: ERR HOST_STS1 = %08x\n", \
			DRIVER_NAME, sh_mmcif_read(&host->regs->ce_host_sts1));
	debug("%s: ERR HOST_STS2 = %08x\n", \
			DRIVER_NAME, sh_mmcif_read(&host->regs->ce_host_sts2));

	if (state1 & STS1_CMDSEQ) {
		debug("%s: Forced end of command sequence\n", DRIVER_NAME);
		sh_mmcif_bitset(CMD_CTRL_BREAK, &host->regs->ce_cmd_ctrl);
		sh_mmcif_bitset(~CMD_CTRL_BREAK, &host->regs->ce_cmd_ctrl);
		while (1) {
			timeout--;
			if (timeout < 0) {
				printf(DRIVER_NAME": Forceed end of " \
					"command sequence timeout err\n");
				return -EILSEQ;
			}
			if (!(sh_mmcif_read(&host->regs->ce_host_sts1)
								& STS1_CMDSEQ))
				break;
		}
		sh_mmcif_sync_reset(host);
		return -EILSEQ;
	}

	if (state2 & STS2_CRC_ERR)
		ret = -EILSEQ;
	else if (state2 & STS2_TIMEOUT_ERR)
		ret = -ETIMEDOUT;
	else
		ret = -EILSEQ;
	return ret;
}

static int sh_mmcif_single_read(struct sh_mmcif_host *host,
				struct mmc_data *data)
{
	long time;
	u32 blocksize, i;
	unsigned long *p = (unsigned long *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	host->wait_int = 0;

	/* buf read enable */
	sh_mmcif_bitset(MASK_MBUFREN, &host->regs->ce_int_mask);
	time = mmcif_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_mmcif_error_manage(host);

	host->wait_int = 0;
	blocksize = (BLOCK_SIZE_MASK &
			sh_mmcif_read(&host->regs->ce_block_set)) + 3;
	for (i = 0; i < blocksize / 4; i++)
		*p++ = sh_mmcif_read(&host->regs->ce_data);

	/* buffer read end */
	sh_mmcif_bitset(MASK_MBUFRE, &host->regs->ce_int_mask);
	time = mmcif_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_mmcif_error_manage(host);

	host->wait_int = 0;
	return 0;
}

static int sh_mmcif_multi_read(struct sh_mmcif_host *host,
				struct mmc_data *data)
{
	long time;
	u32 blocksize, i, j;
	unsigned long *p = (unsigned long *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	host->wait_int = 0;
	blocksize = BLOCK_SIZE_MASK & sh_mmcif_read(&host->regs->ce_block_set);
	for (j = 0; j < data->blocks; j++) {
		sh_mmcif_bitset(MASK_MBUFREN, &host->regs->ce_int_mask);
		time = mmcif_wait_interrupt_flag(host);
		if (time == 0 || host->sd_error != 0)
			return sh_mmcif_error_manage(host);

		host->wait_int = 0;
		for (i = 0; i < blocksize / 4; i++)
			*p++ = sh_mmcif_read(&host->regs->ce_data);

		WATCHDOG_RESET();
	}
	return 0;
}

static int sh_mmcif_single_write(struct sh_mmcif_host *host,
				 struct mmc_data *data)
{
	long time;
	u32 blocksize, i;
	const unsigned long *p = (unsigned long *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	host->wait_int = 0;
	sh_mmcif_bitset(MASK_MBUFWEN, &host->regs->ce_int_mask);

	time = mmcif_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_mmcif_error_manage(host);

	host->wait_int = 0;
	blocksize = (BLOCK_SIZE_MASK &
			sh_mmcif_read(&host->regs->ce_block_set)) + 3;
	for (i = 0; i < blocksize / 4; i++)
		sh_mmcif_write(*p++, &host->regs->ce_data);

	/* buffer write end */
	sh_mmcif_bitset(MASK_MDTRANE, &host->regs->ce_int_mask);

	time = mmcif_wait_interrupt_flag(host);
	if (time == 0 || host->sd_error != 0)
		return sh_mmcif_error_manage(host);

	host->wait_int = 0;
	return 0;
}

static int sh_mmcif_multi_write(struct sh_mmcif_host *host,
				struct mmc_data *data)
{
	long time;
	u32 i, j, blocksize;
	const unsigned long *p = (unsigned long *)data->dest;

	if ((unsigned long)p & 0x00000001) {
		printf("%s: The data pointer is unaligned.", __func__);
		return -EIO;
	}

	host->wait_int = 0;
	blocksize = BLOCK_SIZE_MASK & sh_mmcif_read(&host->regs->ce_block_set);
	for (j = 0; j < data->blocks; j++) {
		sh_mmcif_bitset(MASK_MBUFWEN, &host->regs->ce_int_mask);

		time = mmcif_wait_interrupt_flag(host);

		if (time == 0 || host->sd_error != 0)
			return sh_mmcif_error_manage(host);

		host->wait_int = 0;
		for (i = 0; i < blocksize / 4; i++)
			sh_mmcif_write(*p++, &host->regs->ce_data);

		WATCHDOG_RESET();
	}
	return 0;
}

static void sh_mmcif_get_response(struct sh_mmcif_host *host,
					struct mmc_cmd *cmd)
{
	if (cmd->resp_type & MMC_RSP_136) {
		cmd->response[0] = sh_mmcif_read(&host->regs->ce_resp3);
		cmd->response[1] = sh_mmcif_read(&host->regs->ce_resp2);
		cmd->response[2] = sh_mmcif_read(&host->regs->ce_resp1);
		cmd->response[3] = sh_mmcif_read(&host->regs->ce_resp0);
		debug(" RESP %08x, %08x, %08x, %08x\n", cmd->response[0],
			 cmd->response[1], cmd->response[2], cmd->response[3]);
	} else {
		cmd->response[0] = sh_mmcif_read(&host->regs->ce_resp0);
	}
}

static void sh_mmcif_get_cmd12response(struct sh_mmcif_host *host,
					struct mmc_cmd *cmd)
{
	cmd->response[0] = sh_mmcif_read(&host->regs->ce_resp_cmd12);
}

static u32 sh_mmcif_set_cmd(struct sh_mmcif_host *host,
				struct mmc_data *data, struct mmc_cmd *cmd)
{
	u32 tmp = 0;
	u32 opc = cmd->cmdidx;

	/* Response Type check */
	switch (cmd->resp_type) {
	case MMC_RSP_NONE:
		tmp |= CMD_SET_RTYP_NO;
		break;
	case MMC_RSP_R1:
	case MMC_RSP_R1b:
	case MMC_RSP_R3:
		tmp |= CMD_SET_RTYP_6B;
		break;
	case MMC_RSP_R2:
		tmp |= CMD_SET_RTYP_17B;
		break;
	default:
		printf(DRIVER_NAME": Not support type response.\n");
		break;
	}

	/* RBSY */
	if (opc == MMC_CMD_SWITCH)
		tmp |= CMD_SET_RBSY;

	/* WDAT / DATW */
	if (host->data) {
		tmp |= CMD_SET_WDAT;
		switch (host->bus_width) {
		case MMC_BUS_WIDTH_1:
			tmp |= CMD_SET_DATW_1;
			break;
		case MMC_BUS_WIDTH_4:
			tmp |= CMD_SET_DATW_4;
			break;
		case MMC_BUS_WIDTH_8:
			tmp |= CMD_SET_DATW_8;
			break;
		default:
			printf(DRIVER_NAME": Not support bus width.\n");
			break;
		}
	}
	/* DWEN */
	if (opc == MMC_CMD_WRITE_SINGLE_BLOCK ||
	    opc == MMC_CMD_WRITE_MULTIPLE_BLOCK)
		tmp |= CMD_SET_DWEN;
	/* CMLTE/CMD12EN */
	if (opc == MMC_CMD_READ_MULTIPLE_BLOCK ||
	    opc == MMC_CMD_WRITE_MULTIPLE_BLOCK) {
		tmp |= CMD_SET_CMLTE | CMD_SET_CMD12EN;
		sh_mmcif_bitset(data->blocks << 16, &host->regs->ce_block_set);
	}
	/* RIDXC[1:0] check bits */
	if (opc == MMC_CMD_SEND_OP_COND || opc == MMC_CMD_ALL_SEND_CID ||
	    opc == MMC_CMD_SEND_CSD || opc == MMC_CMD_SEND_CID)
		tmp |= CMD_SET_RIDXC_BITS;
	/* RCRC7C[1:0] check bits */
	if (opc == MMC_CMD_SEND_OP_COND)
		tmp |= CMD_SET_CRC7C_BITS;
	/* RCRC7C[1:0] internal CRC7 */
	if (opc == MMC_CMD_ALL_SEND_CID ||
		opc == MMC_CMD_SEND_CSD || opc == MMC_CMD_SEND_CID)
		tmp |= CMD_SET_CRC7C_INTERNAL;

	return opc = ((opc << 24) | tmp);
}

static u32 sh_mmcif_data_trans(struct sh_mmcif_host *host,
				struct mmc_data *data, u16 opc)
{
	u32 ret;

	switch (opc) {
	case MMC_CMD_READ_MULTIPLE_BLOCK:
		ret = sh_mmcif_multi_read(host, data);
		break;
	case MMC_CMD_WRITE_MULTIPLE_BLOCK:
		ret = sh_mmcif_multi_write(host, data);
		break;
	case MMC_CMD_WRITE_SINGLE_BLOCK:
		ret = sh_mmcif_single_write(host, data);
		break;
	case MMC_CMD_READ_SINGLE_BLOCK:
	case MMC_CMD_SEND_EXT_CSD:
		ret = sh_mmcif_single_read(host, data);
		break;
	default:
		printf(DRIVER_NAME": NOT SUPPORT CMD = d'%08d\n", opc);
		ret = -EINVAL;
		break;
	}
	return ret;
}

static int sh_mmcif_start_cmd(struct sh_mmcif_host *host,
				struct mmc_data *data, struct mmc_cmd *cmd)
{
	long time;
	int ret = 0, mask = 0;
	u32 opc = cmd->cmdidx;

	if (opc == MMC_CMD_STOP_TRANSMISSION) {
		/* MMCIF sends the STOP command automatically */
		if (host->last_cmd == MMC_CMD_READ_MULTIPLE_BLOCK)
			sh_mmcif_bitset(MASK_MCMD12DRE,
					&host->regs->ce_int_mask);
		else
			sh_mmcif_bitset(MASK_MCMD12RBE,
					&host->regs->ce_int_mask);

		time = mmcif_wait_interrupt_flag(host);
		if (time == 0 || host->sd_error != 0)
			return sh_mmcif_error_manage(host);

		sh_mmcif_get_cmd12response(host, cmd);
		return 0;
	}
	if (opc == MMC_CMD_SWITCH)
		mask = MASK_MRBSYE;
	else
		mask = MASK_MCRSPE;

	mask |=	MASK_MCMDVIO | MASK_MBUFVIO | MASK_MWDATERR |
		MASK_MRDATERR | MASK_MRIDXERR | MASK_MRSPERR |
		MASK_MCCSTO | MASK_MCRCSTO | MASK_MWDATTO |
		MASK_MRDATTO | MASK_MRBSYTO | MASK_MRSPTO;

	if (host->data) {
		sh_mmcif_write(0, &host->regs->ce_block_set);
		sh_mmcif_write(data->blocksize, &host->regs->ce_block_set);
	}
	opc = sh_mmcif_set_cmd(host, data, cmd);

	sh_mmcif_write(INT_START_MAGIC, &host->regs->ce_int);
	sh_mmcif_write(mask, &host->regs->ce_int_mask);

	debug("CMD%d ARG:%08x\n", cmd->cmdidx, cmd->cmdarg);
	/* set arg */
	sh_mmcif_write(cmd->cmdarg, &host->regs->ce_arg);
	host->wait_int = 0;
	/* set cmd */
	sh_mmcif_write(opc, &host->regs->ce_cmd_set);

	time = mmcif_wait_interrupt_flag(host);
	if (time == 0)
		return sh_mmcif_error_manage(host);

	if (host->sd_error) {
		switch (cmd->cmdidx) {
		case MMC_CMD_ALL_SEND_CID:
		case MMC_CMD_SELECT_CARD:
		case MMC_CMD_APP_CMD:
			ret = -ETIMEDOUT;
			break;
		default:
			printf(DRIVER_NAME": Cmd(d'%d) err\n", cmd->cmdidx);
			ret = sh_mmcif_error_manage(host);
			break;
		}
		host->sd_error = 0;
		host->wait_int = 0;
		return ret;
	}

	/* if no response */
	if (!(opc & 0x00C00000))
		return 0;

	if (host->wait_int == 1) {
		sh_mmcif_get_response(host, cmd);
		host->wait_int = 0;
	}
	if (host->data)
		ret = sh_mmcif_data_trans(host, data, cmd->cmdidx);
	host->last_cmd = cmd->cmdidx;

	return ret;
}

static int sh_mmcif_send_cmd_common(struct sh_mmcif_host *host,
				    struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret;

	WATCHDOG_RESET();

	switch (cmd->cmdidx) {
	case MMC_CMD_APP_CMD:
		return -ETIMEDOUT;
	case MMC_CMD_SEND_EXT_CSD: /* = SD_SEND_IF_COND (8) */
		if (data)
			/* ext_csd */
			break;
		else
			/* send_if_cond cmd (not support) */
			return -ETIMEDOUT;
	default:
		break;
	}
	host->sd_error = 0;
	host->data = data;
	ret = sh_mmcif_start_cmd(host, data, cmd);
	host->data = NULL;

	return ret;
}

static int sh_mmcif_set_ios_common(struct sh_mmcif_host *host, struct mmc *mmc)
{
	if (mmc->clock)
		sh_mmcif_clock_control(host, mmc->clock);

	if (mmc->bus_width == 8)
		host->bus_width = MMC_BUS_WIDTH_8;
	else if (mmc->bus_width == 4)
		host->bus_width = MMC_BUS_WIDTH_4;
	else
		host->bus_width = MMC_BUS_WIDTH_1;

	debug("clock = %d, buswidth = %d\n", mmc->clock, mmc->bus_width);

	return 0;
}

static int sh_mmcif_initialize_common(struct sh_mmcif_host *host)
{
	sh_mmcif_sync_reset(host);
	sh_mmcif_write(MASK_ALL, &host->regs->ce_int_mask);
	return 0;
}

#ifndef CONFIG_DM_MMC
static void *mmc_priv(struct mmc *mmc)
{
	return (void *)mmc->priv;
}

static int sh_mmcif_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd,
			    struct mmc_data *data)
{
	struct sh_mmcif_host *host = mmc_priv(mmc);

	return sh_mmcif_send_cmd_common(host, cmd, data);
}

static int sh_mmcif_set_ios(struct mmc *mmc)
{
	struct sh_mmcif_host *host = mmc_priv(mmc);

	return sh_mmcif_set_ios_common(host, mmc);
}

static int sh_mmcif_initialize(struct mmc *mmc)
{
	struct sh_mmcif_host *host = mmc_priv(mmc);

	return sh_mmcif_initialize_common(host);
}

static const struct mmc_ops sh_mmcif_ops = {
	.send_cmd       = sh_mmcif_send_cmd,
	.set_ios        = sh_mmcif_set_ios,
	.init           = sh_mmcif_initialize,
};

static struct mmc_config sh_mmcif_cfg = {
	.name		= DRIVER_NAME,
	.ops		= &sh_mmcif_ops,
	.host_caps	= MMC_MODE_HS | MMC_MODE_HS_52MHz | MMC_MODE_4BIT |
			  MMC_MODE_8BIT,
	.voltages	= MMC_VDD_32_33 | MMC_VDD_33_34,
	.b_max		= CONFIG_SYS_MMC_MAX_BLK_COUNT,
};

int mmcif_mmc_init(void)
{
	struct mmc *mmc;
	struct sh_mmcif_host *host = NULL;

	host = malloc(sizeof(struct sh_mmcif_host));
	if (!host)
		return -ENOMEM;
	memset(host, 0, sizeof(*host));

	host->regs = (struct sh_mmcif_regs *)CONFIG_SH_MMCIF_ADDR;
	host->clk = CONFIG_SH_MMCIF_CLK;

	sh_mmcif_cfg.f_min = MMC_CLK_DIV_MIN(host->clk);
	sh_mmcif_cfg.f_max = MMC_CLK_DIV_MAX(host->clk);

	mmc = mmc_create(&sh_mmcif_cfg, host);
	if (mmc == NULL) {
		free(host);
		return -ENOMEM;
	}

	return 0;
}

#else
struct sh_mmcif_plat {
	struct mmc_config cfg;
	struct mmc mmc;
};

int sh_mmcif_dm_send_cmd(struct udevice *dev, struct mmc_cmd *cmd,
			struct mmc_data *data)
{
	struct sh_mmcif_host *host = dev_get_priv(dev);

	return sh_mmcif_send_cmd_common(host, cmd, data);
}

int sh_mmcif_dm_set_ios(struct udevice *dev)
{
	struct sh_mmcif_host *host = dev_get_priv(dev);
	struct mmc *mmc = mmc_get_mmc_dev(dev);

	return sh_mmcif_set_ios_common(host, mmc);
}

static const struct dm_mmc_ops sh_mmcif_dm_ops = {
	.send_cmd	= sh_mmcif_dm_send_cmd,
	.set_ios	= sh_mmcif_dm_set_ios,
};

static int sh_mmcif_dm_bind(struct udevice *dev)
{
	struct sh_mmcif_plat *plat = dev_get_platdata(dev);

	return mmc_bind(dev, &plat->mmc, &plat->cfg);
}

static int sh_mmcif_dm_probe(struct udevice *dev)
{
	struct sh_mmcif_plat *plat = dev_get_platdata(dev);
	struct sh_mmcif_host *host = dev_get_priv(dev);
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(dev);
	struct clk sh_mmcif_clk;
	fdt_addr_t base;
	int ret;

	base = devfdt_get_addr(dev);
	if (base == FDT_ADDR_T_NONE)
		return -EINVAL;

	host->regs = (struct sh_mmcif_regs *)devm_ioremap(dev, base, SZ_2K);
	if (!host->regs)
		return -ENOMEM;

	ret = clk_get_by_index(dev, 0, &sh_mmcif_clk);
	if (ret) {
		debug("failed to get clock, ret=%d\n", ret);
		return ret;
	}

	ret = clk_enable(&sh_mmcif_clk);
	if (ret) {
		debug("failed to enable clock, ret=%d\n", ret);
		return ret;
	}

	host->clk = clk_set_rate(&sh_mmcif_clk, 97500000);

	plat->cfg.name = dev->name;
	plat->cfg.host_caps = MMC_MODE_HS_52MHz | MMC_MODE_HS;

	switch (fdtdec_get_int(gd->fdt_blob, dev_of_offset(dev), "bus-width",
			       1)) {
	case 8:
		plat->cfg.host_caps |= MMC_MODE_8BIT;
		break;
	case 4:
		plat->cfg.host_caps |= MMC_MODE_4BIT;
		break;
	case 1:
		break;
	default:
		dev_err(dev, "Invalid \"bus-width\" value\n");
		return -EINVAL;
	}

	sh_mmcif_initialize_common(host);

	plat->cfg.voltages = MMC_VDD_165_195 | MMC_VDD_32_33 | MMC_VDD_33_34;
	plat->cfg.f_min = MMC_CLK_DIV_MIN(host->clk);
	plat->cfg.f_max = MMC_CLK_DIV_MAX(host->clk);
	plat->cfg.b_max = CONFIG_SYS_MMC_MAX_BLK_COUNT;

	upriv->mmc = &plat->mmc;

	return 0;
}

static const struct udevice_id sh_mmcif_sd_match[] = {
	{ .compatible = "renesas,sh-mmcif" },
	{ /* sentinel */ }
};

U_BOOT_DRIVER(sh_mmcif_mmc) = {
	.name			= "sh-mmcif",
	.id			= UCLASS_MMC,
	.of_match		= sh_mmcif_sd_match,
	.bind			= sh_mmcif_dm_bind,
	.probe			= sh_mmcif_dm_probe,
	.priv_auto_alloc_size	= sizeof(struct sh_mmcif_host),
	.platdata_auto_alloc_size = sizeof(struct sh_mmcif_plat),
	.ops			= &sh_mmcif_dm_ops,
};
#endif
