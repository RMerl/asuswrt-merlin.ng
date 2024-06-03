// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2008, Freescale Semiconductor, Inc
 * Andy Fleming
 *
 * Based vaguely on the Linux code
 */

#include <config.h>
#include <common.h>
#include <command.h>
#include <dm.h>
#include <dm/device-internal.h>
#include <errno.h>
#include <mmc.h>
#include <part.h>
#include <power/regulator.h>
#include <malloc.h>
#include <memalign.h>
#include <linux/list.h>
#include <div64.h>
#include "mmc_private.h"

static int mmc_set_signal_voltage(struct mmc *mmc, uint signal_voltage);
static int mmc_power_cycle(struct mmc *mmc);
#if !CONFIG_IS_ENABLED(MMC_TINY)
static int mmc_select_mode_and_width(struct mmc *mmc, uint card_caps);
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
static int mmc_wait_dat0(struct mmc *mmc, int state, int timeout)
{
	return -ENOSYS;
}
#endif

__weak int board_mmc_getwp(struct mmc *mmc)
{
	return -1;
}

int mmc_getwp(struct mmc *mmc)
{
	int wp;

	wp = board_mmc_getwp(mmc);

	if (wp < 0) {
		if (mmc->cfg->ops->getwp)
			wp = mmc->cfg->ops->getwp(mmc);
		else
			wp = 0;
	}

	return wp;
}

__weak int board_mmc_getcd(struct mmc *mmc)
{
	return -1;
}
#endif

#ifdef CONFIG_MMC_TRACE
void mmmc_trace_before_send(struct mmc *mmc, struct mmc_cmd *cmd)
{
	printf("CMD_SEND:%d\n", cmd->cmdidx);
	printf("\t\tARG\t\t\t 0x%08x\n", cmd->cmdarg);
}

void mmmc_trace_after_send(struct mmc *mmc, struct mmc_cmd *cmd, int ret)
{
	int i;
	u8 *ptr;

	if (ret) {
		printf("\t\tRET\t\t\t %d\n", ret);
	} else {
		switch (cmd->resp_type) {
		case MMC_RSP_NONE:
			printf("\t\tMMC_RSP_NONE\n");
			break;
		case MMC_RSP_R1:
			printf("\t\tMMC_RSP_R1,5,6,7 \t 0x%08x \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R1b:
			printf("\t\tMMC_RSP_R1b\t\t 0x%08x \n",
				cmd->response[0]);
			break;
		case MMC_RSP_R2:
			printf("\t\tMMC_RSP_R2\t\t 0x%08x \n",
				cmd->response[0]);
			printf("\t\t          \t\t 0x%08x \n",
				cmd->response[1]);
			printf("\t\t          \t\t 0x%08x \n",
				cmd->response[2]);
			printf("\t\t          \t\t 0x%08x \n",
				cmd->response[3]);
			printf("\n");
			printf("\t\t\t\t\tDUMPING DATA\n");
			for (i = 0; i < 4; i++) {
				int j;
				printf("\t\t\t\t\t%03d - ", i*4);
				ptr = (u8 *)&cmd->response[i];
				ptr += 3;
				for (j = 0; j < 4; j++)
					printf("%02x ", *ptr--);
				printf("\n");
			}
			break;
		case MMC_RSP_R3:
			printf("\t\tMMC_RSP_R3,4\t\t 0x%08x \n",
				cmd->response[0]);
			break;
		default:
			printf("\t\tERROR MMC rsp not supported\n");
			break;
		}
	}
}

void mmc_trace_state(struct mmc *mmc, struct mmc_cmd *cmd)
{
	int status;

	status = (cmd->response[0] & MMC_STATUS_CURR_STATE) >> 9;
	printf("CURR STATE:%d\n", status);
}
#endif

#if CONFIG_IS_ENABLED(MMC_VERBOSE) || defined(DEBUG)
const char *mmc_mode_name(enum bus_mode mode)
{
	static const char *const names[] = {
	      [MMC_LEGACY]	= "MMC legacy",
	      [SD_LEGACY]	= "SD Legacy",
	      [MMC_HS]		= "MMC High Speed (26MHz)",
	      [SD_HS]		= "SD High Speed (50MHz)",
	      [UHS_SDR12]	= "UHS SDR12 (25MHz)",
	      [UHS_SDR25]	= "UHS SDR25 (50MHz)",
	      [UHS_SDR50]	= "UHS SDR50 (100MHz)",
	      [UHS_SDR104]	= "UHS SDR104 (208MHz)",
	      [UHS_DDR50]	= "UHS DDR50 (50MHz)",
	      [MMC_HS_52]	= "MMC High Speed (52MHz)",
	      [MMC_DDR_52]	= "MMC DDR52 (52MHz)",
	      [MMC_HS_200]	= "HS200 (200MHz)",
	      [MMC_HS_400]	= "HS400 (200MHz)",
	};

	if (mode >= MMC_MODES_END)
		return "Unknown mode";
	else
		return names[mode];
}
#endif

static uint mmc_mode2freq(struct mmc *mmc, enum bus_mode mode)
{
	static const int freqs[] = {
	      [MMC_LEGACY]	= 25000000,
	      [SD_LEGACY]	= 25000000,
	      [MMC_HS]		= 26000000,
	      [SD_HS]		= 50000000,
	      [MMC_HS_52]	= 52000000,
	      [MMC_DDR_52]	= 52000000,
	      [UHS_SDR12]	= 25000000,
	      [UHS_SDR25]	= 50000000,
	      [UHS_SDR50]	= 100000000,
	      [UHS_DDR50]	= 50000000,
	      [UHS_SDR104]	= 208000000,
	      [MMC_HS_200]	= 200000000,
	      [MMC_HS_400]	= 200000000,
	};

	if (mode == MMC_LEGACY)
		return mmc->legacy_speed;
	else if (mode >= MMC_MODES_END)
		return 0;
	else
		return freqs[mode];
}

static int mmc_select_mode(struct mmc *mmc, enum bus_mode mode)
{
	mmc->selected_mode = mode;
	mmc->tran_speed = mmc_mode2freq(mmc, mode);
	mmc->ddr_mode = mmc_is_mode_ddr(mode);
	pr_debug("selecting mode %s (freq : %d MHz)\n", mmc_mode_name(mode),
		 mmc->tran_speed / 1000000);
	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
int mmc_send_cmd(struct mmc *mmc, struct mmc_cmd *cmd, struct mmc_data *data)
{
	int ret;

	mmmc_trace_before_send(mmc, cmd);
	ret = mmc->cfg->ops->send_cmd(mmc, cmd, data);
	mmmc_trace_after_send(mmc, cmd, ret);

	return ret;
}
#endif

int mmc_send_status(struct mmc *mmc, int timeout)
{
	struct mmc_cmd cmd;
	int err, retries = 5;

	cmd.cmdidx = MMC_CMD_SEND_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	if (!mmc_host_is_spi(mmc))
		cmd.cmdarg = mmc->rca << 16;

	while (1) {
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (!err) {
			if ((cmd.response[0] & MMC_STATUS_RDY_FOR_DATA) &&
			    (cmd.response[0] & MMC_STATUS_CURR_STATE) !=
			     MMC_STATE_PRG)
				break;

			if (cmd.response[0] & MMC_STATUS_MASK) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
				pr_err("Status Error: 0x%08x\n",
				       cmd.response[0]);
#endif
				return -ECOMM;
			}
		} else if (--retries < 0)
			return err;

		if (timeout-- <= 0)
			break;

		udelay(1000);
	}

	mmc_trace_state(mmc, &cmd);
	if (timeout <= 0) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		pr_err("Timeout waiting card ready\n");
#endif
		return -ETIMEDOUT;
	}

	return 0;
}

int mmc_set_blocklen(struct mmc *mmc, int len)
{
	struct mmc_cmd cmd;
	int err;

	if (mmc->ddr_mode)
		return 0;

	cmd.cmdidx = MMC_CMD_SET_BLOCKLEN;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = len;

	err = mmc_send_cmd(mmc, &cmd, NULL);

#ifdef CONFIG_MMC_QUIRKS
	if (err && (mmc->quirks & MMC_QUIRK_RETRY_SET_BLOCKLEN)) {
		int retries = 4;
		/*
		 * It has been seen that SET_BLOCKLEN may fail on the first
		 * attempt, let's try a few more time
		 */
		do {
			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (!err)
				break;
		} while (retries--);
	}
#endif

	return err;
}

#ifdef MMC_SUPPORTS_TUNING
static const u8 tuning_blk_pattern_4bit[] = {
	0xff, 0x0f, 0xff, 0x00, 0xff, 0xcc, 0xc3, 0xcc,
	0xc3, 0x3c, 0xcc, 0xff, 0xfe, 0xff, 0xfe, 0xef,
	0xff, 0xdf, 0xff, 0xdd, 0xff, 0xfb, 0xff, 0xfb,
	0xbf, 0xff, 0x7f, 0xff, 0x77, 0xf7, 0xbd, 0xef,
	0xff, 0xf0, 0xff, 0xf0, 0x0f, 0xfc, 0xcc, 0x3c,
	0xcc, 0x33, 0xcc, 0xcf, 0xff, 0xef, 0xff, 0xee,
	0xff, 0xfd, 0xff, 0xfd, 0xdf, 0xff, 0xbf, 0xff,
	0xbb, 0xff, 0xf7, 0xff, 0xf7, 0x7f, 0x7b, 0xde,
};

static const u8 tuning_blk_pattern_8bit[] = {
	0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00, 0x00,
	0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc, 0xcc,
	0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff, 0xff,
	0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee, 0xff,
	0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd, 0xdd,
	0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff, 0xbb,
	0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff, 0xff,
	0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee, 0xff,
	0xff, 0xff, 0xff, 0x00, 0xff, 0xff, 0xff, 0x00,
	0x00, 0xff, 0xff, 0xcc, 0xcc, 0xcc, 0x33, 0xcc,
	0xcc, 0xcc, 0x33, 0x33, 0xcc, 0xcc, 0xcc, 0xff,
	0xff, 0xff, 0xee, 0xff, 0xff, 0xff, 0xee, 0xee,
	0xff, 0xff, 0xff, 0xdd, 0xff, 0xff, 0xff, 0xdd,
	0xdd, 0xff, 0xff, 0xff, 0xbb, 0xff, 0xff, 0xff,
	0xbb, 0xbb, 0xff, 0xff, 0xff, 0x77, 0xff, 0xff,
	0xff, 0x77, 0x77, 0xff, 0x77, 0xbb, 0xdd, 0xee,
};

int mmc_send_tuning(struct mmc *mmc, u32 opcode, int *cmd_error)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	const u8 *tuning_block_pattern;
	int size, err;

	if (mmc->bus_width == 8) {
		tuning_block_pattern = tuning_blk_pattern_8bit;
		size = sizeof(tuning_blk_pattern_8bit);
	} else if (mmc->bus_width == 4) {
		tuning_block_pattern = tuning_blk_pattern_4bit;
		size = sizeof(tuning_blk_pattern_4bit);
	} else {
		return -EINVAL;
	}

	ALLOC_CACHE_ALIGN_BUFFER(u8, data_buf, size);

	cmd.cmdidx = opcode;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R1;

	data.dest = (void *)data_buf;
	data.blocks = 1;
	data.blocksize = size;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);
	if (err)
		return err;

	if (memcmp(data_buf, tuning_block_pattern, size))
		return -EIO;

	return 0;
}
#endif

