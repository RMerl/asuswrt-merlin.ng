// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2020-2022 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <errno.h>
#include <fdtdec.h>
#include <hang.h>
#include <ram.h>
#include <reset.h>
#include "sdram_soc64.h"
#include <wait_bit.h>
#include <asm/arch/firewall.h>
#include <asm/arch/handoff_soc64.h>
#include <asm/arch/misc.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>
#include <linux/err.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

/* MPFE NOC registers */
#define FPGA2SDRAM_MGR_MAIN_SIDEBANDMGR_FLAGOUTSET0	0xF8024050

/* Memory reset manager */
#define MEM_RST_MGR_STATUS	0x8

/* Register and bit in memory reset manager */
#define MEM_RST_MGR_STATUS_RESET_COMPLETE	BIT(0)
#define MEM_RST_MGR_STATUS_PWROKIN_STATUS	BIT(1)
#define MEM_RST_MGR_STATUS_CONTROLLER_RST	BIT(2)
#define MEM_RST_MGR_STATUS_AXI_RST		BIT(3)

#define TIMEOUT_200MS     200
#define TIMEOUT_5000MS    5000

/* DDR4 umctl2 */
#define DDR4_MSTR_OFFSET		0x0
#define DDR4_FREQ_RATIO			BIT(22)

#define DDR4_STAT_OFFSET		0x4
#define DDR4_STAT_SELFREF_TYPE		GENMASK(5, 4)
#define DDR4_STAT_SELFREF_TYPE_SHIFT	4
#define DDR4_STAT_OPERATING_MODE	GENMASK(2, 0)

#define DDR4_MRCTRL0_OFFSET		0x10
#define DDR4_MRCTRL0_MR_TYPE		BIT(0)
#define DDR4_MRCTRL0_MPR_EN		BIT(1)
#define DDR4_MRCTRL0_MR_RANK		GENMASK(5, 4)
#define DDR4_MRCTRL0_MR_RANK_SHIFT	4
#define DDR4_MRCTRL0_MR_ADDR		GENMASK(15, 12)
#define DDR4_MRCTRL0_MR_ADDR_SHIFT	12
#define DDR4_MRCTRL0_MR_WR		BIT(31)

#define DDR4_MRCTRL1_OFFSET		0x14
#define DDR4_MRCTRL1_MR_DATA		0x3FFFF

#define DDR4_MRSTAT_OFFSET		0x18
#define DDR4_MRSTAT_MR_WR_BUSY		BIT(0)

#define DDR4_MRCTRL2_OFFSET		0x1C

#define DDR4_PWRCTL_OFFSET			0x30
#define DDR4_PWRCTL_SELFREF_EN			BIT(0)
#define DDR4_PWRCTL_POWERDOWN_EN		BIT(1)
#define DDR4_PWRCTL_EN_DFI_DRAM_CLK_DISABLE	BIT(3)
#define DDR4_PWRCTL_SELFREF_SW			BIT(5)

#define DDR4_PWRTMG_OFFSET		0x34
#define DDR4_HWLPCTL_OFFSET		0x38
#define DDR4_RFSHCTL0_OFFSET		0x50
#define DDR4_RFSHCTL1_OFFSET		0x54

#define DDR4_RFSHCTL3_OFFSET			0x60
#define DDR4_RFSHCTL3_DIS_AUTO_REFRESH		BIT(0)
#define DDR4_RFSHCTL3_REFRESH_MODE		GENMASK(6, 4)
#define DDR4_RFSHCTL3_REFRESH_MODE_SHIFT	4

#define DDR4_ECCCFG0_OFFSET		0x70
#define DDR4_ECC_MODE			GENMASK(2, 0)
#define DDR4_DIS_SCRUB			BIT(4)
#define LPDDR4_ECCCFG0_ECC_REGION_MAP_GRANU_SHIFT	30
#define LPDDR4_ECCCFG0_ECC_REGION_MAP_SHIFT	8

#define DDR4_ECCCFG1_OFFSET		0x74
#define LPDDR4_ECCCFG1_ECC_REGIONS_PARITY_LOCK	BIT(4)

#define DDR4_CRCPARCTL0_OFFSET			0xC0
#define DDR4_CRCPARCTL0_DFI_ALERT_ERR_INIT_CLR	BIT(1)

#define DDR4_CRCPARCTL1_OFFSET			0xC4
#define DDR4_CRCPARCTL1_CRC_PARITY_RETRY_ENABLE	BIT(8)
#define DDR4_CRCPARCTL1_ALERT_WAIT_FOR_SW	BIT(9)

#define DDR4_CRCPARSTAT_OFFSET			0xCC
#define DDR4_CRCPARSTAT_DFI_ALERT_ERR_INT	BIT(16)
#define DDR4_CRCPARSTAT_DFI_ALERT_ERR_FATL_INT	BIT(17)
#define DDR4_CRCPARSTAT_DFI_ALERT_ERR_NO_SW	BIT(19)
#define DDR4_CRCPARSTAT_CMD_IN_ERR_WINDOW	BIT(29)

#define DDR4_INIT0_OFFSET			0xD0
#define DDR4_INIT0_SKIP_RAM_INIT		GENMASK(31, 30)

#define DDR4_RANKCTL_OFFSET			0xF4
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP		GENMASK(7, 4)
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP		GENMASK(11, 8)
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP_MSB	BIT(24)
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP_MSB	BIT(26)
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP_SHIFT	4
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP_SHIFT	8
#define DDR4_RANKCTL_DIFF_RANK_RD_GAP_MSB_SHIFT	24
#define DDR4_RANKCTL_DIFF_RANK_WR_GAP_MSB_SHIFT	26

#define DDR4_RANKCTL1_OFFSET	0xF8
#define DDR4_RANKCTL1_WR2RD_DR	GENMASK(5, 0)

#define DDR4_DRAMTMG2_OFFSET	0x108
#define DDR4_DRAMTMG2_WR2RD	GENMASK(5, 0)
#define DDR4_DRAMTMG2_RD2WR	GENMASK(13, 8)
#define DDR4_DRAMTMG2_RD2WR_SHIFT	8

#define DDR4_DRAMTMG9_OFFSET	0x124
#define DDR4_DRAMTMG9_W2RD_S	GENMASK(5, 0)

#define DDR4_DFITMG1_OFFSET	0x194
#define DDR4_DFITMG1_DFI_T_WRDATA_DELAY	GENMASK(20, 16)
#define DDR4_DFITMG1_DFI_T_WRDATA_SHIFT	16

#define DDR4_DFIMISC_OFFSET			0x1B0
#define DDR4_DFIMISC_DFI_INIT_COMPLETE_EN	BIT(0)
#define DDR4_DFIMISC_DFI_INIT_START		BIT(5)

#define DDR4_DFISTAT_OFFSET		0x1BC
#define DDR4_DFI_INIT_COMPLETE		BIT(0)

#define DDR4_DBG0_OFFSET		0x300

#define DDR4_DBG1_OFFSET		0x304
#define DDR4_DBG1_DISDQ			BIT(0)
#define DDR4_DBG1_DIS_HIF		BIT(1)

#define DDR4_DBGCAM_OFFSET			0x308
#define DDR4_DBGCAM_DBG_RD_Q_EMPTY		BIT(25)
#define DDR4_DBGCAM_DBG_WR_Q_EMPTY		BIT(26)
#define DDR4_DBGCAM_RD_DATA_PIPELINE_EMPTY	BIT(28)
#define DDR4_DBGCAM_WR_DATA_PIPELINE_EMPTY	BIT(29)

#define DDR4_SWCTL_OFFSET		0x320
#define DDR4_SWCTL_SW_DONE		BIT(0)

#define DDR4_SWSTAT_OFFSET		0x324
#define DDR4_SWSTAT_SW_DONE_ACK		BIT(0)

#define DDR4_PSTAT_OFFSET		0x3FC
#define DDR4_PSTAT_RD_PORT_BUSY_0	BIT(0)
#define DDR4_PSTAT_WR_PORT_BUSY_0	BIT(16)

#define DDR4_PCTRL0_OFFSET		0x490
#define DDR4_PCTRL0_PORT_EN		BIT(0)

#define DDR4_SBRCTL_OFFSET		0xF24
#define DDR4_SBRCTL_SCRUB_INTERVAL	0x1FFF00
#define DDR4_SBRCTL_SCRUB_EN		BIT(0)
#define DDR4_SBRCTL_SCRUB_WRITE		BIT(2)
#define DDR4_SBRCTL_SCRUB_BURST_1	BIT(4)

#define DDR4_SBRSTAT_OFFSET		0xF28
#define DDR4_SBRSTAT_SCRUB_BUSY BIT(0)
#define DDR4_SBRSTAT_SCRUB_DONE BIT(1)

#define DDR4_SBRWDATA0_OFFSET		0xF2C
#define DDR4_SBRWDATA1_OFFSET		0xF30
#define DDR4_SBRSTART0_OFFSET		0xF38
#define DDR4_SBRSTART1_OFFSET		0xF3C
#define DDR4_SBRRANGE0_OFFSET		0xF40
#define DDR4_SBRRANGE1_OFFSET		0xF44

/* DDR PHY */
#define DDR_PHY_TXODTDRVSTREN_B0_P0		0x2009A
#define DDR_PHY_RXPBDLYTG0_R0			0x200D0
#define DDR_PHY_DBYTE0_TXDQDLYTG0_U0_P0		0x201A0

#define DDR_PHY_DBYTE0_TXDQDLYTG0_U1_P0		0x203A0
#define DDR_PHY_DBYTE1_TXDQDLYTG0_U0_P0		0x221A0
#define DDR_PHY_DBYTE1_TXDQDLYTG0_U1_P0		0x223A0
#define DDR_PHY_TXDQDLYTG0_COARSE_DELAY		GENMASK(9, 6)
#define DDR_PHY_TXDQDLYTG0_COARSE_DELAY_SHIFT	6

#define DDR_PHY_CALRATE_OFFSET			0x40110
#define DDR_PHY_CALZAP_OFFSET			0x40112
#define DDR_PHY_SEQ0BDLY0_P0_OFFSET		0x40016
#define DDR_PHY_SEQ0BDLY1_P0_OFFSET		0x40018
#define DDR_PHY_SEQ0BDLY2_P0_OFFSET		0x4001A
#define DDR_PHY_SEQ0BDLY3_P0_OFFSET		0x4001C

