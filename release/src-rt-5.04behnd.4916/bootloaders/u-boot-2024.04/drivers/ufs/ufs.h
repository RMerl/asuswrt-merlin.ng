/* SPDX-License-Identifier: GPL-2.0+ */
#ifndef __UFS_H
#define __UFS_H

#include "unipro.h"

struct udevice;

#define UFS_CDB_SIZE	16
#define UPIU_TRANSACTION_UIC_CMD 0x1F
#define UIC_CMD_SIZE (sizeof(u32) * 4)
#define RESPONSE_UPIU_SENSE_DATA_LENGTH	18
#define UFS_MAX_LUNS		0x7F

enum {
	TASK_REQ_UPIU_SIZE_DWORDS	= 8,
	TASK_RSP_UPIU_SIZE_DWORDS	= 8,
	ALIGNED_UPIU_SIZE		= 512,
};

/* UFS device power modes */
enum ufs_dev_pwr_mode {
	UFS_ACTIVE_PWR_MODE	= 1,
	UFS_SLEEP_PWR_MODE	= 2,
	UFS_POWERDOWN_PWR_MODE	= 3,
};

enum ufs_notify_change_status {
	PRE_CHANGE,
	POST_CHANGE,
};

struct ufs_pa_layer_attr {
	u32 gear_rx;
	u32 gear_tx;
	u32 lane_rx;
	u32 lane_tx;
	u32 pwr_rx;
	u32 pwr_tx;
	u32 hs_rate;
};

struct ufs_pwr_mode_info {
	bool is_valid;
	struct ufs_pa_layer_attr info;
};

enum ufs_desc_def_size {
	QUERY_DESC_DEVICE_DEF_SIZE		= 0x40,
	QUERY_DESC_CONFIGURATION_DEF_SIZE	= 0x90,
	QUERY_DESC_UNIT_DEF_SIZE		= 0x23,
	QUERY_DESC_INTERCONNECT_DEF_SIZE	= 0x06,
	QUERY_DESC_GEOMETRY_DEF_SIZE		= 0x48,
	QUERY_DESC_POWER_DEF_SIZE		= 0x62,
	QUERY_DESC_HEALTH_DEF_SIZE		= 0x25,
};

struct ufs_desc_size {
	int dev_desc;
	int pwr_desc;
	int geom_desc;
	int interc_desc;
	int unit_desc;
	int conf_desc;
	int hlth_desc;
};

/*
 * Request Descriptor Definitions
 */

/* Transfer request command type */
enum {
	UTP_CMD_TYPE_SCSI		= 0x0,
	UTP_CMD_TYPE_UFS		= 0x1,
	UTP_CMD_TYPE_DEV_MANAGE		= 0x2,
};

/* UTP Transfer Request Command Offset */
#define UPIU_COMMAND_TYPE_OFFSET	28

/* Offset of the response code in the UPIU header */
#define UPIU_RSP_CODE_OFFSET		8

/* To accommodate UFS2.0 required Command type */
enum {
	UTP_CMD_TYPE_UFS_STORAGE	= 0x1,
};

enum {
	UTP_SCSI_COMMAND		= 0x00000000,
	UTP_NATIVE_UFS_COMMAND		= 0x10000000,
	UTP_DEVICE_MANAGEMENT_FUNCTION	= 0x20000000,
	UTP_REQ_DESC_INT_CMD		= 0x01000000,
};

/* UTP Transfer Request Data Direction (DD) */
enum {
	UTP_NO_DATA_TRANSFER	= 0x00000000,
	UTP_HOST_TO_DEVICE	= 0x02000000,
	UTP_DEVICE_TO_HOST	= 0x04000000,
};

/* Overall command status values */
enum {
	OCS_SUCCESS			= 0x0,
	OCS_INVALID_CMD_TABLE_ATTR	= 0x1,
	OCS_INVALID_PRDT_ATTR		= 0x2,
	OCS_MISMATCH_DATA_BUF_SIZE	= 0x3,
	OCS_MISMATCH_RESP_UPIU_SIZE	= 0x4,
	OCS_PEER_COMM_FAILURE		= 0x5,
	OCS_ABORTED			= 0x6,
	OCS_FATAL_ERROR			= 0x7,
	OCS_INVALID_COMMAND_STATUS	= 0x0F,
	MASK_OCS			= 0x0F,
};

/* The maximum length of the data byte count field in the PRDT is 256KB */
#define PRDT_DATA_BYTE_COUNT_MAX	(256 * 1024)
/* The granularity of the data byte count field in the PRDT is 32-bit */
#define PRDT_DATA_BYTE_COUNT_PAD	4

#define GENERAL_UPIU_REQUEST_SIZE (sizeof(struct utp_upiu_req))
#define QUERY_DESC_MAX_SIZE       255
#define QUERY_DESC_MIN_SIZE       2
#define QUERY_DESC_HDR_SIZE       2
#define QUERY_OSF_SIZE            (GENERAL_UPIU_REQUEST_SIZE - \
					(sizeof(struct utp_upiu_header)))
