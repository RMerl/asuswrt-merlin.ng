// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2018 Intel Corporation <www.intel.com>
 */

#include <common.h>
#include <altera.h>
#include <log.h>
#include <watchdog.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/smc_api.h>
#include <linux/delay.h>
#include <linux/intel-smc.h>

#define RECONFIG_STATUS_POLL_RESP_TIMEOUT_MS		60000
#define RECONFIG_STATUS_INTERVAL_DELAY_US		1000000

#if !defined(CONFIG_SPL_BUILD) && defined(CONFIG_SPL_ATF)

#define BITSTREAM_CHUNK_SIZE				0xFFFF0
#define RECONFIG_STATUS_POLL_RETRY_MAX			100

/*
 * Polling the FPGA configuration status.
 * Return 0 for success, non-zero for error.
 */
static int reconfig_status_polling_resp(void)
{
	int ret;
	unsigned long start = get_timer(0);

	while (1) {
		ret = invoke_smc(INTEL_SIP_SMC_FPGA_CONFIG_ISDONE, NULL, 0,
				 NULL, 0);

		if (!ret)
			return 0;	/* configuration success */

		if (ret != INTEL_SIP_SMC_STATUS_BUSY)
			return ret;

		if (get_timer(start) > RECONFIG_STATUS_POLL_RESP_TIMEOUT_MS)
			return -ETIMEDOUT;	/* time out */

		puts(".");
		udelay(RECONFIG_STATUS_INTERVAL_DELAY_US);
		schedule();
	}

	return -ETIMEDOUT;
}

static int send_bitstream(const void *rbf_data, size_t rbf_size)
{
	int i;
	u64 res_buf[3];
	u64 args[2];
	u32 xfer_count = 0;
	int ret, wr_ret = 0, retry = 0;
	size_t buf_size = (rbf_size > BITSTREAM_CHUNK_SIZE) ?
				BITSTREAM_CHUNK_SIZE : rbf_size;

	while (rbf_size || xfer_count) {
		if (!wr_ret && rbf_size) {
			args[0] = (u64)rbf_data;
			args[1] = buf_size;
			wr_ret = invoke_smc(INTEL_SIP_SMC_FPGA_CONFIG_WRITE,
					    args, 2, NULL, 0);

			debug("wr_ret = %d, rbf_data = %p, buf_size = %08lx\n",
			      wr_ret, rbf_data, buf_size);

			if (wr_ret)
				continue;

			rbf_size -= buf_size;
			rbf_data += buf_size;

			if (buf_size >= rbf_size)
				buf_size = rbf_size;

			xfer_count++;
			puts(".");
		} else {
			ret = invoke_smc(
				INTEL_SIP_SMC_FPGA_CONFIG_COMPLETED_WRITE,
				NULL, 0, res_buf, ARRAY_SIZE(res_buf));
			if (!ret) {
				for (i = 0; i < ARRAY_SIZE(res_buf); i++) {
					if (!res_buf[i])
						break;
					xfer_count--;
					wr_ret = 0;
					retry = 0;
				}
			} else if (ret !=
				   INTEL_SIP_SMC_STATUS_BUSY)
				return ret;
			else if (!xfer_count)
				return INTEL_SIP_SMC_STATUS_ERROR;

			if (++retry >= RECONFIG_STATUS_POLL_RETRY_MAX)
				return -ETIMEDOUT;

			udelay(20000);
		}
		schedule();
	}

	return 0;
}

/*
 * This is the interface used by FPGA driver.
 * Return 0 for success, non-zero for error.
 */
int intel_sdm_mb_load(Altera_desc *desc, const void *rbf_data, size_t rbf_size)
{
	int ret;
	u64 arg = 1;

	debug("Invoking FPGA_CONFIG_START...\n");

	ret = invoke_smc(INTEL_SIP_SMC_FPGA_CONFIG_START, &arg, 1, NULL, 0);

	if (ret) {
		puts("Failure in RECONFIG mailbox command!\n");
		return ret;
	}

	ret = send_bitstream(rbf_data, rbf_size);
	if (ret) {
		puts("Error sending bitstream!\n");
		return ret;
	}

	/* Make sure we don't send MBOX_RECONFIG_STATUS too fast */
	udelay(RECONFIG_STATUS_INTERVAL_DELAY_US);

	debug("Polling with MBOX_RECONFIG_STATUS...\n");
	ret = reconfig_status_polling_resp();
	if (ret) {
		puts("FPGA reconfiguration failed!");
		return ret;
	}

	puts("FPGA reconfiguration OK!\n");

	return ret;
}

#else