#define DDR_PHY_MEMRESETL_OFFSET		0x400C0
#define DDR_PHY_MEMRESETL_VALUE			BIT(0)
#define DDR_PHY_PROTECT_MEMRESET		BIT(1)

#define DDR_PHY_CALBUSY_OFFSET			0x4012E
#define DDR_PHY_CALBUSY				BIT(0)

#define DDR_PHY_TRAIN_IMEM_OFFSET		0xA0000
#define DDR_PHY_TRAIN_DMEM_OFFSET		0xA8000

#define DMEM_MB_CDD_RR_1_0_OFFSET		0xA802C
#define DMEM_MB_CDD_RR_0_1_OFFSET		0xA8030
#define DMEM_MB_CDD_WW_1_0_OFFSET		0xA8038
#define DMEM_MB_CDD_WW_0_1_OFFSET		0xA803C
#define DMEM_MB_CDD_RW_1_1_OFFSET		0xA8046
#define DMEM_MB_CDD_RW_1_0_OFFSET		0xA8048
#define DMEM_MB_CDD_RW_0_1_OFFSET		0xA804A
#define DMEM_MB_CDD_RW_0_0_OFFSET		0xA804C

#define DMEM_MB_CDD_CHA_RR_1_0_OFFSET		0xA8026
#define DMEM_MB_CDD_CHA_RR_0_1_OFFSET		0xA8026
#define DMEM_MB_CDD_CHB_RR_1_0_OFFSET		0xA8058
#define DMEM_MB_CDD_CHB_RR_0_1_OFFSET		0xA805A
#define DMEM_MB_CDD_CHA_WW_1_0_OFFSET		0xA8030
#define DMEM_MB_CDD_CHA_WW_0_1_OFFSET		0xA8030
#define DMEM_MB_CDD_CHB_WW_1_0_OFFSET		0xA8062
#define DMEM_MB_CDD_CHB_WW_0_1_OFFSET		0xA8064

#define DMEM_MB_CDD_CHA_RW_1_1_OFFSET		0xA8028
#define DMEM_MB_CDD_CHA_RW_1_0_OFFSET		0xA8028
#define DMEM_MB_CDD_CHA_RW_0_1_OFFSET		0xA802A
#define DMEM_MB_CDD_CHA_RW_0_0_OFFSET		0xA802A

#define DMEM_MB_CDD_CHB_RW_1_1_OFFSET		0xA805A
#define DMEM_MB_CDD_CHB_RW_1_0_OFFSET		0xA805C
#define DMEM_MB_CDD_CHB_RW_0_1_OFFSET		0xA805c
#define DMEM_MB_CDD_CHB_RW_0_0_OFFSET		0xA805E

#define DDR_PHY_SEQ0DISABLEFLAG0_OFFSET		0x120018
#define DDR_PHY_SEQ0DISABLEFLAG1_OFFSET		0x12001A
#define DDR_PHY_SEQ0DISABLEFLAG2_OFFSET		0x12001C
#define DDR_PHY_SEQ0DISABLEFLAG3_OFFSET		0x12001E
#define DDR_PHY_SEQ0DISABLEFLAG4_OFFSET		0x120020
#define DDR_PHY_SEQ0DISABLEFLAG5_OFFSET		0x120022
#define DDR_PHY_SEQ0DISABLEFLAG6_OFFSET		0x120024
#define DDR_PHY_SEQ0DISABLEFLAG7_OFFSET		0x120026

#define DDR_PHY_UCCLKHCLKENABLES_OFFSET		0x180100
#define DDR_PHY_UCCLKHCLKENABLES_UCCLKEN	BIT(0)
#define DDR_PHY_UCCLKHCLKENABLES_HCLKEN		BIT(1)

#define DDR_PHY_UCTWRITEPROT_OFFSET		0x180066
#define DDR_PHY_UCTWRITEPROT			BIT(0)

#define DDR_PHY_APBONLY0_OFFSET			0x1A0000
#define DDR_PHY_MICROCONTMUXSEL			BIT(0)

#define DDR_PHY_UCTSHADOWREGS_OFFSET			0x1A0008
#define DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW	BIT(0)

#define DDR_PHY_DCTWRITEPROT_OFFSET		0x1A0062
#define DDR_PHY_DCTWRITEPROT			BIT(0)

#define DDR_PHY_UCTWRITEONLYSHADOW_OFFSET	0x1A0064
#define DDR_PHY_UCTDATWRITEONLYSHADOW_OFFSET	0x1A0068

#define DDR_PHY_MICRORESET_OFFSET		0x1A0132
#define DDR_PHY_MICRORESET_STALL		BIT(0)
#define DDR_PHY_MICRORESET_RESET		BIT(3)

#define DDR_PHY_TXODTDRVSTREN_B0_P1		0x22009A

/* For firmware training */
#define HW_DBG_TRACE_CONTROL_OFFSET	0x18
#define FW_TRAINING_COMPLETED_STAT	0x07
#define FW_TRAINING_FAILED_STAT		0xFF
#define FW_COMPLETION_MSG_ONLY_MODE	0xFF
#define FW_STREAMING_MSG_ID		0x08
#define GET_LOWHW_DATA(x)		((x) & 0xFFFF)
#define GET_LOWB_DATA(x)		((x) & 0xFF)
#define GET_HIGHB_DATA(x)		(((x) & 0xFF00) >> 8)

/* Operating mode */
#define OPM_INIT			0x000
#define OPM_NORMAL			0x001
#define OPM_PWR_D0WN			0x010
#define OPM_SELF_SELFREF		0x011
#define OPM_DDR4_DEEP_PWR_DOWN		0x100

/* Refresh mode */
#define FIXED_1X		0
#define FIXED_2X		BIT(0)
#define FIXED_4X		BIT(4)

/* Address of mode register */
#define MR0	0x0000
#define MR1	0x0001
#define MR2	0x0010
#define MR3	0x0011
#define MR4	0x0100
#define MR5	0x0101
#define MR6	0x0110
#define MR7	0x0111

/* MR rank */
#define RANK0		0x1
#define RANK1		0x2
#define ALL_RANK	0x3

#define MR5_BIT4	BIT(4)

/* Value for ecc_region_map */
#define ALL_PROTECTED	0x7F

/* Region size for ECCCFG0.ecc_region_map */
enum region_size {
	ONE_EIGHT,
	ONE_SIXTEENTH,
	ONE_THIRTY_SECOND,
	ONE_SIXTY_FOURTH
};

enum ddr_type {
	DDRTYPE_LPDDR4_0,
	DDRTYPE_LPDDR4_1,
	DDRTYPE_DDR4,
	DDRTYPE_UNKNOWN
};

/* Reset type */
enum reset_type {
	POR_RESET,
	WARM_RESET,
	COLD_RESET
};

/* DDR handoff structure */
struct ddr_handoff {
	/* Memory reset manager base */
	phys_addr_t mem_reset_base;

	/* First controller attributes */
	phys_addr_t cntlr_handoff_base;
	phys_addr_t cntlr_base;
	size_t cntlr_total_length;
	enum ddr_type cntlr_t;
	size_t cntlr_handoff_length;

	/* Second controller attributes*/
	phys_addr_t cntlr2_handoff_base;
	phys_addr_t cntlr2_base;
	size_t cntlr2_total_length;
	enum ddr_type cntlr2_t;
	size_t cntlr2_handoff_length;

	/* PHY attributes */
	phys_addr_t phy_handoff_base;
	phys_addr_t phy_base;
	size_t phy_total_length;
	size_t phy_handoff_length;

	/* PHY engine attributes */
	phys_addr_t phy_engine_handoff_base;
	size_t phy_engine_total_length;
	size_t phy_engine_handoff_length;

	/* Calibration attributes */
	phys_addr_t train_imem_base;
	phys_addr_t train_dmem_base;
	size_t train_imem_length;
	size_t train_dmem_length;
};

/* Message mode */
enum message_mode {
	MAJOR_MESSAGE,
	STREAMING_MESSAGE
};

static int clr_ca_parity_error_status(phys_addr_t umctl2_base)
{
	int ret;

	debug("%s: Clear C/A parity error status in MR5[4]\n", __func__);

	/* Set mode register MRS */
	clrbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET, DDR4_MRCTRL0_MPR_EN);

	/* Set mode register to write operation */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET, DDR4_MRCTRL0_MR_TYPE);

	/* Set the address of mode rgister to 0x101(MR5) */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET,
		     (MR5 << DDR4_MRCTRL0_MR_ADDR_SHIFT) &
		     DDR4_MRCTRL0_MR_ADDR);

	/* Set MR rank to rank 1 */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET,
		     (RANK1 << DDR4_MRCTRL0_MR_RANK_SHIFT) &
		     DDR4_MRCTRL0_MR_RANK);

	/* Clear C/A parity error status in MR5[4] */
	clrbits_le32(umctl2_base + DDR4_MRCTRL1_OFFSET, MR5_BIT4);

	/* Trigger mode register read or write operation */
	setbits_le32(umctl2_base + DDR4_MRCTRL0_OFFSET, DDR4_MRCTRL0_MR_WR);

	/* Wait for retry done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_MRSTAT_OFFSET), DDR4_MRSTAT_MR_WR_BUSY,
				false, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" no outstanding MR transaction\n");
		return ret;
	}

	return 0;
}

static int ddr_retry_software_sequence(phys_addr_t umctl2_base)
{
	u32 value;
	int ret;

	/* Check software can perform MRS/MPR/PDA? */
	value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
		      DDR4_CRCPARSTAT_DFI_ALERT_ERR_NO_SW;

	if (value) {
		/* Clear interrupt bit for DFI alert error */
		setbits_le32(umctl2_base + DDR4_CRCPARCTL0_OFFSET,
			     DDR4_CRCPARCTL0_DFI_ALERT_ERR_INIT_CLR);
	}

	debug("%s: Software can perform MRS/MPR/PDA\n", __func__);

	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_MRSTAT_OFFSET),
				DDR4_MRSTAT_MR_WR_BUSY,
				false, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" no outstanding MR transaction\n");
		return ret;
	}

	ret = clr_ca_parity_error_status(umctl2_base);
	if (ret)
		return ret;

	if (!value) {
		/* Clear interrupt bit for DFI alert error */
		setbits_le32(umctl2_base + DDR4_CRCPARCTL0_OFFSET,
			     DDR4_CRCPARCTL0_DFI_ALERT_ERR_INIT_CLR);
	}

	return 0;
}

