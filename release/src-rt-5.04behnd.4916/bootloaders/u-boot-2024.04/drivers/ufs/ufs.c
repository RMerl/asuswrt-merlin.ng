// SPDX-License-Identifier: GPL-2.0+
/**
 * ufs.c - Universal Flash Storage (UFS) driver
 *
 * Taken from Linux Kernel v5.2 (drivers/scsi/ufs/ufshcd.c) and ported
 * to u-boot.
 *
 * Copyright (C) 2019 Texas Instruments Incorporated - https://www.ti.com
 */

#include <bouncebuf.h>
#include <charset.h>
#include <common.h>
#include <dm.h>
#include <log.h>
#include <dm/device_compat.h>
#include <dm/devres.h>
#include <dm/lists.h>
#include <dm/device-internal.h>
#include <malloc.h>
#include <hexdump.h>
#include <scsi.h>
#include <asm/io.h>
#include <asm/dma-mapping.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/dma-mapping.h>

#include "ufs.h"

#define UFSHCD_ENABLE_INTRS	(UTP_TRANSFER_REQ_COMPL |\
				 UTP_TASK_REQ_COMPL |\
				 UFSHCD_ERROR_MASK)
/* maximum number of link-startup retries */
#define DME_LINKSTARTUP_RETRIES 3

/* maximum number of retries for a general UIC command  */
#define UFS_UIC_COMMAND_RETRIES 3

/* Query request retries */
#define QUERY_REQ_RETRIES 3
/* Query request timeout */
#define QUERY_REQ_TIMEOUT 1500 /* 1.5 seconds */

/* maximum timeout in ms for a general UIC command */
#define UFS_UIC_CMD_TIMEOUT	1000
/* NOP OUT retries waiting for NOP IN response */
#define NOP_OUT_RETRIES    10
/* Timeout after 30 msecs if NOP OUT hangs without response */
#define NOP_OUT_TIMEOUT    30 /* msecs */

/* Only use one Task Tag for all requests */
#define TASK_TAG	0

/* Expose the flag value from utp_upiu_query.value */
#define MASK_QUERY_UPIU_FLAG_LOC 0xFF

#define MAX_PRDT_ENTRY	262144

/* maximum bytes per request */
#define UFS_MAX_BYTES	(128 * 256 * 1024)

static inline bool ufshcd_is_hba_active(struct ufs_hba *hba);
static inline void ufshcd_hba_stop(struct ufs_hba *hba);
static int ufshcd_hba_enable(struct ufs_hba *hba);

/*
 * ufshcd_wait_for_register - wait for register value to change
 */
static int ufshcd_wait_for_register(struct ufs_hba *hba, u32 reg, u32 mask,
				    u32 val, unsigned long timeout_ms)
{
	int err = 0;
	unsigned long start = get_timer(0);

	/* ignore bits that we don't intend to wait on */
	val = val & mask;

	while ((ufshcd_readl(hba, reg) & mask) != val) {
		if (get_timer(start) > timeout_ms) {
			if ((ufshcd_readl(hba, reg) & mask) != val)
				err = -ETIMEDOUT;
			break;
		}
	}

	return err;
}

/**
 * ufshcd_init_pwr_info - setting the POR (power on reset)
 * values in hba power info
 */
static void ufshcd_init_pwr_info(struct ufs_hba *hba)
{
	hba->pwr_info.gear_rx = UFS_PWM_G1;
	hba->pwr_info.gear_tx = UFS_PWM_G1;
	hba->pwr_info.lane_rx = 1;
	hba->pwr_info.lane_tx = 1;
	hba->pwr_info.pwr_rx = SLOWAUTO_MODE;
	hba->pwr_info.pwr_tx = SLOWAUTO_MODE;
	hba->pwr_info.hs_rate = 0;
}

/**
 * ufshcd_print_pwr_info - print power params as saved in hba
 * power info
 */
static void ufshcd_print_pwr_info(struct ufs_hba *hba)
{
	static const char * const names[] = {
		"INVALID MODE",
		"FAST MODE",
		"SLOW_MODE",
		"INVALID MODE",
		"FASTAUTO_MODE",
		"SLOWAUTO_MODE",
		"INVALID MODE",
	};

	dev_err(hba->dev, "[RX, TX]: gear=[%d, %d], lane[%d, %d], pwr[%s, %s], rate = %d\n",
		hba->pwr_info.gear_rx, hba->pwr_info.gear_tx,
		hba->pwr_info.lane_rx, hba->pwr_info.lane_tx,
		names[hba->pwr_info.pwr_rx],
		names[hba->pwr_info.pwr_tx],
		hba->pwr_info.hs_rate);
}

/**
 * ufshcd_ready_for_uic_cmd - Check if controller is ready
 *                            to accept UIC commands
 */
static inline bool ufshcd_ready_for_uic_cmd(struct ufs_hba *hba)
{
	if (ufshcd_readl(hba, REG_CONTROLLER_STATUS) & UIC_COMMAND_READY)
		return true;
	else
		return false;
}

/**
 * ufshcd_get_uic_cmd_result - Get the UIC command result
 */
static inline int ufshcd_get_uic_cmd_result(struct ufs_hba *hba)
{
	return ufshcd_readl(hba, REG_UIC_COMMAND_ARG_2) &
	       MASK_UIC_COMMAND_RESULT;
}

/**
 * ufshcd_get_dme_attr_val - Get the value of attribute returned by UIC command
 */
static inline u32 ufshcd_get_dme_attr_val(struct ufs_hba *hba)
{
	return ufshcd_readl(hba, REG_UIC_COMMAND_ARG_3);
}

/**
 * ufshcd_is_device_present - Check if any device connected to
 *			      the host controller
 */
static inline bool ufshcd_is_device_present(struct ufs_hba *hba)
{
	return (ufshcd_readl(hba, REG_CONTROLLER_STATUS) &
						DEVICE_PRESENT) ? true : false;
}

/**
 * ufshcd_send_uic_cmd - UFS Interconnect layer command API
 *
 */
static int ufshcd_send_uic_cmd(struct ufs_hba *hba, struct uic_command *uic_cmd)
{
	unsigned long start = 0;
	u32 intr_status;
	u32 enabled_intr_status;

	if (!ufshcd_ready_for_uic_cmd(hba)) {
		dev_err(hba->dev,
			"Controller not ready to accept UIC commands\n");
		return -EIO;
	}

	debug("sending uic command:%d\n", uic_cmd->command);

	/* Write Args */
	ufshcd_writel(hba, uic_cmd->argument1, REG_UIC_COMMAND_ARG_1);
	ufshcd_writel(hba, uic_cmd->argument2, REG_UIC_COMMAND_ARG_2);
	ufshcd_writel(hba, uic_cmd->argument3, REG_UIC_COMMAND_ARG_3);

	/* Write UIC Cmd */
	ufshcd_writel(hba, uic_cmd->command & COMMAND_OPCODE_MASK,
		      REG_UIC_COMMAND);

	start = get_timer(0);
	do {
		intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
		enabled_intr_status = intr_status & hba->intr_mask;
		ufshcd_writel(hba, intr_status, REG_INTERRUPT_STATUS);

		if (get_timer(start) > UFS_UIC_CMD_TIMEOUT) {
			dev_err(hba->dev,
				"Timedout waiting for UIC response\n");

			return -ETIMEDOUT;
		}

		if (enabled_intr_status & UFSHCD_ERROR_MASK) {
			dev_err(hba->dev, "Error in status:%08x\n",
				enabled_intr_status);

			return -1;
		}
	} while (!(enabled_intr_status & UFSHCD_UIC_MASK));

	uic_cmd->argument2 = ufshcd_get_uic_cmd_result(hba);
	uic_cmd->argument3 = ufshcd_get_dme_attr_val(hba);

	debug("Sent successfully\n");

	return 0;
}

/**
 * ufshcd_dme_set_attr - UIC command for DME_SET, DME_PEER_SET
 *
 */
int ufshcd_dme_set_attr(struct ufs_hba *hba, u32 attr_sel, u8 attr_set,
			u32 mib_val, u8 peer)
{
	struct uic_command uic_cmd = {0};
	static const char *const action[] = {
		"dme-set",
		"dme-peer-set"
	};
	const char *set = action[!!peer];
	int ret;
	int retries = UFS_UIC_COMMAND_RETRIES;

	uic_cmd.command = peer ?
		UIC_CMD_DME_PEER_SET : UIC_CMD_DME_SET;
	uic_cmd.argument1 = attr_sel;
	uic_cmd.argument2 = UIC_ARG_ATTR_TYPE(attr_set);
	uic_cmd.argument3 = mib_val;

	do {
		/* for peer attributes we retry upon failure */
		ret = ufshcd_send_uic_cmd(hba, &uic_cmd);
		if (ret)
			dev_dbg(hba->dev, "%s: attr-id 0x%x val 0x%x error code %d\n",
				set, UIC_GET_ATTR_ID(attr_sel), mib_val, ret);
	} while (ret && peer && --retries);

	if (ret)
		dev_err(hba->dev, "%s: attr-id 0x%x val 0x%x failed %d retries\n",
			set, UIC_GET_ATTR_ID(attr_sel), mib_val,
			UFS_UIC_COMMAND_RETRIES - retries);

	return ret;
}