#define RESPONSE_UPIU_SENSE_DATA_LENGTH	18
#define UPIU_HEADER_DWORD(byte3, byte2, byte1, byte0)\
			cpu_to_be32((byte3 << 24) | (byte2 << 16) |\
			 (byte1 << 8) | (byte0))
/*
 * UFS Protocol Information Unit related definitions
 */

/* Task management functions */
enum {
	UFS_ABORT_TASK		= 0x01,
	UFS_ABORT_TASK_SET	= 0x02,
	UFS_CLEAR_TASK_SET	= 0x04,
	UFS_LOGICAL_RESET	= 0x08,
	UFS_QUERY_TASK		= 0x80,
	UFS_QUERY_TASK_SET	= 0x81,
};

/* UTP UPIU Transaction Codes Initiator to Target */
enum {
	UPIU_TRANSACTION_NOP_OUT	= 0x00,
	UPIU_TRANSACTION_COMMAND	= 0x01,
	UPIU_TRANSACTION_DATA_OUT	= 0x02,
	UPIU_TRANSACTION_TASK_REQ	= 0x04,
	UPIU_TRANSACTION_QUERY_REQ	= 0x16,
};

/* UTP UPIU Transaction Codes Target to Initiator */
enum {
	UPIU_TRANSACTION_NOP_IN		= 0x20,
	UPIU_TRANSACTION_RESPONSE	= 0x21,
	UPIU_TRANSACTION_DATA_IN	= 0x22,
	UPIU_TRANSACTION_TASK_RSP	= 0x24,
	UPIU_TRANSACTION_READY_XFER	= 0x31,
	UPIU_TRANSACTION_QUERY_RSP	= 0x36,
	UPIU_TRANSACTION_REJECT_UPIU	= 0x3F,
};

/* UPIU Read/Write flags */
enum {
	UPIU_CMD_FLAGS_NONE	= 0x00,
	UPIU_CMD_FLAGS_WRITE	= 0x20,
	UPIU_CMD_FLAGS_READ	= 0x40,
};

/* UPIU Task Attributes */
enum {
	UPIU_TASK_ATTR_SIMPLE	= 0x00,
	UPIU_TASK_ATTR_ORDERED	= 0x01,
	UPIU_TASK_ATTR_HEADQ	= 0x02,
	UPIU_TASK_ATTR_ACA	= 0x03,
};

/* UPIU Query request function */
enum {
	UPIU_QUERY_FUNC_STANDARD_READ_REQUEST           = 0x01,
	UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST          = 0x81,
};

/* Offset of the response code in the UPIU header */
#define UPIU_RSP_CODE_OFFSET		8

enum {
	MASK_SCSI_STATUS		= 0xFF,
	MASK_TASK_RESPONSE              = 0xFF00,
	MASK_RSP_UPIU_RESULT            = 0xFFFF,
	MASK_QUERY_DATA_SEG_LEN         = 0xFFFF,
	MASK_RSP_UPIU_DATA_SEG_LEN	= 0xFFFF,
	MASK_RSP_EXCEPTION_EVENT        = 0x10000,
	MASK_TM_SERVICE_RESP		= 0xFF,
	MASK_TM_FUNC			= 0xFF,
};

/* UTP QUERY Transaction Specific Fields OpCode */
enum query_opcode {
	UPIU_QUERY_OPCODE_NOP		= 0x0,
	UPIU_QUERY_OPCODE_READ_DESC	= 0x1,
	UPIU_QUERY_OPCODE_WRITE_DESC	= 0x2,
	UPIU_QUERY_OPCODE_READ_ATTR	= 0x3,
	UPIU_QUERY_OPCODE_WRITE_ATTR	= 0x4,
	UPIU_QUERY_OPCODE_READ_FLAG	= 0x5,
	UPIU_QUERY_OPCODE_SET_FLAG	= 0x6,
	UPIU_QUERY_OPCODE_CLEAR_FLAG	= 0x7,
	UPIU_QUERY_OPCODE_TOGGLE_FLAG	= 0x8,
};

/* Query response result code */
enum {
	QUERY_RESULT_SUCCESS                    = 0x00,
	QUERY_RESULT_NOT_READABLE               = 0xF6,
	QUERY_RESULT_NOT_WRITEABLE              = 0xF7,
	QUERY_RESULT_ALREADY_WRITTEN            = 0xF8,
	QUERY_RESULT_INVALID_LENGTH             = 0xF9,
	QUERY_RESULT_INVALID_VALUE              = 0xFA,
	QUERY_RESULT_INVALID_SELECTOR           = 0xFB,
	QUERY_RESULT_INVALID_INDEX              = 0xFC,
	QUERY_RESULT_INVALID_IDN                = 0xFD,
	QUERY_RESULT_INVALID_OPCODE             = 0xFE,
	QUERY_RESULT_GENERAL_FAILURE            = 0xFF,
};

enum {
	UPIU_COMMAND_SET_TYPE_SCSI	= 0x0,
	UPIU_COMMAND_SET_TYPE_UFS	= 0x1,
	UPIU_COMMAND_SET_TYPE_QUERY	= 0x2,
};