static int ensure_retry_procedure_complete(phys_addr_t umctl2_base)
{
	u32 value;
	u32 start = get_timer(0);
	int ret;

	/* Check parity/crc/error window is emptied ? */
	value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
		      DDR4_CRCPARSTAT_CMD_IN_ERR_WINDOW;

	/* Polling until parity/crc/error window is emptied */
	while (value) {
		if (get_timer(start) > TIMEOUT_200MS) {
			debug("%s: Timeout while waiting for",
			      __func__);
			debug(" parity/crc/error window empty\n");
			return -ETIMEDOUT;
		}

		/* Check software intervention is enabled? */
		value = readl(umctl2_base + DDR4_CRCPARCTL1_OFFSET) &
			      DDR4_CRCPARCTL1_ALERT_WAIT_FOR_SW;
		if (value) {
			debug("%s: Software intervention is enabled\n",
			      __func__);

			/* Check dfi alert error interrupt is set? */
			value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
				      DDR4_CRCPARSTAT_DFI_ALERT_ERR_INT;

			if (value) {
				ret = ddr_retry_software_sequence(umctl2_base);
				debug("%s: DFI alert error interrupt ",
				      __func__);
				debug("is set\n");

				if (ret)
					return ret;
			}

			/*
			 * Check fatal parity error interrupt is set?
			 */
			value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
				      DDR4_CRCPARSTAT_DFI_ALERT_ERR_FATL_INT;
			if (value) {
				printf("%s: Fatal parity error  ",
				       __func__);
				printf("interrupt is set, Hang it!!\n");
				hang();
			}
		}

		value = readl(umctl2_base + DDR4_CRCPARSTAT_OFFSET) &
			      DDR4_CRCPARSTAT_CMD_IN_ERR_WINDOW;

		udelay(1);
		schedule();
	}

	return 0;
}

static int enable_quasi_dynamic_reg_grp3(phys_addr_t umctl2_base,
					 enum ddr_type umctl2_type)
{
	u32 i, value, backup;
	int ret = 0;

	/* Disable input traffic per port */
	clrbits_le32(umctl2_base + DDR4_PCTRL0_OFFSET, DDR4_PCTRL0_PORT_EN);

	/* Polling AXI port until idle */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_PSTAT_OFFSET),
				DDR4_PSTAT_WR_PORT_BUSY_0 |
				DDR4_PSTAT_RD_PORT_BUSY_0, false,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" controller idle\n");
		return ret;
	}

	/* Backup user setting */
	backup = readl(umctl2_base + DDR4_DBG1_OFFSET);

	/* Disable input traffic to the controller */
	setbits_le32(umctl2_base + DDR4_DBG1_OFFSET, DDR4_DBG1_DIS_HIF);

	/*
	 * Ensure CAM/data pipelines are empty.
	 * Poll until CAM/data pipelines are set at least twice,
	 * timeout at 200ms
	 */
	for (i = 0; i < 2; i++) {
		ret = wait_for_bit_le32((const void *)(umctl2_base +
					DDR4_DBGCAM_OFFSET),
					DDR4_DBGCAM_WR_DATA_PIPELINE_EMPTY |
					DDR4_DBGCAM_RD_DATA_PIPELINE_EMPTY |
					DDR4_DBGCAM_DBG_WR_Q_EMPTY |
					DDR4_DBGCAM_DBG_RD_Q_EMPTY, true,
					TIMEOUT_200MS, false);
		if (ret) {
			debug("%s: loop(%u): Timeout while waiting for",
			      __func__, i + 1);
			debug(" CAM/data pipelines are empty\n");

			goto out;
		}
	}

	if (umctl2_type == DDRTYPE_DDR4) {
		/* Check DDR4 retry is enabled ? */
		value = readl(umctl2_base + DDR4_CRCPARCTL1_OFFSET) &
			      DDR4_CRCPARCTL1_CRC_PARITY_RETRY_ENABLE;

		if (value) {
			debug("%s: DDR4 retry is enabled\n", __func__);

			ret = ensure_retry_procedure_complete(umctl2_base);
			if (ret) {
				debug("%s: Timeout while waiting for",
				      __func__);
				debug(" retry procedure complete\n");

				goto out;
			}
		}
	}

	debug("%s: Quasi-dynamic group 3 registers are enabled\n", __func__);

out:
	/* Restore user setting */
	writel(backup, umctl2_base + DDR4_DBG1_OFFSET);

	return ret;
}

static enum ddr_type get_ddr_type(phys_addr_t ddr_type_location)
{
	u32 ddr_type_magic = readl(ddr_type_location);

	if (ddr_type_magic == SOC64_HANDOFF_DDR_UMCTL2_DDR4_TYPE)
		return DDRTYPE_DDR4;

	if (ddr_type_magic == SOC64_HANDOFF_DDR_UMCTL2_LPDDR4_0_TYPE)
		return DDRTYPE_LPDDR4_0;

	if (ddr_type_magic == SOC64_HANDOFF_DDR_UMCTL2_LPDDR4_1_TYPE)
		return DDRTYPE_LPDDR4_1;

	return DDRTYPE_UNKNOWN;
}

static void use_lpddr4_interleaving(bool set)
{
	if (set) {
		printf("Starting LPDDR4 interleaving configuration ...\n");
		setbits_le32(FPGA2SDRAM_MGR_MAIN_SIDEBANDMGR_FLAGOUTSET0,
			     BIT(5));
	} else {
		printf("Starting LPDDR4 non-interleaving configuration ...\n");
		clrbits_le32(FPGA2SDRAM_MGR_MAIN_SIDEBANDMGR_FLAGOUTSET0,
			     BIT(5));
	}
}

static void use_ddr4(enum ddr_type type)
{
	if (type == DDRTYPE_DDR4) {
		printf("Starting DDR4 configuration ...\n");
		setbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_DDR_MODE,
			     SYSMGR_SOC64_DDR_MODE_MSK);
	} else if (type == DDRTYPE_LPDDR4_0)  {
		printf("Starting LPDDR4 configuration ...\n");
		clrbits_le32(socfpga_get_sysmgr_addr() + SYSMGR_SOC64_DDR_MODE,
			     SYSMGR_SOC64_DDR_MODE_MSK);

		use_lpddr4_interleaving(false);
	}
}

static int scrubber_ddr_config(phys_addr_t umctl2_base,
			       enum ddr_type umctl2_type)
{
	u32 backup[9];
	int ret;

	/* Reset to default value, prevent scrubber stop due to lower power */
	writel(0, umctl2_base + DDR4_PWRCTL_OFFSET);

	/* Backup user settings */
	backup[0] = readl(umctl2_base + DDR4_SBRCTL_OFFSET);
	backup[1] = readl(umctl2_base + DDR4_SBRWDATA0_OFFSET);
	backup[2] = readl(umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDRTYPE_DDR4) {
		backup[3] = readl(umctl2_base + DDR4_SBRWDATA1_OFFSET);
		backup[4] = readl(umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	backup[5] = readl(umctl2_base + DDR4_SBRRANGE0_OFFSET);
	backup[6] = readl(umctl2_base + DDR4_SBRRANGE1_OFFSET);
	backup[7] = readl(umctl2_base + DDR4_ECCCFG0_OFFSET);
	backup[8] = readl(umctl2_base + DDR4_ECCCFG1_OFFSET);

	if (umctl2_type != DDRTYPE_DDR4) {
		/* Lock ECC region, ensure this regions is not being accessed */
		setbits_le32(umctl2_base + DDR4_ECCCFG1_OFFSET,
			     LPDDR4_ECCCFG1_ECC_REGIONS_PARITY_LOCK);
	}
	/* Disable input traffic per port */
	clrbits_le32(umctl2_base + DDR4_PCTRL0_OFFSET, DDR4_PCTRL0_PORT_EN);
	/* Disables scrubber */
	clrbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);
	/* Polling all scrub writes data have been sent */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SBRSTAT_OFFSET), DDR4_SBRSTAT_SCRUB_BUSY,
				false, TIMEOUT_5000MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" sending all scrub data\n");
		return ret;
	}

	/* LPDDR4 supports inline ECC only */
	if (umctl2_type != DDRTYPE_DDR4) {
		/*
		 * Setting all regions for protected, this is required for
		 * srubber to init whole LPDDR4 expect ECC region
		 */
		writel(((ONE_EIGHT <<
		       LPDDR4_ECCCFG0_ECC_REGION_MAP_GRANU_SHIFT) |
		       (ALL_PROTECTED << LPDDR4_ECCCFG0_ECC_REGION_MAP_SHIFT)),
		       umctl2_base + DDR4_ECCCFG0_OFFSET);
	}

	/* Scrub_burst = 1, scrub_mode = 1(performs writes) */
	writel(DDR4_SBRCTL_SCRUB_BURST_1 | DDR4_SBRCTL_SCRUB_WRITE,
	       umctl2_base + DDR4_SBRCTL_OFFSET);

	/* Zeroing whole DDR */
	writel(0, umctl2_base + DDR4_SBRWDATA0_OFFSET);
	writel(0, umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDRTYPE_DDR4) {
		writel(0, umctl2_base + DDR4_SBRWDATA1_OFFSET);
		writel(0, umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	writel(0, umctl2_base + DDR4_SBRRANGE0_OFFSET);
	writel(0, umctl2_base + DDR4_SBRRANGE1_OFFSET);

	/* Enables scrubber */
	setbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);
	/* Polling all scrub writes commands have been sent */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SBRSTAT_OFFSET), DDR4_SBRSTAT_SCRUB_DONE,
				true, TIMEOUT_5000MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" sending all scrub commands\n");
		return ret;
	}

	/* Polling all scrub writes data have been sent */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SBRSTAT_OFFSET), DDR4_SBRSTAT_SCRUB_BUSY,
				false, TIMEOUT_5000MS, false);
	if (ret) {
		printf("%s: Timeout while waiting for", __func__);
		printf(" sending all scrub data\n");
		return ret;
	}

	/* Disables scrubber */
	clrbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET, DDR4_SBRCTL_SCRUB_EN);

	/* Restore user settings */
	writel(backup[0], umctl2_base + DDR4_SBRCTL_OFFSET);
	writel(backup[1], umctl2_base + DDR4_SBRWDATA0_OFFSET);
	writel(backup[2], umctl2_base + DDR4_SBRSTART0_OFFSET);
	if (umctl2_type == DDRTYPE_DDR4) {
		writel(backup[3], umctl2_base + DDR4_SBRWDATA1_OFFSET);
		writel(backup[4], umctl2_base + DDR4_SBRSTART1_OFFSET);
	}
	writel(backup[5], umctl2_base + DDR4_SBRRANGE0_OFFSET);
	writel(backup[6], umctl2_base + DDR4_SBRRANGE1_OFFSET);
	writel(backup[7], umctl2_base + DDR4_ECCCFG0_OFFSET);
	writel(backup[8], umctl2_base + DDR4_ECCCFG1_OFFSET);

	/* Enables ECC scrub on scrubber */
	if (!(readl(umctl2_base + DDR4_SBRCTL_OFFSET) &
	    DDR4_SBRCTL_SCRUB_WRITE)) {
		/* Enables scrubber */
		setbits_le32(umctl2_base + DDR4_SBRCTL_OFFSET,
			     DDR4_SBRCTL_SCRUB_EN);
	}

	return 0;
}