static const struct mbox_cfgstat_state {
	int			err_no;
	const char		*error_name;
} mbox_cfgstat_state[] = {
	{MBOX_CFGSTAT_STATE_IDLE, "FPGA in idle mode."},
	{MBOX_CFGSTAT_STATE_CONFIG, "FPGA in config mode."},
	{MBOX_CFGSTAT_STATE_FAILACK, "Acknowledgment failed!"},
	{MBOX_CFGSTAT_STATE_ERROR_INVALID, "Invalid bitstream!"},
	{MBOX_CFGSTAT_STATE_ERROR_CORRUPT, "Corrupted bitstream!"},
	{MBOX_CFGSTAT_STATE_ERROR_AUTH, "Authentication failed!"},
	{MBOX_CFGSTAT_STATE_ERROR_CORE_IO, "I/O error!"},
	{MBOX_CFGSTAT_STATE_ERROR_HARDWARE, "Hardware error!"},
	{MBOX_CFGSTAT_STATE_ERROR_FAKE, "Fake error!"},
	{MBOX_CFGSTAT_STATE_ERROR_BOOT_INFO, "Error in boot info!"},
	{MBOX_CFGSTAT_STATE_ERROR_QSPI_ERROR, "Error in QSPI!"},
	{MBOX_RESP_ERROR, "Mailbox general error!"},
	{-ETIMEDOUT, "I/O timeout error"},
	{-1, "Unknown error!"}
};

#define MBOX_CFGSTAT_MAX ARRAY_SIZE(mbox_cfgstat_state)

static const char *mbox_cfgstat_to_str(int err)
{
	int i;

	for (i = 0; i < MBOX_CFGSTAT_MAX - 1; i++) {
		if (mbox_cfgstat_state[i].err_no == err)
			return mbox_cfgstat_state[i].error_name;
	}

	return mbox_cfgstat_state[MBOX_CFGSTAT_MAX - 1].error_name;
}

/*
 * Add the ongoing transaction's command ID into pending list and return
 * the command ID for next transfer.
 */
static u8 add_transfer(u32 *xfer_pending_list, size_t list_size, u8 id)
{
	int i;

	for (i = 0; i < list_size; i++) {
		if (xfer_pending_list[i])
			continue;
		xfer_pending_list[i] = id;
		debug("ID(%d) added to transaction pending list\n", id);
		/*
		 * Increment command ID for next transaction.
		 * Valid command ID (4 bits) is from 1 to 15.
		 */
		id = (id % 15) + 1;
		break;
	}

	return id;
}

/*
 * Check whether response ID match the command ID in the transfer
 * pending list. If a match is found in the transfer pending list,
 * it clears the transfer pending list and return the matched
 * command ID.
 */
static int get_and_clr_transfer(u32 *xfer_pending_list, size_t list_size,
				u8 id)
{
	int i;

	for (i = 0; i < list_size; i++) {
		if (id != xfer_pending_list[i])
			continue;
		xfer_pending_list[i] = 0;
		return id;
	}

	return 0;
}

/*
 * Polling the FPGA configuration status.
 * Return 0 for success, non-zero for error.
 */
static int reconfig_status_polling_resp(void)
{
	int ret;
	unsigned long start = get_timer(0);

	while (1) {
		ret = mbox_get_fpga_config_status(MBOX_RECONFIG_STATUS);
		if (!ret)
			return 0;	/* configuration success */

		if (ret != MBOX_CFGSTAT_STATE_CONFIG)
			return ret;

		if (get_timer(start) > RECONFIG_STATUS_POLL_RESP_TIMEOUT_MS)
			break;	/* time out */

		puts(".");
		udelay(RECONFIG_STATUS_INTERVAL_DELAY_US);
		schedule();
	}

	return -ETIMEDOUT;
}

static u32 get_resp_hdr(u32 *r_index, u32 *w_index, u32 *resp_count,
			u32 *resp_buf, u32 buf_size, u32 client_id)
{
	u32 buf[MBOX_RESP_BUFFER_SIZE];
	u32 mbox_hdr;
	u32 resp_len;
	u32 hdr_len;
	u32 i;

	if (*resp_count < buf_size) {
		u32 rcv_len_max = buf_size - *resp_count;

		if (rcv_len_max > MBOX_RESP_BUFFER_SIZE)
			rcv_len_max = MBOX_RESP_BUFFER_SIZE;
		resp_len = mbox_rcv_resp(buf, rcv_len_max);

		for (i = 0; i < resp_len; i++) {
			resp_buf[(*w_index)++] = buf[i];
			*w_index %= buf_size;
			(*resp_count)++;
		}
	}

	/* No response in buffer */
	if (*resp_count == 0)
		return 0;

	mbox_hdr = resp_buf[*r_index];

	hdr_len = MBOX_RESP_LEN_GET(mbox_hdr);

	/* Insufficient header length to return a mailbox header */
	if ((*resp_count - 1) < hdr_len)
		return 0;

	*r_index += (hdr_len + 1);
	*r_index %= buf_size;
	*resp_count -= (hdr_len + 1);

	/* Make sure response belongs to us */
	if (MBOX_RESP_CLIENT_GET(mbox_hdr) != client_id)
		return 0;

	return mbox_hdr;
}

