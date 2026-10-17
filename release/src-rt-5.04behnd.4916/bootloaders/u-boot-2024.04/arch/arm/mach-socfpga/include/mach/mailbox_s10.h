/* SPDX-License-Identifier: GPL-2.0
 *
 * Copyright (C) 2017-2018 Intel Corporation <www.intel.com>
 *
 */

#ifndef _MAILBOX_S10_H_
#define _MAILBOX_S10_H_

/* user define Uboot ID */
#include <linux/bitops.h>
#define MBOX_CLIENT_ID_UBOOT	0xB
#define MBOX_ID_UBOOT		0x1

#define MBOX_CMD_DIRECT	0
#define MBOX_CMD_INDIRECT	1

#define MBOX_MAX_CMD_INDEX	2047
#define MBOX_CMD_BUFFER_SIZE	32
#define MBOX_RESP_BUFFER_SIZE	16

#define MBOX_HDR_CMD_LSB	0
#define MBOX_HDR_CMD_MSK	(BIT(11) - 1)
#define MBOX_HDR_I_LSB		11
#define MBOX_HDR_I_MSK		BIT(11)
#define MBOX_HDR_LEN_LSB	12
#define MBOX_HDR_LEN_MSK	0x007FF000
#define MBOX_HDR_ID_LSB		24
#define MBOX_HDR_ID_MSK		0x0F000000
#define MBOX_HDR_CLIENT_LSB	28
#define MBOX_HDR_CLIENT_MSK	0xF0000000

/* Interrupt flags */
#define MBOX_FLAGS_INT_COE	BIT(0)	/* COUT update interrupt enable */
#define MBOX_FLAGS_INT_RIE	BIT(1)	/* RIN update interrupt enable */
#define MBOX_FLAGS_INT_UAE	BIT(8)	/* Urgent ACK interrupt enable */
#define MBOX_ALL_INTRS		(MBOX_FLAGS_INT_COE | \
				 MBOX_FLAGS_INT_RIE | \
				 MBOX_FLAGS_INT_UAE)

/* Status */
#define MBOX_STATUS_UA_MSK	BIT(8)

#define MBOX_CMD_HEADER(client, id, len, indirect, cmd)     \
	((((cmd) << MBOX_HDR_CMD_LSB) & MBOX_HDR_CMD_MSK) | \
	(((indirect) << MBOX_HDR_I_LSB) & MBOX_HDR_I_MSK) | \
	(((len) << MBOX_HDR_LEN_LSB) & MBOX_HDR_LEN_MSK)  | \
	(((id) << MBOX_HDR_ID_LSB) & MBOX_HDR_ID_MSK)     | \
	(((client) << MBOX_HDR_CLIENT_LSB) & MBOX_HDR_CLIENT_MSK))

#define MBOX_RESP_ERR_GET(resp)				\
	(((resp) & MBOX_HDR_CMD_MSK) >> MBOX_HDR_CMD_LSB)
#define MBOX_RESP_LEN_GET(resp)			\
	(((resp) & MBOX_HDR_LEN_MSK) >> MBOX_HDR_LEN_LSB)
#define MBOX_RESP_ID_GET(resp)				\
	(((resp) & MBOX_HDR_ID_MSK) >> MBOX_HDR_ID_LSB)
#define MBOX_RESP_CLIENT_GET(resp)			\
	(((resp) & MBOX_HDR_CLIENT_MSK) >> MBOX_HDR_CLIENT_LSB)

/* Response error list */
enum ALT_SDM_MBOX_RESP_CODE {
	/* CMD completed successfully, but check resp ARGS for any errors */
	MBOX_RESP_STATOK = 0,
	/* CMD is incorrectly formatted in some way */
	MBOX_RESP_INVALID_COMMAND = 1,
	/* BootROM Command code not undesrtood */
	MBOX_RESP_UNKNOWN_BR = 2,
	/* CMD code not recognized by firmware */
	MBOX_RESP_UNKNOWN = 3,
	/* Length setting is not a valid length for this CMD type */
	MBOX_RESP_INVALID_LEN = 4,
	/* Indirect setting is not valid for this CMD type */
	MBOX_RESP_INVALID_INDIRECT_SETTING = 5,
	/* HW source which is not allowed to send CMD type */
	MBOX_RESP_CMD_INVALID_ON_SRC = 6,
	/* Client with ID not associated with any running PR CMD tries to run
	 * RECONFIG_DATA RECONFIG_STATUS and accessing QSPI / SDMMC using ID
	 * without exclusive access
	 */
	MBOX_RESP_CLIENT_ID_NO_MATCH = 8,
	/* Address provided to the system is invalid (alignment, range
	 * permission)
	 */
	MBOX_RESP_INVALID_ADDR = 0x9,
	/* Signature authentication failed */
	MBOX_RESP_AUTH_FAIL = 0xA,
	/* CMD timed out */
	MBOX_RESP_TIMEOUT = 0xB,
	/* HW (i.e. QSPI) is not ready (initialized or configured) */
	MBOX_RESP_HW_NOT_RDY = 0xC,
	/* Invalid license for IID registration */
	MBOX_RESP_PUF_ACCCES_FAILED = 0x80,
	MBOX_PUF_ENROLL_DISABLE = 0x81,
	MBOX_RESP_PUF_ENROLL_FAIL = 0x82,
	MBOX_RESP_PUF_RAM_TEST_FAIL = 0x83,
	MBOX_RESP_ATTEST_CERT_GEN_FAIL = 0x84,
	/* Operation not allowed under current security settings */
	MBOX_RESP_NOT_ALLOWED_UNDER_SECURITY_SETTINGS = 0x85,
	MBOX_RESP_PUF_TRNG_FAIL = 0x86,
	MBOX_RESP_FUSE_ALREADY_BLOWN = 0x87,
	MBOX_RESP_INVALID_SIGNATURE = 0x88,
	MBOX_RESP_INVALID_HASH = 0x8b,
	MBOX_RESP_INVALID_CERTIFICATE = 0x91,
	/* Indicates that the device (FPGA or HPS) is not configured */
	MBOX_RESP_NOT_CONFIGURED = 0x100,
	/* Indicates that the device is busy */
	MBOX_RESP_DEVICE_BUSY = 0x1FF,
	/* Indicates that there is no valid response available */
	MBOX_RESP_NO_VALID_RESP_AVAILABLE = 0x2FF,
	/* General Error */
	MBOX_RESP_ERROR = 0x3FF,
};