static void handoff_process(struct ddr_handoff *ddr_handoff_info,
			    phys_addr_t handoff_base, size_t length,
			    phys_addr_t base)
{
	u32 handoff_table[length];
	u32 i, value = 0;

	/* Execute configuration handoff */
	socfpga_handoff_read((void *)handoff_base, handoff_table, length);

	for (i = 0; i < length; i = i + 2) {
		debug("%s: wr = 0x%08x ",  __func__, handoff_table[i + 1]);
		if (ddr_handoff_info && base == ddr_handoff_info->phy_base) {
			/*
			 * Convert PHY odd offset to even offset that
			 * supported by ARM processor.
			 */
			value = handoff_table[i] << 1;

			writew(handoff_table[i + 1],
			       (uintptr_t)(value + base));
			debug("rd = 0x%08x ",
			      readw((uintptr_t)(value + base)));
			debug("PHY offset: 0x%08x ", handoff_table[i + 1]);
		} else {
			value = handoff_table[i];
			writel(handoff_table[i + 1], (uintptr_t)(value +
			       base));
			 debug("rd = 0x%08x ",
			       readl((uintptr_t)(value + base)));
		}

		debug("Absolute addr: 0x%08llx, APB offset: 0x%08x\n",
		      value + base, value);
	}
}

static int init_umctl2(phys_addr_t umctl2_handoff_base,
		       phys_addr_t umctl2_base, enum ddr_type umctl2_type,
		       size_t umctl2_handoff_length,
		       u32 *user_backup)
{
	int ret;

	if (umctl2_type == DDRTYPE_DDR4)
		printf("Initializing DDR4 controller ...\n");
	else if (umctl2_type == DDRTYPE_LPDDR4_0)
		printf("Initializing LPDDR4_0 controller ...\n");
	else if (umctl2_type == DDRTYPE_LPDDR4_1)
		printf("Initializing LPDDR4_1 controller ...\n");

	/* Prevent controller from issuing read/write to SDRAM */
	setbits_le32(umctl2_base + DDR4_DBG1_OFFSET, DDR4_DBG1_DISDQ);

	/* Put SDRAM into self-refresh */
	setbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET, DDR4_PWRCTL_SELFREF_EN);

	/* Enable quasi-dynamic programing of the controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Ensure the controller is in initialization mode */
	ret = wait_for_bit_le32((const void *)(umctl2_base + DDR4_STAT_OFFSET),
				DDR4_STAT_OPERATING_MODE, false, TIMEOUT_200MS,
				false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" init operating mode\n");
		return ret;
	}

	debug("%s: UMCTL2 handoff base address = 0x%p table length = 0x%08x\n",
	      __func__, (u32 *)umctl2_handoff_base,
	      (u32)umctl2_handoff_length);

	handoff_process(NULL, umctl2_handoff_base, umctl2_handoff_length,
			umctl2_base);

	/* Backup user settings, restore after DDR up running */
	*user_backup = readl(umctl2_base + DDR4_PWRCTL_OFFSET);

	/* Disable self resfresh */
	clrbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET, DDR4_PWRCTL_SELFREF_EN);

	if (umctl2_type == DDRTYPE_LPDDR4_0 ||
	    umctl2_type == DDRTYPE_LPDDR4_1) {
		/* Setting selfref_sw to 1, based on lpddr4 requirement */
		setbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET,
			     DDR4_PWRCTL_SELFREF_SW);

		/* Backup user settings, restore after DDR up running */
		user_backup++;
		*user_backup = readl(umctl2_base + DDR4_INIT0_OFFSET) &
				     DDR4_INIT0_SKIP_RAM_INIT;

		/*
		 * Setting INIT0.skip_dram_init to 0x3, based on lpddr4
		 * requirement
		 */
		setbits_le32(umctl2_base + DDR4_INIT0_OFFSET,
			     DDR4_INIT0_SKIP_RAM_INIT);
	}

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Enable controller from issuing read/write to SDRAM */
	clrbits_le32(umctl2_base + DDR4_DBG1_OFFSET, DDR4_DBG1_DISDQ);

	return 0;
}

static int phy_pre_handoff_config(phys_addr_t umctl2_base,
				  enum ddr_type umctl2_type)
{
	int ret;
	u32 value;

	if (umctl2_type == DDRTYPE_DDR4) {
		/* Check DDR4 retry is enabled ? */
		value = readl(umctl2_base + DDR4_CRCPARCTL1_OFFSET) &
			      DDR4_CRCPARCTL1_CRC_PARITY_RETRY_ENABLE;

		if (value) {
			debug("%s: DDR4 retry is enabled\n", __func__);
			debug("%s: Disable auto refresh is not supported\n",
			      __func__);
		} else {
			/* Disable auto refresh */
			setbits_le32(umctl2_base + DDR4_RFSHCTL3_OFFSET,
				     DDR4_RFSHCTL3_DIS_AUTO_REFRESH);
		}
	}

	/* Disable selfref_en & powerdown_en, nvr disable dfi dram clk */
	clrbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET,
		     DDR4_PWRCTL_EN_DFI_DRAM_CLK_DISABLE |
		     DDR4_PWRCTL_POWERDOWN_EN | DDR4_PWRCTL_SELFREF_EN);

	/* Enable quasi-dynamic programing of the controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Masking dfi init complete */
	clrbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_COMPLETE_EN);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET), DDR4_SWSTAT_SW_DONE_ACK,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
	}

	return ret;
}

static int init_phy(struct ddr_handoff *ddr_handoff_info)
{
	int ret;

	printf("Initializing DDR PHY ...\n");

	if (ddr_handoff_info->cntlr_t == DDRTYPE_DDR4 ||
	    ddr_handoff_info->cntlr_t == DDRTYPE_LPDDR4_0) {
		ret = phy_pre_handoff_config(ddr_handoff_info->cntlr_base,
					     ddr_handoff_info->cntlr_t);
		if (ret)
			return ret;
	}

	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1) {
		ret = phy_pre_handoff_config
			(ddr_handoff_info->cntlr2_base,
			 ddr_handoff_info->cntlr2_t);
		if (ret)
			return ret;
	}

	/* Execute PHY configuration handoff */
	handoff_process(ddr_handoff_info, ddr_handoff_info->phy_handoff_base,
			ddr_handoff_info->phy_handoff_length,
			ddr_handoff_info->phy_base);

	printf("DDR PHY configuration is completed\n");

	return 0;
}

static void phy_init_engine(struct ddr_handoff *handoff)
{
	printf("Load PHY Init Engine ...\n");

	/* Execute PIE production code handoff */
	handoff_process(handoff, handoff->phy_engine_handoff_base,
			handoff->phy_engine_handoff_length, handoff->phy_base);

	printf("End of loading PHY Init Engine\n");
}