static int mmc_read_blocks(struct mmc *mmc, void *dst, lbaint_t start,
			   lbaint_t blkcnt)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	if (blkcnt > 1)
		cmd.cmdidx = MMC_CMD_READ_MULTIPLE_BLOCK;
	else
		cmd.cmdidx = MMC_CMD_READ_SINGLE_BLOCK;

	if (mmc->high_capacity)
		cmd.cmdarg = start;
	else
		cmd.cmdarg = start * mmc->read_bl_len;

	cmd.resp_type = MMC_RSP_R1;

	data.dest = dst;
	data.blocks = blkcnt;
	data.blocksize = mmc->read_bl_len;
	data.flags = MMC_DATA_READ;

	if (mmc_send_cmd(mmc, &cmd, &data))
		return 0;

	if (blkcnt > 1) {
		cmd.cmdidx = MMC_CMD_STOP_TRANSMISSION;
		cmd.cmdarg = 0;
		cmd.resp_type = MMC_RSP_R1b;
		if (mmc_send_cmd(mmc, &cmd, NULL)) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
			pr_err("mmc fail to send stop cmd\n");
#endif
			return 0;
		}
	}

	return blkcnt;
}

#if CONFIG_IS_ENABLED(BLK)
ulong mmc_bread(struct udevice *dev, lbaint_t start, lbaint_t blkcnt, void *dst)
#else
ulong mmc_bread(struct blk_desc *block_dev, lbaint_t start, lbaint_t blkcnt,
		void *dst)
#endif
{
#if CONFIG_IS_ENABLED(BLK)
	struct blk_desc *block_dev = dev_get_uclass_platdata(dev);
#endif
	int dev_num = block_dev->devnum;
	int err;
	lbaint_t cur, blocks_todo = blkcnt;

	if (blkcnt == 0)
		return 0;

	struct mmc *mmc = find_mmc_device(dev_num);
	if (!mmc)
		return 0;

	if (CONFIG_IS_ENABLED(MMC_TINY))
		err = mmc_switch_part(mmc, block_dev->hwpart);
	else
		err = blk_dselect_hwpart(block_dev, block_dev->hwpart);

	if (err < 0)
		return 0;

	if ((start + blkcnt) > block_dev->lba) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		pr_err("MMC: block number 0x" LBAF " exceeds max(0x" LBAF ")\n",
		       start + blkcnt, block_dev->lba);
#endif
		return 0;
	}

	if (mmc_set_blocklen(mmc, mmc->read_bl_len)) {
		pr_debug("%s: Failed to set blocklen\n", __func__);
		return 0;
	}

	do {
		cur = (blocks_todo > mmc->cfg->b_max) ?
			mmc->cfg->b_max : blocks_todo;
		if (mmc_read_blocks(mmc, dst, start, cur) != cur) {
			pr_debug("%s: Failed to read blocks\n", __func__);
			return 0;
		}
		blocks_todo -= cur;
		start += cur;
		dst += cur * mmc->read_bl_len;
	} while (blocks_todo > 0);

	return blkcnt;
}

static int mmc_go_idle(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	udelay(1000);

	cmd.cmdidx = MMC_CMD_GO_IDLE_STATE;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_NONE;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	udelay(2000);

	return 0;
}

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
static int mmc_switch_voltage(struct mmc *mmc, int signal_voltage)
{
	struct mmc_cmd cmd;
	int err = 0;

	/*
	 * Send CMD11 only if the request is to switch the card to
	 * 1.8V signalling.
	 */
	if (signal_voltage == MMC_SIGNAL_VOLTAGE_330)
		return mmc_set_signal_voltage(mmc, signal_voltage);

	cmd.cmdidx = SD_CMD_SWITCH_UHS18V;
	cmd.cmdarg = 0;
	cmd.resp_type = MMC_RSP_R1;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		return err;

	if (!mmc_host_is_spi(mmc) && (cmd.response[0] & MMC_STATUS_ERROR))
		return -EIO;

	/*
	 * The card should drive cmd and dat[0:3] low immediately
	 * after the response of cmd11, but wait 100 us to be sure
	 */
	err = mmc_wait_dat0(mmc, 0, 100);
	if (err == -ENOSYS)
		udelay(100);
	else if (err)
		return -ETIMEDOUT;

	/*
	 * During a signal voltage level switch, the clock must be gated
	 * for 5 ms according to the SD spec
	 */
	mmc_set_clock(mmc, mmc->clock, MMC_CLK_DISABLE);

	err = mmc_set_signal_voltage(mmc, signal_voltage);
	if (err)
		return err;

	/* Keep clock gated for at least 10 ms, though spec only says 5 ms */
	mdelay(10);
	mmc_set_clock(mmc, mmc->clock, MMC_CLK_ENABLE);

	/*
	 * Failure to switch is indicated by the card holding
	 * dat[0:3] low. Wait for at least 1 ms according to spec
	 */
	err = mmc_wait_dat0(mmc, 1, 1000);
	if (err == -ENOSYS)
		udelay(1000);
	else if (err)
		return -ETIMEDOUT;

	return 0;
}
#endif

static int sd_send_op_cond(struct mmc *mmc, bool uhs_en)
{
	int timeout = 1000;
	int err;
	struct mmc_cmd cmd;

	while (1) {
		cmd.cmdidx = MMC_CMD_APP_CMD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		cmd.cmdidx = SD_CMD_APP_SEND_OP_COND;
		cmd.resp_type = MMC_RSP_R3;

		/*
		 * Most cards do not answer if some reserved bits
		 * in the ocr are set. However, Some controller
		 * can set bit 7 (reserved for low voltages), but
		 * how to manage low voltages SD card is not yet
		 * specified.
		 */
		cmd.cmdarg = mmc_host_is_spi(mmc) ? 0 :
			(mmc->cfg->voltages & 0xff8000);

		if (mmc->version == SD_VERSION_2)
			cmd.cmdarg |= OCR_HCS;

		if (uhs_en)
			cmd.cmdarg |= OCR_S18R;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		if (cmd.response[0] & OCR_BUSY)
			break;

		if (timeout-- <= 0)
			return -EOPNOTSUPP;

		udelay(1000);
	}

	if (mmc->version != SD_VERSION_2)
		mmc->version = SD_VERSION_1_0;

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}

	mmc->ocr = cmd.response[0];

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	if (uhs_en && !(mmc_host_is_spi(mmc)) && (cmd.response[0] & 0x41000000)
	    == 0x41000000) {
		err = mmc_switch_voltage(mmc, MMC_SIGNAL_VOLTAGE_180);
		if (err)
			return err;
	}
#endif

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 0;

	return 0;
}

static int mmc_send_op_cond_iter(struct mmc *mmc, int use_arg)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = MMC_CMD_SEND_OP_COND;
	cmd.resp_type = MMC_RSP_R3;
	cmd.cmdarg = 0;
	if (use_arg && !mmc_host_is_spi(mmc))
		cmd.cmdarg = OCR_HCS |
			(mmc->cfg->voltages &
			(mmc->ocr & OCR_VOLTAGE_MASK)) |
			(mmc->ocr & OCR_ACCESS_MODE);

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		return err;
	mmc->ocr = cmd.response[0];
	return 0;
}

static int mmc_send_op_cond(struct mmc *mmc)
{
	int err, i;

	/* Some cards seem to need this */
	mmc_go_idle(mmc);

 	/* Asking to the card its capabilities */
	for (i = 0; i < 2; i++) {
		err = mmc_send_op_cond_iter(mmc, i != 0);
		if (err)
			return err;

		/* exit if not busy (flag seems to be inverted) */
		if (mmc->ocr & OCR_BUSY)
			break;
	}
	mmc->op_cond_pending = 1;
	return 0;
}

static int mmc_complete_op_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	ulong start;
	int err;

	mmc->op_cond_pending = 0;
	if (!(mmc->ocr & OCR_BUSY)) {
		/* Some cards seem to need this */
		mmc_go_idle(mmc);

		start = get_timer(0);
		while (1) {
			err = mmc_send_op_cond_iter(mmc, 1);
			if (err)
				return err;
			if (mmc->ocr & OCR_BUSY)
				break;
			if (get_timer(start) > timeout)
				return -EOPNOTSUPP;
			udelay(100);
		}
	}

	if (mmc_host_is_spi(mmc)) { /* read OCR for spi */
		cmd.cmdidx = MMC_CMD_SPI_READ_OCR;
		cmd.resp_type = MMC_RSP_R3;
		cmd.cmdarg = 0;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		mmc->ocr = cmd.response[0];
	}

	mmc->version = MMC_VERSION_UNKNOWN;

	mmc->high_capacity = ((mmc->ocr & OCR_HCS) == OCR_HCS);
	mmc->rca = 1;

	return 0;
}


static int mmc_send_ext_csd(struct mmc *mmc, u8 *ext_csd)
{
	struct mmc_cmd cmd;
	struct mmc_data data;
	int err;

	/* Get the Card Status Register */
	cmd.cmdidx = MMC_CMD_SEND_EXT_CSD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	data.dest = (char *)ext_csd;
	data.blocks = 1;
	data.blocksize = MMC_MAX_BLOCK_LEN;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	return err;
}

static int __mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value,
			bool send_status)
{
	struct mmc_cmd cmd;
	int timeout = 1000;
	int retries = 3;
	int ret;

	cmd.cmdidx = MMC_CMD_SWITCH;
	cmd.resp_type = MMC_RSP_R1b;
	cmd.cmdarg = (MMC_SWITCH_MODE_WRITE_BYTE << 24) |
				 (index << 16) |
				 (value << 8);

	while (retries > 0) {
		ret = mmc_send_cmd(mmc, &cmd, NULL);

		if (ret) {
			retries--;
			continue;
		}

		if (!send_status) {
			mdelay(50);
			return 0;
		}

		/* Waiting for the ready status */
		return mmc_send_status(mmc, timeout);
	}

	return ret;

}

int mmc_switch(struct mmc *mmc, u8 set, u8 index, u8 value)
{
	return __mmc_switch(mmc, set, index, value, true);
}

#if !CONFIG_IS_ENABLED(MMC_TINY)
static int mmc_set_card_speed(struct mmc *mmc, enum bus_mode mode,
			      bool hsdowngrade)
{
	int err;
	int speed_bits;

	ALLOC_CACHE_ALIGN_BUFFER(u8, test_csd, MMC_MAX_BLOCK_LEN);

	switch (mode) {
	case MMC_HS:
	case MMC_HS_52:
	case MMC_DDR_52:
		speed_bits = EXT_CSD_TIMING_HS;
		break;
#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)
	case MMC_HS_200:
		speed_bits = EXT_CSD_TIMING_HS200;
		break;
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	case MMC_HS_400:
		speed_bits = EXT_CSD_TIMING_HS400;
		break;
#endif
	case MMC_LEGACY:
		speed_bits = EXT_CSD_TIMING_LEGACY;
		break;
	default:
		return -EINVAL;
	}

	err = __mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_HS_TIMING,
			   speed_bits, !hsdowngrade);
	if (err)
		return err;

