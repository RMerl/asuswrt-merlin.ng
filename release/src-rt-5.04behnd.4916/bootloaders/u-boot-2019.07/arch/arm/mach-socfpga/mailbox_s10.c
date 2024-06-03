// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017-2018 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <wait_bit.h>
#include <asm/io.h>
#include <asm/arch/mailbox_s10.h>
#include <asm/arch/system_manager.h>
#include <asm/secure.h>

DECLARE_GLOBAL_DATA_PTR;

#define MBOX_READL(reg)			\
	 readl(SOCFPGA_MAILBOX_ADDRESS + (reg))

#define MBOX_WRITEL(data, reg)		\
	writel(data, SOCFPGA_MAILBOX_ADDRESS + (reg))

#define MBOX_READ_RESP_BUF(rout)	\
	MBOX_READL(MBOX_RESP_BUF + ((rout) * sizeof(u32)))

#define MBOX_WRITE_CMD_BUF(data, cin)	\
	MBOX_WRITEL(data, MBOX_CMD_BUF + ((cin) * sizeof(u32)))

static __always_inline int mbox_polling_resp(u32 rout)
{
	u32 rin;
	unsigned long i = ~0;

	while (i) {
		rin = MBOX_READL(MBOX_RIN);
		if (rout != rin)
			return 0;

		i--;
	}

	return -ETIMEDOUT;
}

/* Check for available slot and write to circular buffer.
 * It also update command valid offset (cin) register.
 */
static __always_inline int mbox_fill_cmd_circular_buff(u32 header, u32 len,
						       u32 *arg)
{
	u32 cin;
	u32 cout;
	u32 i;

	cin = MBOX_READL(MBOX_CIN) % MBOX_CMD_BUFFER_SIZE;
	cout = MBOX_READL(MBOX_COUT) % MBOX_CMD_BUFFER_SIZE;

	/* if command buffer is full or not enough free space
	 * to fit the data. Note, len is in u32 unit.
	 */
	if (((cin + 1) % MBOX_CMD_BUFFER_SIZE) == cout ||
	    ((MBOX_CMD_BUFFER_SIZE - cin + cout - 1) %
	     MBOX_CMD_BUFFER_SIZE) < (len + 1))
		return -ENOMEM;

	/* write header to circular buffer */
	MBOX_WRITE_CMD_BUF(header, cin++);
	/* wrapping around when it reach the buffer size */
	cin %= MBOX_CMD_BUFFER_SIZE;

	/* write arguments */
	for (i = 0; i < len; i++) {
		MBOX_WRITE_CMD_BUF(arg[i], cin++);
		/* wrapping around when it reach the buffer size */
		cin %= MBOX_CMD_BUFFER_SIZE;
	}

	/* write command valid offset */
	MBOX_WRITEL(cin, MBOX_CIN);

	return 0;
}

/* Check the command and fill it into circular buffer */
static __always_inline int mbox_prepare_cmd_only(u8 id, u32 cmd,
						 u8 is_indirect, u32 len,
						 u32 *arg)
{
	u32 header;
	int ret;

	/* Total length is command + argument length */
	if ((len + 1) > MBOX_CMD_BUFFER_SIZE)
		return -EINVAL;

	if (cmd > MBOX_MAX_CMD_INDEX)
		return -EINVAL;

	header = MBOX_CMD_HEADER(MBOX_CLIENT_ID_UBOOT, id, len,
				 (is_indirect) ? 1 : 0, cmd);

	ret = mbox_fill_cmd_circular_buff(header, len, arg);

	return ret;
}

/* Send command only without waiting for responses from SDM */
static __always_inline int mbox_send_cmd_only_common(u8 id, u32 cmd,
						     u8 is_indirect, u32 len,
						     u32 *arg)
{
	int ret = mbox_prepare_cmd_only(id, cmd, is_indirect, len, arg);
	/* write doorbell */
	MBOX_WRITEL(1, MBOX_DOORBELL_TO_SDM);

	return ret;
}

/* Return number of responses received in buffer */
static __always_inline int __mbox_rcv_resp(u32 *resp_buf, u32 resp_buf_max_len)
{
	u32 rin;
	u32 rout;
	u32 resp_len = 0;

	/* clear doorbell from SDM if it was SET */
	if (MBOX_READL(MBOX_DOORBELL_FROM_SDM) & 1)
		MBOX_WRITEL(0, MBOX_DOORBELL_FROM_SDM);

	/* read current response offset */
	rout = MBOX_READL(MBOX_ROUT);
	/* read response valid offset */
	rin = MBOX_READL(MBOX_RIN);

	while (rin != rout && (resp_len < resp_buf_max_len)) {
		/* Response received */
		if (resp_buf)
			resp_buf[resp_len++] = MBOX_READ_RESP_BUF(rout);

		rout++;
		/* wrapping around when it reach the buffer size */
		rout %= MBOX_RESP_BUFFER_SIZE;
		/* update next ROUT */
		MBOX_WRITEL(rout, MBOX_ROUT);
	}

	return resp_len;
}