/* Flag idn for Query Requests*/
enum flag_idn {
	QUERY_FLAG_IDN_FDEVICEINIT			= 0x01,
	QUERY_FLAG_IDN_PERMANENT_WPE			= 0x02,
	QUERY_FLAG_IDN_PWR_ON_WPE			= 0x03,
	QUERY_FLAG_IDN_BKOPS_EN				= 0x04,
	QUERY_FLAG_IDN_LIFE_SPAN_MODE_ENABLE		= 0x05,
	QUERY_FLAG_IDN_PURGE_ENABLE			= 0x06,
	QUERY_FLAG_IDN_RESERVED2			= 0x07,
	QUERY_FLAG_IDN_FPHYRESOURCEREMOVAL		= 0x08,
	QUERY_FLAG_IDN_BUSY_RTC				= 0x09,
	QUERY_FLAG_IDN_RESERVED3			= 0x0A,
	QUERY_FLAG_IDN_PERMANENTLY_DISABLE_FW_UPDATE	= 0x0B,
};

/* Attribute idn for Query requests */
enum attr_idn {
	QUERY_ATTR_IDN_BOOT_LU_EN		= 0x00,
	QUERY_ATTR_IDN_RESERVED			= 0x01,
	QUERY_ATTR_IDN_POWER_MODE		= 0x02,
	QUERY_ATTR_IDN_ACTIVE_ICC_LVL		= 0x03,
	QUERY_ATTR_IDN_OOO_DATA_EN		= 0x04,
	QUERY_ATTR_IDN_BKOPS_STATUS		= 0x05,
	QUERY_ATTR_IDN_PURGE_STATUS		= 0x06,
	QUERY_ATTR_IDN_MAX_DATA_IN		= 0x07,
	QUERY_ATTR_IDN_MAX_DATA_OUT		= 0x08,
	QUERY_ATTR_IDN_DYN_CAP_NEEDED		= 0x09,
	QUERY_ATTR_IDN_REF_CLK_FREQ		= 0x0A,
	QUERY_ATTR_IDN_CONF_DESC_LOCK		= 0x0B,
	QUERY_ATTR_IDN_MAX_NUM_OF_RTT		= 0x0C,
	QUERY_ATTR_IDN_EE_CONTROL		= 0x0D,
	QUERY_ATTR_IDN_EE_STATUS		= 0x0E,
	QUERY_ATTR_IDN_SECONDS_PASSED		= 0x0F,
	QUERY_ATTR_IDN_CNTX_CONF		= 0x10,
	QUERY_ATTR_IDN_CORR_PRG_BLK_NUM		= 0x11,
	QUERY_ATTR_IDN_RESERVED2		= 0x12,
	QUERY_ATTR_IDN_RESERVED3		= 0x13,
	QUERY_ATTR_IDN_FFU_STATUS		= 0x14,
	QUERY_ATTR_IDN_PSA_STATE		= 0x15,
	QUERY_ATTR_IDN_PSA_DATA_SIZE		= 0x16,
};

/* Descriptor idn for Query requests */
enum desc_idn {
	QUERY_DESC_IDN_DEVICE		= 0x0,
	QUERY_DESC_IDN_CONFIGURATION	= 0x1,
	QUERY_DESC_IDN_UNIT		= 0x2,
	QUERY_DESC_IDN_RFU_0		= 0x3,
	QUERY_DESC_IDN_INTERCONNECT	= 0x4,
	QUERY_DESC_IDN_STRING		= 0x5,
	QUERY_DESC_IDN_RFU_1		= 0x6,
	QUERY_DESC_IDN_GEOMETRY		= 0x7,
	QUERY_DESC_IDN_POWER		= 0x8,
	QUERY_DESC_IDN_HEALTH           = 0x9,
	QUERY_DESC_IDN_MAX,
};

enum desc_header_offset {
	QUERY_DESC_LENGTH_OFFSET	= 0x00,
	QUERY_DESC_DESC_TYPE_OFFSET	= 0x01,
};

struct ufshcd_sg_entry {
	__le32    base_addr;
	__le32    upper_addr;
	__le32    reserved;
	__le32    size;
};

#define MAX_BUFF	128
/**
 * struct utp_transfer_cmd_desc - UFS Command Descriptor structure
 * @command_upiu: Command UPIU Frame address
 * @response_upiu: Response UPIU Frame address
 * @prd_table: Physical Region Descriptor
 */
struct utp_transfer_cmd_desc {
	u8 command_upiu[ALIGNED_UPIU_SIZE];
	u8 response_upiu[ALIGNED_UPIU_SIZE];
	struct ufshcd_sg_entry    prd_table[MAX_BUFF];
};

/**
 * struct request_desc_header - Descriptor Header common to both UTRD and UTMRD
 * @dword0: Descriptor Header DW0
 * @dword1: Descriptor Header DW1
 * @dword2: Descriptor Header DW2
 * @dword3: Descriptor Header DW3
 */
struct request_desc_header {
	__le32 dword_0;
	__le32 dword_1;
	__le32 dword_2;
	__le32 dword_3;
};