#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	/*
	 * In case the eMMC is in HS200/HS400 mode and we are downgrading
	 * to HS mode, the card clock are still running much faster than
	 * the supported HS mode clock, so we can not reliably read out
	 * Extended CSD. Reconfigure the controller to run at HS mode.
	 */
	if (hsdowngrade) {
		mmc_select_mode(mmc, MMC_HS);
		mmc_set_clock(mmc, mmc_mode2freq(mmc, MMC_HS), false);
	}
#endif

	if ((mode == MMC_HS) || (mode == MMC_HS_52)) {
		/* Now check to see that it worked */
		err = mmc_send_ext_csd(mmc, test_csd);
		if (err)
			return err;

		/* No high-speed support */
		if (!test_csd[EXT_CSD_HS_TIMING])
			return -ENOTSUPP;
	}

	return 0;
}

static int mmc_get_capabilities(struct mmc *mmc)
{
	u8 *ext_csd = mmc->ext_csd;
	char cardtype;

	mmc->card_caps = MMC_MODE_1BIT | MMC_CAP(MMC_LEGACY);

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Only version 4 supports high-speed */
	if (mmc->version < MMC_VERSION_4)
		return 0;

	if (!ext_csd) {
		pr_err("No ext_csd found!\n"); /* this should enver happen */
		return -ENOTSUPP;
	}

	mmc->card_caps |= MMC_MODE_4BIT | MMC_MODE_8BIT;

	cardtype = ext_csd[EXT_CSD_CARD_TYPE];
	mmc->cardtype = cardtype;

#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)
	if (cardtype & (EXT_CSD_CARD_TYPE_HS200_1_2V |
			EXT_CSD_CARD_TYPE_HS200_1_8V)) {
		mmc->card_caps |= MMC_MODE_HS200;
	}
#endif
#if CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	if (cardtype & (EXT_CSD_CARD_TYPE_HS400_1_2V |
			EXT_CSD_CARD_TYPE_HS400_1_8V)) {
		mmc->card_caps |= MMC_MODE_HS400;
	}
#endif
	if (cardtype & EXT_CSD_CARD_TYPE_52) {
		if (cardtype & EXT_CSD_CARD_TYPE_DDR_52)
			mmc->card_caps |= MMC_MODE_DDR_52MHz;
		mmc->card_caps |= MMC_MODE_HS_52MHz;
	}
	if (cardtype & EXT_CSD_CARD_TYPE_26)
		mmc->card_caps |= MMC_MODE_HS;

	return 0;
}
#endif

static int mmc_set_capacity(struct mmc *mmc, int part_num)
{
	switch (part_num) {
	case 0:
		mmc->capacity = mmc->capacity_user;
		break;
	case 1:
	case 2:
		mmc->capacity = mmc->capacity_boot;
		break;
	case 3:
		mmc->capacity = mmc->capacity_rpmb;
		break;
	case 4:
	case 5:
	case 6:
	case 7:
		mmc->capacity = mmc->capacity_gp[part_num - 4];
		break;
	default:
		return -1;
	}

	mmc_get_blk_desc(mmc)->lba = lldiv(mmc->capacity, mmc->read_bl_len);

	return 0;
}

#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
static int mmc_boot_part_access_chk(struct mmc *mmc, unsigned int part_num)
{
	int forbidden = 0;
	bool change = false;

	if (part_num & PART_ACCESS_MASK)
		forbidden = MMC_CAP(MMC_HS_200) | MMC_CAP(MMC_HS_400);

	if (MMC_CAP(mmc->selected_mode) & forbidden) {
		pr_debug("selected mode (%s) is forbidden for part %d\n",
			 mmc_mode_name(mmc->selected_mode), part_num);
		change = true;
	} else if (mmc->selected_mode != mmc->best_mode) {
		pr_debug("selected mode is not optimal\n");
		change = true;
	}

	if (change)
		return mmc_select_mode_and_width(mmc,
						 mmc->card_caps & ~forbidden);

	return 0;
}
#else
static inline int mmc_boot_part_access_chk(struct mmc *mmc,
					   unsigned int part_num)
{
	return 0;
}
#endif

int mmc_switch_part(struct mmc *mmc, unsigned int part_num)
{
	int ret;

	ret = mmc_boot_part_access_chk(mmc, part_num);
	if (ret)
		return ret;

	ret = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_PART_CONF,
			 (mmc->part_config & ~PART_ACCESS_MASK)
			 | (part_num & PART_ACCESS_MASK));

	/*
	 * Set the capacity if the switch succeeded or was intended
	 * to return to representing the raw device.
	 */
	if ((ret == 0) || ((ret == -ENODEV) && (part_num == 0))) {
		ret = mmc_set_capacity(mmc, part_num);
		mmc_get_blk_desc(mmc)->hwpart = part_num;
	}

	return ret;
}

#if CONFIG_IS_ENABLED(MMC_HW_PARTITIONING)
int mmc_hwpart_config(struct mmc *mmc,
		      const struct mmc_hwpart_conf *conf,
		      enum mmc_hwpart_conf_mode mode)
{
	u8 part_attrs = 0;
	u32 enh_size_mult;
	u32 enh_start_addr;
	u32 gp_size_mult[4];
	u32 max_enh_size_mult;
	u32 tot_enh_size_mult = 0;
	u8 wr_rel_set;
	int i, pidx, err;
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);

	if (mode < MMC_HWPART_CONF_CHECK || mode > MMC_HWPART_CONF_COMPLETE)
		return -EINVAL;

	if (IS_SD(mmc) || (mmc->version < MMC_VERSION_4_41)) {
		pr_err("eMMC >= 4.4 required for enhanced user data area\n");
		return -EMEDIUMTYPE;
	}

	if (!(mmc->part_support & PART_SUPPORT)) {
		pr_err("Card does not support partitioning\n");
		return -EMEDIUMTYPE;
	}

	if (!mmc->hc_wp_grp_size) {
		pr_err("Card does not define HC WP group size\n");
		return -EMEDIUMTYPE;
	}

	/* check partition alignment and total enhanced size */
	if (conf->user.enh_size) {
		if (conf->user.enh_size % mmc->hc_wp_grp_size ||
		    conf->user.enh_start % mmc->hc_wp_grp_size) {
			pr_err("User data enhanced area not HC WP group "
			       "size aligned\n");
			return -EINVAL;
		}
		part_attrs |= EXT_CSD_ENH_USR;
		enh_size_mult = conf->user.enh_size / mmc->hc_wp_grp_size;
		if (mmc->high_capacity) {
			enh_start_addr = conf->user.enh_start;
		} else {
			enh_start_addr = (conf->user.enh_start << 9);
		}
	} else {
		enh_size_mult = 0;
		enh_start_addr = 0;
	}
	tot_enh_size_mult += enh_size_mult;

	for (pidx = 0; pidx < 4; pidx++) {
		if (conf->gp_part[pidx].size % mmc->hc_wp_grp_size) {
			pr_err("GP%i partition not HC WP group size "
			       "aligned\n", pidx+1);
			return -EINVAL;
		}
		gp_size_mult[pidx] = conf->gp_part[pidx].size / mmc->hc_wp_grp_size;
		if (conf->gp_part[pidx].size && conf->gp_part[pidx].enhanced) {
			part_attrs |= EXT_CSD_ENH_GP(pidx);
			tot_enh_size_mult += gp_size_mult[pidx];
		}
	}

	if (part_attrs && ! (mmc->part_support & ENHNCD_SUPPORT)) {
		pr_err("Card does not support enhanced attribute\n");
		return -EMEDIUMTYPE;
	}

	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err)
		return err;

	max_enh_size_mult =
		(ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT+2] << 16) +
		(ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT+1] << 8) +
		ext_csd[EXT_CSD_MAX_ENH_SIZE_MULT];
	if (tot_enh_size_mult > max_enh_size_mult) {
		pr_err("Total enhanced size exceeds maximum (%u > %u)\n",
		       tot_enh_size_mult, max_enh_size_mult);
		return -EMEDIUMTYPE;
	}

	/* The default value of EXT_CSD_WR_REL_SET is device
	 * dependent, the values can only be changed if the
	 * EXT_CSD_HS_CTRL_REL bit is set. The values can be
	 * changed only once and before partitioning is completed. */
	wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];
	if (conf->user.wr_rel_change) {
		if (conf->user.wr_rel_set)
			wr_rel_set |= EXT_CSD_WR_DATA_REL_USR;
		else
			wr_rel_set &= ~EXT_CSD_WR_DATA_REL_USR;
	}
	for (pidx = 0; pidx < 4; pidx++) {
		if (conf->gp_part[pidx].wr_rel_change) {
			if (conf->gp_part[pidx].wr_rel_set)
				wr_rel_set |= EXT_CSD_WR_DATA_REL_GP(pidx);
			else
				wr_rel_set &= ~EXT_CSD_WR_DATA_REL_GP(pidx);
		}
	}

	if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET] &&
	    !(ext_csd[EXT_CSD_WR_REL_PARAM] & EXT_CSD_HS_CTRL_REL)) {
		puts("Card does not support host controlled partition write "
		     "reliability settings\n");
		return -EMEDIUMTYPE;
	}

	if (ext_csd[EXT_CSD_PARTITION_SETTING] &
	    EXT_CSD_PARTITION_SETTING_COMPLETED) {
		pr_err("Card already partitioned\n");
		return -EPERM;
	}

	if (mode == MMC_HWPART_CONF_CHECK)
		return 0;

	/* Partitioning requires high-capacity size definitions */
	if (!(ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01)) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ERASE_GROUP_DEF, 1);

		if (err)
			return err;

		ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;

		/* update erase group size to be high-capacity */
		mmc->erase_grp_size =
			ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;

	}

	/* all OK, write the configuration */
	for (i = 0; i < 4; i++) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ENH_START_ADDR+i,
				 (enh_start_addr >> (i*8)) & 0xFF);
		if (err)
			return err;
	}
	for (i = 0; i < 3; i++) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ENH_SIZE_MULT+i,
				 (enh_size_mult >> (i*8)) & 0xFF);
		if (err)
			return err;
	}
	for (pidx = 0; pidx < 4; pidx++) {
		for (i = 0; i < 3; i++) {
			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
					 EXT_CSD_GP_SIZE_MULT+pidx*3+i,
					 (gp_size_mult[pidx] >> (i*8)) & 0xFF);
			if (err)
				return err;
		}
	}
	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PARTITIONS_ATTRIBUTE, part_attrs);
	if (err)
		return err;

	if (mode == MMC_HWPART_CONF_SET)
		return 0;

	/* The WR_REL_SET is a write-once register but shall be
	 * written before setting PART_SETTING_COMPLETED. As it is
	 * write-once we can only write it when completing the
	 * partitioning. */
	if (wr_rel_set != ext_csd[EXT_CSD_WR_REL_SET]) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_WR_REL_SET, wr_rel_set);
		if (err)
			return err;
	}

	/* Setting PART_SETTING_COMPLETED confirms the partition
	 * configuration but it only becomes effective after power
	 * cycle, so we do not adjust the partition related settings
	 * in the mmc struct. */

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
			 EXT_CSD_PARTITION_SETTING,
			 EXT_CSD_PARTITION_SETTING_COMPLETED);
	if (err)
		return err;

	return 0;
}
#endif

