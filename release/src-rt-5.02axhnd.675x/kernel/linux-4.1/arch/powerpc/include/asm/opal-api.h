/*
 * OPAL API definitions.
 *
 * Copyright 2011-2015 IBM Corp.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version
 * 2 of the License, or (at your option) any later version.
 */

#ifndef __OPAL_API_H
#define __OPAL_API_H

/****** OPAL APIs ******/

/* Return codes */
#define OPAL_SUCCESS		0
#define OPAL_PARAMETER		-1
#define OPAL_BUSY		-2
#define OPAL_PARTIAL		-3
#define OPAL_CONSTRAINED	-4
#define OPAL_CLOSED		-5
#define OPAL_HARDWARE		-6
#define OPAL_UNSUPPORTED	-7
#define OPAL_PERMISSION		-8
#define OPAL_NO_MEM		-9
#define OPAL_RESOURCE		-10
#define OPAL_INTERNAL_ERROR	-11
#define OPAL_BUSY_EVENT		-12
#define OPAL_HARDWARE_FROZEN	-13
#define OPAL_WRONG_STATE	-14
#define OPAL_ASYNC_COMPLETION	-15
#define OPAL_EMPTY		-16
#define OPAL_I2C_TIMEOUT	-17
#define OPAL_I2C_INVALID_CMD	-18
#define OPAL_I2C_LBUS_PARITY	-19
#define OPAL_I2C_BKEND_OVERRUN	-20
#define OPAL_I2C_BKEND_ACCESS	-21
#define OPAL_I2C_ARBT_LOST	-22
#define OPAL_I2C_NACK_RCVD	-23
#define OPAL_I2C_STOP_ERR	-24