/**
 * struct utp_transfer_req_desc - UTRD structure
 * @header: UTRD header DW-0 to DW-3
 * @command_desc_base_addr_lo: UCD base address low DW-4
 * @command_desc_base_addr_hi: UCD base address high DW-5
 * @response_upiu_length: response UPIU length DW-6
 * @response_upiu_offset: response UPIU offset DW-6
 * @prd_table_length: Physical region descriptor length DW-7
 * @prd_table_offset: Physical region descriptor offset DW-7
 */
struct utp_transfer_req_desc {
	/* DW 0-3 */
	struct request_desc_header header;

	/* DW 4-5*/
	__le32  command_desc_base_addr_lo;
	__le32  command_desc_base_addr_hi;

	/* DW 6 */
	__le16  response_upiu_length;
	__le16  response_upiu_offset;

	/* DW 7 */
	__le16  prd_table_length;
	__le16  prd_table_offset;
};

/**
 * struct utp_upiu_header - UPIU header structure
 * @dword_0: UPIU header DW-0
 * @dword_1: UPIU header DW-1
 * @dword_2: UPIU header DW-2
 */
struct utp_upiu_header {
	__be32 dword_0;
	__be32 dword_1;
	__be32 dword_2;
};

/**
 * struct utp_upiu_query - upiu request buffer structure for
 * query request.
 * @opcode: command to perform B-0
 * @idn: a value that indicates the particular type of data B-1
 * @index: Index to further identify data B-2
 * @selector: Index to further identify data B-3
 * @reserved_osf: spec reserved field B-4,5
 * @length: number of descriptor bytes to read/write B-6,7
 * @value: Attribute value to be written DW-5
 * @reserved: spec reserved DW-6,7
 */
struct utp_upiu_query {
	__u8 opcode;
	__u8 idn;
	__u8 index;
	__u8 selector;
	__be16 reserved_osf;
	__be16 length;
	__be32 value;
	__be32 reserved[2];
};

/**
 * struct utp_upiu_cmd - Command UPIU structure
 * @data_transfer_len: Data Transfer Length DW-3
 * @cdb: Command Descriptor Block CDB DW-4 to DW-7
 */
struct utp_upiu_cmd {
	__be32 exp_data_transfer_len;
	u8 cdb[UFS_CDB_SIZE];
};

/*
 * UTMRD structure.
 */
struct utp_task_req_desc {
	/* DW 0-3 */
	struct request_desc_header header;

	/* DW 4-11 - Task request UPIU structure */
	struct utp_upiu_header	req_header;
	__be32			input_param1;
	__be32			input_param2;
	__be32			input_param3;
	__be32			__reserved1[2];

	/* DW 12-19 - Task Management Response UPIU structure */
	struct utp_upiu_header	rsp_header;
	__be32			output_param1;
	__be32			output_param2;
	__be32			__reserved2[3];
};

/**
 * struct utp_upiu_req - general upiu request structure
 * @header:UPIU header structure DW-0 to DW-2
 * @sc: fields structure for scsi command DW-3 to DW-7
 * @qr: fields structure for query request DW-3 to DW-7
 */
struct utp_upiu_req {
	struct utp_upiu_header header;
	union {
		struct utp_upiu_cmd		sc;
		struct utp_upiu_query		qr;
		struct utp_upiu_query		tr;
		/* use utp_upiu_query to host the 4 dwords of uic command */
		struct utp_upiu_query		uc;
	};
};

/**
 * struct utp_cmd_rsp - Response UPIU structure
 * @residual_transfer_count: Residual transfer count DW-3
 * @reserved: Reserved double words DW-4 to DW-7
 * @sense_data_len: Sense data length DW-8 U16
 * @sense_data: Sense data field DW-8 to DW-12
 */
struct utp_cmd_rsp {
	__be32 residual_transfer_count;
	__be32 reserved[4];
	__be16 sense_data_len;
	u8 sense_data[RESPONSE_UPIU_SENSE_DATA_LENGTH];
};

/**
 * struct utp_upiu_rsp - general upiu response structure
 * @header: UPIU header structure DW-0 to DW-2
 * @sr: fields structure for scsi command DW-3 to DW-12
 * @qr: fields structure for query request DW-3 to DW-7
 */
struct utp_upiu_rsp {
	struct utp_upiu_header header;
	union {
		struct utp_cmd_rsp sr;
		struct utp_upiu_query qr;
	};
};

#define MAX_MODEL_LEN 16
/**
 * ufs_dev_desc - ufs device details from the device descriptor
 *
 * @wmanufacturerid: card details
 * @model: card model
 */
struct ufs_dev_desc {
	u16 wmanufacturerid;
	char model[MAX_MODEL_LEN + 1];
};