/**
 * ufshcd_dme_get_attr - UIC command for DME_GET, DME_PEER_GET
 *
 */
int ufshcd_dme_get_attr(struct ufs_hba *hba, u32 attr_sel,
			u32 *mib_val, u8 peer)
{
	struct uic_command uic_cmd = {0};
	static const char *const action[] = {
		"dme-get",
		"dme-peer-get"
	};
	const char *get = action[!!peer];
	int ret;
	int retries = UFS_UIC_COMMAND_RETRIES;

	uic_cmd.command = peer ?
		UIC_CMD_DME_PEER_GET : UIC_CMD_DME_GET;
	uic_cmd.argument1 = attr_sel;

	do {
		/* for peer attributes we retry upon failure */
		ret = ufshcd_send_uic_cmd(hba, &uic_cmd);
		if (ret)
			dev_dbg(hba->dev, "%s: attr-id 0x%x error code %d\n",
				get, UIC_GET_ATTR_ID(attr_sel), ret);
	} while (ret && peer && --retries);

	if (ret)
		dev_err(hba->dev, "%s: attr-id 0x%x failed %d retries\n",
			get, UIC_GET_ATTR_ID(attr_sel),
			UFS_UIC_COMMAND_RETRIES - retries);

	if (mib_val && !ret)
		*mib_val = uic_cmd.argument3;

	return ret;
}

static int ufshcd_disable_tx_lcc(struct ufs_hba *hba, bool peer)
{
	u32 tx_lanes, i, err = 0;

	if (!peer)
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDTXDATALANES),
			       &tx_lanes);
	else
		ufshcd_dme_peer_get(hba, UIC_ARG_MIB(PA_CONNECTEDTXDATALANES),
				    &tx_lanes);
	for (i = 0; i < tx_lanes; i++) {
		if (!peer)
			err = ufshcd_dme_set(hba,
					     UIC_ARG_MIB_SEL(TX_LCC_ENABLE,
					     UIC_ARG_MPHY_TX_GEN_SEL_INDEX(i)),
					     0);
		else
			err = ufshcd_dme_peer_set(hba,
					UIC_ARG_MIB_SEL(TX_LCC_ENABLE,
					UIC_ARG_MPHY_TX_GEN_SEL_INDEX(i)),
					0);
		if (err) {
			dev_err(hba->dev, "%s: TX LCC Disable failed, peer = %d, lane = %d, err = %d\n",
				__func__, peer, i, err);
			break;
		}
	}

	return err;
}

static inline int ufshcd_disable_device_tx_lcc(struct ufs_hba *hba)
{
	return ufshcd_disable_tx_lcc(hba, true);
}

/**
 * ufshcd_dme_link_startup - Notify Unipro to perform link startup
 *
 */
static int ufshcd_dme_link_startup(struct ufs_hba *hba)
{
	struct uic_command uic_cmd = {0};
	int ret;

	uic_cmd.command = UIC_CMD_DME_LINK_STARTUP;

	ret = ufshcd_send_uic_cmd(hba, &uic_cmd);
	if (ret)
		dev_dbg(hba->dev,
			"dme-link-startup: error code %d\n", ret);
	return ret;
}

/**
 * ufshcd_disable_intr_aggr - Disables interrupt aggregation.
 *
 */
static inline void ufshcd_disable_intr_aggr(struct ufs_hba *hba)
{
	ufshcd_writel(hba, 0, REG_UTP_TRANSFER_REQ_INT_AGG_CONTROL);
}

/**
 * ufshcd_get_lists_status - Check UCRDY, UTRLRDY and UTMRLRDY
 */
static inline int ufshcd_get_lists_status(u32 reg)
{
	return !((reg & UFSHCD_STATUS_READY) == UFSHCD_STATUS_READY);
}

/**
 * ufshcd_enable_run_stop_reg - Enable run-stop registers,
 *			When run-stop registers are set to 1, it indicates the
 *			host controller that it can process the requests
 */
static void ufshcd_enable_run_stop_reg(struct ufs_hba *hba)
{
	ufshcd_writel(hba, UTP_TASK_REQ_LIST_RUN_STOP_BIT,
		      REG_UTP_TASK_REQ_LIST_RUN_STOP);
	ufshcd_writel(hba, UTP_TRANSFER_REQ_LIST_RUN_STOP_BIT,
		      REG_UTP_TRANSFER_REQ_LIST_RUN_STOP);
}

/**
 * ufshcd_enable_intr - enable interrupts
 */
static void ufshcd_enable_intr(struct ufs_hba *hba, u32 intrs)
{
	u32 set = ufshcd_readl(hba, REG_INTERRUPT_ENABLE);
	u32 rw;

	if (hba->version == UFSHCI_VERSION_10) {
		rw = set & INTERRUPT_MASK_RW_VER_10;
		set = rw | ((set ^ intrs) & intrs);
	} else {
		set |= intrs;
	}

	ufshcd_writel(hba, set, REG_INTERRUPT_ENABLE);

	hba->intr_mask = set;
}

/**
 * ufshcd_make_hba_operational - Make UFS controller operational
 *
 * To bring UFS host controller to operational state,
 * 1. Enable required interrupts
 * 2. Configure interrupt aggregation
 * 3. Program UTRL and UTMRL base address
 * 4. Configure run-stop-registers
 *
 */
static int ufshcd_make_hba_operational(struct ufs_hba *hba)
{
	int err = 0;
	u32 reg;

	/* Enable required interrupts */
	ufshcd_enable_intr(hba, UFSHCD_ENABLE_INTRS);

	/* Disable interrupt aggregation */
	ufshcd_disable_intr_aggr(hba);

	/* Configure UTRL and UTMRL base address registers */
	ufshcd_writel(hba, lower_32_bits((dma_addr_t)hba->utrdl),
		      REG_UTP_TRANSFER_REQ_LIST_BASE_L);
	ufshcd_writel(hba, upper_32_bits((dma_addr_t)hba->utrdl),
		      REG_UTP_TRANSFER_REQ_LIST_BASE_H);
	ufshcd_writel(hba, lower_32_bits((dma_addr_t)hba->utmrdl),
		      REG_UTP_TASK_REQ_LIST_BASE_L);
	ufshcd_writel(hba, upper_32_bits((dma_addr_t)hba->utmrdl),
		      REG_UTP_TASK_REQ_LIST_BASE_H);

	/*
	 * UCRDY, UTMRLDY and UTRLRDY bits must be 1
	 */
	reg = ufshcd_readl(hba, REG_CONTROLLER_STATUS);
	if (!(ufshcd_get_lists_status(reg))) {
		ufshcd_enable_run_stop_reg(hba);
	} else {
		dev_err(hba->dev,
			"Host controller not ready to process requests\n");
		err = -EIO;
		goto out;
	}

out:
	return err;
}

/**
 * ufshcd_link_startup - Initialize unipro link startup
 */
static int ufshcd_link_startup(struct ufs_hba *hba)
{
	int ret;
	int retries = DME_LINKSTARTUP_RETRIES;
	bool link_startup_again = true;

link_startup:
	do {
		ufshcd_ops_link_startup_notify(hba, PRE_CHANGE);

		ret = ufshcd_dme_link_startup(hba);

		/* check if device is detected by inter-connect layer */
		if (!ret && !ufshcd_is_device_present(hba)) {
			dev_err(hba->dev, "%s: Device not present\n", __func__);
			ret = -ENXIO;
			goto out;
		}

		/*
		 * DME link lost indication is only received when link is up,
		 * but we can't be sure if the link is up until link startup
		 * succeeds. So reset the local Uni-Pro and try again.
		 */
		if (ret && ufshcd_hba_enable(hba))
			goto out;
	} while (ret && retries--);

	if (ret)
		/* failed to get the link up... retire */
		goto out;

	if (link_startup_again) {
		link_startup_again = false;
		retries = DME_LINKSTARTUP_RETRIES;
		goto link_startup;
	}

	/* Mark that link is up in PWM-G1, 1-lane, SLOW-AUTO mode */
	ufshcd_init_pwr_info(hba);

	if (hba->quirks & UFSHCD_QUIRK_BROKEN_LCC) {
		ret = ufshcd_disable_device_tx_lcc(hba);
		if (ret)
			goto out;
	}

	/* Include any host controller configuration via UIC commands */
	ret = ufshcd_ops_link_startup_notify(hba, POST_CHANGE);
	if (ret)
		goto out;

	ret = ufshcd_make_hba_operational(hba);
out:
	if (ret)
		dev_err(hba->dev, "link startup failed %d\n", ret);

	return ret;
}

/**
 * ufshcd_hba_stop - Send controller to reset state
 */
static inline void ufshcd_hba_stop(struct ufs_hba *hba)
{
	int err;

	ufshcd_writel(hba, CONTROLLER_DISABLE,  REG_CONTROLLER_ENABLE);
	err = ufshcd_wait_for_register(hba, REG_CONTROLLER_ENABLE,
				       CONTROLLER_ENABLE, CONTROLLER_DISABLE,
				       10);
	if (err)
		dev_err(hba->dev, "%s: Controller disable failed\n", __func__);
}

/**
 * ufshcd_is_hba_active - Get controller state
 */
static inline bool ufshcd_is_hba_active(struct ufs_hba *hba)
{
	return (ufshcd_readl(hba, REG_CONTROLLER_ENABLE) & CONTROLLER_ENABLE)
		? false : true;
}