#if !CONFIG_IS_ENABLED(DM_MMC)
int mmc_getcd(struct mmc *mmc)
{
	int cd;

	cd = board_mmc_getcd(mmc);

	if (cd < 0) {
		if (mmc->cfg->ops->getcd)
			cd = mmc->cfg->ops->getcd(mmc);
		else
			cd = 1;
	}

	return cd;
}
#endif

#if !CONFIG_IS_ENABLED(MMC_TINY)
static int sd_switch(struct mmc *mmc, int mode, int group, u8 value, u8 *resp)
{
	struct mmc_cmd cmd;
	struct mmc_data data;

	/* Switch the frequency */
	cmd.cmdidx = SD_CMD_SWITCH_FUNC;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = (mode << 31) | 0xffffff;
	cmd.cmdarg &= ~(0xf << (group * 4));
	cmd.cmdarg |= value << (group * 4);

	data.dest = (char *)resp;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	return mmc_send_cmd(mmc, &cmd, &data);
}

static int sd_get_capabilities(struct mmc *mmc)
{
	int err;
	struct mmc_cmd cmd;
	ALLOC_CACHE_ALIGN_BUFFER(__be32, scr, 2);
	ALLOC_CACHE_ALIGN_BUFFER(__be32, switch_status, 16);
	struct mmc_data data;
	int timeout;
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	u32 sd3_bus_mode;
#endif

	mmc->card_caps = MMC_MODE_1BIT | MMC_CAP(SD_LEGACY);

	if (mmc_host_is_spi(mmc))
		return 0;

	/* Read the SCR to find out if this card supports higher speeds */
	cmd.cmdidx = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	cmd.cmdidx = SD_CMD_APP_SEND_SCR;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

	timeout = 3;

retry_scr:
	data.dest = (char *)scr;
	data.blocksize = 8;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);

	if (err) {
		if (timeout--)
			goto retry_scr;

		return err;
	}

	mmc->scr[0] = __be32_to_cpu(scr[0]);
	mmc->scr[1] = __be32_to_cpu(scr[1]);

	switch ((mmc->scr[0] >> 24) & 0xf) {
	case 0:
		mmc->version = SD_VERSION_1_0;
		break;
	case 1:
		mmc->version = SD_VERSION_1_10;
		break;
	case 2:
		mmc->version = SD_VERSION_2;
		if ((mmc->scr[0] >> 15) & 0x1)
			mmc->version = SD_VERSION_3;
		break;
	default:
		mmc->version = SD_VERSION_1_0;
		break;
	}

	if (mmc->scr[0] & SD_DATA_4BIT)
		mmc->card_caps |= MMC_MODE_4BIT;

	/* Version 1.0 doesn't support switching */
	if (mmc->version == SD_VERSION_1_0)
		return 0;

	timeout = 4;
	while (timeout--) {
		err = sd_switch(mmc, SD_SWITCH_CHECK, 0, 1,
				(u8 *)switch_status);

		if (err)
			return err;

		/* The high-speed function is busy.  Try again */
		if (!(__be32_to_cpu(switch_status[7]) & SD_HIGHSPEED_BUSY))
			break;
	}

	/* If high-speed isn't supported, we return */
	if (__be32_to_cpu(switch_status[3]) & SD_HIGHSPEED_SUPPORTED)
		mmc->card_caps |= MMC_CAP(SD_HS);

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	/* Version before 3.0 don't support UHS modes */
	if (mmc->version < SD_VERSION_3)
		return 0;

	sd3_bus_mode = __be32_to_cpu(switch_status[3]) >> 16 & 0x1f;
	if (sd3_bus_mode & SD_MODE_UHS_SDR104)
		mmc->card_caps |= MMC_CAP(UHS_SDR104);
	if (sd3_bus_mode & SD_MODE_UHS_SDR50)
		mmc->card_caps |= MMC_CAP(UHS_SDR50);
	if (sd3_bus_mode & SD_MODE_UHS_SDR25)
		mmc->card_caps |= MMC_CAP(UHS_SDR25);
	if (sd3_bus_mode & SD_MODE_UHS_SDR12)
		mmc->card_caps |= MMC_CAP(UHS_SDR12);
	if (sd3_bus_mode & SD_MODE_UHS_DDR50)
		mmc->card_caps |= MMC_CAP(UHS_DDR50);
#endif

	return 0;
}

static int sd_set_card_speed(struct mmc *mmc, enum bus_mode mode)
{
	int err;

	ALLOC_CACHE_ALIGN_BUFFER(uint, switch_status, 16);
	int speed;

	/* SD version 1.00 and 1.01 does not support CMD 6 */
	if (mmc->version == SD_VERSION_1_0)
		return 0;

	switch (mode) {
	case SD_LEGACY:
		speed = UHS_SDR12_BUS_SPEED;
		break;
	case SD_HS:
		speed = HIGH_SPEED_BUS_SPEED;
		break;
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	case UHS_SDR12:
		speed = UHS_SDR12_BUS_SPEED;
		break;
	case UHS_SDR25:
		speed = UHS_SDR25_BUS_SPEED;
		break;
	case UHS_SDR50:
		speed = UHS_SDR50_BUS_SPEED;
		break;
	case UHS_DDR50:
		speed = UHS_DDR50_BUS_SPEED;
		break;
	case UHS_SDR104:
		speed = UHS_SDR104_BUS_SPEED;
		break;
#endif
	default:
		return -EINVAL;
	}

	err = sd_switch(mmc, SD_SWITCH_SWITCH, 0, speed, (u8 *)switch_status);
	if (err)
		return err;

	if (((__be32_to_cpu(switch_status[4]) >> 24) & 0xF) != speed)
		return -ENOTSUPP;

	return 0;
}

static int sd_select_bus_width(struct mmc *mmc, int w)
{
	int err;
	struct mmc_cmd cmd;

	if ((w != 4) && (w != 1))
		return -EINVAL;

	cmd.cmdidx = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		return err;

	cmd.cmdidx = SD_CMD_APP_SET_BUS_WIDTH;
	cmd.resp_type = MMC_RSP_R1;
	if (w == 4)
		cmd.cmdarg = 2;
	else if (w == 1)
		cmd.cmdarg = 0;
	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		return err;

	return 0;
}
#endif

#if CONFIG_IS_ENABLED(MMC_WRITE)
static int sd_read_ssr(struct mmc *mmc)
{
	static const unsigned int sd_au_size[] = {
		0,		SZ_16K / 512,		SZ_32K / 512,
		SZ_64K / 512,	SZ_128K / 512,		SZ_256K / 512,
		SZ_512K / 512,	SZ_1M / 512,		SZ_2M / 512,
		SZ_4M / 512,	SZ_8M / 512,		(SZ_8M + SZ_4M) / 512,
		SZ_16M / 512,	(SZ_16M + SZ_8M) / 512,	SZ_32M / 512,
		SZ_64M / 512,
	};
	int err, i;
	struct mmc_cmd cmd;
	ALLOC_CACHE_ALIGN_BUFFER(uint, ssr, 16);
	struct mmc_data data;
	int timeout = 3;
	unsigned int au, eo, et, es;

	cmd.cmdidx = MMC_CMD_APP_CMD;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);
	if (err)
		return err;

	cmd.cmdidx = SD_CMD_APP_SD_STATUS;
	cmd.resp_type = MMC_RSP_R1;
	cmd.cmdarg = 0;

retry_ssr:
	data.dest = (char *)ssr;
	data.blocksize = 64;
	data.blocks = 1;
	data.flags = MMC_DATA_READ;

	err = mmc_send_cmd(mmc, &cmd, &data);
	if (err) {
		if (timeout--)
			goto retry_ssr;

		return err;
	}

	for (i = 0; i < 16; i++)
		ssr[i] = be32_to_cpu(ssr[i]);

	au = (ssr[2] >> 12) & 0xF;
	if ((au <= 9) || (mmc->version == SD_VERSION_3)) {
		mmc->ssr.au = sd_au_size[au];
		es = (ssr[3] >> 24) & 0xFF;
		es |= (ssr[2] & 0xFF) << 8;
		et = (ssr[3] >> 18) & 0x3F;
		if (es && et) {
			eo = (ssr[3] >> 16) & 0x3;
			mmc->ssr.erase_timeout = (et * 1000) / es;
			mmc->ssr.erase_offset = eo * 1000;
		}
	} else {
		pr_debug("Invalid Allocation Unit Size.\n");
	}

	return 0;
}
#endif
/* frequency bases */
/* divided by 10 to be nice to platforms without floating point */
static const int fbase[] = {
	10000,
	100000,
	1000000,
	10000000,
};

/* Multiplier values for TRAN_SPEED.  Multiplied by 10 to be nice
 * to platforms without floating point.
 */
static const u8 multipliers[] = {
	0,	/* reserved */
	10,
	12,
	13,
	15,
	20,
	25,
	30,
	35,
	40,
	45,
	50,
	55,
	60,
	70,
	80,
};