/* Device descriptor parameters offsets in bytes*/
enum device_desc_param {
	DEVICE_DESC_PARAM_LEN			= 0x0,
	DEVICE_DESC_PARAM_TYPE			= 0x1,
	DEVICE_DESC_PARAM_DEVICE_TYPE		= 0x2,
	DEVICE_DESC_PARAM_DEVICE_CLASS		= 0x3,
	DEVICE_DESC_PARAM_DEVICE_SUB_CLASS	= 0x4,
	DEVICE_DESC_PARAM_PRTCL			= 0x5,
	DEVICE_DESC_PARAM_NUM_LU		= 0x6,
	DEVICE_DESC_PARAM_NUM_WLU		= 0x7,
	DEVICE_DESC_PARAM_BOOT_ENBL		= 0x8,
	DEVICE_DESC_PARAM_DESC_ACCSS_ENBL	= 0x9,
	DEVICE_DESC_PARAM_INIT_PWR_MODE		= 0xA,
	DEVICE_DESC_PARAM_HIGH_PR_LUN		= 0xB,
	DEVICE_DESC_PARAM_SEC_RMV_TYPE		= 0xC,
	DEVICE_DESC_PARAM_SEC_LU		= 0xD,
	DEVICE_DESC_PARAM_BKOP_TERM_LT		= 0xE,
	DEVICE_DESC_PARAM_ACTVE_ICC_LVL		= 0xF,
	DEVICE_DESC_PARAM_SPEC_VER		= 0x10,
	DEVICE_DESC_PARAM_MANF_DATE		= 0x12,
	DEVICE_DESC_PARAM_MANF_NAME		= 0x14,
	DEVICE_DESC_PARAM_PRDCT_NAME		= 0x15,
	DEVICE_DESC_PARAM_SN			= 0x16,
	DEVICE_DESC_PARAM_OEM_ID		= 0x17,
	DEVICE_DESC_PARAM_MANF_ID		= 0x18,
	DEVICE_DESC_PARAM_UD_OFFSET		= 0x1A,
	DEVICE_DESC_PARAM_UD_LEN		= 0x1B,
	DEVICE_DESC_PARAM_RTT_CAP		= 0x1C,
	DEVICE_DESC_PARAM_FRQ_RTC		= 0x1D,
	DEVICE_DESC_PARAM_UFS_FEAT		= 0x1F,
	DEVICE_DESC_PARAM_FFU_TMT		= 0x20,
	DEVICE_DESC_PARAM_Q_DPTH		= 0x21,
	DEVICE_DESC_PARAM_DEV_VER		= 0x22,
	DEVICE_DESC_PARAM_NUM_SEC_WPA		= 0x24,
	DEVICE_DESC_PARAM_PSA_MAX_DATA		= 0x25,
	DEVICE_DESC_PARAM_PSA_TMT		= 0x29,
	DEVICE_DESC_PARAM_PRDCT_REV		= 0x2A,
};

struct ufs_hba;

enum {
	UFSHCD_MAX_CHANNEL	= 0,
	UFSHCD_MAX_ID		= 1,
};

enum dev_cmd_type {
	DEV_CMD_TYPE_NOP		= 0x0,
	DEV_CMD_TYPE_QUERY		= 0x1,
};

/**
 * struct uic_command - UIC command structure
 * @command: UIC command
 * @argument1: UIC command argument 1
 * @argument2: UIC command argument 2
 * @argument3: UIC command argument 3
 * @cmd_active: Indicate if UIC command is outstanding
 * @result: UIC command result
 * @done: UIC command completion
 */
struct uic_command {
	u32 command;
	u32 argument1;
	u32 argument2;
	u32 argument3;
	int cmd_active;
	int result;
};

/* GenSelectorIndex calculation macros for M-PHY attributes */
#define UIC_ARG_MPHY_TX_GEN_SEL_INDEX(lane) (lane)
#define UIC_ARG_MPHY_RX_GEN_SEL_INDEX(lane) (PA_MAXDATALANES + (lane))

#define UIC_ARG_MIB_SEL(attr, sel)	((((attr) & 0xFFFF) << 16) |\
					 ((sel) & 0xFFFF))
#define UIC_ARG_MIB(attr)		UIC_ARG_MIB_SEL(attr, 0)
#define UIC_ARG_ATTR_TYPE(t)		(((t) & 0xFF) << 16)
#define UIC_GET_ATTR_ID(v)		(((v) >> 16) & 0xFFFF)

/* Link Status*/
enum link_status {
	UFSHCD_LINK_IS_DOWN	= 1,
	UFSHCD_LINK_IS_UP	= 2,
};

#define UIC_ARG_MIB_SEL(attr, sel)	((((attr) & 0xFFFF) << 16) |\
					 ((sel) & 0xFFFF))
#define UIC_ARG_MIB(attr)		UIC_ARG_MIB_SEL(attr, 0)
#define UIC_ARG_ATTR_TYPE(t)		(((t) & 0xFF) << 16)
#define UIC_GET_ATTR_ID(v)		(((v) >> 16) & 0xFFFF)

/* UIC Commands */
enum uic_cmd_dme {
	UIC_CMD_DME_GET			= 0x01,
	UIC_CMD_DME_SET			= 0x02,
	UIC_CMD_DME_PEER_GET		= 0x03,
	UIC_CMD_DME_PEER_SET		= 0x04,
	UIC_CMD_DME_POWERON		= 0x10,
	UIC_CMD_DME_POWEROFF		= 0x11,
	UIC_CMD_DME_ENABLE		= 0x12,
	UIC_CMD_DME_RESET		= 0x14,
	UIC_CMD_DME_END_PT_RST		= 0x15,
	UIC_CMD_DME_LINK_STARTUP	= 0x16,
	UIC_CMD_DME_HIBER_ENTER		= 0x17,
	UIC_CMD_DME_HIBER_EXIT		= 0x18,
	UIC_CMD_DME_TEST_MODE		= 0x1A,
};