/**
 * ufshcd_hba_start - Start controller initialization sequence
 */
static inline void ufshcd_hba_start(struct ufs_hba *hba)
{
	ufshcd_writel(hba, CONTROLLER_ENABLE, REG_CONTROLLER_ENABLE);
}

/**
 * ufshcd_hba_enable - initialize the controller
 */
static int ufshcd_hba_enable(struct ufs_hba *hba)
{
	int retry;

	if (!ufshcd_is_hba_active(hba))
		/* change controller state to "reset state" */
		ufshcd_hba_stop(hba);

	ufshcd_ops_hce_enable_notify(hba, PRE_CHANGE);

	/* start controller initialization sequence */
	ufshcd_hba_start(hba);

	/*
	 * To initialize a UFS host controller HCE bit must be set to 1.
	 * During initialization the HCE bit value changes from 1->0->1.
	 * When the host controller completes initialization sequence
	 * it sets the value of HCE bit to 1. The same HCE bit is read back
	 * to check if the controller has completed initialization sequence.
	 * So without this delay the value HCE = 1, set in the previous
	 * instruction might be read back.
	 * This delay can be changed based on the controller.
	 */
	mdelay(1);

	/* wait for the host controller to complete initialization */
	retry = 10;
	while (ufshcd_is_hba_active(hba)) {
		if (retry) {
			retry--;
		} else {
			dev_err(hba->dev, "Controller enable failed\n");
			return -EIO;
		}
		mdelay(5);
	}

	/* enable UIC related interrupts */
	ufshcd_enable_intr(hba, UFSHCD_UIC_MASK);

	ufshcd_ops_hce_enable_notify(hba, POST_CHANGE);

	return 0;
}

/**
 * ufshcd_host_memory_configure - configure local reference block with
 *				memory offsets
 */
static void ufshcd_host_memory_configure(struct ufs_hba *hba)
{
	struct utp_transfer_req_desc *utrdlp;
	dma_addr_t cmd_desc_dma_addr;
	u16 response_offset;
	u16 prdt_offset;

	utrdlp = hba->utrdl;
	cmd_desc_dma_addr = (dma_addr_t)hba->ucdl;

	utrdlp->command_desc_base_addr_lo =
				cpu_to_le32(lower_32_bits(cmd_desc_dma_addr));
	utrdlp->command_desc_base_addr_hi =
				cpu_to_le32(upper_32_bits(cmd_desc_dma_addr));

	response_offset = offsetof(struct utp_transfer_cmd_desc, response_upiu);
	prdt_offset = offsetof(struct utp_transfer_cmd_desc, prd_table);

	utrdlp->response_upiu_offset = cpu_to_le16(response_offset >> 2);
	utrdlp->prd_table_offset = cpu_to_le16(prdt_offset >> 2);
	utrdlp->response_upiu_length = cpu_to_le16(ALIGNED_UPIU_SIZE >> 2);

	hba->ucd_req_ptr = (struct utp_upiu_req *)hba->ucdl;
	hba->ucd_rsp_ptr =
		(struct utp_upiu_rsp *)&hba->ucdl->response_upiu;
	hba->ucd_prdt_ptr =
		(struct ufshcd_sg_entry *)&hba->ucdl->prd_table;
}

/**
 * ufshcd_memory_alloc - allocate memory for host memory space data structures
 */
static int ufshcd_memory_alloc(struct ufs_hba *hba)
{
	/* Allocate one Transfer Request Descriptor
	 * Should be aligned to 1k boundary.
	 */
	hba->utrdl = memalign(1024, sizeof(struct utp_transfer_req_desc));
	if (!hba->utrdl) {
		dev_err(hba->dev, "Transfer Descriptor memory allocation failed\n");
		return -ENOMEM;
	}

	/* Allocate one Command Descriptor
	 * Should be aligned to 1k boundary.
	 */
	hba->ucdl = memalign(1024, sizeof(struct utp_transfer_cmd_desc));
	if (!hba->ucdl) {
		dev_err(hba->dev, "Command descriptor memory allocation failed\n");
		return -ENOMEM;
	}

	return 0;
}

/**
 * ufshcd_get_intr_mask - Get the interrupt bit mask
 */
static inline u32 ufshcd_get_intr_mask(struct ufs_hba *hba)
{
	u32 intr_mask = 0;

	switch (hba->version) {
	case UFSHCI_VERSION_10:
		intr_mask = INTERRUPT_MASK_ALL_VER_10;
		break;
	case UFSHCI_VERSION_11:
	case UFSHCI_VERSION_20:
		intr_mask = INTERRUPT_MASK_ALL_VER_11;
		break;
	case UFSHCI_VERSION_21:
	default:
		intr_mask = INTERRUPT_MASK_ALL_VER_21;
		break;
	}

	return intr_mask;
}

/**
 * ufshcd_get_ufs_version - Get the UFS version supported by the HBA
 */
static inline u32 ufshcd_get_ufs_version(struct ufs_hba *hba)
{
	return ufshcd_readl(hba, REG_UFS_VERSION);
}

/**
 * ufshcd_get_upmcrs - Get the power mode change request status
 */
static inline u8 ufshcd_get_upmcrs(struct ufs_hba *hba)
{
	return (ufshcd_readl(hba, REG_CONTROLLER_STATUS) >> 8) & 0x7;
}

/**
 * ufshcd_cache_flush_and_invalidate - Flush and invalidate cache
 *
 * Flush and invalidate cache in aligned address..address+size range.
 * The invalidation is in place to avoid stale data in cache.
 */
static void ufshcd_cache_flush_and_invalidate(void *addr, unsigned long size)
{
	uintptr_t aaddr = (uintptr_t)addr & ~(ARCH_DMA_MINALIGN - 1);
	unsigned long asize = ALIGN(size, ARCH_DMA_MINALIGN);

	flush_dcache_range(aaddr, aaddr + asize);
	invalidate_dcache_range(aaddr, aaddr + asize);
}

/**
 * ufshcd_prepare_req_desc_hdr() - Fills the requests header
 * descriptor according to request
 */
static void ufshcd_prepare_req_desc_hdr(struct ufs_hba *hba,
					u32 *upiu_flags,
					enum dma_data_direction cmd_dir)
{
	struct utp_transfer_req_desc *req_desc = hba->utrdl;
	u32 data_direction;
	u32 dword_0;

	if (cmd_dir == DMA_FROM_DEVICE) {
		data_direction = UTP_DEVICE_TO_HOST;
		*upiu_flags = UPIU_CMD_FLAGS_READ;
	} else if (cmd_dir == DMA_TO_DEVICE) {
		data_direction = UTP_HOST_TO_DEVICE;
		*upiu_flags = UPIU_CMD_FLAGS_WRITE;
	} else {
		data_direction = UTP_NO_DATA_TRANSFER;
		*upiu_flags = UPIU_CMD_FLAGS_NONE;
	}

	dword_0 = data_direction | (0x1 << UPIU_COMMAND_TYPE_OFFSET);

	/* Enable Interrupt for command */
	dword_0 |= UTP_REQ_DESC_INT_CMD;

	/* Transfer request descriptor header fields */
	req_desc->header.dword_0 = cpu_to_le32(dword_0);
	/* dword_1 is reserved, hence it is set to 0 */
	req_desc->header.dword_1 = 0;
	/*
	 * assigning invalid value for command status. Controller
	 * updates OCS on command completion, with the command
	 * status
	 */
	req_desc->header.dword_2 =
		cpu_to_le32(OCS_INVALID_COMMAND_STATUS);
	/* dword_3 is reserved, hence it is set to 0 */
	req_desc->header.dword_3 = 0;

	req_desc->prd_table_length = 0;

	ufshcd_cache_flush_and_invalidate(req_desc, sizeof(*req_desc));
}

static void ufshcd_prepare_utp_query_req_upiu(struct ufs_hba *hba,
					      u32 upiu_flags)
{
	struct utp_upiu_req *ucd_req_ptr = hba->ucd_req_ptr;
	struct ufs_query *query = &hba->dev_cmd.query;
	u16 len = be16_to_cpu(query->request.upiu_req.length);

	/* Query request header */
	ucd_req_ptr->header.dword_0 =
				UPIU_HEADER_DWORD(UPIU_TRANSACTION_QUERY_REQ,
						  upiu_flags, 0, TASK_TAG);
	ucd_req_ptr->header.dword_1 =
				UPIU_HEADER_DWORD(0, query->request.query_func,
						  0, 0);

	/* Data segment length only need for WRITE_DESC */
	if (query->request.upiu_req.opcode == UPIU_QUERY_OPCODE_WRITE_DESC)
		ucd_req_ptr->header.dword_2 =
				UPIU_HEADER_DWORD(0, 0, (len >> 8), (u8)len);
	else
		ucd_req_ptr->header.dword_2 = 0;

	/* Copy the Query Request buffer as is */
	memcpy(&ucd_req_ptr->qr, &query->request.upiu_req, QUERY_OSF_SIZE);

	/* Copy the Descriptor */
	if (query->request.upiu_req.opcode == UPIU_QUERY_OPCODE_WRITE_DESC) {
		memcpy(ucd_req_ptr + 1, query->descriptor, len);
		ufshcd_cache_flush_and_invalidate(ucd_req_ptr, 2 * sizeof(*ucd_req_ptr));
	} else {
		ufshcd_cache_flush_and_invalidate(ucd_req_ptr, sizeof(*ucd_req_ptr));
	}

	memset(hba->ucd_rsp_ptr, 0, sizeof(struct utp_upiu_rsp));
	ufshcd_cache_flush_and_invalidate(hba->ucd_rsp_ptr, sizeof(*hba->ucd_rsp_ptr));
}