static inline int bus_width(uint cap)
{
	if (cap == MMC_MODE_8BIT)
		return 8;
	if (cap == MMC_MODE_4BIT)
		return 4;
	if (cap == MMC_MODE_1BIT)
		return 1;
	pr_warn("invalid bus witdh capability 0x%x\n", cap);
	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
#ifdef MMC_SUPPORTS_TUNING
static int mmc_execute_tuning(struct mmc *mmc, uint opcode)
{
	return -ENOTSUPP;
}
#endif

static void mmc_send_init_stream(struct mmc *mmc)
{
}

static int mmc_set_ios(struct mmc *mmc)
{
	int ret = 0;

	if (mmc->cfg->ops->set_ios)
		ret = mmc->cfg->ops->set_ios(mmc);

	return ret;
}
#endif

int mmc_set_clock(struct mmc *mmc, uint clock, bool disable)
{
	if (!disable) {
		if (clock > mmc->cfg->f_max)
			clock = mmc->cfg->f_max;

		if (clock < mmc->cfg->f_min)
			clock = mmc->cfg->f_min;
	}

	mmc->clock = clock;
	mmc->clk_disable = disable;

	debug("clock is %s (%dHz)\n", disable ? "disabled" : "enabled", clock);

	return mmc_set_ios(mmc);
}

static int mmc_set_bus_width(struct mmc *mmc, uint width)
{
	mmc->bus_width = width;

	return mmc_set_ios(mmc);
}

#if CONFIG_IS_ENABLED(MMC_VERBOSE) || defined(DEBUG)
/*
 * helper function to display the capabilities in a human
 * friendly manner. The capabilities include bus width and
 * supported modes.
 */
void mmc_dump_capabilities(const char *text, uint caps)
{
	enum bus_mode mode;

	pr_debug("%s: widths [", text);
	if (caps & MMC_MODE_8BIT)
		pr_debug("8, ");
	if (caps & MMC_MODE_4BIT)
		pr_debug("4, ");
	if (caps & MMC_MODE_1BIT)
		pr_debug("1, ");
	pr_debug("\b\b] modes [");
	for (mode = MMC_LEGACY; mode < MMC_MODES_END; mode++)
		if (MMC_CAP(mode) & caps)
			pr_debug("%s, ", mmc_mode_name(mode));
	pr_debug("\b\b]\n");
}
#endif

struct mode_width_tuning {
	enum bus_mode mode;
	uint widths;
#ifdef MMC_SUPPORTS_TUNING
	uint tuning;
#endif
};

#if CONFIG_IS_ENABLED(MMC_IO_VOLTAGE)
int mmc_voltage_to_mv(enum mmc_voltage voltage)
{
	switch (voltage) {
	case MMC_SIGNAL_VOLTAGE_000: return 0;
	case MMC_SIGNAL_VOLTAGE_330: return 3300;
	case MMC_SIGNAL_VOLTAGE_180: return 1800;
	case MMC_SIGNAL_VOLTAGE_120: return 1200;
	}
	return -EINVAL;
}

static int mmc_set_signal_voltage(struct mmc *mmc, uint signal_voltage)
{
	int err;

	if (mmc->signal_voltage == signal_voltage)
		return 0;

	mmc->signal_voltage = signal_voltage;
	err = mmc_set_ios(mmc);
	if (err)
		pr_debug("unable to set voltage (err %d)\n", err);

	return err;
}
#else
static inline int mmc_set_signal_voltage(struct mmc *mmc, uint signal_voltage)
{
	return 0;
}
#endif

#if !CONFIG_IS_ENABLED(MMC_TINY)
static const struct mode_width_tuning sd_modes_by_pref[] = {
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
#ifdef MMC_SUPPORTS_TUNING
	{
		.mode = UHS_SDR104,
		.widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
		.tuning = MMC_CMD_SEND_TUNING_BLOCK
	},
#endif
	{
		.mode = UHS_SDR50,
		.widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
	},
	{
		.mode = UHS_DDR50,
		.widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
	},
	{
		.mode = UHS_SDR25,
		.widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
	},
#endif
	{
		.mode = SD_HS,
		.widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
	},
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	{
		.mode = UHS_SDR12,
		.widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
	},
#endif
	{
		.mode = SD_LEGACY,
		.widths = MMC_MODE_4BIT | MMC_MODE_1BIT,
	}
};

#define for_each_sd_mode_by_pref(caps, mwt) \
	for (mwt = sd_modes_by_pref;\
	     mwt < sd_modes_by_pref + ARRAY_SIZE(sd_modes_by_pref);\
	     mwt++) \
		if (caps & MMC_CAP(mwt->mode))

static int sd_select_mode_and_width(struct mmc *mmc, uint card_caps)
{
	int err;
	uint widths[] = {MMC_MODE_4BIT, MMC_MODE_1BIT};
	const struct mode_width_tuning *mwt;
#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT)
	bool uhs_en = (mmc->ocr & OCR_S18R) ? true : false;
#else
	bool uhs_en = false;
#endif
	uint caps;

#ifdef DEBUG
	mmc_dump_capabilities("sd card", card_caps);
	mmc_dump_capabilities("host", mmc->host_caps);
#endif

	/* Restrict card's capabilities by what the host can do */
	caps = card_caps & mmc->host_caps;

	if (!uhs_en)
		caps &= ~UHS_CAPS;

	for_each_sd_mode_by_pref(caps, mwt) {
		uint *w;

		for (w = widths; w < widths + ARRAY_SIZE(widths); w++) {
			if (*w & caps & mwt->widths) {
				pr_debug("trying mode %s width %d (at %d MHz)\n",
					 mmc_mode_name(mwt->mode),
					 bus_width(*w),
					 mmc_mode2freq(mmc, mwt->mode) / 1000000);

				/* configure the bus width (card + host) */
				err = sd_select_bus_width(mmc, bus_width(*w));
				if (err)
					goto error;
				mmc_set_bus_width(mmc, bus_width(*w));

				/* configure the bus mode (card) */
				err = sd_set_card_speed(mmc, mwt->mode);
				if (err)
					goto error;

				/* configure the bus mode (host) */
				mmc_select_mode(mmc, mwt->mode);
				mmc_set_clock(mmc, mmc->tran_speed,
						MMC_CLK_ENABLE);

#ifdef MMC_SUPPORTS_TUNING
				/* execute tuning if needed */
				if (mwt->tuning && !mmc_host_is_spi(mmc)) {
					err = mmc_execute_tuning(mmc,
								 mwt->tuning);
					if (err) {
						pr_debug("tuning failed\n");
						goto error;
					}
				}
#endif

#if CONFIG_IS_ENABLED(MMC_WRITE)
				err = sd_read_ssr(mmc);
				if (err)
					pr_warn("unable to read ssr\n");
#endif
				if (!err)
					return 0;

error:
				/* revert to a safer bus speed */
				mmc_select_mode(mmc, SD_LEGACY);
				mmc_set_clock(mmc, mmc->tran_speed,
						MMC_CLK_ENABLE);
			}
		}
	}

	pr_err("unable to select a mode\n");
	return -ENOTSUPP;
}

/*
 * read the compare the part of ext csd that is constant.
 * This can be used to check that the transfer is working
 * as expected.
 */
static int mmc_read_and_compare_ext_csd(struct mmc *mmc)
{
	int err;
	const u8 *ext_csd = mmc->ext_csd;
	ALLOC_CACHE_ALIGN_BUFFER(u8, test_csd, MMC_MAX_BLOCK_LEN);

	if (mmc->version < MMC_VERSION_4)
		return 0;

	err = mmc_send_ext_csd(mmc, test_csd);
	if (err)
		return err;

	/* Only compare read only fields */
	if (ext_csd[EXT_CSD_PARTITIONING_SUPPORT]
		== test_csd[EXT_CSD_PARTITIONING_SUPPORT] &&
	    ext_csd[EXT_CSD_HC_WP_GRP_SIZE]
		== test_csd[EXT_CSD_HC_WP_GRP_SIZE] &&
	    ext_csd[EXT_CSD_REV]
		== test_csd[EXT_CSD_REV] &&
	    ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
		== test_csd[EXT_CSD_HC_ERASE_GRP_SIZE] &&
	    memcmp(&ext_csd[EXT_CSD_SEC_CNT],
		   &test_csd[EXT_CSD_SEC_CNT], 4) == 0)
		return 0;

	return -EBADMSG;
}

#if CONFIG_IS_ENABLED(MMC_IO_VOLTAGE)
static int mmc_set_lowest_voltage(struct mmc *mmc, enum bus_mode mode,
				  uint32_t allowed_mask)
{
	u32 card_mask = 0;

	switch (mode) {
	case MMC_HS_400:
	case MMC_HS_200:
		if (mmc->cardtype & (EXT_CSD_CARD_TYPE_HS200_1_8V |
		    EXT_CSD_CARD_TYPE_HS400_1_8V))
			card_mask |= MMC_SIGNAL_VOLTAGE_180;
		if (mmc->cardtype & (EXT_CSD_CARD_TYPE_HS200_1_2V |
		    EXT_CSD_CARD_TYPE_HS400_1_2V))
			card_mask |= MMC_SIGNAL_VOLTAGE_120;
		break;
	case MMC_DDR_52:
		if (mmc->cardtype & EXT_CSD_CARD_TYPE_DDR_1_8V)
			card_mask |= MMC_SIGNAL_VOLTAGE_330 |
				     MMC_SIGNAL_VOLTAGE_180;
		if (mmc->cardtype & EXT_CSD_CARD_TYPE_DDR_1_2V)
			card_mask |= MMC_SIGNAL_VOLTAGE_120;
		break;
	default:
		card_mask |= MMC_SIGNAL_VOLTAGE_330;
		break;
	}

	while (card_mask & allowed_mask) {
		enum mmc_voltage best_match;

		best_match = 1 << (ffs(card_mask & allowed_mask) - 1);
		if (!mmc_set_signal_voltage(mmc,  best_match))
			return 0;

		allowed_mask &= ~best_match;
	}

	return -ENOTSUPP;
}
#else
static inline int mmc_set_lowest_voltage(struct mmc *mmc, enum bus_mode mode,
					 uint32_t allowed_mask)
{
	return 0;
}
#endif

static const struct mode_width_tuning mmc_modes_by_pref[] = {
#if CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	{
		.mode = MMC_HS_400,
		.widths = MMC_MODE_8BIT,
		.tuning = MMC_CMD_SEND_TUNING_BLOCK_HS200
	},
#endif
#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT)
	{
		.mode = MMC_HS_200,
		.widths = MMC_MODE_8BIT | MMC_MODE_4BIT,
		.tuning = MMC_CMD_SEND_TUNING_BLOCK_HS200
	},
#endif
	{
		.mode = MMC_DDR_52,
		.widths = MMC_MODE_8BIT | MMC_MODE_4BIT,
	},
	{
		.mode = MMC_HS_52,
		.widths = MMC_MODE_8BIT | MMC_MODE_4BIT | MMC_MODE_1BIT,
	},
	{
		.mode = MMC_HS,
		.widths = MMC_MODE_8BIT | MMC_MODE_4BIT | MMC_MODE_1BIT,
	},
	{
		.mode = MMC_LEGACY,
		.widths = MMC_MODE_8BIT | MMC_MODE_4BIT | MMC_MODE_1BIT,
	}
};

#define for_each_mmc_mode_by_pref(caps, mwt) \
	for (mwt = mmc_modes_by_pref;\
	    mwt < mmc_modes_by_pref + ARRAY_SIZE(mmc_modes_by_pref);\
	    mwt++) \
		if (caps & MMC_CAP(mwt->mode))

static const struct ext_csd_bus_width {
	uint cap;
	bool is_ddr;
	uint ext_csd_bits;
} ext_csd_bus_width[] = {
	{MMC_MODE_8BIT, true, EXT_CSD_DDR_BUS_WIDTH_8},
	{MMC_MODE_4BIT, true, EXT_CSD_DDR_BUS_WIDTH_4},
	{MMC_MODE_8BIT, false, EXT_CSD_BUS_WIDTH_8},
	{MMC_MODE_4BIT, false, EXT_CSD_BUS_WIDTH_4},
	{MMC_MODE_1BIT, false, EXT_CSD_BUS_WIDTH_1},
};

#if CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
static int mmc_select_hs400(struct mmc *mmc)
{
	int err;

	/* Set timing to HS200 for tuning */
	err = mmc_set_card_speed(mmc, MMC_HS_200, false);
	if (err)
		return err;

	/* configure the bus mode (host) */
	mmc_select_mode(mmc, MMC_HS_200);
	mmc_set_clock(mmc, mmc->tran_speed, false);

	/* execute tuning if needed */
	err = mmc_execute_tuning(mmc, MMC_CMD_SEND_TUNING_BLOCK_HS200);
	if (err) {
		debug("tuning failed\n");
		return err;
	}

	/* Set back to HS */
	mmc_set_card_speed(mmc, MMC_HS, true);

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BUS_WIDTH,
			 EXT_CSD_BUS_WIDTH_8 | EXT_CSD_DDR_FLAG);
	if (err)
		return err;

	err = mmc_set_card_speed(mmc, MMC_HS_400, false);
	if (err)
		return err;

	mmc_select_mode(mmc, MMC_HS_400);
	err = mmc_set_clock(mmc, mmc->tran_speed, false);
	if (err)
		return err;

	return 0;
}
#else
static int mmc_select_hs400(struct mmc *mmc)
{
	return -ENOTSUPP;
}
#endif