/* UIC Config result code / Generic error code */
enum {
	UIC_CMD_RESULT_SUCCESS			= 0x00,
	UIC_CMD_RESULT_INVALID_ATTR		= 0x01,
	UIC_CMD_RESULT_FAILURE			= 0x01,
	UIC_CMD_RESULT_INVALID_ATTR_VALUE	= 0x02,
	UIC_CMD_RESULT_READ_ONLY_ATTR		= 0x03,
	UIC_CMD_RESULT_WRITE_ONLY_ATTR		= 0x04,
	UIC_CMD_RESULT_BAD_INDEX		= 0x05,
	UIC_CMD_RESULT_LOCKED_ATTR		= 0x06,
	UIC_CMD_RESULT_BAD_TEST_FEATURE_INDEX	= 0x07,
	UIC_CMD_RESULT_PEER_COMM_FAILURE	= 0x08,
	UIC_CMD_RESULT_BUSY			= 0x09,
	UIC_CMD_RESULT_DME_FAILURE		= 0x0A,
};

#define MASK_UIC_COMMAND_RESULT			0xFF

/* Host <-> Device UniPro Link state */
enum uic_link_state {
	UIC_LINK_OFF_STATE	= 0, /* Link powered down or disabled */
	UIC_LINK_ACTIVE_STATE	= 1, /* Link is in Fast/Slow/Sleep state */
	UIC_LINK_HIBERN8_STATE	= 2, /* Link is in Hibernate state */
};

/* UIC command interfaces for DME primitives */
#define DME_LOCAL	0
#define DME_PEER	1
#define ATTR_SET_NOR	0	/* NORMAL */
#define ATTR_SET_ST	1	/* STATIC */

int ufshcd_dme_set_attr(struct ufs_hba *hba, u32 attr_sel,
			u8 attr_set, u32 mib_val, u8 peer);
int ufshcd_dme_get_attr(struct ufs_hba *hba, u32 attr_sel,
			u32 *mib_val, u8 peer);

static inline int ufshcd_dme_set(struct ufs_hba *hba, u32 attr_sel,
				 u32 mib_val)
{
	return ufshcd_dme_set_attr(hba, attr_sel, ATTR_SET_NOR,
				   mib_val, DME_LOCAL);
}

static inline int ufshcd_dme_get(struct ufs_hba *hba,
				 u32 attr_sel, u32 *mib_val)
{
	return ufshcd_dme_get_attr(hba, attr_sel, mib_val, DME_LOCAL);
}

static inline int ufshcd_dme_peer_get(struct ufs_hba *hba,
				      u32 attr_sel, u32 *mib_val)
{
	return ufshcd_dme_get_attr(hba, attr_sel, mib_val, DME_PEER);
}

static inline int ufshcd_dme_peer_set(struct ufs_hba *hba, u32 attr_sel,
				      u32 mib_val)
{
	return ufshcd_dme_set_attr(hba, attr_sel, ATTR_SET_NOR,
				   mib_val, DME_PEER);
}

/**
 * struct ufs_query_req - parameters for building a query request
 * @query_func: UPIU header query function
 * @upiu_req: the query request data
 */
struct ufs_query_req {
	u8 query_func;
	struct utp_upiu_query upiu_req;
};

/**
 * struct ufs_query_resp - UPIU QUERY
 * @response: device response code
 * @upiu_res: query response data
 */
struct ufs_query_res {
	u8 response;
	struct utp_upiu_query upiu_res;
};

/**
 * struct ufs_query - holds relevant data structures for query request
 * @request: request upiu and function
 * @descriptor: buffer for sending/receiving descriptor
 * @response: response upiu and response
 */
struct ufs_query {
	struct ufs_query_req request;
	u8 *descriptor;
	struct ufs_query_res response;
};

/**
 * struct ufs_dev_cmd - all assosiated fields with device management commands
 * @type: device management command type - Query, NOP OUT
 * @tag_wq: wait queue until free command slot is available
 */
struct ufs_dev_cmd {
	enum dev_cmd_type type;
	struct ufs_query query;
};

struct ufs_hba_ops {
	int (*init)(struct ufs_hba *hba);
	int (*hce_enable_notify)(struct ufs_hba *hba,
				 enum ufs_notify_change_status);
	int (*link_startup_notify)(struct ufs_hba *hba,
				   enum ufs_notify_change_status);
	int (*phy_initialization)(struct ufs_hba *hba);
};