static inline void ufshcd_prepare_utp_nop_upiu(struct ufs_hba *hba)
{
	struct utp_upiu_req *ucd_req_ptr = hba->ucd_req_ptr;

	memset(ucd_req_ptr, 0, sizeof(struct utp_upiu_req));

	/* command descriptor fields */
	ucd_req_ptr->header.dword_0 =
			UPIU_HEADER_DWORD(UPIU_TRANSACTION_NOP_OUT, 0, 0, TASK_TAG);
	/* clear rest of the fields of basic header */
	ucd_req_ptr->header.dword_1 = 0;
	ucd_req_ptr->header.dword_2 = 0;

	memset(hba->ucd_rsp_ptr, 0, sizeof(struct utp_upiu_rsp));

	ufshcd_cache_flush_and_invalidate(ucd_req_ptr, sizeof(*ucd_req_ptr));
	ufshcd_cache_flush_and_invalidate(hba->ucd_rsp_ptr, sizeof(*hba->ucd_rsp_ptr));
}

/**
 * ufshcd_comp_devman_upiu - UFS Protocol Information Unit(UPIU)
 *			     for Device Management Purposes
 */
static int ufshcd_comp_devman_upiu(struct ufs_hba *hba,
				   enum dev_cmd_type cmd_type)
{
	u32 upiu_flags;
	int ret = 0;

	hba->dev_cmd.type = cmd_type;

	ufshcd_prepare_req_desc_hdr(hba, &upiu_flags, DMA_NONE);
	switch (cmd_type) {
	case DEV_CMD_TYPE_QUERY:
		ufshcd_prepare_utp_query_req_upiu(hba, upiu_flags);
		break;
	case DEV_CMD_TYPE_NOP:
		ufshcd_prepare_utp_nop_upiu(hba);
		break;
	default:
		ret = -EINVAL;
	}

	return ret;
}

static int ufshcd_send_command(struct ufs_hba *hba, unsigned int task_tag)
{
	unsigned long start;
	u32 intr_status;
	u32 enabled_intr_status;

	ufshcd_writel(hba, 1 << task_tag, REG_UTP_TRANSFER_REQ_DOOR_BELL);

	start = get_timer(0);
	do {
		intr_status = ufshcd_readl(hba, REG_INTERRUPT_STATUS);
		enabled_intr_status = intr_status & hba->intr_mask;
		ufshcd_writel(hba, intr_status, REG_INTERRUPT_STATUS);

		if (get_timer(start) > QUERY_REQ_TIMEOUT) {
			dev_err(hba->dev,
				"Timedout waiting for UTP response\n");

			return -ETIMEDOUT;
		}

		if (enabled_intr_status & UFSHCD_ERROR_MASK) {
			dev_err(hba->dev, "Error in status:%08x\n",
				enabled_intr_status);

			return -1;
		}
	} while (!(enabled_intr_status & UTP_TRANSFER_REQ_COMPL));

	return 0;
}

/**
 * ufshcd_get_req_rsp - returns the TR response transaction type
 */
static inline int ufshcd_get_req_rsp(struct utp_upiu_rsp *ucd_rsp_ptr)
{
	return be32_to_cpu(ucd_rsp_ptr->header.dword_0) >> 24;
}

/**
 * ufshcd_get_tr_ocs - Get the UTRD Overall Command Status
 *
 */
static inline int ufshcd_get_tr_ocs(struct ufs_hba *hba)
{
	struct utp_transfer_req_desc *req_desc = hba->utrdl;

	return le32_to_cpu(req_desc->header.dword_2) & MASK_OCS;
}

static inline int ufshcd_get_rsp_upiu_result(struct utp_upiu_rsp *ucd_rsp_ptr)
{
	return be32_to_cpu(ucd_rsp_ptr->header.dword_1) & MASK_RSP_UPIU_RESULT;
}

static int ufshcd_check_query_response(struct ufs_hba *hba)
{
	struct ufs_query_res *query_res = &hba->dev_cmd.query.response;

	/* Get the UPIU response */
	query_res->response = ufshcd_get_rsp_upiu_result(hba->ucd_rsp_ptr) >>
				UPIU_RSP_CODE_OFFSET;
	return query_res->response;
}

/**
 * ufshcd_copy_query_response() - Copy the Query Response and the data
 * descriptor
 */
static int ufshcd_copy_query_response(struct ufs_hba *hba)
{
	struct ufs_query_res *query_res = &hba->dev_cmd.query.response;

	memcpy(&query_res->upiu_res, &hba->ucd_rsp_ptr->qr, QUERY_OSF_SIZE);

	/* Get the descriptor */
	if (hba->dev_cmd.query.descriptor &&
	    hba->ucd_rsp_ptr->qr.opcode == UPIU_QUERY_OPCODE_READ_DESC) {
		u8 *descp = (u8 *)hba->ucd_rsp_ptr +
				GENERAL_UPIU_REQUEST_SIZE;
		u16 resp_len;
		u16 buf_len;

		/* data segment length */
		resp_len = be32_to_cpu(hba->ucd_rsp_ptr->header.dword_2) &
						MASK_QUERY_DATA_SEG_LEN;
		buf_len =
			be16_to_cpu(hba->dev_cmd.query.request.upiu_req.length);
		if (likely(buf_len >= resp_len)) {
			memcpy(hba->dev_cmd.query.descriptor, descp, resp_len);
		} else {
			dev_warn(hba->dev,
				 "%s: Response size is bigger than buffer\n",
				 __func__);
			return -EINVAL;
		}
	}

	return 0;
}

/**
 * ufshcd_exec_dev_cmd - API for sending device management requests
 */
static int ufshcd_exec_dev_cmd(struct ufs_hba *hba, enum dev_cmd_type cmd_type,
			       int timeout)
{
	int err;
	int resp;

	err = ufshcd_comp_devman_upiu(hba, cmd_type);
	if (err)
		return err;

	err = ufshcd_send_command(hba, TASK_TAG);
	if (err)
		return err;

	err = ufshcd_get_tr_ocs(hba);
	if (err) {
		dev_err(hba->dev, "Error in OCS:%d\n", err);
		return -EINVAL;
	}

	resp = ufshcd_get_req_rsp(hba->ucd_rsp_ptr);
	switch (resp) {
	case UPIU_TRANSACTION_NOP_IN:
		break;
	case UPIU_TRANSACTION_QUERY_RSP:
		err = ufshcd_check_query_response(hba);
		if (!err)
			err = ufshcd_copy_query_response(hba);
		break;
	case UPIU_TRANSACTION_REJECT_UPIU:
		/* TODO: handle Reject UPIU Response */
		err = -EPERM;
		dev_err(hba->dev, "%s: Reject UPIU not fully implemented\n",
			__func__);
		break;
	default:
		err = -EINVAL;
		dev_err(hba->dev, "%s: Invalid device management cmd response: %x\n",
			__func__, resp);
	}

	return err;
}

/**
 * ufshcd_init_query() - init the query response and request parameters
 */
static inline void ufshcd_init_query(struct ufs_hba *hba,
				     struct ufs_query_req **request,
				     struct ufs_query_res **response,
				     enum query_opcode opcode,
				     u8 idn, u8 index, u8 selector)
{
	*request = &hba->dev_cmd.query.request;
	*response = &hba->dev_cmd.query.response;
	memset(*request, 0, sizeof(struct ufs_query_req));
	memset(*response, 0, sizeof(struct ufs_query_res));
	(*request)->upiu_req.opcode = opcode;
	(*request)->upiu_req.idn = idn;
	(*request)->upiu_req.index = index;
	(*request)->upiu_req.selector = selector;
}

/**
 * ufshcd_query_flag() - API function for sending flag query requests
 */
int ufshcd_query_flag(struct ufs_hba *hba, enum query_opcode opcode,
		      enum flag_idn idn, bool *flag_res)
{
	struct ufs_query_req *request = NULL;
	struct ufs_query_res *response = NULL;
	int err, index = 0, selector = 0;
	int timeout = QUERY_REQ_TIMEOUT;

	ufshcd_init_query(hba, &request, &response, opcode, idn, index,
			  selector);

	switch (opcode) {
	case UPIU_QUERY_OPCODE_SET_FLAG:
	case UPIU_QUERY_OPCODE_CLEAR_FLAG:
	case UPIU_QUERY_OPCODE_TOGGLE_FLAG:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
		break;
	case UPIU_QUERY_OPCODE_READ_FLAG:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
		if (!flag_res) {
			/* No dummy reads */
			dev_err(hba->dev, "%s: Invalid argument for read request\n",
				__func__);
			err = -EINVAL;
			goto out;
		}
		break;
	default:
		dev_err(hba->dev,
			"%s: Expected query flag opcode but got = %d\n",
			__func__, opcode);
		err = -EINVAL;
		goto out;
	}