/* Send bit stream data to SDM via RECONFIG_DATA mailbox command */
static int send_reconfig_data(const void *rbf_data, size_t rbf_size,
			      u32 xfer_max, u32 buf_size_max)
{
	u32 response_buffer[MBOX_RESP_BUFFER_SIZE];
	u32 xfer_pending[MBOX_RESP_BUFFER_SIZE];
	u32 resp_rindex = 0;
	u32 resp_windex = 0;
	u32 resp_count = 0;
	u32 xfer_count = 0;
	int resp_err = 0;
	u8 cmd_id = 1;
	u32 args[3];
	int ret;

	debug("SDM xfer_max = %d\n", xfer_max);
	debug("SDM buf_size_max = %x\n\n", buf_size_max);

	memset(xfer_pending, 0, sizeof(xfer_pending));

	while (rbf_size || xfer_count) {
		if (!resp_err && rbf_size && xfer_count < xfer_max) {
			args[0] = MBOX_ARG_DESC_COUNT(1);
			args[1] = (u64)rbf_data;
			if (rbf_size >= buf_size_max) {
				args[2] = buf_size_max;
				rbf_size -= buf_size_max;
				rbf_data += buf_size_max;
			} else {
				args[2] = (u64)rbf_size;
				rbf_size = 0;
			}

			resp_err = mbox_send_cmd_only(cmd_id, MBOX_RECONFIG_DATA,
						 MBOX_CMD_INDIRECT, 3, args);
			if (!resp_err) {
				xfer_count++;
				cmd_id = add_transfer(xfer_pending,
						      MBOX_RESP_BUFFER_SIZE,
						      cmd_id);
			}
			puts(".");
		} else {
			u32 resp_hdr = get_resp_hdr(&resp_rindex, &resp_windex,
						    &resp_count,
						    response_buffer,
						    MBOX_RESP_BUFFER_SIZE,
						    MBOX_CLIENT_ID_UBOOT);

			/*
			 * If no valid response header found or
			 * non-zero length from RECONFIG_DATA
			 */
			if (!resp_hdr || MBOX_RESP_LEN_GET(resp_hdr))
				continue;

			/* Check for response's status */
			if (!resp_err) {
				resp_err = MBOX_RESP_ERR_GET(resp_hdr);
				debug("Response error code: %08x\n", resp_err);
			}

			ret = get_and_clr_transfer(xfer_pending,
						   MBOX_RESP_BUFFER_SIZE,
						   MBOX_RESP_ID_GET(resp_hdr));
			if (ret) {
				/* Claim and reuse the ID */
				cmd_id = (u8)ret;
				xfer_count--;
			}

			if (resp_err && !xfer_count)
				return resp_err;
		}
		schedule();
	}

	return 0;
}

/*
 * This is the interface used by FPGA driver.
 * Return 0 for success, non-zero for error.
 */
int intel_sdm_mb_load(Altera_desc *desc, const void *rbf_data, size_t rbf_size)
{
	int ret;
	u32 resp_len = 2;
	u32 resp_buf[2];

	debug("Sending MBOX_RECONFIG...\n");
	ret = mbox_send_cmd(MBOX_ID_UBOOT, MBOX_RECONFIG, MBOX_CMD_DIRECT, 0,
			    NULL, 0, &resp_len, resp_buf);
	if (ret) {
		puts("Failure in RECONFIG mailbox command!\n");
		return ret;
	}

	ret = send_reconfig_data(rbf_data, rbf_size, resp_buf[0], resp_buf[1]);
	if (ret) {
		printf("RECONFIG_DATA error: %08x, %s\n", ret,
		       mbox_cfgstat_to_str(ret));
		return ret;
	}

	/* Make sure we don't send MBOX_RECONFIG_STATUS too fast */
	udelay(RECONFIG_STATUS_INTERVAL_DELAY_US);

	debug("Polling with MBOX_RECONFIG_STATUS...\n");
	ret = reconfig_status_polling_resp();
	if (ret) {
		printf("RECONFIG_STATUS Error: %08x, %s\n", ret,
		       mbox_cfgstat_to_str(ret));
		return ret;
	}

	puts("FPGA reconfiguration OK!\n");

	return ret;
}
#endif