struct ufs_hba {
	struct			udevice *dev;
	void __iomem		*mmio_base;
	struct ufs_hba_ops	*ops;
	struct ufs_desc_size	desc_size;
	u32			capabilities;
	u32			version;
	u32			intr_mask;
	u32			quirks;
/*
 * If UFS host controller is having issue in processing LCC (Line
 * Control Command) coming from device then enable this quirk.
 * When this quirk is enabled, host controller driver should disable
 * the LCC transmission on UFS device (by clearing TX_LCC_ENABLE
 * attribute of device to 0).
 */
#define UFSHCD_QUIRK_BROKEN_LCC				BIT(0)

/*
 * This quirk needs to be enabled if the host controller has
 * 64-bit addressing supported capability but it doesn't work.
 */
#define UFSHCD_QUIRK_BROKEN_64BIT_ADDRESS		BIT(1)

/*
 * This quirk needs to be enabled if the host controller has
 * auto-hibernate capability but it's FASTAUTO only.
 */
#define UFSHCD_QUIRK_HIBERN_FASTAUTO			BIT(2)

	/* Virtual memory reference */
	struct utp_transfer_cmd_desc *ucdl;
	struct utp_transfer_req_desc *utrdl;
	/* TODO: Add Task Manegement Support */
	struct utp_task_req_desc *utmrdl;

	struct utp_upiu_req *ucd_req_ptr;
	struct utp_upiu_rsp *ucd_rsp_ptr;
	struct ufshcd_sg_entry *ucd_prdt_ptr;

	/* Power Mode information */
	enum ufs_dev_pwr_mode curr_dev_pwr_mode;
	struct ufs_pa_layer_attr pwr_info;
	struct ufs_pwr_mode_info max_pwr_info;

	struct ufs_dev_cmd dev_cmd;
};

static inline int ufshcd_ops_init(struct ufs_hba *hba)
{
	if (hba->ops && hba->ops->init)
		return hba->ops->init(hba);

	return 0;
}

static inline int ufshcd_ops_hce_enable_notify(struct ufs_hba *hba,
						bool status)
{
	if (hba->ops && hba->ops->hce_enable_notify)
		return hba->ops->hce_enable_notify(hba, status);

	return 0;
}

static inline int ufshcd_ops_link_startup_notify(struct ufs_hba *hba,
						 bool status)
{
	if (hba->ops && hba->ops->link_startup_notify)
		return hba->ops->link_startup_notify(hba, status);

	return 0;
}

/* Controller UFSHCI version */
enum {
	UFSHCI_VERSION_10 = 0x00010000, /* 1.0 */
	UFSHCI_VERSION_11 = 0x00010100, /* 1.1 */
	UFSHCI_VERSION_20 = 0x00000200, /* 2.0 */
	UFSHCI_VERSION_21 = 0x00000210, /* 2.1 */
	UFSHCI_VERSION_30 = 0x00000300, /* 3.0 */
	UFSHCI_VERSION_31 = 0x00000310, /* 3.1 */
};

/* Interrupt disable masks */
enum {
	/* Interrupt disable mask for UFSHCI v1.0 */
	INTERRUPT_MASK_ALL_VER_10	= 0x30FFF,
	INTERRUPT_MASK_RW_VER_10	= 0x30000,

	/* Interrupt disable mask for UFSHCI v1.1 */
	INTERRUPT_MASK_ALL_VER_11	= 0x31FFF,

	/* Interrupt disable mask for UFSHCI v2.1 */
	INTERRUPT_MASK_ALL_VER_21	= 0x71FFF,
};

/* UFSHCI Registers */
enum {
	REG_CONTROLLER_CAPABILITIES		= 0x00,
	REG_UFS_VERSION				= 0x08,
	REG_CONTROLLER_DEV_ID			= 0x10,
	REG_CONTROLLER_PROD_ID			= 0x14,
	REG_AUTO_HIBERNATE_IDLE_TIMER		= 0x18,
	REG_INTERRUPT_STATUS			= 0x20,
	REG_INTERRUPT_ENABLE			= 0x24,
	REG_CONTROLLER_STATUS			= 0x30,
	REG_CONTROLLER_ENABLE			= 0x34,
	REG_UIC_ERROR_CODE_PHY_ADAPTER_LAYER	= 0x38,
	REG_UIC_ERROR_CODE_DATA_LINK_LAYER	= 0x3C,
	REG_UIC_ERROR_CODE_NETWORK_LAYER	= 0x40,
	REG_UIC_ERROR_CODE_TRANSPORT_LAYER	= 0x44,
	REG_UIC_ERROR_CODE_DME			= 0x48,
	REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL	= 0x4C,
	REG_UTP_TRANSFER_REQ_LIST_BASE_L	= 0x50,
	REG_UTP_TRANSFER_REQ_LIST_BASE_H	= 0x54,
	REG_UTP_TRANSFER_REQ_DOOR_BELL		= 0x58,
	REG_UTP_TRANSFER_REQ_LIST_CLEAR		= 0x5C,
	REG_UTP_TRANSFER_REQ_LIST_RUN_STOP	= 0x60,
	REG_UTP_TASK_REQ_LIST_BASE_L		= 0x70,
	REG_UTP_TASK_REQ_LIST_BASE_H		= 0x74,
	REG_UTP_TASK_REQ_DOOR_BELL		= 0x78,
	REG_UTP_TASK_REQ_LIST_CLEAR		= 0x7C,
	REG_UTP_TASK_REQ_LIST_RUN_STOP		= 0x80,
	REG_UIC_COMMAND				= 0x90,
	REG_UIC_COMMAND_ARG_1			= 0x94,
	REG_UIC_COMMAND_ARG_2			= 0x98,
	REG_UIC_COMMAND_ARG_3			= 0x9C,