	err = ufshcd_exec_dev_cmd(hba, DEV_CMD_TYPE_QUERY, timeout);

	if (err) {
		dev_err(hba->dev,
			"%s: Sending flag query for idn %d failed, err = %d\n",
			__func__, idn, err);
		goto out;
	}

	if (flag_res)
		*flag_res = (be32_to_cpu(response->upiu_res.value) &
				MASK_QUERY_UPIU_FLAG_LOC) & 0x1;

out:
	return err;
}

static int ufshcd_query_flag_retry(struct ufs_hba *hba,
				   enum query_opcode opcode,
				   enum flag_idn idn, bool *flag_res)
{
	int ret;
	int retries;

	for (retries = 0; retries < QUERY_REQ_RETRIES; retries++) {
		ret = ufshcd_query_flag(hba, opcode, idn, flag_res);
		if (ret)
			dev_dbg(hba->dev,
				"%s: failed with error %d, retries %d\n",
				__func__, ret, retries);
		else
			break;
	}

	if (ret)
		dev_err(hba->dev,
			"%s: query attribute, opcode %d, idn %d, failed with error %d after %d retires\n",
			__func__, opcode, idn, ret, retries);
	return ret;
}

static int __ufshcd_query_descriptor(struct ufs_hba *hba,
				     enum query_opcode opcode,
				     enum desc_idn idn, u8 index, u8 selector,
				     u8 *desc_buf, int *buf_len)
{
	struct ufs_query_req *request = NULL;
	struct ufs_query_res *response = NULL;
	int err;

	if (!desc_buf) {
		dev_err(hba->dev, "%s: descriptor buffer required for opcode 0x%x\n",
			__func__, opcode);
		err = -EINVAL;
		goto out;
	}

	if (*buf_len < QUERY_DESC_MIN_SIZE || *buf_len > QUERY_DESC_MAX_SIZE) {
		dev_err(hba->dev, "%s: descriptor buffer size (%d) is out of range\n",
			__func__, *buf_len);
		err = -EINVAL;
		goto out;
	}

	ufshcd_init_query(hba, &request, &response, opcode, idn, index,
			  selector);
	hba->dev_cmd.query.descriptor = desc_buf;
	request->upiu_req.length = cpu_to_be16(*buf_len);

	switch (opcode) {
	case UPIU_QUERY_OPCODE_WRITE_DESC:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_WRITE_REQUEST;
		break;
	case UPIU_QUERY_OPCODE_READ_DESC:
		request->query_func = UPIU_QUERY_FUNC_STANDARD_READ_REQUEST;
		break;
	default:
		dev_err(hba->dev, "%s: Expected query descriptor opcode but got = 0x%.2x\n",
			__func__, opcode);
		err = -EINVAL;
		goto out;
	}

	err = ufshcd_exec_dev_cmd(hba, DEV_CMD_TYPE_QUERY, QUERY_REQ_TIMEOUT);

	if (err) {
		dev_err(hba->dev, "%s: opcode 0x%.2x for idn %d failed, index %d, err = %d\n",
			__func__, opcode, idn, index, err);
		goto out;
	}

	hba->dev_cmd.query.descriptor = NULL;
	*buf_len = be16_to_cpu(response->upiu_res.length);

out:
	return err;
}

/**
 * ufshcd_query_descriptor_retry - API function for sending descriptor requests
 */
int ufshcd_query_descriptor_retry(struct ufs_hba *hba, enum query_opcode opcode,
				  enum desc_idn idn, u8 index, u8 selector,
				  u8 *desc_buf, int *buf_len)
{
	int err;
	int retries;

	for (retries = QUERY_REQ_RETRIES; retries > 0; retries--) {
		err = __ufshcd_query_descriptor(hba, opcode, idn, index,
						selector, desc_buf, buf_len);
		if (!err || err == -EINVAL)
			break;
	}

	return err;
}

/**
 * ufshcd_read_desc_length - read the specified descriptor length from header
 */
static int ufshcd_read_desc_length(struct ufs_hba *hba, enum desc_idn desc_id,
				   int desc_index, int *desc_length)
{
	int ret;
	u8 header[QUERY_DESC_HDR_SIZE];
	int header_len = QUERY_DESC_HDR_SIZE;

	if (desc_id >= QUERY_DESC_IDN_MAX)
		return -EINVAL;

	ret = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    desc_id, desc_index, 0, header,
					    &header_len);

	if (ret) {
		dev_err(hba->dev, "%s: Failed to get descriptor header id %d\n",
			__func__, desc_id);
		return ret;
	} else if (desc_id != header[QUERY_DESC_DESC_TYPE_OFFSET]) {
		dev_warn(hba->dev, "%s: descriptor header id %d and desc_id %d mismatch\n",
			 __func__, header[QUERY_DESC_DESC_TYPE_OFFSET],
			 desc_id);
		ret = -EINVAL;
	}

	*desc_length = header[QUERY_DESC_LENGTH_OFFSET];

	return ret;
}

static void ufshcd_init_desc_sizes(struct ufs_hba *hba)
{
	int err;

	err = ufshcd_read_desc_length(hba, QUERY_DESC_IDN_DEVICE, 0,
				      &hba->desc_size.dev_desc);
	if (err)
		hba->desc_size.dev_desc = QUERY_DESC_DEVICE_DEF_SIZE;

	err = ufshcd_read_desc_length(hba, QUERY_DESC_IDN_POWER, 0,
				      &hba->desc_size.pwr_desc);
	if (err)
		hba->desc_size.pwr_desc = QUERY_DESC_POWER_DEF_SIZE;

	err = ufshcd_read_desc_length(hba, QUERY_DESC_IDN_INTERCONNECT, 0,
				      &hba->desc_size.interc_desc);
	if (err)
		hba->desc_size.interc_desc = QUERY_DESC_INTERCONNECT_DEF_SIZE;

	err = ufshcd_read_desc_length(hba, QUERY_DESC_IDN_CONFIGURATION, 0,
				      &hba->desc_size.conf_desc);
	if (err)
		hba->desc_size.conf_desc = QUERY_DESC_CONFIGURATION_DEF_SIZE;

	err = ufshcd_read_desc_length(hba, QUERY_DESC_IDN_UNIT, 0,
				      &hba->desc_size.unit_desc);
	if (err)
		hba->desc_size.unit_desc = QUERY_DESC_UNIT_DEF_SIZE;

	err = ufshcd_read_desc_length(hba, QUERY_DESC_IDN_GEOMETRY, 0,
				      &hba->desc_size.geom_desc);
	if (err)
		hba->desc_size.geom_desc = QUERY_DESC_GEOMETRY_DEF_SIZE;

	err = ufshcd_read_desc_length(hba, QUERY_DESC_IDN_HEALTH, 0,
				      &hba->desc_size.hlth_desc);
	if (err)
		hba->desc_size.hlth_desc = QUERY_DESC_HEALTH_DEF_SIZE;
}

/**
 * ufshcd_map_desc_id_to_length - map descriptor IDN to its length
 *
 */
int ufshcd_map_desc_id_to_length(struct ufs_hba *hba, enum desc_idn desc_id,
				 int *desc_len)
{
	switch (desc_id) {
	case QUERY_DESC_IDN_DEVICE:
		*desc_len = hba->desc_size.dev_desc;
		break;
	case QUERY_DESC_IDN_POWER:
		*desc_len = hba->desc_size.pwr_desc;
		break;
	case QUERY_DESC_IDN_GEOMETRY:
		*desc_len = hba->desc_size.geom_desc;
		break;
	case QUERY_DESC_IDN_CONFIGURATION:
		*desc_len = hba->desc_size.conf_desc;
		break;
	case QUERY_DESC_IDN_UNIT:
		*desc_len = hba->desc_size.unit_desc;
		break;
	case QUERY_DESC_IDN_INTERCONNECT:
		*desc_len = hba->desc_size.interc_desc;
		break;
	case QUERY_DESC_IDN_STRING:
		*desc_len = QUERY_DESC_MAX_SIZE;
		break;
	case QUERY_DESC_IDN_HEALTH:
		*desc_len = hba->desc_size.hlth_desc;
		break;
	case QUERY_DESC_IDN_RFU_0:
	case QUERY_DESC_IDN_RFU_1:
		*desc_len = 0;
		break;
	default:
		*desc_len = 0;
		return -EINVAL;
	}
	return 0;
}
EXPORT_SYMBOL(ufshcd_map_desc_id_to_length);

/**
 * ufshcd_read_desc_param - read the specified descriptor parameter
 *
 */