/* API Tokens (in r0) */
#define OPAL_INVALID_CALL		       -1
#define OPAL_TEST				0
#define OPAL_CONSOLE_WRITE			1
#define OPAL_CONSOLE_READ			2
#define OPAL_RTC_READ				3
#define OPAL_RTC_WRITE				4
#define OPAL_CEC_POWER_DOWN			5
#define OPAL_CEC_REBOOT				6
#define OPAL_READ_NVRAM				7
#define OPAL_WRITE_NVRAM			8
#define OPAL_HANDLE_INTERRUPT			9
#define OPAL_POLL_EVENTS			10
#define OPAL_PCI_SET_HUB_TCE_MEMORY		11
#define OPAL_PCI_SET_PHB_TCE_MEMORY		12
#define OPAL_PCI_CONFIG_READ_BYTE		13
#define OPAL_PCI_CONFIG_READ_HALF_WORD  	14
#define OPAL_PCI_CONFIG_READ_WORD		15
#define OPAL_PCI_CONFIG_WRITE_BYTE		16
#define OPAL_PCI_CONFIG_WRITE_HALF_WORD		17
#define OPAL_PCI_CONFIG_WRITE_WORD		18
#define OPAL_SET_XIVE				19
#define OPAL_GET_XIVE				20
#define OPAL_GET_COMPLETION_TOKEN_STATUS	21 /* obsolete */
#define OPAL_REGISTER_OPAL_EXCEPTION_HANDLER	22
#define OPAL_PCI_EEH_FREEZE_STATUS		23
#define OPAL_PCI_SHPC				24
#define OPAL_CONSOLE_WRITE_BUFFER_SPACE		25
#define OPAL_PCI_EEH_FREEZE_CLEAR		26
#define OPAL_PCI_PHB_MMIO_ENABLE		27
#define OPAL_PCI_SET_PHB_MEM_WINDOW		28
#define OPAL_PCI_MAP_PE_MMIO_WINDOW		29
#define OPAL_PCI_SET_PHB_TABLE_MEMORY		30
#define OPAL_PCI_SET_PE				31
#define OPAL_PCI_SET_PELTV			32
#define OPAL_PCI_SET_MVE			33
#define OPAL_PCI_SET_MVE_ENABLE			34
#define OPAL_PCI_GET_XIVE_REISSUE		35
#define OPAL_PCI_SET_XIVE_REISSUE		36
#define OPAL_PCI_SET_XIVE_PE			37
#define OPAL_GET_XIVE_SOURCE			38
#define OPAL_GET_MSI_32				39
#define OPAL_GET_MSI_64				40
#define OPAL_START_CPU				41
#define OPAL_QUERY_CPU_STATUS			42
#define OPAL_WRITE_OPPANEL			43 /* unimplemented */
#define OPAL_PCI_MAP_PE_DMA_WINDOW		44
#define OPAL_PCI_MAP_PE_DMA_WINDOW_REAL		45
#define OPAL_PCI_RESET				49
#define OPAL_PCI_GET_HUB_DIAG_DATA		50
#define OPAL_PCI_GET_PHB_DIAG_DATA		51
#define OPAL_PCI_FENCE_PHB			52
#define OPAL_PCI_REINIT				53
#define OPAL_PCI_MASK_PE_ERROR			54
#define OPAL_SET_SLOT_LED_STATUS		55
#define OPAL_GET_EPOW_STATUS			56
#define OPAL_SET_SYSTEM_ATTENTION_LED		57
#define OPAL_RESERVED1				58
#define OPAL_RESERVED2				59
#define OPAL_PCI_NEXT_ERROR			60
#define OPAL_PCI_EEH_FREEZE_STATUS2		61
#define OPAL_PCI_POLL				62
#define OPAL_PCI_MSI_EOI			63
#define OPAL_PCI_GET_PHB_DIAG_DATA2		64
#define OPAL_XSCOM_READ				65
#define OPAL_XSCOM_WRITE			66
#define OPAL_LPC_READ				67
#define OPAL_LPC_WRITE				68
#define OPAL_RETURN_CPU				69
#define OPAL_REINIT_CPUS			70
#define OPAL_ELOG_READ				71
#define OPAL_ELOG_WRITE				72
#define OPAL_ELOG_ACK				73
#define OPAL_ELOG_RESEND			74
#define OPAL_ELOG_SIZE				75
#define OPAL_FLASH_VALIDATE			76
#define OPAL_FLASH_MANAGE			77
#define OPAL_FLASH_UPDATE			78
#define OPAL_RESYNC_TIMEBASE			79
#define OPAL_CHECK_TOKEN			80
#define OPAL_DUMP_INIT				81
#define OPAL_DUMP_INFO				82
#define OPAL_DUMP_READ				83
#define OPAL_DUMP_ACK				84
#define OPAL_GET_MSG				85
#define OPAL_CHECK_ASYNC_COMPLETION		86
#define OPAL_SYNC_HOST_REBOOT			87
#define OPAL_SENSOR_READ			88
#define OPAL_GET_PARAM				89
#define OPAL_SET_PARAM				90
#define OPAL_DUMP_RESEND			91
#define OPAL_ELOG_SEND				92	/* Deprecated */
#define OPAL_PCI_SET_PHB_CAPI_MODE		93
#define OPAL_DUMP_INFO2				94
#define OPAL_WRITE_OPPANEL_ASYNC		95
#define OPAL_PCI_ERR_INJECT			96
#define OPAL_PCI_EEH_FREEZE_SET			97
#define OPAL_HANDLE_HMI				98
#define OPAL_CONFIG_CPU_IDLE_STATE		99
#define OPAL_SLW_SET_REG			100
#define OPAL_REGISTER_DUMP_REGION		101
#define OPAL_UNREGISTER_DUMP_REGION		102
#define OPAL_WRITE_TPO				103
#define OPAL_READ_TPO				104
#define OPAL_GET_DPO_STATUS			105
#define OPAL_OLD_I2C_REQUEST			106	/* Deprecated */
#define OPAL_IPMI_SEND				107
#define OPAL_IPMI_RECV				108
#define OPAL_I2C_REQUEST			109
#define OPAL_FLASH_READ				110
#define OPAL_FLASH_WRITE			111
#define OPAL_FLASH_ERASE			112
#define OPAL_LAST				112

/* Device tree flags */

/* Flags set in power-mgmt nodes in device tree if
 * respective idle states are supported in the platform.
 */
#define OPAL_PM_NAP_ENABLED		0x00010000
#define OPAL_PM_SLEEP_ENABLED		0x00020000
#define OPAL_PM_WINKLE_ENABLED		0x00040000
#define OPAL_PM_SLEEP_ENABLED_ER1	0x00080000 /* with workaround */

#ifndef __ASSEMBLY__

/* Other enums */
enum OpalFreezeState {
	OPAL_EEH_STOPPED_NOT_FROZEN = 0,
	OPAL_EEH_STOPPED_MMIO_FREEZE = 1,
	OPAL_EEH_STOPPED_DMA_FREEZE = 2,
	OPAL_EEH_STOPPED_MMIO_DMA_FREEZE = 3,
	OPAL_EEH_STOPPED_RESET = 4,
	OPAL_EEH_STOPPED_TEMP_UNAVAIL = 5,
	OPAL_EEH_STOPPED_PERM_UNAVAIL = 6
};