/* Support one command and up to 31 words argument length only */
static __always_inline int mbox_send_cmd_common(u8 id, u32 cmd, u8 is_indirect,
						u32 len, u32 *arg, u8 urgent,
						u32 *resp_buf_len,
						u32 *resp_buf)
{
	u32 rin;
	u32 resp;
	u32 rout;
	u32 status;
	u32 resp_len;
	u32 buf_len;
	int ret;

	if (urgent) {
		/* Read status because it is toggled */
		status = MBOX_READL(MBOX_STATUS) & MBOX_STATUS_UA_MSK;
		/* Write urgent command to urgent register */
		MBOX_WRITEL(cmd, MBOX_URG);
	} else {
		ret = mbox_prepare_cmd_only(id, cmd, is_indirect, len, arg);
		if (ret)
			return ret;
	}

	/* write doorbell */
	MBOX_WRITEL(1, MBOX_DOORBELL_TO_SDM);

	while (1) {
		ret = ~0;

		/* Wait for doorbell from SDM */
		while (!MBOX_READL(MBOX_DOORBELL_FROM_SDM) && ret--)
			;
		if (!ret)
			return -ETIMEDOUT;

		/* clear interrupt */
		MBOX_WRITEL(0, MBOX_DOORBELL_FROM_SDM);

		if (urgent) {
			u32 new_status = MBOX_READL(MBOX_STATUS);

			/* Urgent ACK is toggled */
			if ((new_status & MBOX_STATUS_UA_MSK) ^ status)
				return 0;

			return -ECOMM;
		}

		/* read current response offset */
		rout = MBOX_READL(MBOX_ROUT);

		/* read response valid offset */
		rin = MBOX_READL(MBOX_RIN);

		if (rout != rin) {
			/* Response received */
			resp = MBOX_READ_RESP_BUF(rout);
			rout++;
			/* wrapping around when it reach the buffer size */
			rout %= MBOX_RESP_BUFFER_SIZE;
			/* update next ROUT */
			MBOX_WRITEL(rout, MBOX_ROUT);

			/* check client ID and ID */
			if ((MBOX_RESP_CLIENT_GET(resp) ==
			     MBOX_CLIENT_ID_UBOOT) &&
			    (MBOX_RESP_ID_GET(resp) == id)) {
				ret = MBOX_RESP_ERR_GET(resp);
				if (ret)
					return ret;

				if (resp_buf_len) {
					buf_len = *resp_buf_len;
					*resp_buf_len = 0;
				} else {
					buf_len = 0;
				}

				resp_len = MBOX_RESP_LEN_GET(resp);
				while (resp_len) {
					ret = mbox_polling_resp(rout);
					if (ret)
						return ret;
					/* we need to process response buffer
					 * even caller doesn't need it
					 */
					resp = MBOX_READ_RESP_BUF(rout);
					rout++;
					resp_len--;
					rout %= MBOX_RESP_BUFFER_SIZE;
					MBOX_WRITEL(rout, MBOX_ROUT);
					if (buf_len) {
						/* copy response to buffer */
						resp_buf[*resp_buf_len] = resp;
						(*resp_buf_len)++;
						buf_len--;
					}
				}
				return ret;
			}
		}
	};

	return -EIO;
}

int mbox_init(void)
{
	int ret;

	/* enable mailbox interrupts */
	MBOX_WRITEL(MBOX_ALL_INTRS, MBOX_FLAGS);

	/* Ensure urgent request is cleared */
	MBOX_WRITEL(0, MBOX_URG);

	/* Ensure the Doorbell Interrupt is cleared */
	MBOX_WRITEL(0, MBOX_DOORBELL_FROM_SDM);

	ret = mbox_send_cmd(MBOX_ID_UBOOT, MBOX_RESTART, MBOX_CMD_DIRECT, 0,
			    NULL, 1, 0, NULL);
	if (ret)
		return ret;

	/* Renable mailbox interrupts after MBOX_RESTART */
	MBOX_WRITEL(MBOX_ALL_INTRS, MBOX_FLAGS);

	return 0;
}

#ifdef CONFIG_CADENCE_QSPI
int mbox_qspi_close(void)
{
	return mbox_send_cmd(MBOX_ID_UBOOT, MBOX_QSPI_CLOSE, MBOX_CMD_DIRECT,
			     0, NULL, 0, 0, NULL);
}

