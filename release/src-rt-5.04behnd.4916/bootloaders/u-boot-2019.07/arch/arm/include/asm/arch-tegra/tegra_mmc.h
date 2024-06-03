/* SPDX-License-Identifier: GPL-2.0+ */
/*
 * (C) Copyright 2009 SAMSUNG Electronics
 * Minkyu Kang <mk7.kang@samsung.com>
 * Portions Copyright (C) 2011-2012 NVIDIA Corporation
 */

#ifndef __TEGRA_MMC_H_
#define __TEGRA_MMC_H_

#include <common.h>
#include <clk.h>
#include <reset.h>
#include <fdtdec.h>
#include <asm/gpio.h>

/* for mmc_config definition */
#include <mmc.h>

#ifndef __ASSEMBLY__
struct tegra_mmc {
	unsigned int	sysad;		/* _SYSTEM_ADDRESS_0 */
	unsigned short	blksize;	/* _BLOCK_SIZE_BLOCK_COUNT_0 15:00 */
	unsigned short	blkcnt;		/* _BLOCK_SIZE_BLOCK_COUNT_0 31:16 */
	unsigned int	argument;	/* _ARGUMENT_0 */
	unsigned short	trnmod;		/* _CMD_XFER_MODE_0 15:00 xfer mode */
	unsigned short	cmdreg;		/* _CMD_XFER_MODE_0 31:16 cmd reg */
	unsigned int	rspreg0;	/* _RESPONSE_R0_R1_0 CMD RESP 31:00 */
	unsigned int	rspreg1;	/* _RESPONSE_R2_R3_0 CMD RESP 63:32 */
	unsigned int	rspreg2;	/* _RESPONSE_R4_R5_0 CMD RESP 95:64 */
	unsigned int	rspreg3;	/* _RESPONSE_R6_R7_0 CMD RESP 127:96 */
	unsigned int	bdata;		/* _BUFFER_DATA_PORT_0 */
	unsigned int	prnsts;		/* _PRESENT_STATE_0 */
	unsigned char	hostctl;	/* _POWER_CONTROL_HOST_0 7:00 */
	unsigned char	pwrcon;		/* _POWER_CONTROL_HOST_0 15:8 */
	unsigned char	blkgap;		/* _POWER_CONTROL_HOST_9 23:16 */
	unsigned char	wakcon;		/* _POWER_CONTROL_HOST_0 31:24 */
	unsigned short	clkcon;		/* _CLOCK_CONTROL_0 15:00 */
	unsigned char	timeoutcon;	/* _TIMEOUT_CTRL 23:16 */
	unsigned char	swrst;		/* _SW_RESET_ 31:24 */
	unsigned int	norintsts;	/* _INTERRUPT_STATUS_0 */
	unsigned int	norintstsen;	/* _INTERRUPT_STATUS_ENABLE_0 */
	unsigned int	norintsigen;	/* _INTERRUPT_SIGNAL_ENABLE_0 */
	unsigned short	acmd12errsts;	/* _AUTO_CMD12_ERR_STATUS_0 15:00 */
	unsigned char	res1[2];	/* _RESERVED 31:16 */
	unsigned int	capareg;	/* _CAPABILITIES_0 */
	unsigned char	res2[4];	/* RESERVED, offset 44h-47h */
	unsigned int	maxcurr;	/* _MAXIMUM_CURRENT_0 */
	unsigned char	res3[4];	/* RESERVED, offset 4Ch-4Fh */
	unsigned short	setacmd12err;	/* offset 50h */
	unsigned short	setinterr;	/* offset 52h */
	unsigned char	admaerr;	/* offset 54h */
	unsigned char	res4[3];	/* RESERVED, offset 55h-57h */
	unsigned long	admaaddr;	/* offset 58h-5Fh */
	unsigned char	res5[0xa0];	/* RESERVED, offset 60h-FBh */
	unsigned short	slotintstatus;	/* offset FCh */
	unsigned short	hcver;		/* HOST Version */
	unsigned int	venclkctl;	/* _VENDOR_CLOCK_CNTRL_0,    100h */
	unsigned int	venspictl;	/* _VENDOR_SPI_CNTRL_0,      104h */
	unsigned int	venspiintsts;	/* _VENDOR_SPI_INT_STATUS_0, 108h */
	unsigned int	venceatactl;	/* _VENDOR_CEATA_CNTRL_0,    10Ch */
	unsigned int	venbootctl;	/* _VENDOR_BOOT_CNTRL_0,     110h */
	unsigned int	venbootacktout;	/* _VENDOR_BOOT_ACK_TIMEOUT, 114h */
	unsigned int	venbootdattout;	/* _VENDOR_BOOT_DAT_TIMEOUT, 118h */
	unsigned int	vendebouncecnt;	/* _VENDOR_DEBOUNCE_COUNT_0, 11Ch */
	unsigned int	venmiscctl;	/* _VENDOR_MISC_CNTRL_0,     120h */
	unsigned int	res6[47];	/* 0x124 ~ 0x1DC */
	unsigned int	sdmemcmppadctl;	/* _SDMEMCOMPPADCTRL_0,      1E0h */
	unsigned int	autocalcfg;	/* _AUTO_CAL_CONFIG_0,       1E4h */
	unsigned int	autocalintval;	/* _AUTO_CAL_INTERVAL_0,     1E8h */
	unsigned int	autocalsts;	/* _AUTO_CAL_STATUS_0,       1ECh */
};