enum OpalEehFreezeActionToken {
	OPAL_EEH_ACTION_CLEAR_FREEZE_MMIO = 1,
	OPAL_EEH_ACTION_CLEAR_FREEZE_DMA = 2,
	OPAL_EEH_ACTION_CLEAR_FREEZE_ALL = 3,

	OPAL_EEH_ACTION_SET_FREEZE_MMIO = 1,
	OPAL_EEH_ACTION_SET_FREEZE_DMA  = 2,
	OPAL_EEH_ACTION_SET_FREEZE_ALL  = 3
};

enum OpalPciStatusToken {
	OPAL_EEH_NO_ERROR	= 0,
	OPAL_EEH_IOC_ERROR	= 1,
	OPAL_EEH_PHB_ERROR	= 2,
	OPAL_EEH_PE_ERROR	= 3,
	OPAL_EEH_PE_MMIO_ERROR	= 4,
	OPAL_EEH_PE_DMA_ERROR	= 5
};

enum OpalPciErrorSeverity {
	OPAL_EEH_SEV_NO_ERROR	= 0,
	OPAL_EEH_SEV_IOC_DEAD	= 1,
	OPAL_EEH_SEV_PHB_DEAD	= 2,
	OPAL_EEH_SEV_PHB_FENCED	= 3,
	OPAL_EEH_SEV_PE_ER	= 4,
	OPAL_EEH_SEV_INF	= 5
};

enum OpalErrinjectType {
	OPAL_ERR_INJECT_TYPE_IOA_BUS_ERR	= 0,
	OPAL_ERR_INJECT_TYPE_IOA_BUS_ERR64	= 1,
};

enum OpalErrinjectFunc {
	/* IOA bus specific errors */
	OPAL_ERR_INJECT_FUNC_IOA_LD_MEM_ADDR	= 0,
	OPAL_ERR_INJECT_FUNC_IOA_LD_MEM_DATA	= 1,
	OPAL_ERR_INJECT_FUNC_IOA_LD_IO_ADDR	= 2,
	OPAL_ERR_INJECT_FUNC_IOA_LD_IO_DATA	= 3,
	OPAL_ERR_INJECT_FUNC_IOA_LD_CFG_ADDR	= 4,
	OPAL_ERR_INJECT_FUNC_IOA_LD_CFG_DATA	= 5,
	OPAL_ERR_INJECT_FUNC_IOA_ST_MEM_ADDR	= 6,
	OPAL_ERR_INJECT_FUNC_IOA_ST_MEM_DATA	= 7,
	OPAL_ERR_INJECT_FUNC_IOA_ST_IO_ADDR	= 8,
	OPAL_ERR_INJECT_FUNC_IOA_ST_IO_DATA	= 9,
	OPAL_ERR_INJECT_FUNC_IOA_ST_CFG_ADDR	= 10,
	OPAL_ERR_INJECT_FUNC_IOA_ST_CFG_DATA	= 11,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_RD_ADDR	= 12,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_RD_DATA	= 13,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_RD_MASTER	= 14,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_RD_TARGET	= 15,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_WR_ADDR	= 16,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_WR_DATA	= 17,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_WR_MASTER	= 18,
	OPAL_ERR_INJECT_FUNC_IOA_DMA_WR_TARGET	= 19,
};

enum OpalMmioWindowType {
	OPAL_M32_WINDOW_TYPE = 1,
	OPAL_M64_WINDOW_TYPE = 2,
	OPAL_IO_WINDOW_TYPE  = 3
};

enum OpalExceptionHandler {
	OPAL_MACHINE_CHECK_HANDLER	    = 1,
	OPAL_HYPERVISOR_MAINTENANCE_HANDLER = 2,
	OPAL_SOFTPATCH_HANDLER		    = 3
};