int ufshcd_read_desc_param(struct ufs_hba *hba, enum desc_idn desc_id,
			   int desc_index, u8 param_offset, u8 *param_read_buf,
			   u8 param_size)
{
	int ret;
	u8 *desc_buf;
	int buff_len;
	bool is_kmalloc = true;

	/* Safety check */
	if (desc_id >= QUERY_DESC_IDN_MAX || !param_size)
		return -EINVAL;

	/* Get the max length of descriptor from structure filled up at probe
	 * time.
	 */
	ret = ufshcd_map_desc_id_to_length(hba, desc_id, &buff_len);

	/* Sanity checks */
	if (ret || !buff_len) {
		dev_err(hba->dev, "%s: Failed to get full descriptor length\n",
			__func__);
		return ret;
	}

	/* Check whether we need temp memory */
	if (param_offset != 0 || param_size < buff_len) {
		desc_buf = kmalloc(buff_len, GFP_KERNEL);
		if (!desc_buf)
			return -ENOMEM;
	} else {
		desc_buf = param_read_buf;
		is_kmalloc = false;
	}

	/* Request for full descriptor */
	ret = ufshcd_query_descriptor_retry(hba, UPIU_QUERY_OPCODE_READ_DESC,
					    desc_id, desc_index, 0, desc_buf,
					    &buff_len);

	if (ret) {
		dev_err(hba->dev, "%s: Failed reading descriptor. desc_id %d, desc_index %d, param_offset %d, ret %d\n",
			__func__, desc_id, desc_index, param_offset, ret);
		goto out;
	}

	/* Sanity check */
	if (desc_buf[QUERY_DESC_DESC_TYPE_OFFSET] != desc_id) {
		dev_err(hba->dev, "%s: invalid desc_id %d in descriptor header\n",
			__func__, desc_buf[QUERY_DESC_DESC_TYPE_OFFSET]);
		ret = -EINVAL;
		goto out;
	}

	/* Check wherher we will not copy more data, than available */
	if (is_kmalloc && param_size > buff_len)
		param_size = buff_len;

	if (is_kmalloc)
		memcpy(param_read_buf, &desc_buf[param_offset], param_size);
out:
	if (is_kmalloc)
		kfree(desc_buf);
	return ret;
}

/* replace non-printable or non-ASCII characters with spaces */
static inline void ufshcd_remove_non_printable(uint8_t *val)
{
	if (!val)
		return;

	if (*val < 0x20 || *val > 0x7e)
		*val = ' ';
}

/**
 * ufshcd_uic_pwr_ctrl - executes UIC commands (which affects the link power
 * state) and waits for it to take effect.
 *
 */
static int ufshcd_uic_pwr_ctrl(struct ufs_hba *hba, struct uic_command *cmd)
{
	unsigned long start = 0;
	u8 status;
	int ret;

	ret = ufshcd_send_uic_cmd(hba, cmd);
	if (ret) {
		dev_err(hba->dev,
			"pwr ctrl cmd 0x%x with mode 0x%x uic error %d\n",
			cmd->command, cmd->argument3, ret);

		return ret;
	}

	start = get_timer(0);
	do {
		status = ufshcd_get_upmcrs(hba);
		if (get_timer(start) > UFS_UIC_CMD_TIMEOUT) {
			dev_err(hba->dev,
				"pwr ctrl cmd 0x%x failed, host upmcrs:0x%x\n",
				cmd->command, status);
			ret = (status != PWR_OK) ? status : -1;
			break;
		}
	} while (status != PWR_LOCAL);

	return ret;
}

/**
 * ufshcd_uic_change_pwr_mode - Perform the UIC power mode change
 *				using DME_SET primitives.
 */
static int ufshcd_uic_change_pwr_mode(struct ufs_hba *hba, u8 mode)
{
	struct uic_command uic_cmd = {0};
	int ret;

	uic_cmd.command = UIC_CMD_DME_SET;
	uic_cmd.argument1 = UIC_ARG_MIB(PA_PWRMODE);
	uic_cmd.argument3 = mode;
	ret = ufshcd_uic_pwr_ctrl(hba, &uic_cmd);

	return ret;
}

static
void ufshcd_prepare_utp_scsi_cmd_upiu(struct ufs_hba *hba,
				      struct scsi_cmd *pccb, u32 upiu_flags)
{
	struct utp_upiu_req *ucd_req_ptr = hba->ucd_req_ptr;
	unsigned int cdb_len;

	/* command descriptor fields */
	ucd_req_ptr->header.dword_0 =
			UPIU_HEADER_DWORD(UPIU_TRANSACTION_COMMAND, upiu_flags,
					  pccb->lun, TASK_TAG);
	ucd_req_ptr->header.dword_1 =
			UPIU_HEADER_DWORD(UPIU_COMMAND_SET_TYPE_SCSI, 0, 0, 0);

	/* Total EHS length and Data segment length will be zero */
	ucd_req_ptr->header.dword_2 = 0;

	ucd_req_ptr->sc.exp_data_transfer_len = cpu_to_be32(pccb->datalen);

	cdb_len = min_t(unsigned short, pccb->cmdlen, UFS_CDB_SIZE);
	memset(ucd_req_ptr->sc.cdb, 0, UFS_CDB_SIZE);
	memcpy(ucd_req_ptr->sc.cdb, pccb->cmd, cdb_len);

	memset(hba->ucd_rsp_ptr, 0, sizeof(struct utp_upiu_rsp));
	ufshcd_cache_flush_and_invalidate(ucd_req_ptr, sizeof(*ucd_req_ptr));
	ufshcd_cache_flush_and_invalidate(hba->ucd_rsp_ptr, sizeof(*hba->ucd_rsp_ptr));
}

static inline void prepare_prdt_desc(struct ufshcd_sg_entry *entry,
				     unsigned char *buf, ulong len)
{
	entry->size = cpu_to_le32(len) | GENMASK(1, 0);
	entry->base_addr = cpu_to_le32(lower_32_bits((unsigned long)buf));
	entry->upper_addr = cpu_to_le32(upper_32_bits((unsigned long)buf));
}

static void prepare_prdt_table(struct ufs_hba *hba, struct scsi_cmd *pccb)
{
	struct utp_transfer_req_desc *req_desc = hba->utrdl;
	struct ufshcd_sg_entry *prd_table = hba->ucd_prdt_ptr;
	uintptr_t aaddr = (uintptr_t)(pccb->pdata) & ~(ARCH_DMA_MINALIGN - 1);
	ulong datalen = pccb->datalen;
	int table_length;
	u8 *buf;
	int i;

	if (!datalen) {
		req_desc->prd_table_length = 0;
		ufshcd_cache_flush_and_invalidate(req_desc, sizeof(*req_desc));
		return;
	}

	if (pccb->dma_dir == DMA_TO_DEVICE) {	/* Write to device */
		flush_dcache_range(aaddr, aaddr +
				   ALIGN(datalen, ARCH_DMA_MINALIGN));
	}

	/* In any case, invalidate cache to avoid stale data in it. */
	invalidate_dcache_range(aaddr, aaddr +
				ALIGN(datalen, ARCH_DMA_MINALIGN));

	table_length = DIV_ROUND_UP(pccb->datalen, MAX_PRDT_ENTRY);
	buf = pccb->pdata;
	i = table_length;
	while (--i) {
		prepare_prdt_desc(&prd_table[table_length - i - 1], buf,
				  MAX_PRDT_ENTRY - 1);
		buf += MAX_PRDT_ENTRY;
		datalen -= MAX_PRDT_ENTRY;
	}

	prepare_prdt_desc(&prd_table[table_length - i - 1], buf, datalen - 1);

	req_desc->prd_table_length = table_length;
	ufshcd_cache_flush_and_invalidate(prd_table, sizeof(*prd_table) * table_length);
	ufshcd_cache_flush_and_invalidate(req_desc, sizeof(*req_desc));
}

static int ufs_scsi_exec(struct udevice *scsi_dev, struct scsi_cmd *pccb)
{
	struct ufs_hba *hba = dev_get_uclass_priv(scsi_dev->parent);
	u32 upiu_flags;
	int ocs, result = 0;
	u8 scsi_status;

	ufshcd_prepare_req_desc_hdr(hba, &upiu_flags, pccb->dma_dir);
	ufshcd_prepare_utp_scsi_cmd_upiu(hba, pccb, upiu_flags);
	prepare_prdt_table(hba, pccb);

	ufshcd_send_command(hba, TASK_TAG);

	ocs = ufshcd_get_tr_ocs(hba);
	switch (ocs) {
	case OCS_SUCCESS:
		result = ufshcd_get_req_rsp(hba->ucd_rsp_ptr);
		switch (result) {
		case UPIU_TRANSACTION_RESPONSE:
			result = ufshcd_get_rsp_upiu_result(hba->ucd_rsp_ptr);

			scsi_status = result & MASK_SCSI_STATUS;
			if (scsi_status)
				return -EINVAL;

			break;
		case UPIU_TRANSACTION_REJECT_UPIU:
			/* TODO: handle Reject UPIU Response */
			dev_err(hba->dev,
				"Reject UPIU not fully implemented\n");
			return -EINVAL;
		default:
			dev_err(hba->dev,
				"Unexpected request response code = %x\n",
				result);
			return -EINVAL;
		}
		break;
	default:
		dev_err(hba->dev, "OCS error from controller = %x\n", ocs);
		return -EINVAL;
	}

	return 0;
}

static inline int ufshcd_read_desc(struct ufs_hba *hba, enum desc_idn desc_id,
				   int desc_index, u8 *buf, u32 size)
{
	return ufshcd_read_desc_param(hba, desc_id, desc_index, 0, buf, size);
}

static int ufshcd_read_device_desc(struct ufs_hba *hba, u8 *buf, u32 size)
{
	return ufshcd_read_desc(hba, QUERY_DESC_IDN_DEVICE, 0, buf, size);
}

/**
 * ufshcd_read_string_desc - read string descriptor
 *
 */