#define TEGRA_MMC_PWRCTL_SD_BUS_POWER				(1 << 0)
#define TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V1_8			(5 << 1)
#define TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V3_0			(6 << 1)
#define TEGRA_MMC_PWRCTL_SD_BUS_VOLTAGE_V3_3			(7 << 1)

#define TEGRA_MMC_HOSTCTL_DMASEL_MASK				(3 << 3)
#define TEGRA_MMC_HOSTCTL_DMASEL_SDMA				(0 << 3)
#define TEGRA_MMC_HOSTCTL_DMASEL_ADMA2_32BIT			(2 << 3)
#define TEGRA_MMC_HOSTCTL_DMASEL_ADMA2_64BIT			(3 << 3)

#define TEGRA_MMC_TRNMOD_DMA_ENABLE				(1 << 0)
#define TEGRA_MMC_TRNMOD_BLOCK_COUNT_ENABLE			(1 << 1)
#define TEGRA_MMC_TRNMOD_DATA_XFER_DIR_SEL_WRITE		(0 << 4)
#define TEGRA_MMC_TRNMOD_DATA_XFER_DIR_SEL_READ			(1 << 4)
#define TEGRA_MMC_TRNMOD_MULTI_BLOCK_SELECT			(1 << 5)

#define TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_MASK			(3 << 0)
#define TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_NO_RESPONSE		(0 << 0)
#define TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_136		(1 << 0)
#define TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_48		(2 << 0)
#define TEGRA_MMC_CMDREG_RESP_TYPE_SELECT_LENGTH_48_BUSY	(3 << 0)

#define TEGRA_MMC_TRNMOD_CMD_CRC_CHECK				(1 << 3)
#define TEGRA_MMC_TRNMOD_CMD_INDEX_CHECK			(1 << 4)
#define TEGRA_MMC_TRNMOD_DATA_PRESENT_SELECT_DATA_TRANSFER	(1 << 5)

#define TEGRA_MMC_PRNSTS_CMD_INHIBIT_CMD			(1 << 0)
#define TEGRA_MMC_PRNSTS_CMD_INHIBIT_DAT			(1 << 1)

#define TEGRA_MMC_CLKCON_INTERNAL_CLOCK_ENABLE			(1 << 0)
#define TEGRA_MMC_CLKCON_INTERNAL_CLOCK_STABLE			(1 << 1)
#define TEGRA_MMC_CLKCON_SD_CLOCK_ENABLE			(1 << 2)

#define TEGRA_MMC_CLKCON_SDCLK_FREQ_SEL_SHIFT			8
#define TEGRA_MMC_CLKCON_SDCLK_FREQ_SEL_MASK			(0xff << 8)

#define TEGRA_MMC_MISCON_ENABLE_EXT_LOOPBACK			(1 << 17)

#define TEGRA_MMC_SWRST_SW_RESET_FOR_ALL			(1 << 0)
#define TEGRA_MMC_SWRST_SW_RESET_FOR_CMD_LINE			(1 << 1)
#define TEGRA_MMC_SWRST_SW_RESET_FOR_DAT_LINE			(1 << 2)

#define TEGRA_MMC_NORINTSTS_CMD_COMPLETE			(1 << 0)
#define TEGRA_MMC_NORINTSTS_XFER_COMPLETE			(1 << 1)
#define TEGRA_MMC_NORINTSTS_DMA_INTERRUPT			(1 << 3)
#define TEGRA_MMC_NORINTSTS_ERR_INTERRUPT			(1 << 15)
#define TEGRA_MMC_NORINTSTS_CMD_TIMEOUT				(1 << 16)

#define TEGRA_MMC_NORINTSTSEN_CMD_COMPLETE			(1 << 0)
#define TEGRA_MMC_NORINTSTSEN_XFER_COMPLETE			(1 << 1)
#define TEGRA_MMC_NORINTSTSEN_DMA_INTERRUPT			(1 << 3)
#define TEGRA_MMC_NORINTSTSEN_BUFFER_WRITE_READY		(1 << 4)
#define TEGRA_MMC_NORINTSTSEN_BUFFER_READ_READY			(1 << 5)

#define TEGRA_MMC_NORINTSIGEN_XFER_COMPLETE			(1 << 1)

/* SDMMC1/3 settings from section 24.6 of T30 TRM */
#define MEMCOMP_PADCTRL_VREF	7
#define AUTO_CAL_ENABLED	(1 << 29)
#define AUTO_CAL_PD_OFFSET	(0x70 << 8)
#define AUTO_CAL_PU_OFFSET	(0x62 << 0)

#endif	/* __ASSEMBLY__ */
#endif	/* __TEGRA_MMC_H_ */