enum OpalPendingState {
	OPAL_EVENT_OPAL_INTERNAL   = 0x1,
	OPAL_EVENT_NVRAM	   = 0x2,
	OPAL_EVENT_RTC		   = 0x4,
	OPAL_EVENT_CONSOLE_OUTPUT  = 0x8,
	OPAL_EVENT_CONSOLE_INPUT   = 0x10,
	OPAL_EVENT_ERROR_LOG_AVAIL = 0x20,
	OPAL_EVENT_ERROR_LOG	   = 0x40,
	OPAL_EVENT_EPOW		   = 0x80,
	OPAL_EVENT_LED_STATUS	   = 0x100,
	OPAL_EVENT_PCI_ERROR	   = 0x200,
	OPAL_EVENT_DUMP_AVAIL	   = 0x400,
	OPAL_EVENT_MSG_PENDING	   = 0x800,
};

enum OpalThreadStatus {
	OPAL_THREAD_INACTIVE = 0x0,
	OPAL_THREAD_STARTED = 0x1,
	OPAL_THREAD_UNAVAILABLE = 0x2 /* opal-v3 */
};

enum OpalPciBusCompare {
	OpalPciBusAny	= 0,	/* Any bus number match */
	OpalPciBus3Bits	= 2,	/* Match top 3 bits of bus number */
	OpalPciBus4Bits	= 3,	/* Match top 4 bits of bus number */
	OpalPciBus5Bits	= 4,	/* Match top 5 bits of bus number */
	OpalPciBus6Bits	= 5,	/* Match top 6 bits of bus number */
	OpalPciBus7Bits	= 6,	/* Match top 7 bits of bus number */
	OpalPciBusAll	= 7,	/* Match bus number exactly */
};

enum OpalDeviceCompare {
	OPAL_IGNORE_RID_DEVICE_NUMBER = 0,
	OPAL_COMPARE_RID_DEVICE_NUMBER = 1
};

enum OpalFuncCompare {
	OPAL_IGNORE_RID_FUNCTION_NUMBER = 0,
	OPAL_COMPARE_RID_FUNCTION_NUMBER = 1
};

enum OpalPeAction {
	OPAL_UNMAP_PE = 0,
	OPAL_MAP_PE = 1
};

enum OpalPeltvAction {
	OPAL_REMOVE_PE_FROM_DOMAIN = 0,
	OPAL_ADD_PE_TO_DOMAIN = 1
};

enum OpalMveEnableAction {
	OPAL_DISABLE_MVE = 0,
	OPAL_ENABLE_MVE = 1
};

enum OpalM64Action {
	OPAL_DISABLE_M64 = 0,
	OPAL_ENABLE_M64_SPLIT = 1,
	OPAL_ENABLE_M64_NON_SPLIT = 2
};

enum OpalPciResetScope {
	OPAL_RESET_PHB_COMPLETE		= 1,
	OPAL_RESET_PCI_LINK		= 2,
	OPAL_RESET_PHB_ERROR		= 3,
	OPAL_RESET_PCI_HOT		= 4,
	OPAL_RESET_PCI_FUNDAMENTAL	= 5,
	OPAL_RESET_PCI_IODA_TABLE	= 6
};

enum OpalPciReinitScope {
	/*
	 * Note: we chose values that do not overlap
	 * OpalPciResetScope as OPAL v2 used the same
	 * enum for both
	 */
	OPAL_REINIT_PCI_DEV = 1000
};

enum OpalPciResetState {
	OPAL_DEASSERT_RESET = 0,
	OPAL_ASSERT_RESET   = 1
};

/*
 * Address cycle types for LPC accesses. These also correspond
 * to the content of the first cell of the "reg" property for
 * device nodes on the LPC bus
 */
enum OpalLPCAddressType {
	OPAL_LPC_MEM	= 0,
	OPAL_LPC_IO	= 1,
	OPAL_LPC_FW	= 2,
};

enum opal_msg_type {
	OPAL_MSG_ASYNC_COMP = 0,	/* params[0] = token, params[1] = rc,
					 * additional params function-specific
					 */
	OPAL_MSG_MEM_ERR,
	OPAL_MSG_EPOW,
	OPAL_MSG_SHUTDOWN,		/* params[0] = 1 reboot, 0 shutdown */
	OPAL_MSG_HMI_EVT,
	OPAL_MSG_DPO,
	OPAL_MSG_TYPE_MAX,
};

struct opal_msg {
	__be32 msg_type;
	__be32 reserved;
	__be64 params[8];
};

/* System parameter permission */
enum OpalSysparamPerm {
	OPAL_SYSPARAM_READ  = 0x1,
	OPAL_SYSPARAM_WRITE = 0x2,
	OPAL_SYSPARAM_RW    = (OPAL_SYSPARAM_READ | OPAL_SYSPARAM_WRITE),
};