#define for_each_supported_width(caps, ddr, ecbv) \
	for (ecbv = ext_csd_bus_width;\
	    ecbv < ext_csd_bus_width + ARRAY_SIZE(ext_csd_bus_width);\
	    ecbv++) \
		if ((ddr == ecbv->is_ddr) && (caps & ecbv->cap))

static int mmc_select_mode_and_width(struct mmc *mmc, uint card_caps)
{
	int err;
	const struct mode_width_tuning *mwt;
	const struct ext_csd_bus_width *ecbw;

#ifdef DEBUG
	mmc_dump_capabilities("mmc", card_caps);
	mmc_dump_capabilities("host", mmc->host_caps);
#endif

	/* Restrict card's capabilities by what the host can do */
	card_caps &= mmc->host_caps;

	/* Only version 4 of MMC supports wider bus widths */
	if (mmc->version < MMC_VERSION_4)
		return 0;

	if (!mmc->ext_csd) {
		pr_debug("No ext_csd found!\n"); /* this should enver happen */
		return -ENOTSUPP;
	}

#if CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
	/*
	 * In case the eMMC is in HS200/HS400 mode, downgrade to HS mode
	 * before doing anything else, since a transition from either of
	 * the HS200/HS400 mode directly to legacy mode is not supported.
	 */
	if (mmc->selected_mode == MMC_HS_200 ||
	    mmc->selected_mode == MMC_HS_400)
		mmc_set_card_speed(mmc, MMC_HS, true);
	else
#endif
		mmc_set_clock(mmc, mmc->legacy_speed, MMC_CLK_ENABLE);

	for_each_mmc_mode_by_pref(card_caps, mwt) {
		for_each_supported_width(card_caps & mwt->widths,
					 mmc_is_mode_ddr(mwt->mode), ecbw) {
			enum mmc_voltage old_voltage;
			pr_debug("trying mode %s width %d (at %d MHz)\n",
				 mmc_mode_name(mwt->mode),
				 bus_width(ecbw->cap),
				 mmc_mode2freq(mmc, mwt->mode) / 1000000);
			old_voltage = mmc->signal_voltage;
			err = mmc_set_lowest_voltage(mmc, mwt->mode,
						     MMC_ALL_SIGNAL_VOLTAGE);
			if (err)
				continue;

			/* configure the bus width (card + host) */
			err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				    EXT_CSD_BUS_WIDTH,
				    ecbw->ext_csd_bits & ~EXT_CSD_DDR_FLAG);
			if (err)
				goto error;
			mmc_set_bus_width(mmc, bus_width(ecbw->cap));

			if (mwt->mode == MMC_HS_400) {
				err = mmc_select_hs400(mmc);
				if (err) {
					printf("Select HS400 failed %d\n", err);
					goto error;
				}
			} else {
				/* configure the bus speed (card) */
				err = mmc_set_card_speed(mmc, mwt->mode, false);
				if (err)
					goto error;

				/*
				 * configure the bus width AND the ddr mode
				 * (card). The host side will be taken care
				 * of in the next step
				 */
				if (ecbw->ext_csd_bits & EXT_CSD_DDR_FLAG) {
					err = mmc_switch(mmc,
							 EXT_CSD_CMD_SET_NORMAL,
							 EXT_CSD_BUS_WIDTH,
							 ecbw->ext_csd_bits);
					if (err)
						goto error;
				}

				/* configure the bus mode (host) */
				mmc_select_mode(mmc, mwt->mode);
				mmc_set_clock(mmc, mmc->tran_speed,
					      MMC_CLK_ENABLE);
#ifdef MMC_SUPPORTS_TUNING

				/* execute tuning if needed */
				if (mwt->tuning) {
					err = mmc_execute_tuning(mmc,
								 mwt->tuning);
					if (err) {
						pr_debug("tuning failed\n");
						goto error;
					}
				}
#endif
			}

			/* do a transfer to check the configuration */
			err = mmc_read_and_compare_ext_csd(mmc);
			if (!err)
				return 0;
error:
			mmc_set_signal_voltage(mmc, old_voltage);
			/* if an error occured, revert to a safer bus mode */
			mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				   EXT_CSD_BUS_WIDTH, EXT_CSD_BUS_WIDTH_1);
			mmc_select_mode(mmc, MMC_LEGACY);
			mmc_set_bus_width(mmc, 1);
		}
	}

	pr_err("unable to select a mode\n");

	return -ENOTSUPP;
}
#endif

#if CONFIG_IS_ENABLED(MMC_TINY)
DEFINE_CACHE_ALIGN_BUFFER(u8, ext_csd_bkup, MMC_MAX_BLOCK_LEN);
#endif

static int mmc_startup_v4(struct mmc *mmc)
{
	int err, i;
	u64 capacity;
	bool has_parts = false;
	bool part_completed;
	static const u32 mmc_versions[] = {
		MMC_VERSION_4,
		MMC_VERSION_4_1,
		MMC_VERSION_4_2,
		MMC_VERSION_4_3,
		MMC_VERSION_4_4,
		MMC_VERSION_4_41,
		MMC_VERSION_4_5,
		MMC_VERSION_5_0,
		MMC_VERSION_5_1
	};

#if CONFIG_IS_ENABLED(MMC_TINY)
	u8 *ext_csd = ext_csd_bkup;

	if (IS_SD(mmc) || mmc->version < MMC_VERSION_4)
		return 0;

	if (!mmc->ext_csd)
		memset(ext_csd_bkup, 0, sizeof(ext_csd_bkup));

	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err)
		goto error;

	/* store the ext csd for future reference */
	if (!mmc->ext_csd)
		mmc->ext_csd = ext_csd;
#else
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);

	if (IS_SD(mmc) || (mmc->version < MMC_VERSION_4))
		return 0;

	/* check  ext_csd version and capacity */
	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err)
		goto error;

	/* store the ext csd for future reference */
	if (!mmc->ext_csd)
		mmc->ext_csd = malloc(MMC_MAX_BLOCK_LEN);
	if (!mmc->ext_csd)
		return -ENOMEM;
	memcpy(mmc->ext_csd, ext_csd, MMC_MAX_BLOCK_LEN);
#endif
	if (ext_csd[EXT_CSD_REV] >= ARRAY_SIZE(mmc_versions))
		return -EINVAL;

	mmc->version = mmc_versions[ext_csd[EXT_CSD_REV]];

	if (mmc->version >= MMC_VERSION_4_2) {
		/*
		 * According to the JEDEC Standard, the value of
		 * ext_csd's capacity is valid if the value is more
		 * than 2GB
		 */
		capacity = ext_csd[EXT_CSD_SEC_CNT] << 0
				| ext_csd[EXT_CSD_SEC_CNT + 1] << 8
				| ext_csd[EXT_CSD_SEC_CNT + 2] << 16
				| ext_csd[EXT_CSD_SEC_CNT + 3] << 24;
		capacity *= MMC_MAX_BLOCK_LEN;
		if ((capacity >> 20) > 2 * 1024)
			mmc->capacity_user = capacity;
	}

	/* The partition data may be non-zero but it is only
	 * effective if PARTITION_SETTING_COMPLETED is set in
	 * EXT_CSD, so ignore any data if this bit is not set,
	 * except for enabling the high-capacity group size
	 * definition (see below).
	 */
	part_completed = !!(ext_csd[EXT_CSD_PARTITION_SETTING] &
			    EXT_CSD_PARTITION_SETTING_COMPLETED);

	/* store the partition info of emmc */
	mmc->part_support = ext_csd[EXT_CSD_PARTITIONING_SUPPORT];
	if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) ||
	    ext_csd[EXT_CSD_BOOT_MULT])
		mmc->part_config = ext_csd[EXT_CSD_PART_CONF];
	if (part_completed &&
	    (ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & ENHNCD_SUPPORT))
		mmc->part_attr = ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE];

	mmc->capacity_boot = ext_csd[EXT_CSD_BOOT_MULT] << 17;

	mmc->capacity_rpmb = ext_csd[EXT_CSD_RPMB_MULT] << 17;

	for (i = 0; i < 4; i++) {
		int idx = EXT_CSD_GP_SIZE_MULT + i * 3;
		uint mult = (ext_csd[idx + 2] << 16) +
			(ext_csd[idx + 1] << 8) + ext_csd[idx];
		if (mult)
			has_parts = true;
		if (!part_completed)
			continue;
		mmc->capacity_gp[i] = mult;
		mmc->capacity_gp[i] *=
			ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
		mmc->capacity_gp[i] *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
		mmc->capacity_gp[i] <<= 19;
	}

#ifndef CONFIG_SPL_BUILD
	if (part_completed) {
		mmc->enh_user_size =
			(ext_csd[EXT_CSD_ENH_SIZE_MULT + 2] << 16) +
			(ext_csd[EXT_CSD_ENH_SIZE_MULT + 1] << 8) +
			ext_csd[EXT_CSD_ENH_SIZE_MULT];
		mmc->enh_user_size *= ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE];
		mmc->enh_user_size *= ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
		mmc->enh_user_size <<= 19;
		mmc->enh_user_start =
			(ext_csd[EXT_CSD_ENH_START_ADDR + 3] << 24) +
			(ext_csd[EXT_CSD_ENH_START_ADDR + 2] << 16) +
			(ext_csd[EXT_CSD_ENH_START_ADDR + 1] << 8) +
			ext_csd[EXT_CSD_ENH_START_ADDR];
		if (mmc->high_capacity)
			mmc->enh_user_start <<= 9;
	}
#endif

	/*
	 * Host needs to enable ERASE_GRP_DEF bit if device is
	 * partitioned. This bit will be lost every time after a reset
	 * or power off. This will affect erase size.
	 */
	if (part_completed)
		has_parts = true;
	if ((ext_csd[EXT_CSD_PARTITIONING_SUPPORT] & PART_SUPPORT) &&
	    (ext_csd[EXT_CSD_PARTITIONS_ATTRIBUTE] & PART_ENH_ATTRIB))
		has_parts = true;
	if (has_parts) {
		err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL,
				 EXT_CSD_ERASE_GROUP_DEF, 1);

		if (err)
			goto error;

		ext_csd[EXT_CSD_ERASE_GROUP_DEF] = 1;
	}

	if (ext_csd[EXT_CSD_ERASE_GROUP_DEF] & 0x01) {
#if CONFIG_IS_ENABLED(MMC_WRITE)
		/* Read out group size from ext_csd */
		mmc->erase_grp_size =
			ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE] * 1024;