int populate_ddr_handoff(struct ddr_handoff *handoff)
{
	phys_addr_t next_section_header;

	/* DDR handoff */
	handoff->mem_reset_base = SOC64_HANDOFF_DDR_MEMRESET_BASE;
	debug("%s: DDR memory reset base = 0x%x\n", __func__,
	      (u32)handoff->mem_reset_base);
	debug("%s: DDR memory reset address = 0x%x\n", __func__,
	      readl(handoff->mem_reset_base));

	/* Beginning of DDR controller handoff */
	handoff->cntlr_handoff_base = SOC64_HANDOFF_DDR_UMCTL2_SECTION;
	debug("%s: cntlr handoff base = 0x%x\n", __func__,
	      (u32)handoff->cntlr_handoff_base);

	/* Get 1st DDR type */
	handoff->cntlr_t = get_ddr_type(handoff->cntlr_handoff_base +
					SOC64_HANDOFF_DDR_UMCTL2_TYPE_OFFSET);
	if (handoff->cntlr_t == DDRTYPE_LPDDR4_1 ||
	    handoff->cntlr_t == DDRTYPE_UNKNOWN) {
		debug("%s: Wrong DDR handoff format, the 1st DDR ", __func__);
		debug("type must be DDR4 or LPDDR4_0\n");
		return -ENOEXEC;
	}

	/* 1st cntlr base physical address */
	handoff->cntlr_base = readl(handoff->cntlr_handoff_base +
				    SOC64_HANDOFF_DDR_UMCTL2_BASE_ADDR_OFFSET);
	debug("%s: cntlr base = 0x%x\n", __func__, (u32)handoff->cntlr_base);

	/* Get the total length of DDR cntlr handoff section */
	handoff->cntlr_total_length = readl(handoff->cntlr_handoff_base +
					    SOC64_HANDOFF_OFFSET_LENGTH);
	debug("%s: Umctl2 total length in byte = 0x%x\n", __func__,
	      (u32)handoff->cntlr_total_length);

	/* Get the length of user setting data in DDR cntlr handoff section */
	handoff->cntlr_handoff_length = socfpga_get_handoff_size((void *)
						handoff->cntlr_handoff_base);
	debug("%s: Umctl2 handoff length in word(32-bit) = 0x%x\n", __func__,
	      (u32)handoff->cntlr_handoff_length);

	/* Wrong format on user setting data */
	if (handoff->cntlr_handoff_length < 0) {
		debug("%s: Wrong format on user setting data\n", __func__);
		return -ENOEXEC;
	}

	/* Get the next handoff section address */
	next_section_header = handoff->cntlr_handoff_base +
				handoff->cntlr_total_length;
	debug("%s: Next handoff section header location = 0x%llx\n", __func__,
	      next_section_header);

	/*
	 * Checking next section handoff is cntlr or PHY, and changing
	 * subsequent implementation accordingly
	 */
	if (readl(next_section_header) == SOC64_HANDOFF_DDR_UMCTL2_MAGIC) {
		/* Get the next cntlr handoff section address */
		handoff->cntlr2_handoff_base = next_section_header;
		debug("%s: umctl2 2nd handoff base = 0x%x\n", __func__,
		      (u32)handoff->cntlr2_handoff_base);

		/* Get 2nd DDR type */
		handoff->cntlr2_t = get_ddr_type(handoff->cntlr2_handoff_base +
					SOC64_HANDOFF_DDR_UMCTL2_TYPE_OFFSET);
		if (handoff->cntlr2_t == DDRTYPE_LPDDR4_0 ||
		    handoff->cntlr2_t == DDRTYPE_UNKNOWN) {
			debug("%s: Wrong DDR handoff format, the 2nd DDR ",
			      __func__);
			debug("type must be LPDDR4_1\n");
			return -ENOEXEC;
		}

		/* 2nd umctl2 base physical address */
		handoff->cntlr2_base =
			readl(handoff->cntlr2_handoff_base +
			      SOC64_HANDOFF_DDR_UMCTL2_BASE_ADDR_OFFSET);
		debug("%s: cntlr2 base = 0x%x\n", __func__,
		      (u32)handoff->cntlr2_base);

		/* Get the total length of 2nd DDR umctl2 handoff section */
		handoff->cntlr2_total_length =
			readl(handoff->cntlr2_handoff_base +
			      SOC64_HANDOFF_OFFSET_LENGTH);
		debug("%s: Umctl2_2nd total length in byte = 0x%x\n", __func__,
		      (u32)handoff->cntlr2_total_length);

		/*
		 * Get the length of user setting data in DDR umctl2 handoff
		 * section
		 */
		handoff->cntlr2_handoff_length =
			socfpga_get_handoff_size((void *)
						 handoff->cntlr2_handoff_base);
		debug("%s: cntlr2 handoff length in word(32-bit) = 0x%x\n",
		      __func__,
		     (u32)handoff->cntlr2_handoff_length);

		/* Wrong format on user setting data */
		if (handoff->cntlr2_handoff_length < 0) {
			debug("%s: Wrong format on umctl2 user setting data\n",
			      __func__);
			return -ENOEXEC;
		}

		/* Get the next handoff section address */
		next_section_header = handoff->cntlr2_handoff_base +
					handoff->cntlr2_total_length;
		debug("%s: Next handoff section header location = 0x%llx\n",
		      __func__, next_section_header);
	}

	/* Checking next section handoff is PHY ? */
	if (readl(next_section_header) == SOC64_HANDOFF_DDR_PHY_MAGIC) {
		/* DDR PHY handoff */
		handoff->phy_handoff_base = next_section_header;
		debug("%s: PHY handoff base = 0x%x\n", __func__,
		      (u32)handoff->phy_handoff_base);

		/* PHY base physical address */
		handoff->phy_base = readl(handoff->phy_handoff_base +
					SOC64_HANDOFF_DDR_PHY_BASE_OFFSET);
		debug("%s: PHY base = 0x%x\n", __func__,
		      (u32)handoff->phy_base);

		/* Get the total length of PHY handoff section */
		handoff->phy_total_length = readl(handoff->phy_handoff_base +
						SOC64_HANDOFF_OFFSET_LENGTH);
		debug("%s: PHY total length in byte = 0x%x\n", __func__,
		      (u32)handoff->phy_total_length);

		/*
		 * Get the length of user setting data in DDR PHY handoff
		 * section
		 */
		handoff->phy_handoff_length = socfpga_get_handoff_size((void *)
						handoff->phy_handoff_base);
		debug("%s: PHY handoff length in word(32-bit) = 0x%x\n",
		      __func__, (u32)handoff->phy_handoff_length);

		/* Wrong format on PHY user setting data */
		if (handoff->phy_handoff_length < 0) {
			debug("%s: Wrong format on PHY user setting data\n",
			      __func__);
			return -ENOEXEC;
		}

		/* Get the next handoff section address */
		next_section_header = handoff->phy_handoff_base +
					handoff->phy_total_length;
		debug("%s: Next handoff section header location = 0x%llx\n",
		      __func__, next_section_header);
	} else {
		debug("%s: Wrong format for DDR handoff, expect PHY",
		      __func__);
		debug(" handoff section after umctl2 handoff section\n");
		return -ENOEXEC;
	}

	/* Checking next section handoff is PHY init Engine ? */
	if (readl(next_section_header) ==
		SOC64_HANDOFF_DDR_PHY_INIT_ENGINE_MAGIC) {
		/* DDR PHY Engine handoff */
		handoff->phy_engine_handoff_base = next_section_header;
		debug("%s: PHY init engine handoff base = 0x%x\n", __func__,
		      (u32)handoff->phy_engine_handoff_base);

		/* Get the total length of PHY init engine handoff section */
		handoff->phy_engine_total_length =
				readl(handoff->phy_engine_handoff_base +
				      SOC64_HANDOFF_OFFSET_LENGTH);
		debug("%s: PHY engine total length in byte = 0x%x\n", __func__,
		      (u32)handoff->phy_engine_total_length);

		/*
		 * Get the length of user setting data in DDR PHY init engine
		 * handoff section
		 */
		handoff->phy_engine_handoff_length =
			socfpga_get_handoff_size((void *)
					handoff->phy_engine_handoff_base);
		debug("%s: PHY engine handoff length in word(32-bit) = 0x%x\n",
		      __func__, (u32)handoff->phy_engine_handoff_length);

		/* Wrong format on PHY init engine setting data */
		if (handoff->phy_engine_handoff_length < 0) {
			debug("%s: Wrong format on PHY init engine ",
			      __func__);
			debug("user setting data\n");
			return -ENOEXEC;
		}
	} else {
		debug("%s: Wrong format for DDR handoff, expect PHY",
		      __func__);
		debug(" init engine handoff section after PHY handoff\n");
		debug(" section\n");
		return -ENOEXEC;
	}

	handoff->train_imem_base = handoff->phy_base +
						DDR_PHY_TRAIN_IMEM_OFFSET;
	debug("%s: PHY train IMEM base = 0x%x\n",
	      __func__, (u32)handoff->train_imem_base);

	handoff->train_dmem_base = handoff->phy_base +
						DDR_PHY_TRAIN_DMEM_OFFSET;
	debug("%s: PHY train DMEM base = 0x%x\n",
	      __func__, (u32)handoff->train_dmem_base);

	handoff->train_imem_length = SOC64_HANDOFF_DDR_TRAIN_IMEM_LENGTH;
	debug("%s: PHY train IMEM length = 0x%x\n",
	      __func__, (u32)handoff->train_imem_length);

	handoff->train_dmem_length = SOC64_HANDOFF_DDR_TRAIN_DMEM_LENGTH;
	debug("%s: PHY train DMEM length = 0x%x\n",
	      __func__, (u32)handoff->train_dmem_length);

	return 0;
}

int enable_ddr_clock(struct udevice *dev)
{
	struct clk *ddr_clk;
	int ret;

	/* Enable clock before init DDR */
	ddr_clk = devm_clk_get(dev, "mem_clk");
	if (!IS_ERR(ddr_clk)) {
		ret = clk_enable(ddr_clk);
		if (ret) {
			printf("%s: Failed to enable DDR clock\n", __func__);
			return ret;
		}
	} else {
		ret = PTR_ERR(ddr_clk);
		debug("%s: Failed to get DDR clock from dts\n", __func__);
		return ret;
	}

	printf("%s: DDR clock is enabled\n", __func__);

	return 0;
}

static int ddr_start_dfi_init(phys_addr_t umctl2_base,
			      enum ddr_type umctl2_type)
{
	int ret;

	debug("%s: Start DFI init\n", __func__);

	/* Enable quasi-dynamic programing of controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Start DFI init sequence */
	setbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_START);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET),
				DDR4_SWSTAT_SW_DONE_ACK, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
	}

	return ret;
}

static int ddr_check_dfi_init_complete(phys_addr_t umctl2_base,
				       enum ddr_type umctl2_type)
{
	int ret;

	/* Polling DFI init complete */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_DFISTAT_OFFSET),
				DDR4_DFI_INIT_COMPLETE, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" DFI init done\n");
		return ret;
	}

	debug("%s: DFI init completed.\n", __func__);

	/* Enable quasi-dynamic programing of controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Stop DFI init sequence */
	clrbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_START);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET),
				DDR4_SWSTAT_SW_DONE_ACK, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
		return ret;
	}

	debug("%s:DDR programming done\n", __func__);

	return ret;
}

static int ddr_trigger_sdram_init(phys_addr_t umctl2_base,
				  enum ddr_type umctl2_type)
{
	int ret;

	/* Enable quasi-dynamic programing of controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Unmasking dfi init complete */
	setbits_le32(umctl2_base + DDR4_DFIMISC_OFFSET,
		     DDR4_DFIMISC_DFI_INIT_COMPLETE_EN);

	/* Software exit from self-refresh */
	clrbits_le32(umctl2_base + DDR4_PWRCTL_OFFSET, DDR4_PWRCTL_SELFREF_SW);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET),
				DDR4_SWSTAT_SW_DONE_ACK, true,
				TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
		return ret;
	}

	debug("%s:DDR programming done\n", __func__);
	return ret;
}

static int ddr_post_handoff_config(phys_addr_t umctl2_base,
				   enum ddr_type umctl2_type)
{
	int ret = 0;
	u32 value;
	u32 start = get_timer(0);