enum {
	OPAL_IPMI_MSG_FORMAT_VERSION_1 = 1,
};

struct opal_ipmi_msg {
	uint8_t version;
	uint8_t netfn;
	uint8_t cmd;
	uint8_t data[];
};

/* FSP memory errors handling */
enum OpalMemErr_Version {
	OpalMemErr_V1 = 1,
};

enum OpalMemErrType {
	OPAL_MEM_ERR_TYPE_RESILIENCE	= 0,
	OPAL_MEM_ERR_TYPE_DYN_DALLOC,
};

/* Memory Reilience error type */
enum OpalMemErr_ResilErrType {
	OPAL_MEM_RESILIENCE_CE		= 0,
	OPAL_MEM_RESILIENCE_UE,
	OPAL_MEM_RESILIENCE_UE_SCRUB,
};

/* Dynamic Memory Deallocation type */
enum OpalMemErr_DynErrType {
	OPAL_MEM_DYNAMIC_DEALLOC	= 0,
};

struct OpalMemoryErrorData {
	enum OpalMemErr_Version	version:8;	/* 0x00 */
	enum OpalMemErrType	type:8;		/* 0x01 */
	__be16			flags;		/* 0x02 */
	uint8_t			reserved_1[4];	/* 0x04 */

	union {
		/* Memory Resilience corrected/uncorrected error info */
		struct {
			enum OpalMemErr_ResilErrType	resil_err_type:8;
			uint8_t				reserved_1[7];
			__be64				physical_address_start;
			__be64				physical_address_end;
		} resilience;
		/* Dynamic memory deallocation error info */
		struct {
			enum OpalMemErr_DynErrType	dyn_err_type:8;
			uint8_t				reserved_1[7];
			__be64				physical_address_start;
			__be64				physical_address_end;
		} dyn_dealloc;
	} u;
};

/* HMI interrupt event */
enum OpalHMI_Version {
	OpalHMIEvt_V1 = 1,
};

enum OpalHMI_Severity {
	OpalHMI_SEV_NO_ERROR = 0,
	OpalHMI_SEV_WARNING = 1,
	OpalHMI_SEV_ERROR_SYNC = 2,
	OpalHMI_SEV_FATAL = 3,
};

enum OpalHMI_Disposition {
	OpalHMI_DISPOSITION_RECOVERED = 0,
	OpalHMI_DISPOSITION_NOT_RECOVERED = 1,
};

enum OpalHMI_ErrType {
	OpalHMI_ERROR_MALFUNC_ALERT	= 0,
	OpalHMI_ERROR_PROC_RECOV_DONE,
	OpalHMI_ERROR_PROC_RECOV_DONE_AGAIN,
	OpalHMI_ERROR_PROC_RECOV_MASKED,
	OpalHMI_ERROR_TFAC,
	OpalHMI_ERROR_TFMR_PARITY,
	OpalHMI_ERROR_HA_OVERFLOW_WARN,
	OpalHMI_ERROR_XSCOM_FAIL,
	OpalHMI_ERROR_XSCOM_DONE,
	OpalHMI_ERROR_SCOM_FIR,
	OpalHMI_ERROR_DEBUG_TRIG_FIR,
	OpalHMI_ERROR_HYP_RESOURCE,
	OpalHMI_ERROR_CAPP_RECOVERY,
};

struct OpalHMIEvent {
	uint8_t		version;	/* 0x00 */
	uint8_t		severity;	/* 0x01 */
	uint8_t		type;		/* 0x02 */
	uint8_t		disposition;	/* 0x03 */
	uint8_t		reserved_1[4];	/* 0x04 */

	__be64		hmer;
	/* TFMR register. Valid only for TFAC and TFMR_PARITY error type. */
	__be64		tfmr;
};

enum {
	OPAL_P7IOC_DIAG_TYPE_NONE	= 0,
	OPAL_P7IOC_DIAG_TYPE_RGC	= 1,
	OPAL_P7IOC_DIAG_TYPE_BI		= 2,
	OPAL_P7IOC_DIAG_TYPE_CI		= 3,
	OPAL_P7IOC_DIAG_TYPE_MISC	= 4,
	OPAL_P7IOC_DIAG_TYPE_I2C	= 5,
	OPAL_P7IOC_DIAG_TYPE_LAST	= 6
};

struct OpalIoP7IOCErrorData {
	__be16 type;