/* Mailbox command list */
#define MBOX_RESTART		2
#define MBOX_CONFIG_STATUS	4
#define MBOX_RECONFIG		6
#define MBOX_RECONFIG_MSEL	7
#define MBOX_RECONFIG_DATA	8
#define MBOX_RECONFIG_STATUS	9
#define MBOX_VAB_SRC_CERT		11
#define MBOX_QSPI_OPEN		50
#define MBOX_QSPI_CLOSE		51
#define MBOX_QSPI_DIRECT	59
#define MBOX_REBOOT_HPS		71

/* Mailbox registers */
#define MBOX_CIN			0	/* command valid offset */
#define MBOX_ROUT			4	/* response output offset */
#define MBOX_URG			8	/* urgent command */
#define MBOX_FLAGS			0x0c	/* interrupt enables */
#define MBOX_COUT			0x20	/* command free offset */
#define MBOX_RIN			0x24	/* respond valid offset */
#define MBOX_STATUS			0x2c	/* mailbox status */
#define MBOX_CMD_BUF			0x40	/* circular command buffer */
#define MBOX_RESP_BUF			0xc0	/* circular response buffer */
#define MBOX_DOORBELL_TO_SDM		0x400	/* Doorbell to SDM */
#define MBOX_DOORBELL_FROM_SDM		0x480	/* Doorbell from SDM */

/* Status and bit information returned by RECONFIG_STATUS */
#define RECONFIG_STATUS_RESPONSE_LEN			6
#define RECONFIG_STATUS_STATE				0
#define RECONFIG_STATUS_PIN_STATUS			2
#define RECONFIG_STATUS_SOFTFUNC_STATUS			3

/* Macros for specifying number of arguments in mailbox command */
#define MBOX_NUM_ARGS(n, b)				(((n) & 0xFF) << (b))
#define MBOX_DIRECT_COUNT(n)				MBOX_NUM_ARGS((n), 0)
#define MBOX_ARG_DESC_COUNT(n)				MBOX_NUM_ARGS((n), 8)
#define MBOX_RESP_DESC_COUNT(n)				MBOX_NUM_ARGS((n), 16)

#define MBOX_CFGSTAT_STATE_IDLE				0x00000000
#define MBOX_CFGSTAT_STATE_CONFIG			0x10000000
#define MBOX_CFGSTAT_STATE_FAILACK			0x08000000
#define MBOX_CFGSTAT_STATE_ERROR_INVALID		0xf0000001
#define MBOX_CFGSTAT_STATE_ERROR_CORRUPT		0xf0000002
#define MBOX_CFGSTAT_STATE_ERROR_AUTH			0xf0000003
#define MBOX_CFGSTAT_STATE_ERROR_CORE_IO		0xf0000004
#define MBOX_CFGSTAT_STATE_ERROR_HARDWARE		0xf0000005
#define MBOX_CFGSTAT_STATE_ERROR_FAKE			0xf0000006
#define MBOX_CFGSTAT_STATE_ERROR_BOOT_INFO		0xf0000007
#define MBOX_CFGSTAT_STATE_ERROR_QSPI_ERROR		0xf0000008

#define RCF_SOFTFUNC_STATUS_CONF_DONE			BIT(0)
#define RCF_SOFTFUNC_STATUS_INIT_DONE			BIT(1)
#define RCF_SOFTFUNC_STATUS_SEU_ERROR			BIT(3)
#define RCF_PIN_STATUS_NSTATUS				BIT(31)

int mbox_send_cmd(u8 id, u32 cmd, u8 is_indirect, u32 len, u32 *arg, u8 urgent,
		  u32 *resp_buf_len, u32 *resp_buf);
int mbox_send_cmd_psci(u8 id, u32 cmd, u8 is_indirect, u32 len, u32 *arg,
		       u8 urgent, u32 *resp_buf_len, u32 *resp_buf);
int mbox_send_cmd_only(u8 id, u32 cmd, u8 is_indirect, u32 len, u32 *arg);
int mbox_send_cmd_only_psci(u8 id, u32 cmd, u8 is_indirect, u32 len, u32 *arg);
int mbox_rcv_resp(u32 *resp_buf, u32 resp_buf_max_len);
int mbox_rcv_resp_psci(u32 *resp_buf, u32 resp_buf_max_len);
int mbox_init(void);

#ifdef CONFIG_CADENCE_QSPI
int mbox_qspi_close(void);
int mbox_qspi_open(void);
#endif

int mbox_reset_cold(void);
int mbox_get_fpga_config_status(u32 cmd);
int mbox_get_fpga_config_status_psci(u32 cmd);
#endif /* _MAILBOX_S10_H_ */