	do {
		if (get_timer(start) > TIMEOUT_200MS) {
			debug("%s: Timeout while waiting for",
			      __func__);
			debug(" DDR enters normal operating mode\n");
			return -ETIMEDOUT;
		}

		udelay(1);
		schedule();

		/* Polling until SDRAM entered normal operating mode */
		value = readl(umctl2_base + DDR4_STAT_OFFSET) &
			      DDR4_STAT_OPERATING_MODE;
	} while (value != OPM_NORMAL);

	printf("DDR entered normal operating mode\n");

	/* Enabling auto refresh */
	clrbits_le32(umctl2_base + DDR4_RFSHCTL3_OFFSET,
		     DDR4_RFSHCTL3_DIS_AUTO_REFRESH);

	/* Checking ECC is enabled? */
	value = readl(umctl2_base + DDR4_ECCCFG0_OFFSET) & DDR4_ECC_MODE;
	if (value) {
		printf("ECC is enabled\n");
		ret = scrubber_ddr_config(umctl2_base, umctl2_type);
		if (ret)
			printf("Failed to enable ECC\n");
	}

	return ret;
}

static int configure_training_firmware(struct ddr_handoff *ddr_handoff_info,
				       const void *train_imem,
				       const void *train_dmem)
{
	int ret = 0;

	printf("Configuring training firmware ...\n");

	/* Reset SDRAM */
	writew(DDR_PHY_PROTECT_MEMRESET,
	       (uintptr_t)(ddr_handoff_info->phy_base +
	       DDR_PHY_MEMRESETL_OFFSET));

	/* Enable access to the PHY configuration registers */
	clrbits_le16(ddr_handoff_info->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	/* Copy train IMEM bin */
	memcpy((void *)ddr_handoff_info->train_imem_base, train_imem,
	       ddr_handoff_info->train_imem_length);

	ret = memcmp((void *)ddr_handoff_info->train_imem_base, train_imem,
		     ddr_handoff_info->train_imem_length);
	if (ret) {
		debug("%s: Failed to copy train IMEM binary\n", __func__);
		/* Isolate the APB access from internal CSRs */
		setbits_le16(ddr_handoff_info->phy_base +
			     DDR_PHY_APBONLY0_OFFSET, DDR_PHY_MICROCONTMUXSEL);
		return ret;
	}

	memcpy((void *)ddr_handoff_info->train_dmem_base, train_dmem,
	       ddr_handoff_info->train_dmem_length);

	ret = memcmp((void *)ddr_handoff_info->train_dmem_base, train_dmem,
		     ddr_handoff_info->train_dmem_length);
	if (ret)
		debug("%s: Failed to copy train DMEM binary\n", __func__);

	/* Isolate the APB access from internal CSRs */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	return ret;
}

static void calibrating_sdram(struct ddr_handoff *ddr_handoff_info)
{
	/* Init mailbox protocol - set 1 to DCTWRITEPROT[0] */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	/* Init mailbox protocol - set 1 to UCTWRITEPROT[0] */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_UCTWRITEPROT_OFFSET,
		     DDR_PHY_UCTWRITEPROT);

	/* Reset and stalling ARC processor */
	setbits_le16(ddr_handoff_info->phy_base + DDR_PHY_MICRORESET_OFFSET,
		     DDR_PHY_MICRORESET_RESET | DDR_PHY_MICRORESET_STALL);

	/* Release ARC processor */
	clrbits_le16(ddr_handoff_info->phy_base + DDR_PHY_MICRORESET_OFFSET,
		     DDR_PHY_MICRORESET_RESET);

	/* Starting PHY firmware execution */
	clrbits_le16(ddr_handoff_info->phy_base + DDR_PHY_MICRORESET_OFFSET,
		     DDR_PHY_MICRORESET_STALL);
}

static int get_mail(struct ddr_handoff *handoff, enum message_mode mode,
		    u32 *message_id)
{
	int ret;

	/* Polling major messages from PMU */
	ret = wait_for_bit_le16((const void *)(handoff->phy_base +
				DDR_PHY_UCTSHADOWREGS_OFFSET),
				DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW,
				false, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for",
		      __func__);
		debug(" major messages from PMU\n");
		return ret;
	}

	*message_id = readw((uintptr_t)(handoff->phy_base +
			    DDR_PHY_UCTWRITEONLYSHADOW_OFFSET));

	if (mode == STREAMING_MESSAGE)
		*message_id |= readw((uintptr_t)((handoff->phy_base +
				     DDR_PHY_UCTDATWRITEONLYSHADOW_OFFSET))) <<
				     SZ_16;

	/* Ack the receipt of the major message */
	clrbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	ret = wait_for_bit_le16((const void *)(handoff->phy_base +
				DDR_PHY_UCTSHADOWREGS_OFFSET),
				DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for",
		      __func__);
		debug(" ack the receipt of the major message completed\n");
		return ret;
	}

	/* Complete protocol */
	setbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	return ret;
}

static int get_mail_streaming(struct ddr_handoff *handoff,
			      enum message_mode mode, u32 *index)
{
	int ret;

	*index = readw((uintptr_t)(handoff->phy_base +
		       DDR_PHY_UCTWRITEONLYSHADOW_OFFSET));

	if (mode == STREAMING_MESSAGE)
		*index |= readw((uintptr_t)((handoff->phy_base +
				DDR_PHY_UCTDATWRITEONLYSHADOW_OFFSET))) <<
				SZ_16;

	/* Ack the receipt of the major message */
	clrbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	ret = wait_for_bit_le16((const void *)(handoff->phy_base +
				DDR_PHY_UCTSHADOWREGS_OFFSET),
				DDR_PHY_UCTSHADOWREGS_UCTWRITEPROTESHADOW,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for",
		      __func__);
		debug(" ack the receipt of the major message completed\n");
		return ret;
	}

	/* Complete protocol */
	setbits_le16(handoff->phy_base + DDR_PHY_DCTWRITEPROT_OFFSET,
		     DDR_PHY_DCTWRITEPROT);

	return 0;
}

static int decode_streaming_message(struct ddr_handoff *ddr_handoff_info,
				    u32 *streaming_index)
{
	int i = 0, ret;
	u32 temp;

	temp = *streaming_index;

	while (i < GET_LOWHW_DATA(temp)) {
		ret = get_mail(ddr_handoff_info, STREAMING_MESSAGE,
			       streaming_index);
		if (ret)
			return ret;

		printf("args[%d]: 0x%x ", i, *streaming_index);
		i++;
	}

	return 0;
}

static int poll_for_training_complete(struct ddr_handoff *ddr_handoff_info)
{
	int ret;
	u32 message_id = 0;
	u32 streaming_index = 0;

	do {
		ret = get_mail(ddr_handoff_info, MAJOR_MESSAGE, &message_id);
		if (ret)
			return ret;

		 printf("Major message id = 0%x\n", message_id);

		if (message_id == FW_STREAMING_MSG_ID) {
			ret = get_mail_streaming(ddr_handoff_info,
						 STREAMING_MESSAGE,
						 &streaming_index);
			if (ret)
				return ret;

			printf("streaming index 0%x : ", streaming_index);

			decode_streaming_message(ddr_handoff_info,
						 &streaming_index);

			printf("\n");
		}
	} while ((message_id != FW_TRAINING_COMPLETED_STAT) &&
	       (message_id != FW_TRAINING_FAILED_STAT));

	if (message_id == FW_TRAINING_COMPLETED_STAT) {
		printf("DDR firmware training completed\n");
	} else if (message_id == FW_TRAINING_FAILED_STAT) {
		printf("DDR firmware training failed\n");
		hang();
	}

	return 0;
}

static void enable_phy_clk_for_csr_access(struct ddr_handoff *handoff,
					  bool enable)
{
	if (enable) {
		/* Enable PHY clk */
		setbits_le16((uintptr_t)(handoff->phy_base +
			     DDR_PHY_UCCLKHCLKENABLES_OFFSET),
			     DDR_PHY_UCCLKHCLKENABLES_UCCLKEN |
			     DDR_PHY_UCCLKHCLKENABLES_HCLKEN);
	} else {
		/* Disable PHY clk */
		clrbits_le16((uintptr_t)(handoff->phy_base +
			     DDR_PHY_UCCLKHCLKENABLES_OFFSET),
			     DDR_PHY_UCCLKHCLKENABLES_UCCLKEN |
			     DDR_PHY_UCCLKHCLKENABLES_HCLKEN);
	}
}

/* helper function for updating train result to umctl2 RANKCTL register */
static void set_cal_res_to_rankctrl(u32 reg_addr, u16 update_value,
				    u32 mask, u32 msb_mask, u32 shift)
{
	u32 reg, value;

	reg = readl((uintptr_t)reg_addr);

	debug("max value divided by 2 is 0x%x\n", update_value);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("update with train result\n");

	value = (reg & mask) >> shift;

	value += update_value + 3;

	/* reg value greater than 0xF, set one to diff_rank_wr_gap_msb */
	if (value > 0xF)
		setbits_le32((u32 *)(uintptr_t)reg_addr, msb_mask);
	else
		clrbits_le32((u32 *)(uintptr_t)reg_addr, msb_mask);

	reg = readl((uintptr_t)reg_addr);

	value = (value << shift) & mask;

	/* update register */
	writel((reg & (~mask)) | value, (uintptr_t)reg_addr);

	reg = readl((uintptr_t)reg_addr);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("update with train result\n");
}

/* helper function for updating train result to register */
static void set_cal_res_to_reg(u32 reg_addr, u16 update_value, u32 mask,
			       u32 shift)
{
	u32 reg, value;

	reg = readl((uintptr_t)reg_addr);

	debug("max value divided by 2 is 0x%x\n", update_value);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("update with train result\n");

	value = (reg & mask) >> shift;

	value = ((value + update_value + 3) << shift) & mask;

	/* update register */
	writel((reg & (~mask)) | value, (uintptr_t)reg_addr);

	reg = readl((uintptr_t)reg_addr);
	debug("umclt2 register 0x%x value is 0%x before ", reg_addr, reg);
	debug("update with train result\n");
}