int mbox_qspi_open(void)
{
	static const struct socfpga_system_manager *sysmgr_regs =
		(struct socfpga_system_manager *)SOCFPGA_SYSMGR_ADDRESS;

	int ret;
	u32 resp_buf[1];
	u32 resp_buf_len;

	ret = mbox_send_cmd(MBOX_ID_UBOOT, MBOX_QSPI_OPEN, MBOX_CMD_DIRECT,
			    0, NULL, 0, 0, NULL);
	if (ret) {
		/* retry again by closing and reopen the QSPI again */
		ret = mbox_qspi_close();
		if (ret)
			return ret;

		ret = mbox_send_cmd(MBOX_ID_UBOOT, MBOX_QSPI_OPEN,
				    MBOX_CMD_DIRECT, 0, NULL, 0, 0, NULL);
		if (ret)
			return ret;
	}

	/* HPS will directly control the QSPI controller, no longer mailbox */
	resp_buf_len = 1;
	ret = mbox_send_cmd(MBOX_ID_UBOOT, MBOX_QSPI_DIRECT, MBOX_CMD_DIRECT,
			    0, NULL, 0, (u32 *)&resp_buf_len,
			    (u32 *)&resp_buf);
	if (ret)
		goto error;

	/* We are getting QSPI ref clock and set into sysmgr boot register */
	printf("QSPI: Reference clock at %d Hz\n", resp_buf[0]);
	writel(resp_buf[0], &sysmgr_regs->boot_scratch_cold0);

	return 0;

error:
	mbox_qspi_close();

	return ret;
}
#endif /* CONFIG_CADENCE_QSPI */

int mbox_reset_cold(void)
{
	int ret;

	ret = mbox_send_cmd(MBOX_ID_UBOOT, MBOX_REBOOT_HPS, MBOX_CMD_DIRECT,
			    0, NULL, 0, 0, NULL);
	if (ret) {
		/* mailbox sent failure, wait for watchdog to kick in */
		hang();
	}
	return 0;
}

/* Accepted commands: CONFIG_STATUS or RECONFIG_STATUS */
static __always_inline int mbox_get_fpga_config_status_common(u32 cmd)
{
	u32 reconfig_status_resp_len;
	u32 reconfig_status_resp[RECONFIG_STATUS_RESPONSE_LEN];
	int ret;

	reconfig_status_resp_len = RECONFIG_STATUS_RESPONSE_LEN;
	ret = mbox_send_cmd_common(MBOX_ID_UBOOT, cmd,
				   MBOX_CMD_DIRECT, 0, NULL, 0,
				   &reconfig_status_resp_len,
				   reconfig_status_resp);

	if (ret)
		return ret;

	/* Check for any error */
	ret = reconfig_status_resp[RECONFIG_STATUS_STATE];
	if (ret && ret != MBOX_CFGSTAT_STATE_CONFIG)
		return ret;

	/* Make sure nStatus is not 0 */
	ret = reconfig_status_resp[RECONFIG_STATUS_PIN_STATUS];
	if (!(ret & RCF_PIN_STATUS_NSTATUS))
		return MBOX_CFGSTAT_STATE_ERROR_HARDWARE;

	ret = reconfig_status_resp[RECONFIG_STATUS_SOFTFUNC_STATUS];
	if (ret & RCF_SOFTFUNC_STATUS_SEU_ERROR)
		return MBOX_CFGSTAT_STATE_ERROR_HARDWARE;

	if ((ret & RCF_SOFTFUNC_STATUS_CONF_DONE) &&
	    (ret & RCF_SOFTFUNC_STATUS_INIT_DONE) &&
	    !reconfig_status_resp[RECONFIG_STATUS_STATE])
		return 0;	/* configuration success */

	return MBOX_CFGSTAT_STATE_CONFIG;
}

int mbox_get_fpga_config_status(u32 cmd)
{
	return mbox_get_fpga_config_status_common(cmd);
}

int __secure mbox_get_fpga_config_status_psci(u32 cmd)
{
	return mbox_get_fpga_config_status_common(cmd);
}

int mbox_send_cmd(u8 id, u32 cmd, u8 is_indirect, u32 len, u32 *arg,
		  u8 urgent, u32 *resp_buf_len, u32 *resp_buf)
{
	return mbox_send_cmd_common(id, cmd, is_indirect, len, arg, urgent,
			       resp_buf_len, resp_buf);
}

int __secure mbox_send_cmd_psci(u8 id, u32 cmd, u8 is_indirect, u32 len,
				u32 *arg, u8 urgent, u32 *resp_buf_len,
				u32 *resp_buf)
{
	return mbox_send_cmd_common(id, cmd, is_indirect, len, arg, urgent,
			       resp_buf_len, resp_buf);
}

int mbox_send_cmd_only(u8 id, u32 cmd, u8 is_indirect, u32 len, u32 *arg)
{
	return mbox_send_cmd_only_common(id, cmd, is_indirect, len, arg);
}

int __secure mbox_send_cmd_only_psci(u8 id, u32 cmd, u8 is_indirect, u32 len,
				     u32 *arg)
{
	return mbox_send_cmd_only_common(id, cmd, is_indirect, len, arg);
}

int mbox_rcv_resp(u32 *resp_buf, u32 resp_buf_max_len)
{
	return __mbox_rcv_resp(resp_buf, resp_buf_max_len);
}

int __secure mbox_rcv_resp_psci(u32 *resp_buf, u32 resp_buf_max_len)
{
	return __mbox_rcv_resp(resp_buf, resp_buf_max_len);
}