	UFSHCI_REG_SPACE_SIZE			= 0xA0,

	REG_UFS_CCAP				= 0x100,
	REG_UFS_CRYPTOCAP			= 0x104,

	UFSHCI_CRYPTO_REG_SPACE_SIZE		= 0x400,
};

/* Controller capability masks */
enum {
	MASK_TRANSFER_REQUESTS_SLOTS		= 0x0000001F,
	MASK_TASK_MANAGEMENT_REQUEST_SLOTS	= 0x00070000,
	MASK_AUTO_HIBERN8_SUPPORT		= 0x00800000,
	MASK_64_ADDRESSING_SUPPORT		= 0x01000000,
	MASK_OUT_OF_ORDER_DATA_DELIVERY_SUPPORT	= 0x02000000,
	MASK_UIC_DME_TEST_MODE_SUPPORT		= 0x04000000,
};

/* Interrupt Status 20h */
#define UTP_TRANSFER_REQ_COMPL			0x1
#define UIC_DME_END_PT_RESET			0x2
#define UIC_ERROR				0x4
#define UIC_TEST_MODE				0x8
#define UIC_POWER_MODE				0x10
#define UIC_HIBERNATE_EXIT			0x20
#define UIC_HIBERNATE_ENTER			0x40
#define UIC_LINK_LOST				0x80
#define UIC_LINK_STARTUP			0x100
#define UTP_TASK_REQ_COMPL			0x200
#define UIC_COMMAND_COMPL			0x400
#define DEVICE_FATAL_ERROR			0x800
#define CONTROLLER_FATAL_ERROR			0x10000
#define SYSTEM_BUS_FATAL_ERROR			0x20000

#define UFSHCD_UIC_PWR_MASK	(UIC_HIBERNATE_ENTER |\
				UIC_HIBERNATE_EXIT |\
				UIC_POWER_MODE)

#define UFSHCD_UIC_MASK		(UIC_COMMAND_COMPL | UIC_POWER_MODE)

#define UFSHCD_ERROR_MASK	(UIC_ERROR |\
				DEVICE_FATAL_ERROR |\
				CONTROLLER_FATAL_ERROR |\
				SYSTEM_BUS_FATAL_ERROR)

#define INT_FATAL_ERRORS	(DEVICE_FATAL_ERROR |\
				CONTROLLER_FATAL_ERROR |\
				SYSTEM_BUS_FATAL_ERROR)

/* Host Controller Enable 0x34h */
#define CONTROLLER_ENABLE	0x1
#define CONTROLLER_DISABLE	0x0
/* HCS - Host Controller Status 30h */
#define DEVICE_PRESENT				0x1
#define UTP_TRANSFER_REQ_LIST_READY		0x2
#define UTP_TASK_REQ_LIST_READY			0x4
#define UIC_COMMAND_READY			0x8
#define HOST_ERROR_INDICATOR			0x10
#define DEVICE_ERROR_INDICATOR			0x20
#define UIC_POWER_MODE_CHANGE_REQ_STATUS_MASK	UFS_MASK(0x7, 8)

#define UFSHCD_STATUS_READY	(UTP_TRANSFER_REQ_LIST_READY |\
				UTP_TASK_REQ_LIST_READY |\
				UIC_COMMAND_READY)

enum {
	PWR_OK		= 0x0,
	PWR_LOCAL	= 0x01,
	PWR_REMOTE	= 0x02,
	PWR_BUSY	= 0x03,
	PWR_ERROR_CAP	= 0x04,
	PWR_FATAL_ERROR	= 0x05,
};

/* UICCMD - UIC Command */
#define COMMAND_OPCODE_MASK		0xFF
#define GEN_SELECTOR_INDEX_MASK		0xFFFF

#define MIB_ATTRIBUTE_MASK		UFS_MASK(0xFFFF, 16)
#define RESET_LEVEL			0xFF

#define ATTR_SET_TYPE_MASK		UFS_MASK(0xFF, 16)
#define CFG_RESULT_CODE_MASK		0xFF
#define GENERIC_ERROR_CODE_MASK		0xFF

#define ufshcd_writel(hba, val, reg)   \
	writel((val), (hba)->mmio_base + (reg))
#define ufshcd_readl(hba, reg) \
	readl((hba)->mmio_base + (reg))

/* UTRLRSR - UTP Transfer Request Run-Stop Register 60h */
#define UTP_TRANSFER_REQ_LIST_RUN_STOP_BIT	0x1

/* UTMRLRSR - UTP Task Management Request Run-Stop Register 80h */
#define UTP_TASK_REQ_LIST_RUN_STOP_BIT		0x1

int ufshcd_probe(struct udevice *dev, struct ufs_hba_ops *hba_ops);

#endif