static u16 get_max_txdqsdlytg0_ux_p0(struct ddr_handoff *handoff, u32 reg,
				     u8 numdbyte, u16 upd_val)
{
	u32 b_addr;
	u16 val;
	u8 byte;

	/* Getting max value from DBYTEx TxDqsDlyTg0_ux_p0 */
	for (byte = 0; byte < numdbyte; byte++) {
		b_addr = byte << 13;

		/* TxDqsDlyTg0[9:6] is the coarse delay */
		val = (readw((uintptr_t)(handoff->phy_base +
			     reg + b_addr)) &
			     DDR_PHY_TXDQDLYTG0_COARSE_DELAY) >>
			     DDR_PHY_TXDQDLYTG0_COARSE_DELAY_SHIFT;

		upd_val = max(val, upd_val);
	}

	return upd_val;
}

static int set_cal_res_to_umctl2(struct ddr_handoff *handoff,
				 phys_addr_t umctl2_base,
				 enum ddr_type umctl2_type)
{
	int ret;
	u8 numdbyte = 0x8;
	u16 upd_val, val;
	u32 dramtmg2_reg_addr, rankctl_reg_addr, reg_addr;

	/* Enable quasi-dynamic programing of the controller registers */
	clrbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	ret = enable_quasi_dynamic_reg_grp3(umctl2_base, umctl2_type);
	if (ret)
		return ret;

	/* Enable access to the PHY configuration registers */
	clrbits_le16(handoff->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	if (umctl2_type == DDRTYPE_DDR4) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_WW_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_WW_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_0) {
		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_WW_1_0_OFFSET)));

		upd_val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
					 DMEM_MB_CDD_CHA_WW_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_1) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_WW_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_CHB_WW_0_1_OFFSET)));
	}

	upd_val = max(val, upd_val);
	debug("max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	debug("Update train value to umctl2 RANKCTL.diff_rank_wr_gap\n");
	rankctl_reg_addr = umctl2_base + DDR4_RANKCTL_OFFSET;
	/* Update train value to umctl2 RANKCTL.diff_rank_wr_gap */
	set_cal_res_to_rankctrl(rankctl_reg_addr, upd_val,
				DDR4_RANKCTL_DIFF_RANK_WR_GAP,
				DDR4_RANKCTL_DIFF_RANK_WR_GAP_MSB,
				DDR4_RANKCTL_DIFF_RANK_WR_GAP_SHIFT);

	debug("Update train value to umctl2 DRAMTMG2.W2RD\n");
	dramtmg2_reg_addr = umctl2_base + DDR4_DRAMTMG2_OFFSET;
	/* Update train value to umctl2 dramtmg2.wr2rd */
	set_cal_res_to_reg(dramtmg2_reg_addr, upd_val, DDR4_DRAMTMG2_WR2RD, 0);

	if (umctl2_type == DDRTYPE_DDR4) {
		debug("Update train value to umctl2 DRAMTMG9.W2RD_S\n");
		reg_addr = umctl2_base + DDR4_DRAMTMG9_OFFSET;
		/* Update train value to umctl2 dramtmg9.wr2rd_s */
		set_cal_res_to_reg(reg_addr, upd_val, DDR4_DRAMTMG9_W2RD_S, 0);
	}

	if (umctl2_type == DDRTYPE_DDR4) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_RR_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_RR_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_0) {
		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_RR_1_0_OFFSET)));

		upd_val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
					 DMEM_MB_CDD_CHA_RR_0_1_OFFSET)));
	} else if (umctl2_type == DDRTYPE_LPDDR4_1) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_RR_1_0_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_CHB_RR_0_1_OFFSET)));
	}

	upd_val = max(val, upd_val);
	debug("max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	debug("Update train value to umctl2 RANKCTL.diff_rank_rd_gap\n");
	/* Update train value to umctl2 RANKCTL.diff_rank_rd_gap */
	set_cal_res_to_rankctrl(rankctl_reg_addr, upd_val,
				DDR4_RANKCTL_DIFF_RANK_RD_GAP,
				DDR4_RANKCTL_DIFF_RANK_RD_GAP_MSB,
				DDR4_RANKCTL_DIFF_RANK_RD_GAP_SHIFT);

	if (umctl2_type == DDRTYPE_DDR4) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_RW_1_1_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_RW_1_0_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_RW_0_1_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_RW_0_0_OFFSET)));

		upd_val = max(val, upd_val);
	} else if (umctl2_type == DDRTYPE_LPDDR4_0) {
		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_RW_1_1_OFFSET)));

		upd_val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
					 DMEM_MB_CDD_CHA_RW_1_0_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHA_RW_0_1_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHA_RW_0_0_OFFSET)));

		upd_val = max(val, upd_val);
	} else if (umctl2_type == DDRTYPE_LPDDR4_1) {
		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_RW_1_1_OFFSET)));

		upd_val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
					DMEM_MB_CDD_CHB_RW_1_0_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_HIGHB_DATA(readw((uintptr_t)(handoff->phy_base +
				     DMEM_MB_CDD_CHB_RW_0_1_OFFSET)));

		upd_val = max(val, upd_val);

		val = GET_LOWB_DATA(readw((uintptr_t)(handoff->phy_base +
				    DMEM_MB_CDD_CHB_RW_0_0_OFFSET)));

		upd_val = max(val, upd_val);
	}

	debug("max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	debug("Update train value to umctl2 dramtmg2.rd2wr\n");
	/* Update train value to umctl2 dramtmg2.rd2wr */
	set_cal_res_to_reg(dramtmg2_reg_addr, upd_val, DDR4_DRAMTMG2_RD2WR,
			   DDR4_DRAMTMG2_RD2WR_SHIFT);

	/* Checking ECC is enabled?, lpddr4 using inline ECC */
	val = readl(umctl2_base + DDR4_ECCCFG0_OFFSET) & DDR4_ECC_MODE;
	if (val && umctl2_type == DDRTYPE_DDR4)
		numdbyte = 0x9;

	upd_val = 0;

	/* Getting max value from DBYTEx TxDqsDlyTg0_u0_p0 */
	upd_val = get_max_txdqsdlytg0_ux_p0(handoff,
					    DDR_PHY_DBYTE0_TXDQDLYTG0_U0_P0,
					    numdbyte, upd_val);

	/* Getting max value from DBYTEx TxDqsDlyTg0_u1_p0 */
	upd_val = get_max_txdqsdlytg0_ux_p0(handoff,
					    DDR_PHY_DBYTE0_TXDQDLYTG0_U1_P0,
					    numdbyte, upd_val);

	debug("TxDqsDlyTg0 max value is 0x%x\n", upd_val);

	/* Divided by two is required when running in freq ratio 1:2 */
	if (!(readl(umctl2_base + DDR4_MSTR_OFFSET) & DDR4_FREQ_RATIO))
		upd_val = DIV_ROUND_CLOSEST(upd_val, 2);

	reg_addr = umctl2_base + DDR4_DFITMG1_OFFSET;
	/* Update train value to umctl2 dfitmg1.dfi_wrdata_delay */
	set_cal_res_to_reg(reg_addr, upd_val, DDR4_DFITMG1_DFI_T_WRDATA_DELAY,
			   DDR4_DFITMG1_DFI_T_WRDATA_SHIFT);

	/* Complete quasi-dynamic register programming */
	setbits_le32(umctl2_base + DDR4_SWCTL_OFFSET, DDR4_SWCTL_SW_DONE);

	/* Polling programming done */
	ret = wait_for_bit_le32((const void *)(umctl2_base +
				DDR4_SWSTAT_OFFSET), DDR4_SWSTAT_SW_DONE_ACK,
				true, TIMEOUT_200MS, false);
	if (ret) {
		debug("%s: Timeout while waiting for", __func__);
		debug(" programming done\n");
	}

	/* Isolate the APB access from internal CSRs */
	setbits_le16(handoff->phy_base + DDR_PHY_APBONLY0_OFFSET,
		     DDR_PHY_MICROCONTMUXSEL);

	return ret;
}

static int update_training_result(struct ddr_handoff *ddr_handoff_info)
{
	int ret = 0;

	/* Updating training result to first DDR controller */
	if (ddr_handoff_info->cntlr_t == DDRTYPE_DDR4 ||
	    ddr_handoff_info->cntlr_t == DDRTYPE_LPDDR4_0) {
		ret = set_cal_res_to_umctl2(ddr_handoff_info,
					    ddr_handoff_info->cntlr_base,
					    ddr_handoff_info->cntlr_t);
		if (ret) {
			debug("%s: Failed to update train result to ",
			      __func__);
			debug("first DDR controller\n");
			return ret;
		}
	}

	/* Updating training result to 2nd DDR controller */
	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1) {
		ret = set_cal_res_to_umctl2(ddr_handoff_info,
					    ddr_handoff_info->cntlr2_base,
					    ddr_handoff_info->cntlr2_t);
		if (ret) {
			debug("%s: Failed to update train result to ",
			      __func__);
			debug("2nd DDR controller\n");
		}
	}

	return ret;
}

static int start_ddr_calibration(struct ddr_handoff *ddr_handoff_info)
{
	int ret;

	/* Implement 1D training firmware */
	ret = configure_training_firmware(ddr_handoff_info,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_IMEM_1D_SECTION,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_DMEM_1D_SECTION);
	if (ret) {
		debug("%s: Failed to configure 1D training firmware\n",
		      __func__);
		return ret;
	}

	calibrating_sdram(ddr_handoff_info);

	ret = poll_for_training_complete(ddr_handoff_info);
	if (ret) {
		debug("%s: Failed to get FW training completed\n",
		      __func__);
		return ret;
	}

	/* Updating training result to DDR controller */
	ret = update_training_result(ddr_handoff_info);
	if (ret)
		return ret;

	/* Implement 2D training firmware */
	ret = configure_training_firmware(ddr_handoff_info,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_IMEM_2D_SECTION,
		(const void *)SOC64_HANDOFF_DDR_TRAIN_DMEM_2D_SECTION);
	if (ret) {
		debug("%s: Failed to update train result to ", __func__);
		debug("DDR controller\n");
		return ret;
	}

	calibrating_sdram(ddr_handoff_info);

	ret = poll_for_training_complete(ddr_handoff_info);
	if (ret)
		debug("%s: Failed to get FW training completed\n",
		      __func__);

	return ret;
}