int ufshcd_read_string_desc(struct ufs_hba *hba, int desc_index,
			    u8 *buf, u32 size, bool ascii)
{
	int err = 0;

	err = ufshcd_read_desc(hba, QUERY_DESC_IDN_STRING, desc_index, buf,
			       size);

	if (err) {
		dev_err(hba->dev, "%s: reading String Desc failed after %d retries. err = %d\n",
			__func__, QUERY_REQ_RETRIES, err);
		goto out;
	}

	if (ascii) {
		int desc_len;
		int ascii_len;
		int i;
		u8 *buff_ascii;

		desc_len = buf[0];
		/* remove header and divide by 2 to move from UTF16 to UTF8 */
		ascii_len = (desc_len - QUERY_DESC_HDR_SIZE) / 2 + 1;
		if (size < ascii_len + QUERY_DESC_HDR_SIZE) {
			dev_err(hba->dev, "%s: buffer allocated size is too small\n",
				__func__);
			err = -ENOMEM;
			goto out;
		}

		buff_ascii = kmalloc(ascii_len, GFP_KERNEL);
		if (!buff_ascii) {
			err = -ENOMEM;
			goto out;
		}

		/*
		 * the descriptor contains string in UTF16 format
		 * we need to convert to utf-8 so it can be displayed
		 */
		utf16_to_utf8(buff_ascii,
			      (uint16_t *)&buf[QUERY_DESC_HDR_SIZE], ascii_len);

		/* replace non-printable or non-ASCII characters with spaces */
		for (i = 0; i < ascii_len; i++)
			ufshcd_remove_non_printable(&buff_ascii[i]);

		memset(buf + QUERY_DESC_HDR_SIZE, 0,
		       size - QUERY_DESC_HDR_SIZE);
		memcpy(buf + QUERY_DESC_HDR_SIZE, buff_ascii, ascii_len);
		buf[QUERY_DESC_LENGTH_OFFSET] = ascii_len + QUERY_DESC_HDR_SIZE;
		kfree(buff_ascii);
	}
out:
	return err;
}

static int ufs_get_device_desc(struct ufs_hba *hba,
			       struct ufs_dev_desc *dev_desc)
{
	int err;
	size_t buff_len;
	u8 model_index;
	u8 *desc_buf;

	buff_len = max_t(size_t, hba->desc_size.dev_desc,
			 QUERY_DESC_MAX_SIZE + 1);
	desc_buf = kmalloc(buff_len, GFP_KERNEL);
	if (!desc_buf) {
		err = -ENOMEM;
		goto out;
	}

	err = ufshcd_read_device_desc(hba, desc_buf, hba->desc_size.dev_desc);
	if (err) {
		dev_err(hba->dev, "%s: Failed reading Device Desc. err = %d\n",
			__func__, err);
		goto out;
	}

	/*
	 * getting vendor (manufacturerID) and Bank Index in big endian
	 * format
	 */
	dev_desc->wmanufacturerid = desc_buf[DEVICE_DESC_PARAM_MANF_ID] << 8 |
				     desc_buf[DEVICE_DESC_PARAM_MANF_ID + 1];

	model_index = desc_buf[DEVICE_DESC_PARAM_PRDCT_NAME];

	/* Zero-pad entire buffer for string termination. */
	memset(desc_buf, 0, buff_len);

	err = ufshcd_read_string_desc(hba, model_index, desc_buf,
				      QUERY_DESC_MAX_SIZE, true/*ASCII*/);
	if (err) {
		dev_err(hba->dev, "%s: Failed reading Product Name. err = %d\n",
			__func__, err);
		goto out;
	}

	desc_buf[QUERY_DESC_MAX_SIZE] = '\0';
	strlcpy(dev_desc->model, (char *)(desc_buf + QUERY_DESC_HDR_SIZE),
		min_t(u8, desc_buf[QUERY_DESC_LENGTH_OFFSET],
		      MAX_MODEL_LEN));

	/* Null terminate the model string */
	dev_desc->model[MAX_MODEL_LEN] = '\0';

out:
	kfree(desc_buf);
	return err;
}

/**
 * ufshcd_get_max_pwr_mode - reads the max power mode negotiated with device
 */
static int ufshcd_get_max_pwr_mode(struct ufs_hba *hba)
{
	struct ufs_pa_layer_attr *pwr_info = &hba->max_pwr_info.info;

	if (hba->max_pwr_info.is_valid)
		return 0;

	if (hba->quirks & UFSHCD_QUIRK_HIBERN_FASTAUTO) {
		pwr_info->pwr_tx = FASTAUTO_MODE;
		pwr_info->pwr_rx = FASTAUTO_MODE;
	} else {
		pwr_info->pwr_tx = FAST_MODE;
		pwr_info->pwr_rx = FAST_MODE;
	}
	pwr_info->hs_rate = PA_HS_MODE_B;

	/* Get the connected lane count */
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDRXDATALANES),
		       &pwr_info->lane_rx);
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_CONNECTEDTXDATALANES),
		       &pwr_info->lane_tx);

	if (!pwr_info->lane_rx || !pwr_info->lane_tx) {
		dev_err(hba->dev, "%s: invalid connected lanes value. rx=%d, tx=%d\n",
			__func__, pwr_info->lane_rx, pwr_info->lane_tx);
		return -EINVAL;
	}

	/*
	 * First, get the maximum gears of HS speed.
	 * If a zero value, it means there is no HSGEAR capability.
	 * Then, get the maximum gears of PWM speed.
	 */
	ufshcd_dme_get(hba, UIC_ARG_MIB(PA_MAXRXHSGEAR), &pwr_info->gear_rx);
	if (!pwr_info->gear_rx) {
		ufshcd_dme_get(hba, UIC_ARG_MIB(PA_MAXRXPWMGEAR),
			       &pwr_info->gear_rx);
		if (!pwr_info->gear_rx) {
			dev_err(hba->dev, "%s: invalid max pwm rx gear read = %d\n",
				__func__, pwr_info->gear_rx);
			return -EINVAL;
		}
		pwr_info->pwr_rx = SLOW_MODE;
	}

	ufshcd_dme_peer_get(hba, UIC_ARG_MIB(PA_MAXRXHSGEAR),
			    &pwr_info->gear_tx);
	if (!pwr_info->gear_tx) {
		ufshcd_dme_peer_get(hba, UIC_ARG_MIB(PA_MAXRXPWMGEAR),
				    &pwr_info->gear_tx);
		if (!pwr_info->gear_tx) {
			dev_err(hba->dev, "%s: invalid max pwm tx gear read = %d\n",
				__func__, pwr_info->gear_tx);
			return -EINVAL;
		}
		pwr_info->pwr_tx = SLOW_MODE;
	}

	hba->max_pwr_info.is_valid = true;
	return 0;
}

static int ufshcd_change_power_mode(struct ufs_hba *hba,
				    struct ufs_pa_layer_attr *pwr_mode)
{
	int ret;

	/* if already configured to the requested pwr_mode */
	if (pwr_mode->gear_rx == hba->pwr_info.gear_rx &&
	    pwr_mode->gear_tx == hba->pwr_info.gear_tx &&
	    pwr_mode->lane_rx == hba->pwr_info.lane_rx &&
	    pwr_mode->lane_tx == hba->pwr_info.lane_tx &&
	    pwr_mode->pwr_rx == hba->pwr_info.pwr_rx &&
	    pwr_mode->pwr_tx == hba->pwr_info.pwr_tx &&
	    pwr_mode->hs_rate == hba->pwr_info.hs_rate) {
		dev_dbg(hba->dev, "%s: power already configured\n", __func__);
		return 0;
	}

	/*
	 * Configure attributes for power mode change with below.
	 * - PA_RXGEAR, PA_ACTIVERXDATALANES, PA_RXTERMINATION,
	 * - PA_TXGEAR, PA_ACTIVETXDATALANES, PA_TXTERMINATION,
	 * - PA_HSSERIES
	 */
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_RXGEAR), pwr_mode->gear_rx);
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_ACTIVERXDATALANES),
		       pwr_mode->lane_rx);
	if (pwr_mode->pwr_rx == FASTAUTO_MODE || pwr_mode->pwr_rx == FAST_MODE)
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_RXTERMINATION), TRUE);
	else
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_RXTERMINATION), FALSE);

	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXGEAR), pwr_mode->gear_tx);
	ufshcd_dme_set(hba, UIC_ARG_MIB(PA_ACTIVETXDATALANES),
		       pwr_mode->lane_tx);
	if (pwr_mode->pwr_tx == FASTAUTO_MODE || pwr_mode->pwr_tx == FAST_MODE)
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXTERMINATION), TRUE);
	else
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_TXTERMINATION), FALSE);

	if (pwr_mode->pwr_rx == FASTAUTO_MODE ||
	    pwr_mode->pwr_tx == FASTAUTO_MODE ||
	    pwr_mode->pwr_rx == FAST_MODE ||
	    pwr_mode->pwr_tx == FAST_MODE)
		ufshcd_dme_set(hba, UIC_ARG_MIB(PA_HSSERIES),
			       pwr_mode->hs_rate);

	ret = ufshcd_uic_change_pwr_mode(hba, pwr_mode->pwr_rx << 4 |
					 pwr_mode->pwr_tx);

	if (ret) {
		dev_err(hba->dev,
			"%s: power mode change failed %d\n", __func__, ret);

		return ret;
	}

	/* Copy new Power Mode to power info */
	memcpy(&hba->pwr_info, pwr_mode, sizeof(struct ufs_pa_layer_attr));

	return ret;
}