	/* GEM */
	__be64 gemXfir;
	__be64 gemRfir;
	__be64 gemRirqfir;
	__be64 gemMask;
	__be64 gemRwof;

	/* LEM */
	__be64 lemFir;
	__be64 lemErrMask;
	__be64 lemAction0;
	__be64 lemAction1;
	__be64 lemWof;

	union {
		struct OpalIoP7IOCRgcErrorData {
			__be64 rgcStatus;	/* 3E1C10 */
			__be64 rgcLdcp;		/* 3E1C18 */
		}rgc;
		struct OpalIoP7IOCBiErrorData {
			__be64 biLdcp0;		/* 3C0100, 3C0118 */
			__be64 biLdcp1;		/* 3C0108, 3C0120 */
			__be64 biLdcp2;		/* 3C0110, 3C0128 */
			__be64 biFenceStatus;	/* 3C0130, 3C0130 */

			uint8_t biDownbound;	/* BI Downbound or Upbound */
		}bi;
		struct OpalIoP7IOCCiErrorData {
			__be64 ciPortStatus;	/* 3Dn008 */
			__be64 ciPortLdcp;	/* 3Dn010 */

			uint8_t ciPort;		/* Index of CI port: 0/1 */
		}ci;
	};
};

/**
 * This structure defines the overlay which will be used to store PHB error
 * data upon request.
 */
enum {
	OPAL_PHB_ERROR_DATA_VERSION_1 = 1,
};

enum {
	OPAL_PHB_ERROR_DATA_TYPE_P7IOC = 1,
	OPAL_PHB_ERROR_DATA_TYPE_PHB3 = 2
};

enum {
	OPAL_P7IOC_NUM_PEST_REGS = 128,
	OPAL_PHB3_NUM_PEST_REGS = 256
};

struct OpalIoPhbErrorCommon {
	__be32 version;
	__be32 ioType;
	__be32 len;
};

struct OpalIoP7IOCPhbErrorData {
	struct OpalIoPhbErrorCommon common;

	__be32 brdgCtl;

	// P7IOC utl regs
	__be32 portStatusReg;
	__be32 rootCmplxStatus;
	__be32 busAgentStatus;

	// P7IOC cfg regs
	__be32 deviceStatus;
	__be32 slotStatus;
	__be32 linkStatus;
	__be32 devCmdStatus;
	__be32 devSecStatus;

	// cfg AER regs
	__be32 rootErrorStatus;
	__be32 uncorrErrorStatus;
	__be32 corrErrorStatus;
	__be32 tlpHdr1;
	__be32 tlpHdr2;
	__be32 tlpHdr3;
	__be32 tlpHdr4;
	__be32 sourceId;

	__be32 rsv3;

	// Record data about the call to allocate a buffer.
	__be64 errorClass;
	__be64 correlator;

	//P7IOC MMIO Error Regs
	__be64 p7iocPlssr;                // n120
	__be64 p7iocCsr;                  // n110
	__be64 lemFir;                    // nC00
	__be64 lemErrorMask;              // nC18
	__be64 lemWOF;                    // nC40
	__be64 phbErrorStatus;            // nC80
	__be64 phbFirstErrorStatus;       // nC88
	__be64 phbErrorLog0;              // nCC0
	__be64 phbErrorLog1;              // nCC8
	__be64 mmioErrorStatus;           // nD00
	__be64 mmioFirstErrorStatus;      // nD08
	__be64 mmioErrorLog0;             // nD40
	__be64 mmioErrorLog1;             // nD48
	__be64 dma0ErrorStatus;           // nD80
	__be64 dma0FirstErrorStatus;      // nD88
	__be64 dma0ErrorLog0;             // nDC0
	__be64 dma0ErrorLog1;             // nDC8
	__be64 dma1ErrorStatus;           // nE00
	__be64 dma1FirstErrorStatus;      // nE08
	__be64 dma1ErrorLog0;             // nE40
	__be64 dma1ErrorLog1;             // nE48
	__be64 pestA[OPAL_P7IOC_NUM_PEST_REGS];
	__be64 pestB[OPAL_P7IOC_NUM_PEST_REGS];
};

struct OpalIoPhb3ErrorData {
	struct OpalIoPhbErrorCommon common;

	__be32 brdgCtl;

	/* PHB3 UTL regs */
	__be32 portStatusReg;
	__be32 rootCmplxStatus;
	__be32 busAgentStatus;

