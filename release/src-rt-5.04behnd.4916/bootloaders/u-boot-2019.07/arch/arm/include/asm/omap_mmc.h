/*
 * (C) Copyright 2008
 * Texas Instruments, <www.ti.com>
 * Syed Mohammed Khasim <khasim@ti.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation's version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#ifndef OMAP_MMC_H_
#define OMAP_MMC_H_

#include <mmc.h>

struct hsmmc {
#ifndef CONFIG_OMAP34XX
	unsigned int hl_rev;
	unsigned int hl_hwinfo;
	unsigned int hl_sysconfig;
	unsigned char res0[0xf4];
#endif
	unsigned char res1[0x10];
	unsigned int sysconfig;		/* 0x10 */
	unsigned int sysstatus;		/* 0x14 */
	unsigned char res2[0x14];
	unsigned int con;		/* 0x2C */
	unsigned int pwcnt;		/* 0x30 */
	unsigned int dll;		/* 0x34 */
	unsigned char res3[0xcc];
	unsigned int blk;		/* 0x104 */
	unsigned int arg;		/* 0x108 */
	unsigned int cmd;		/* 0x10C */
	unsigned int rsp10;		/* 0x110 */
	unsigned int rsp32;		/* 0x114 */
	unsigned int rsp54;		/* 0x118 */
	unsigned int rsp76;		/* 0x11C */
	unsigned int data;		/* 0x120 */
	unsigned int pstate;		/* 0x124 */
	unsigned int hctl;		/* 0x128 */
	unsigned int sysctl;		/* 0x12C */
	unsigned int stat;		/* 0x130 */
	unsigned int ie;		/* 0x134 */
	unsigned char res4[0x4];
	unsigned int ac12;		/* 0x13C */
	unsigned int capa;		/* 0x140 */
	unsigned int capa2;		/* 0x144 */
	unsigned char res5[0xc];
	unsigned int admaes;		/* 0x154 */
	unsigned int admasal;		/* 0x158 */
};

struct omap_hsmmc_plat {
	struct mmc_config cfg;
	struct hsmmc *base_addr;
	struct mmc *mmc;
	u32 controller_flags;
	const char *hw_rev;
};

/*
 * OMAP HS MMC Bit definitions
 */
#define MADMA_EN			(0x1 << 0)
#define MMC_SOFTRESET			(0x1 << 1)
#define RESETDONE			(0x1 << 0)
#define NOOPENDRAIN			(0x0 << 0)
#define OPENDRAIN			(0x1 << 0)
#define OD				(0x1 << 0)
#define INIT_NOINIT			(0x0 << 1)
#define INIT_INITSTREAM			(0x1 << 1)
#define HR_NOHOSTRESP			(0x0 << 2)
#define STR_BLOCK			(0x0 << 3)
#define MODE_FUNC			(0x0 << 4)
#define DW8_1_4BITMODE			(0x0 << 5)
#define MIT_CTO				(0x0 << 6)
#define CDP_ACTIVEHIGH			(0x0 << 7)
#define WPP_ACTIVEHIGH			(0x0 << 8)
#define RESERVED_MASK			(0x3 << 9)
#define CTPL_MMC_SD			(0x0 << 11)
#define DDR				(0x1 << 19)
#define DMA_MASTER			(0x1 << 20)
#define BLEN_512BYTESLEN		(0x200 << 0)
#define NBLK_STPCNT			(0x0 << 16)
#define DE_ENABLE			(0x1 << 0)
#define BCE_ENABLE			(0x1 << 1)
#define ACEN_ENABLE			(0x1 << 2)
#define DDIR_OFFSET			(4)
#define DDIR_MASK			(0x1 << 4)
#define DDIR_WRITE			(0x0 << 4)
#define DDIR_READ			(0x1 << 4)
#define MSBS_SGLEBLK			(0x0 << 5)
#define MSBS_MULTIBLK			(0x1 << 5)
#define RSP_TYPE_OFFSET			(16)
#define RSP_TYPE_MASK			(0x3 << 16)
#define RSP_TYPE_NORSP			(0x0 << 16)
#define RSP_TYPE_LGHT136		(0x1 << 16)
#define RSP_TYPE_LGHT48			(0x2 << 16)
#define RSP_TYPE_LGHT48B		(0x3 << 16)
#define CCCE_NOCHECK			(0x0 << 19)
#define CCCE_CHECK			(0x1 << 19)
#define CICE_NOCHECK			(0x0 << 20)
#define CICE_CHECK			(0x1 << 20)
#define DP_OFFSET			(21)
#define DP_MASK				(0x1 << 21)
#define DP_NO_DATA			(0x0 << 21)
#define DP_DATA				(0x1 << 21)
#define CMD_TYPE_NORMAL			(0x0 << 22)
#define INDEX_OFFSET			(24)
#define INDEX_MASK			(0x3f << 24)
#define INDEX(i)			(i << 24)
#define DATI_MASK			(0x1 << 1)
#define CMDI_MASK			(0x1 << 0)
#define DTW_1_BITMODE			(0x0 << 1)
#define DTW_4_BITMODE			(0x1 << 1)
#define DTW_8_BITMODE                   (0x1 << 5) /* CON[DW8]*/
#define SDBP_PWROFF			(0x0 << 8)
#define SDBP_PWRON			(0x1 << 8)
#define SDVS_MASK			(0x7 << 9)
#define SDVS_1V8			(0x5 << 9)
#define SDVS_3V0			(0x6 << 9)
#define SDVS_3V3			(0x7 << 9)
#define DMA_SELECT			(0x2 << 3)
#define ICE_MASK			(0x1 << 0)
#define ICE_STOP			(0x0 << 0)
#define ICS_MASK			(0x1 << 1)
#define ICS_NOTREADY			(0x0 << 1)
#define ICE_OSCILLATE			(0x1 << 0)
#define CEN_MASK			(0x1 << 2)
#define CEN_ENABLE			(0x1 << 2)
#define CLKD_OFFSET			(6)
#define CLKD_MASK			(0x3FF << 6)
#define DTO_MASK			(0xF << 16)
#define DTO_15THDTO			(0xE << 16)
#define SOFTRESETALL			(0x1 << 24)
#define CC_MASK				(0x1 << 0)
#define TC_MASK				(0x1 << 1)
#define BWR_MASK			(0x1 << 4)
#define BRR_MASK			(0x1 << 5)
#define ERRI_MASK			(0x1 << 15)
#define IE_CC				(0x01 << 0)
#define IE_TC				(0x01 << 1)
#define IE_BWR				(0x01 << 4)
#define IE_BRR				(0x01 << 5)
#define IE_CTO				(0x01 << 16)
#define IE_CCRC				(0x01 << 17)
#define IE_CEB				(0x01 << 18)
#define IE_CIE				(0x01 << 19)
#define IE_DTO				(0x01 << 20)
#define IE_DCRC				(0x01 << 21)
#define IE_DEB				(0x01 << 22)
#define IE_ADMAE			(0x01 << 25)
#define IE_CERR				(0x01 << 28)
#define IE_BADA				(0x01 << 29)