#endif
		/*
		 * if high capacity and partition setting completed
		 * SEC_COUNT is valid even if it is smaller than 2 GiB
		 * JEDEC Standard JESD84-B45, 6.2.4
		 */
		if (mmc->high_capacity && part_completed) {
			capacity = (ext_csd[EXT_CSD_SEC_CNT]) |
				(ext_csd[EXT_CSD_SEC_CNT + 1] << 8) |
				(ext_csd[EXT_CSD_SEC_CNT + 2] << 16) |
				(ext_csd[EXT_CSD_SEC_CNT + 3] << 24);
			capacity *= MMC_MAX_BLOCK_LEN;
			mmc->capacity_user = capacity;
		}
	}
#if CONFIG_IS_ENABLED(MMC_WRITE)
	else {
		/* Calculate the group size from the csd value. */
		int erase_gsz, erase_gmul;

		erase_gsz = (mmc->csd[2] & 0x00007c00) >> 10;
		erase_gmul = (mmc->csd[2] & 0x000003e0) >> 5;
		mmc->erase_grp_size = (erase_gsz + 1)
			* (erase_gmul + 1);
	}
#endif
#if CONFIG_IS_ENABLED(MMC_HW_PARTITIONING)
	mmc->hc_wp_grp_size = 1024
		* ext_csd[EXT_CSD_HC_ERASE_GRP_SIZE]
		* ext_csd[EXT_CSD_HC_WP_GRP_SIZE];
#endif

	mmc->wr_rel_set = ext_csd[EXT_CSD_WR_REL_SET];

	return 0;
error:
	if (mmc->ext_csd) {
#if !CONFIG_IS_ENABLED(MMC_TINY)
		free(mmc->ext_csd);
#endif
		mmc->ext_csd = NULL;
	}
	return err;
}

static int mmc_startup(struct mmc *mmc)
{
	int err, i;
	uint mult, freq;
	u64 cmult, csize;
	struct mmc_cmd cmd;
	struct blk_desc *bdesc;

#ifdef CONFIG_MMC_SPI_CRC_ON
	if (mmc_host_is_spi(mmc)) { /* enable CRC check for spi */
		cmd.cmdidx = MMC_CMD_SPI_CRC_ON_OFF;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = 1;
		err = mmc_send_cmd(mmc, &cmd, NULL);
		if (err)
			return err;
	}
#endif

	/* Put the Card in Identify Mode */
	cmd.cmdidx = mmc_host_is_spi(mmc) ? MMC_CMD_SEND_CID :
		MMC_CMD_ALL_SEND_CID; /* cmd not supported in spi */
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = 0;

	err = mmc_send_cmd(mmc, &cmd, NULL);

#ifdef CONFIG_MMC_QUIRKS
	if (err && (mmc->quirks & MMC_QUIRK_RETRY_SEND_CID)) {
		int retries = 4;
		/*
		 * It has been seen that SEND_CID may fail on the first
		 * attempt, let's try a few more time
		 */
		do {
			err = mmc_send_cmd(mmc, &cmd, NULL);
			if (!err)
				break;
		} while (retries--);
	}
#endif

	if (err)
		return err;

	memcpy(mmc->cid, cmd.response, 16);

	/*
	 * For MMC cards, set the Relative Address.
	 * For SD cards, get the Relatvie Address.
	 * This also puts the cards into Standby State
	 */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = SD_CMD_SEND_RELATIVE_ADDR;
		cmd.cmdarg = mmc->rca << 16;
		cmd.resp_type = MMC_RSP_R6;

		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;

		if (IS_SD(mmc))
			mmc->rca = (cmd.response[0] >> 16) & 0xffff;
	}

	/* Get the Card-Specific Data */
	cmd.cmdidx = MMC_CMD_SEND_CSD;
	cmd.resp_type = MMC_RSP_R2;
	cmd.cmdarg = mmc->rca << 16;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	mmc->csd[0] = cmd.response[0];
	mmc->csd[1] = cmd.response[1];
	mmc->csd[2] = cmd.response[2];
	mmc->csd[3] = cmd.response[3];

	if (mmc->version == MMC_VERSION_UNKNOWN) {
		int version = (cmd.response[0] >> 26) & 0xf;

		switch (version) {
		case 0:
			mmc->version = MMC_VERSION_1_2;
			break;
		case 1:
			mmc->version = MMC_VERSION_1_4;
			break;
		case 2:
			mmc->version = MMC_VERSION_2_2;
			break;
		case 3:
			mmc->version = MMC_VERSION_3;
			break;
		case 4:
			mmc->version = MMC_VERSION_4;
			break;
		default:
			mmc->version = MMC_VERSION_1_2;
			break;
		}
	}

	/* divide frequency by 10, since the mults are 10x bigger */
	freq = fbase[(cmd.response[0] & 0x7)];
	mult = multipliers[((cmd.response[0] >> 3) & 0xf)];

	mmc->legacy_speed = freq * mult;
	mmc_select_mode(mmc, MMC_LEGACY);

	mmc->dsr_imp = ((cmd.response[1] >> 12) & 0x1);
	mmc->read_bl_len = 1 << ((cmd.response[1] >> 16) & 0xf);
#if CONFIG_IS_ENABLED(MMC_WRITE)

	if (IS_SD(mmc))
		mmc->write_bl_len = mmc->read_bl_len;
	else
		mmc->write_bl_len = 1 << ((cmd.response[3] >> 22) & 0xf);
#endif

	if (mmc->high_capacity) {
		csize = (mmc->csd[1] & 0x3f) << 16
			| (mmc->csd[2] & 0xffff0000) >> 16;
		cmult = 8;
	} else {
		csize = (mmc->csd[1] & 0x3ff) << 2
			| (mmc->csd[2] & 0xc0000000) >> 30;
		cmult = (mmc->csd[2] & 0x00038000) >> 15;
	}

	mmc->capacity_user = (csize + 1) << (cmult + 2);
	mmc->capacity_user *= mmc->read_bl_len;
	mmc->capacity_boot = 0;
	mmc->capacity_rpmb = 0;
	for (i = 0; i < 4; i++)
		mmc->capacity_gp[i] = 0;

	if (mmc->read_bl_len > MMC_MAX_BLOCK_LEN)
		mmc->read_bl_len = MMC_MAX_BLOCK_LEN;

#if CONFIG_IS_ENABLED(MMC_WRITE)
	if (mmc->write_bl_len > MMC_MAX_BLOCK_LEN)
		mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
#endif

	if ((mmc->dsr_imp) && (0xffffffff != mmc->dsr)) {
		cmd.cmdidx = MMC_CMD_SET_DSR;
		cmd.cmdarg = (mmc->dsr & 0xffff) << 16;
		cmd.resp_type = MMC_RSP_NONE;
		if (mmc_send_cmd(mmc, &cmd, NULL))
			pr_warn("MMC: SET_DSR failed\n");
	}

	/* Select the card, and put it into Transfer Mode */
	if (!mmc_host_is_spi(mmc)) { /* cmd not supported in spi */
		cmd.cmdidx = MMC_CMD_SELECT_CARD;
		cmd.resp_type = MMC_RSP_R1;
		cmd.cmdarg = mmc->rca << 16;
		err = mmc_send_cmd(mmc, &cmd, NULL);

		if (err)
			return err;
	}

	/*
	 * For SD, its erase group is always one sector
	 */
#if CONFIG_IS_ENABLED(MMC_WRITE)
	mmc->erase_grp_size = 1;
#endif
	mmc->part_config = MMCPART_NOAVAILABLE;

	err = mmc_startup_v4(mmc);
	if (err)
		return err;

	err = mmc_set_capacity(mmc, mmc_get_blk_desc(mmc)->hwpart);
	if (err)
		return err;

#if CONFIG_IS_ENABLED(MMC_TINY)
	mmc_set_clock(mmc, mmc->legacy_speed, false);
	mmc_select_mode(mmc, IS_SD(mmc) ? SD_LEGACY : MMC_LEGACY);
	mmc_set_bus_width(mmc, 1);
#else
	if (IS_SD(mmc)) {
		err = sd_get_capabilities(mmc);
		if (err)
			return err;
		err = sd_select_mode_and_width(mmc, mmc->card_caps);
	} else {
		err = mmc_get_capabilities(mmc);
		if (err)
			return err;
		mmc_select_mode_and_width(mmc, mmc->card_caps);
	}
#endif
	if (err)
		return err;

	mmc->best_mode = mmc->selected_mode;

	/* Fix the block length for DDR mode */
	if (mmc->ddr_mode) {
		mmc->read_bl_len = MMC_MAX_BLOCK_LEN;
#if CONFIG_IS_ENABLED(MMC_WRITE)
		mmc->write_bl_len = MMC_MAX_BLOCK_LEN;
#endif
	}

	/* fill in device description */
	bdesc = mmc_get_blk_desc(mmc);
	bdesc->lun = 0;
	bdesc->hwpart = 0;
	bdesc->type = 0;
	bdesc->blksz = mmc->read_bl_len;
	bdesc->log2blksz = LOG2(bdesc->blksz);
	bdesc->lba = lldiv(mmc->capacity, mmc->read_bl_len);
#if !defined(CONFIG_SPL_BUILD) || \
		(defined(CONFIG_SPL_LIBCOMMON_SUPPORT) && \
		!defined(CONFIG_USE_TINY_PRINTF))
	sprintf(bdesc->vendor, "Man %06x Snr %04x%04x",
		mmc->cid[0] >> 24, (mmc->cid[2] & 0xffff),
		(mmc->cid[3] >> 16) & 0xffff);
	sprintf(bdesc->product, "%c%c%c%c%c%c", mmc->cid[0] & 0xff,
		(mmc->cid[1] >> 24), (mmc->cid[1] >> 16) & 0xff,
		(mmc->cid[1] >> 8) & 0xff, mmc->cid[1] & 0xff,
		(mmc->cid[2] >> 24) & 0xff);
	sprintf(bdesc->revision, "%d.%d", (mmc->cid[2] >> 20) & 0xf,
		(mmc->cid[2] >> 16) & 0xf);
#else
	bdesc->vendor[0] = 0;
	bdesc->product[0] = 0;
	bdesc->revision[0] = 0;
#endif

#if !defined(CONFIG_DM_MMC) && (!defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBDISK_SUPPORT))
	part_init(bdesc);
#endif

	return 0;
}

static int mmc_send_if_cond(struct mmc *mmc)
{
	struct mmc_cmd cmd;
	int err;

	cmd.cmdidx = SD_CMD_SEND_IF_COND;
	/* We set the bit if the host supports voltages between 2.7 and 3.6 V */
	cmd.cmdarg = ((mmc->cfg->voltages & 0xff8000) != 0) << 8 | 0xaa;
	cmd.resp_type = MMC_RSP_R7;

	err = mmc_send_cmd(mmc, &cmd, NULL);

	if (err)
		return err;

	if ((cmd.response[0] & 0xff) != 0xaa)
		return -EOPNOTSUPP;
	else
		mmc->version = SD_VERSION_2;

	return 0;
}

#if !CONFIG_IS_ENABLED(DM_MMC)
/* board-specific MMC power initializations. */
__weak void board_mmc_power_init(void)
{
}
#endif