	/* PHB3 cfg regs */
	__be32 deviceStatus;
	__be32 slotStatus;
	__be32 linkStatus;
	__be32 devCmdStatus;
	__be32 devSecStatus;

	/* cfg AER regs */
	__be32 rootErrorStatus;
	__be32 uncorrErrorStatus;
	__be32 corrErrorStatus;
	__be32 tlpHdr1;
	__be32 tlpHdr2;
	__be32 tlpHdr3;
	__be32 tlpHdr4;
	__be32 sourceId;

	__be32 rsv3;

	/* Record data about the call to allocate a buffer */
	__be64 errorClass;
	__be64 correlator;

	/* PHB3 MMIO Error Regs */
	__be64 nFir;			/* 000 */
	__be64 nFirMask;		/* 003 */
	__be64 nFirWOF;		/* 008 */
	__be64 phbPlssr;		/* 120 */
	__be64 phbCsr;		/* 110 */
	__be64 lemFir;		/* C00 */
	__be64 lemErrorMask;		/* C18 */
	__be64 lemWOF;		/* C40 */
	__be64 phbErrorStatus;	/* C80 */
	__be64 phbFirstErrorStatus;	/* C88 */
	__be64 phbErrorLog0;		/* CC0 */
	__be64 phbErrorLog1;		/* CC8 */
	__be64 mmioErrorStatus;	/* D00 */
	__be64 mmioFirstErrorStatus;	/* D08 */
	__be64 mmioErrorLog0;		/* D40 */
	__be64 mmioErrorLog1;		/* D48 */
	__be64 dma0ErrorStatus;	/* D80 */
	__be64 dma0FirstErrorStatus;	/* D88 */
	__be64 dma0ErrorLog0;		/* DC0 */
	__be64 dma0ErrorLog1;		/* DC8 */
	__be64 dma1ErrorStatus;	/* E00 */
	__be64 dma1FirstErrorStatus;	/* E08 */
	__be64 dma1ErrorLog0;		/* E40 */
	__be64 dma1ErrorLog1;		/* E48 */
	__be64 pestA[OPAL_PHB3_NUM_PEST_REGS];
	__be64 pestB[OPAL_PHB3_NUM_PEST_REGS];
};

enum {
	OPAL_REINIT_CPUS_HILE_BE	= (1 << 0),
	OPAL_REINIT_CPUS_HILE_LE	= (1 << 1),
};

typedef struct oppanel_line {
	__be64 line;
	__be64 line_len;
} oppanel_line_t;

/*
 * SG entries
 *
 * WARNING: The current implementation requires each entry
 * to represent a block that is 4k aligned *and* each block
 * size except the last one in the list to be as well.
 */
struct opal_sg_entry {
	__be64 data;
	__be64 length;
};

/*
 * Candiate image SG list.
 *
 * length = VER | length
 */
struct opal_sg_list {
	__be64 length;
	__be64 next;
	struct opal_sg_entry entry[];
};

/*
 * Dump region ID range usable by the OS
 */
#define OPAL_DUMP_REGION_HOST_START		0x80
#define OPAL_DUMP_REGION_LOG_BUF		0x80
#define OPAL_DUMP_REGION_HOST_END		0xFF

/* CAPI modes for PHB */
enum {
	OPAL_PHB_CAPI_MODE_PCIE		= 0,
	OPAL_PHB_CAPI_MODE_CAPI		= 1,
	OPAL_PHB_CAPI_MODE_SNOOP_OFF    = 2,
	OPAL_PHB_CAPI_MODE_SNOOP_ON	= 3,
};

/* OPAL I2C request */
struct opal_i2c_request {
	uint8_t	type;
#define OPAL_I2C_RAW_READ	0
#define OPAL_I2C_RAW_WRITE	1
#define OPAL_I2C_SM_READ	2
#define OPAL_I2C_SM_WRITE	3
	uint8_t flags;
#define OPAL_I2C_ADDR_10	0x01	/* Not supported yet */
	uint8_t	subaddr_sz;		/* Max 4 */
	uint8_t reserved;
	__be16 addr;			/* 7 or 10 bit address */
	__be16 reserved2;
	__be32 subaddr;		/* Sub-address if any */
	__be32 size;			/* Data size */
	__be64 buffer_ra;		/* Buffer real address */
};

#endif /* __ASSEMBLY__ */

#endif /* __OPAL_API_H */