/**
 * ufshcd_verify_dev_init() - Verify device initialization
 *
 */
static int ufshcd_verify_dev_init(struct ufs_hba *hba)
{
	int retries;
	int err;

	for (retries = NOP_OUT_RETRIES; retries > 0; retries--) {
		err = ufshcd_exec_dev_cmd(hba, DEV_CMD_TYPE_NOP,
					  NOP_OUT_TIMEOUT);
		if (!err || err == -ETIMEDOUT)
			break;

		dev_dbg(hba->dev, "%s: error %d retrying\n", __func__, err);
	}

	if (err)
		dev_err(hba->dev, "%s: NOP OUT failed %d\n", __func__, err);

	return err;
}

/**
 * ufshcd_complete_dev_init() - checks device readiness
 */
static int ufshcd_complete_dev_init(struct ufs_hba *hba)
{
	int i;
	int err;
	bool flag_res = 1;

	err = ufshcd_query_flag_retry(hba, UPIU_QUERY_OPCODE_SET_FLAG,
				      QUERY_FLAG_IDN_FDEVICEINIT, NULL);
	if (err) {
		dev_err(hba->dev,
			"%s setting fDeviceInit flag failed with error %d\n",
			__func__, err);
		goto out;
	}

	/* poll for max. 1000 iterations for fDeviceInit flag to clear */
	for (i = 0; i < 1000 && !err && flag_res; i++)
		err = ufshcd_query_flag_retry(hba, UPIU_QUERY_OPCODE_READ_FLAG,
					      QUERY_FLAG_IDN_FDEVICEINIT,
					      &flag_res);

	if (err)
		dev_err(hba->dev,
			"%s reading fDeviceInit flag failed with error %d\n",
			__func__, err);
	else if (flag_res)
		dev_err(hba->dev,
			"%s fDeviceInit was not cleared by the device\n",
			__func__);

out:
	return err;
}

static void ufshcd_def_desc_sizes(struct ufs_hba *hba)
{
	hba->desc_size.dev_desc = QUERY_DESC_DEVICE_DEF_SIZE;
	hba->desc_size.pwr_desc = QUERY_DESC_POWER_DEF_SIZE;
	hba->desc_size.interc_desc = QUERY_DESC_INTERCONNECT_DEF_SIZE;
	hba->desc_size.conf_desc = QUERY_DESC_CONFIGURATION_DEF_SIZE;
	hba->desc_size.unit_desc = QUERY_DESC_UNIT_DEF_SIZE;
	hba->desc_size.geom_desc = QUERY_DESC_GEOMETRY_DEF_SIZE;
	hba->desc_size.hlth_desc = QUERY_DESC_HEALTH_DEF_SIZE;
}

int ufs_start(struct ufs_hba *hba)
{
	struct ufs_dev_desc card = {0};
	int ret;

	ret = ufshcd_link_startup(hba);
	if (ret)
		return ret;

	ret = ufshcd_verify_dev_init(hba);
	if (ret)
		return ret;

	ret = ufshcd_complete_dev_init(hba);
	if (ret)
		return ret;

	/* Init check for device descriptor sizes */
	ufshcd_init_desc_sizes(hba);

	ret = ufs_get_device_desc(hba, &card);
	if (ret) {
		dev_err(hba->dev, "%s: Failed getting device info. err = %d\n",
			__func__, ret);

		return ret;
	}

	if (ufshcd_get_max_pwr_mode(hba)) {
		dev_err(hba->dev,
			"%s: Failed getting max supported power mode\n",
			__func__);
	} else {
		ret = ufshcd_change_power_mode(hba, &hba->max_pwr_info.info);
		if (ret) {
			dev_err(hba->dev, "%s: Failed setting power mode, err = %d\n",
				__func__, ret);

			return ret;
		}

		printf("Device at %s up at:", hba->dev->name);
		ufshcd_print_pwr_info(hba);
	}

	return 0;
}

int ufshcd_probe(struct udevice *ufs_dev, struct ufs_hba_ops *hba_ops)
{
	struct ufs_hba *hba = dev_get_uclass_priv(ufs_dev);
	struct scsi_plat *scsi_plat;
	struct udevice *scsi_dev;
	void __iomem *mmio_base;
	int err;

	device_find_first_child(ufs_dev, &scsi_dev);
	if (!scsi_dev)
		return -ENODEV;

	scsi_plat = dev_get_uclass_plat(scsi_dev);
	scsi_plat->max_id = UFSHCD_MAX_ID;
	scsi_plat->max_lun = UFS_MAX_LUNS;
	scsi_plat->max_bytes_per_req = UFS_MAX_BYTES;

	hba->dev = ufs_dev;
	hba->ops = hba_ops;

	if (device_is_on_pci_bus(ufs_dev)) {
		mmio_base = dm_pci_map_bar(ufs_dev, PCI_BASE_ADDRESS_0, 0, 0,
					   PCI_REGION_TYPE, PCI_REGION_MEM);
	} else {
		mmio_base = dev_read_addr_ptr(ufs_dev);
	}
	hba->mmio_base = mmio_base;

	/* Set descriptor lengths to specification defaults */
	ufshcd_def_desc_sizes(hba);

	ufshcd_ops_init(hba);

	/* Read capabilties registers */
	hba->capabilities = ufshcd_readl(hba, REG_CONTROLLER_CAPABILITIES);
	if (hba->quirks & UFSHCD_QUIRK_BROKEN_64BIT_ADDRESS)
		hba->capabilities &= ~MASK_64_ADDRESSING_SUPPORT;

	/* Get UFS version supported by the controller */
	hba->version = ufshcd_get_ufs_version(hba);
	if (hba->version != UFSHCI_VERSION_10 &&
	    hba->version != UFSHCI_VERSION_11 &&
	    hba->version != UFSHCI_VERSION_20 &&
	    hba->version != UFSHCI_VERSION_21 &&
	    hba->version != UFSHCI_VERSION_30 &&
	    hba->version != UFSHCI_VERSION_31)
		dev_err(hba->dev, "invalid UFS version 0x%x\n",
			hba->version);

	/* Get Interrupt bit mask per version */
	hba->intr_mask = ufshcd_get_intr_mask(hba);

	/* Allocate memory for host memory space */
	err = ufshcd_memory_alloc(hba);
	if (err) {
		dev_err(hba->dev, "Memory allocation failed\n");
		return err;
	}

	/* Configure Local data structures */
	ufshcd_host_memory_configure(hba);

	/*
	 * In order to avoid any spurious interrupt immediately after
	 * registering UFS controller interrupt handler, clear any pending UFS
	 * interrupt status and disable all the UFS interrupts.
	 */
	ufshcd_writel(hba, ufshcd_readl(hba, REG_INTERRUPT_STATUS),
		      REG_INTERRUPT_STATUS);
	ufshcd_writel(hba, 0, REG_INTERRUPT_ENABLE);

	err = ufshcd_hba_enable(hba);
	if (err) {
		dev_err(hba->dev, "Host controller enable failed\n");
		return err;
	}

	err = ufs_start(hba);
	if (err)
		return err;

	return 0;
}

int ufs_scsi_bind(struct udevice *ufs_dev, struct udevice **scsi_devp)
{
	int ret = device_bind_driver(ufs_dev, "ufs_scsi", "ufs_scsi",
				     scsi_devp);

	return ret;
}

#if IS_ENABLED(CONFIG_BOUNCE_BUFFER)
static int ufs_scsi_buffer_aligned(struct udevice *scsi_dev, struct bounce_buffer *state)
{
#ifdef CONFIG_PHYS_64BIT
	struct ufs_hba *hba = dev_get_uclass_priv(scsi_dev->parent);
	uintptr_t ubuf = (uintptr_t)state->user_buffer;
	size_t len = state->len_aligned;

	/* Check if below 32bit boundary */
	if ((hba->quirks & UFSHCD_QUIRK_BROKEN_64BIT_ADDRESS) &&
	    ((ubuf >> 32) || (ubuf + len) >> 32)) {
		dev_dbg(scsi_dev, "Buffer above 32bit boundary %lx-%lx\n",
			ubuf, ubuf + len);
		return 0;
	}
#endif
	return 1;
}
#endif	/* CONFIG_BOUNCE_BUFFER */

static struct scsi_ops ufs_ops = {
	.exec		= ufs_scsi_exec,
#if IS_ENABLED(CONFIG_BOUNCE_BUFFER)
	.buffer_aligned	= ufs_scsi_buffer_aligned,
#endif	/* CONFIG_BOUNCE_BUFFER */
};

int ufs_probe_dev(int index)
{
	struct udevice *dev;

	return uclass_get_device(UCLASS_UFS, index, &dev);
}

int ufs_probe(void)
{
	struct udevice *dev;
	int ret, i;

	for (i = 0;; i++) {
		ret = uclass_get_device(UCLASS_UFS, i, &dev);
		if (ret == -ENODEV)
			break;
	}

	return 0;
}

U_BOOT_DRIVER(ufs_scsi) = {
	.id = UCLASS_SCSI,
	.name = "ufs_scsi",
	.ops = &ufs_ops,
};