static int mmc_power_init(struct mmc *mmc)
{
#if CONFIG_IS_ENABLED(DM_MMC)
#if CONFIG_IS_ENABLED(DM_REGULATOR)
	int ret;

	ret = device_get_supply_regulator(mmc->dev, "vmmc-supply",
					  &mmc->vmmc_supply);
	if (ret)
		pr_debug("%s: No vmmc supply\n", mmc->dev->name);

	ret = device_get_supply_regulator(mmc->dev, "vqmmc-supply",
					  &mmc->vqmmc_supply);
	if (ret)
		pr_debug("%s: No vqmmc supply\n", mmc->dev->name);
#endif
#else /* !CONFIG_DM_MMC */
	/*
	 * Driver model should use a regulator, as above, rather than calling
	 * out to board code.
	 */
	board_mmc_power_init();
#endif
	return 0;
}

/*
 * put the host in the initial state:
 * - turn on Vdd (card power supply)
 * - configure the bus width and clock to minimal values
 */
static void mmc_set_initial_state(struct mmc *mmc)
{
	int err;

	/* First try to set 3.3V. If it fails set to 1.8V */
	err = mmc_set_signal_voltage(mmc, MMC_SIGNAL_VOLTAGE_330);
	if (err != 0)
		err = mmc_set_signal_voltage(mmc, MMC_SIGNAL_VOLTAGE_180);
	if (err != 0)
		pr_warn("mmc: failed to set signal voltage\n");

	mmc_select_mode(mmc, MMC_LEGACY);
	mmc_set_bus_width(mmc, 1);
	mmc_set_clock(mmc, 0, MMC_CLK_ENABLE);
}

static int mmc_power_on(struct mmc *mmc)
{
#if CONFIG_IS_ENABLED(DM_MMC) && CONFIG_IS_ENABLED(DM_REGULATOR)
	if (mmc->vmmc_supply) {
		int ret = regulator_set_enable(mmc->vmmc_supply, true);

		if (ret) {
			puts("Error enabling VMMC supply\n");
			return ret;
		}
	}
#endif
	return 0;
}

static int mmc_power_off(struct mmc *mmc)
{
	mmc_set_clock(mmc, 0, MMC_CLK_DISABLE);
#if CONFIG_IS_ENABLED(DM_MMC) && CONFIG_IS_ENABLED(DM_REGULATOR)
	if (mmc->vmmc_supply) {
		int ret = regulator_set_enable(mmc->vmmc_supply, false);

		if (ret) {
			pr_debug("Error disabling VMMC supply\n");
			return ret;
		}
	}
#endif
	return 0;
}

static int mmc_power_cycle(struct mmc *mmc)
{
	int ret;

	ret = mmc_power_off(mmc);
	if (ret)
		return ret;
	/*
	 * SD spec recommends at least 1ms of delay. Let's wait for 2ms
	 * to be on the safer side.
	 */
	udelay(2000);
	return mmc_power_on(mmc);
}

int mmc_get_op_cond(struct mmc *mmc)
{
	bool uhs_en = supports_uhs(mmc->cfg->host_caps);
	int err;

	if (mmc->has_init)
		return 0;

#ifdef CONFIG_FSL_ESDHC_ADAPTER_IDENT
	mmc_adapter_card_type_ident();
#endif
	err = mmc_power_init(mmc);
	if (err)
		return err;

#ifdef CONFIG_MMC_QUIRKS
	mmc->quirks = MMC_QUIRK_RETRY_SET_BLOCKLEN |
		      MMC_QUIRK_RETRY_SEND_CID;
#endif

	err = mmc_power_cycle(mmc);
	if (err) {
		/*
		 * if power cycling is not supported, we should not try
		 * to use the UHS modes, because we wouldn't be able to
		 * recover from an error during the UHS initialization.
		 */
		pr_debug("Unable to do a full power cycle. Disabling the UHS modes for safety\n");
		uhs_en = false;
		mmc->host_caps &= ~UHS_CAPS;
		err = mmc_power_on(mmc);
	}
	if (err)
		return err;

#if CONFIG_IS_ENABLED(DM_MMC)
	/* The device has already been probed ready for use */
#else
	/* made sure it's not NULL earlier */
	err = mmc->cfg->ops->init(mmc);
	if (err)
		return err;
#endif
	mmc->ddr_mode = 0;

retry:
	mmc_set_initial_state(mmc);
	mmc_send_init_stream(mmc);

	/* Reset the Card */
	err = mmc_go_idle(mmc);

	if (err)
		return err;

	/* The internal partition reset to user partition(0) at every CMD0*/
	mmc_get_blk_desc(mmc)->hwpart = 0;

	/* Test for SD version 2 */
	err = mmc_send_if_cond(mmc);

	/* Now try to get the SD card's operating condition */
	err = sd_send_op_cond(mmc, uhs_en);
	if (err && uhs_en) {
		uhs_en = false;
		mmc_power_cycle(mmc);
		goto retry;
	}

	/* If the command timed out, we check for an MMC card */
	if (err == -ETIMEDOUT) {
		err = mmc_send_op_cond(mmc);

		if (err) {
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
			pr_err("Card did not respond to voltage select!\n");
#endif
			return -EOPNOTSUPP;
		}
	}

	return err;
}

int mmc_start_init(struct mmc *mmc)
{
	bool no_card;
	int err = 0;

	/*
	 * all hosts are capable of 1 bit bus-width and able to use the legacy
	 * timings.
	 */
	mmc->host_caps = mmc->cfg->host_caps | MMC_CAP(SD_LEGACY) |
			 MMC_CAP(MMC_LEGACY) | MMC_MODE_1BIT;

#if !defined(CONFIG_MMC_BROKEN_CD)
	/* we pretend there's no card when init is NULL */
	no_card = mmc_getcd(mmc) == 0;
#else
	no_card = 0;
#endif
#if !CONFIG_IS_ENABLED(DM_MMC)
	no_card = no_card || (mmc->cfg->ops->init == NULL);
#endif
	if (no_card) {
		mmc->has_init = 0;
#if !defined(CONFIG_SPL_BUILD) || defined(CONFIG_SPL_LIBCOMMON_SUPPORT)
		pr_err("MMC: no card present\n");
#endif
		return -ENOMEDIUM;
	}

	err = mmc_get_op_cond(mmc);

	if (!err)
		mmc->init_in_progress = 1;

	return err;
}

static int mmc_complete_init(struct mmc *mmc)
{
	int err = 0;

	mmc->init_in_progress = 0;
	if (mmc->op_cond_pending)
		err = mmc_complete_op_cond(mmc);

	if (!err)
		err = mmc_startup(mmc);
	if (err)
		mmc->has_init = 0;
	else
		mmc->has_init = 1;
	return err;
}

int mmc_init(struct mmc *mmc)
{
	int err = 0;
	__maybe_unused ulong start;
#if CONFIG_IS_ENABLED(DM_MMC)
	struct mmc_uclass_priv *upriv = dev_get_uclass_priv(mmc->dev);

	upriv->mmc = mmc;
#endif
	if (mmc->has_init)
		return 0;

	start = get_timer(0);

	if (!mmc->init_in_progress)
		err = mmc_start_init(mmc);

	if (!err)
		err = mmc_complete_init(mmc);
	if (err)
		pr_info("%s: %d, time %lu\n", __func__, err, get_timer(start));

	return err;
}

#if CONFIG_IS_ENABLED(MMC_UHS_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS200_SUPPORT) || \
    CONFIG_IS_ENABLED(MMC_HS400_SUPPORT)
int mmc_deinit(struct mmc *mmc)
{
	u32 caps_filtered;

	if (!mmc->has_init)
		return 0;

	if (IS_SD(mmc)) {
		caps_filtered = mmc->card_caps &
			~(MMC_CAP(UHS_SDR12) | MMC_CAP(UHS_SDR25) |
			  MMC_CAP(UHS_SDR50) | MMC_CAP(UHS_DDR50) |
			  MMC_CAP(UHS_SDR104));

		return sd_select_mode_and_width(mmc, caps_filtered);
	} else {
		caps_filtered = mmc->card_caps &
			~(MMC_CAP(MMC_HS_200) | MMC_CAP(MMC_HS_400));

		return mmc_select_mode_and_width(mmc, caps_filtered);
	}
}
#endif

int mmc_set_dsr(struct mmc *mmc, u16 val)
{
	mmc->dsr = val;
	return 0;
}

/* CPU-specific MMC initializations */
__weak int cpu_mmc_init(bd_t *bis)
{
	return -1;
}

/* board-specific MMC initializations. */
__weak int board_mmc_init(bd_t *bis)
{
	return -1;
}

void mmc_set_preinit(struct mmc *mmc, int preinit)
{
	mmc->preinit = preinit;
}

#if CONFIG_IS_ENABLED(DM_MMC)
static int mmc_probe(bd_t *bis)
{
	int ret, i;
	struct uclass *uc;
	struct udevice *dev;

	ret = uclass_get(UCLASS_MMC, &uc);
	if (ret)
		return ret;

	/*
	 * Try to add them in sequence order. Really with driver model we
	 * should allow holes, but the current MMC list does not allow that.
	 * So if we request 0, 1, 3 we will get 0, 1, 2.
	 */
	for (i = 0; ; i++) {
		ret = uclass_get_device_by_seq(UCLASS_MMC, i, &dev);
		if (ret == -ENODEV)
			break;
	}
	uclass_foreach_dev(dev, uc) {
		ret = device_probe(dev);
		if (ret)
			pr_err("%s - probe failed: %d\n", dev->name, ret);
	}

	return 0;
}
#else
static int mmc_probe(bd_t *bis)
{
	if (board_mmc_init(bis) < 0)
		cpu_mmc_init(bis);

	return 0;
}
#endif

int mmc_initialize(bd_t *bis)
{
	static int initialized = 0;
	int ret;
	if (initialized)	/* Avoid initializing mmc multiple times */
		return 0;
	initialized = 1;

#if !CONFIG_IS_ENABLED(BLK)
#if !CONFIG_IS_ENABLED(MMC_TINY)
	mmc_list_init();
#endif
#endif
	ret = mmc_probe(bis);
	if (ret)
		return ret;

#ifndef CONFIG_SPL_BUILD
	print_mmc_devices(',');
#endif

	mmc_do_preinit();
	return 0;
}

#ifdef CONFIG_CMD_BKOPS_ENABLE
int mmc_set_bkops_enable(struct mmc *mmc)
{
	int err;
	ALLOC_CACHE_ALIGN_BUFFER(u8, ext_csd, MMC_MAX_BLOCK_LEN);

	err = mmc_send_ext_csd(mmc, ext_csd);
	if (err) {
		puts("Could not get ext_csd register values\n");
		return err;
	}

	if (!(ext_csd[EXT_CSD_BKOPS_SUPPORT] & 0x1)) {
		puts("Background operations not supported on device\n");
		return -EMEDIUMTYPE;
	}

	if (ext_csd[EXT_CSD_BKOPS_EN] & 0x1) {
		puts("Background operations already enabled\n");
		return 0;
	}

	err = mmc_switch(mmc, EXT_CSD_CMD_SET_NORMAL, EXT_CSD_BKOPS_EN, 1);
	if (err) {
		puts("Failed to enable manual background operations\n");
		return err;
	}

	puts("Enabled manual background operations\n");

	return 0;
}
#endif