#define VS33_3V3SUP			BIT(24)
#define VS30_3V0SUP			BIT(25)
#define VS18_1V8SUP			BIT(26)

#define AC12_ET				BIT(22)
#define AC12_V1V8_SIGEN			BIT(19)
#define AC12_SCLK_SEL			BIT(23)
#define AC12_UHSMC_MASK			(7 << 16)
#define AC12_UHSMC_DDR50		(4 << 16)
#define AC12_UHSMC_SDR104		(3 << 16)
#define AC12_UHSMC_SDR50		(2 << 16)
#define AC12_UHSMC_SDR25		(1 << 16)
#define AC12_UHSMC_SDR12		(0 << 16)
#define AC12_UHSMC_RES			(0x7 << 16)

/* Driver definitions */
#define MMCSD_SECTOR_SIZE		512
#define MMC_CARD			0
#define SD_CARD				1
#define BYTE_MODE			0
#define SECTOR_MODE			1
#define CLK_INITSEQ			0
#define CLK_400KHZ			1
#define CLK_MISC			2

#define CLKD_MAX			0x3FF	/* max clock divisor: 1023 */

#define RSP_TYPE_NONE	(RSP_TYPE_NORSP   | CCCE_NOCHECK | CICE_NOCHECK)
#define MMC_CMD0	(INDEX(0)  | RSP_TYPE_NONE | DP_NO_DATA | DDIR_WRITE)

/* Clock Configurations and Macros */
#ifdef CONFIG_OMAP54XX
#define MMC_CLOCK_REFERENCE	192 /* MHz */
#else
#define MMC_CLOCK_REFERENCE	96 /* MHz */
#endif

/* DLL */
#define DLL_SWT			BIT(20)
#define DLL_FORCE_SR_C_SHIFT	13
#define DLL_FORCE_SR_C_MASK	0x7f
#define DLL_FORCE_VALUE		BIT(12)
#define DLL_CALIB		BIT(1)

#define MAX_PHASE_DELAY		0x7c

/* CAPA2 */
#define CAPA2_TSDR50		BIT(13)

#define mmc_reg_out(addr, mask, val)\
	writel((readl(addr) & (~(mask))) | ((val) & (mask)), (addr))

#define INT_EN_MASK (IE_BADA | IE_CERR | IE_DEB | IE_DCRC |\
		IE_DTO | IE_CIE | IE_CEB | IE_CCRC | IE_ADMAE | IE_CTO |\
		IE_BRR | IE_BWR | IE_TC | IE_CC)

#define CON_CLKEXTFREE		BIT(16)
#define CON_PADEN		BIT(15)
#define PSTATE_CLEV		BIT(24)
#define PSTATE_DLEV		(0xF << 20)
#define PSTATE_DLEV_DAT0	(0x1 << 20)

int omap_mmc_init(int dev_index, uint host_caps_mask, uint f_max, int cd_gpio,
		int wp_gpio);

void vmmc_pbias_config(uint voltage);
void board_mmc_poweron_ldo(uint voltage);
#endif /* OMAP_MMC_H_ */