static int init_controller(struct ddr_handoff *ddr_handoff_info,
			   u32 *user_backup, u32 *user_backup_2nd)
{
	int ret = 0;

	if (ddr_handoff_info->cntlr_t == DDRTYPE_DDR4  ||
	    ddr_handoff_info->cntlr_t == DDRTYPE_LPDDR4_0) {
		/* Initialize 1st DDR controller */
		ret = init_umctl2(ddr_handoff_info->cntlr_handoff_base,
				  ddr_handoff_info->cntlr_base,
				  ddr_handoff_info->cntlr_t,
				  ddr_handoff_info->cntlr_handoff_length,
				  user_backup);
		if (ret) {
			debug("%s: Failed to inilialize first controller\n",
			      __func__);
			return ret;
		}
	}

	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1) {
		/* Initialize 2nd DDR controller */
		ret = init_umctl2(ddr_handoff_info->cntlr2_handoff_base,
				  ddr_handoff_info->cntlr2_base,
				  ddr_handoff_info->cntlr2_t,
				  ddr_handoff_info->cntlr2_handoff_length,
				  user_backup_2nd);
		if (ret)
			debug("%s: Failed to inilialize 2nd controller\n",
			      __func__);
	}

	return ret;
}

static int dfi_init(struct ddr_handoff *ddr_handoff_info)
{
	int ret;

	ret = ddr_start_dfi_init(ddr_handoff_info->cntlr_base,
				 ddr_handoff_info->cntlr_t);
	if (ret)
		return ret;

	if (ddr_handoff_info->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_start_dfi_init(ddr_handoff_info->cntlr2_base,
					 ddr_handoff_info->cntlr2_t);

	return ret;
}

static int check_dfi_init(struct ddr_handoff *handoff)
{
	int ret;

	ret = ddr_check_dfi_init_complete(handoff->cntlr_base,
					  handoff->cntlr_t);
	if (ret)
		return ret;

	if (handoff->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_check_dfi_init_complete(handoff->cntlr2_base,
						  handoff->cntlr2_t);

	return ret;
}

static int trigger_sdram_init(struct ddr_handoff *handoff)
{
	int ret;

	ret = ddr_trigger_sdram_init(handoff->cntlr_base,
				     handoff->cntlr_t);
	if (ret)
		return ret;

	if (handoff->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_trigger_sdram_init(handoff->cntlr2_base,
					     handoff->cntlr2_t);

	return ret;
}

static int ddr_post_config(struct ddr_handoff *handoff)
{
	int ret;

	ret = ddr_post_handoff_config(handoff->cntlr_base,
				      handoff->cntlr_t);
	if (ret)
		return ret;

	if (handoff->cntlr2_t == DDRTYPE_LPDDR4_1)
		ret = ddr_post_handoff_config(handoff->cntlr2_base,
					      handoff->cntlr2_t);

	return ret;
}

static bool is_ddr_retention_enabled(u32 boot_scratch_cold0_reg)
{
	return boot_scratch_cold0_reg &
	       ALT_SYSMGR_SCRATCH_REG_0_DDR_RETENTION_MASK;
}

static bool is_ddr_bitstream_sha_matching(u32 boot_scratch_cold0_reg)
{
	return boot_scratch_cold0_reg & ALT_SYSMGR_SCRATCH_REG_0_DDR_SHA_MASK;
}

static enum reset_type get_reset_type(u32 boot_scratch_cold0_reg)
{
	return (boot_scratch_cold0_reg &
		ALT_SYSMGR_SCRATCH_REG_0_DDR_RESET_TYPE_MASK) >>
		ALT_SYSMGR_SCRATCH_REG_0_DDR_RESET_TYPE_SHIFT;
}

void reset_type_debug_print(u32 boot_scratch_cold0_reg)
{
	switch (get_reset_type(boot_scratch_cold0_reg)) {
	case POR_RESET:
		debug("%s: POR is triggered\n", __func__);
		break;
	case WARM_RESET:
		debug("%s: Warm reset is triggered\n", __func__);
		break;
	case COLD_RESET:
		debug("%s: Cold reset is triggered\n", __func__);
		break;
	default:
		debug("%s: Invalid reset type\n", __func__);
	}
}

bool is_ddr_init(void)
{
	u32 reg = readl(socfpga_get_sysmgr_addr() +
			SYSMGR_SOC64_BOOT_SCRATCH_COLD0);

	reset_type_debug_print(reg);

	if (get_reset_type(reg) == POR_RESET) {
		debug("%s: DDR init is required\n", __func__);
		return true;
	}

	if (get_reset_type(reg) == WARM_RESET) {
		debug("%s: DDR init is skipped\n", __func__);
		return false;
	}

	if (get_reset_type(reg) == COLD_RESET) {
		if (is_ddr_retention_enabled(reg) &&
		    is_ddr_bitstream_sha_matching(reg)) {
			debug("%s: DDR retention bit is set\n", __func__);
			debug("%s: Matching in DDR bistream\n", __func__);
			debug("%s: DDR init is skipped\n", __func__);
			return false;
		}
	}

	debug("%s: DDR init is required\n", __func__);
	return true;
}

int sdram_mmr_init_full(struct udevice *dev)
{
	u32 user_backup[2], user_backup_2nd[2];
	int ret;
	struct bd_info bd;
	struct ddr_handoff ddr_handoff_info;
	struct altera_sdram_priv *priv = dev_get_priv(dev);

	printf("Checking SDRAM configuration in progress ...\n");
	ret = populate_ddr_handoff(&ddr_handoff_info);
		if (ret) {
			debug("%s: Failed to populate DDR handoff\n",
			      __func__);
			return ret;
		}

	/* Set the MPFE NoC mux to correct DDR controller type */
	use_ddr4(ddr_handoff_info.cntlr_t);

	if (is_ddr_init()) {
		printf("SDRAM init in progress ...\n");

		/*
		 * Polling reset complete, must be high to ensure DDR subsystem
		 * in complete reset state before init DDR clock and DDR
		 * controller
		 */
		ret = wait_for_bit_le32((const void *)((uintptr_t)(readl
					(ddr_handoff_info.mem_reset_base) +
					MEM_RST_MGR_STATUS)),
					MEM_RST_MGR_STATUS_RESET_COMPLETE,
					true, TIMEOUT_200MS, false);
		if (ret) {
			debug("%s: Timeout while waiting for", __func__);
			debug(" reset complete done\n");
			return ret;
		}

		ret = enable_ddr_clock(dev);
		if (ret)
			return ret;

		ret = init_controller(&ddr_handoff_info, user_backup,
				      user_backup_2nd);
		if (ret) {
			debug("%s: Failed to inilialize DDR controller\n",
			      __func__);
			return ret;
		}

		/* Release the controller from reset */
		setbits_le32((uintptr_t)
			     (readl(ddr_handoff_info.mem_reset_base) +
			     MEM_RST_MGR_STATUS), MEM_RST_MGR_STATUS_AXI_RST |
			     MEM_RST_MGR_STATUS_CONTROLLER_RST |
			     MEM_RST_MGR_STATUS_RESET_COMPLETE);

		printf("DDR controller configuration is completed\n");

		/* Initialize DDR PHY */
		ret = init_phy(&ddr_handoff_info);
		if (ret) {
			debug("%s: Failed to inilialize DDR PHY\n", __func__);
			return ret;
		}

		enable_phy_clk_for_csr_access(&ddr_handoff_info, true);

		ret = start_ddr_calibration(&ddr_handoff_info);
		if (ret) {
			debug("%s: Failed to calibrate DDR\n", __func__);
			return ret;
		}

		enable_phy_clk_for_csr_access(&ddr_handoff_info, false);

		/* Reset ARC processor when no using for security purpose */
		setbits_le16(ddr_handoff_info.phy_base +
			     DDR_PHY_MICRORESET_OFFSET,
			     DDR_PHY_MICRORESET_RESET);

		/* DDR freq set to support DDR4-3200 */
		phy_init_engine(&ddr_handoff_info);

		ret = dfi_init(&ddr_handoff_info);
		if (ret)
			return ret;

		ret = check_dfi_init(&ddr_handoff_info);
		if (ret)
			return ret;

		ret = trigger_sdram_init(&ddr_handoff_info);
		if (ret)
			return ret;

		ret = ddr_post_config(&ddr_handoff_info);
		if (ret)
			return ret;

		/* Restore user settings */
		writel(user_backup[0], ddr_handoff_info.cntlr_base +
		       DDR4_PWRCTL_OFFSET);

		if (ddr_handoff_info.cntlr2_t == DDRTYPE_LPDDR4_0)
			setbits_le32(ddr_handoff_info.cntlr_base +
				     DDR4_INIT0_OFFSET, user_backup[1]);

		if (ddr_handoff_info.cntlr2_t == DDRTYPE_LPDDR4_1) {
			/* Restore user settings */
			writel(user_backup_2nd[0],
			       ddr_handoff_info.cntlr2_base +
			       DDR4_PWRCTL_OFFSET);

			setbits_le32(ddr_handoff_info.cntlr2_base +
				     DDR4_INIT0_OFFSET, user_backup_2nd[1]);
		}

		/* Enable input traffic per port */
		setbits_le32(ddr_handoff_info.cntlr_base + DDR4_PCTRL0_OFFSET,
			     DDR4_PCTRL0_PORT_EN);

		if (ddr_handoff_info.cntlr2_t == DDRTYPE_LPDDR4_1) {
			/* Enable input traffic per port */
			setbits_le32(ddr_handoff_info.cntlr2_base +
				     DDR4_PCTRL0_OFFSET, DDR4_PCTRL0_PORT_EN);
		}

		printf("DDR init success\n");
	}

	/* Get bank configuration from devicetree */
	ret = fdtdec_decode_ram_size(gd->fdt_blob, NULL, 0, NULL,
				     (phys_size_t *)&gd->ram_size, &bd);
	if (ret) {
		debug("%s: Failed to decode memory node\n",  __func__);
		return -1;
	}

	printf("DDR: %lld MiB\n", gd->ram_size >> 20);

	priv->info.base = bd.bi_dram[0].start;
	priv->info.size = gd->ram_size;

	sdram_size_check(&bd);

	sdram_set_firewall(&bd);

	return 0;
}
