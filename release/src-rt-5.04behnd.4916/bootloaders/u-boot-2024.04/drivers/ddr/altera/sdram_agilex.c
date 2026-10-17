// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2019 Intel Corporation <www.intel.com>
 *
 */

#include <common.h>
#include <dm.h>
#include <errno.h>
#include <div64.h>
#include <fdtdec.h>
#include <hang.h>
#include <log.h>
#include <ram.h>
#include <reset.h>
#include <asm/global_data.h>
#include "sdram_soc64.h"
#include <wait_bit.h>
#include <asm/arch/firewall.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/io.h>
#include <linux/sizes.h>

DECLARE_GLOBAL_DATA_PTR;

int sdram_mmr_init_full(struct udevice *dev)
{
	struct altera_sdram_plat *plat = dev_get_plat(dev);
	struct altera_sdram_priv *priv = dev_get_priv(dev);
	u32 i;
	int ret;
	phys_size_t hw_size;
	struct bd_info bd = {0};

	/* Ensure HMC clock is running */
	if (poll_hmc_clock_status()) {
		debug("DDR: Error as HMC clock was not running\n");
		return -EPERM;
	}

	/* Trying 3 times to do a calibration */
	for (i = 0; i < 3; i++) {
		ret = wait_for_bit_le32((const void *)(plat->hmc +
					DDRCALSTAT),
					DDR_HMC_DDRCALSTAT_CAL_MSK, true, 1000,
					false);
		if (!ret)
			break;

		emif_reset(plat);
	}

	if (ret) {
		puts("DDR: Error as SDRAM calibration failed\n");
		return -EPERM;
	}
	debug("DDR: Calibration success\n");

	/*
	 * Configure the DDR IO size
	 * niosreserve0: Used to indicate DDR width &
	 *	bit[7:0] = Number of data bits (bit[6:5] 0x01=32bit, 0x10=64bit)
	 *	bit[8]   = 1 if user-mode OCT is present
	 *	bit[9]   = 1 if warm reset compiled into EMIF Cal Code
	 *	bit[10]  = 1 if warm reset is on during generation in EMIF Cal
	 * niosreserve1: IP ADCDS version encoded as 16 bit value
	 *	bit[2:0] = Variant (0=not special,1=FAE beta, 2=Customer beta,
	 *			    3=EAP, 4-6 are reserved)
	 *	bit[5:3] = Service Pack # (e.g. 1)
	 *	bit[9:6] = Minor Release #
	 *	bit[14:10] = Major Release #
	 */
	/* Configure DDR IO size x16, x32 and x64 mode */
	u32 update_value;

	update_value = hmc_readl(plat, NIOSRESERVED0);
	update_value = (update_value & 0xFF) >> 5;

	/* Configure DDR data rate 0-HAlf-rate 1-Quarter-rate */
	update_value |= (hmc_readl(plat, CTRLCFG3) & 0x4);
	hmc_ecc_writel(plat, update_value, DDRIOCTRL);

	/* Copy values MMR IOHMC dramaddrw to HMC adp DRAMADDRWIDTH */
	hmc_ecc_writel(plat, hmc_readl(plat, DRAMADDRW), DRAMADDRWIDTH);

	/* assigning the SDRAM size */
	phys_size_t size = sdram_calculate_size(plat);

	if (size <= 0)
		hw_size = PHYS_SDRAM_1_SIZE;
	else
		hw_size = size;

	/* Get bank configuration from devicetree */
	ret = fdtdec_decode_ram_size(gd->fdt_blob, NULL, 0, NULL,
				     (phys_size_t *)&gd->ram_size, &bd);
	if (ret) {
		puts("DDR: Failed to decode memory node\n");
		return -ENXIO;
	}

	if (gd->ram_size != hw_size) {
		printf("DDR: Warning: DRAM size from device tree (%lld MiB)\n",
		       gd->ram_size >> 20);
		printf(" mismatch with hardware (%lld MiB).\n",
		       hw_size >> 20);
	}

	if (gd->ram_size > hw_size) {
		printf("DDR: Error: DRAM size from device tree is greater\n");
		printf(" than hardware size.\n");
		hang();
	}

	printf("DDR: %lld MiB\n", gd->ram_size >> 20);

	/* This enables nonsecure access to DDR */
	/* mpuregion0addr_limit */
	FW_MPU_DDR_SCR_WRITEL(gd->ram_size - 1,
			      FW_MPU_DDR_SCR_MPUREGION0ADDR_LIMIT);
	FW_MPU_DDR_SCR_WRITEL(0x1F, FW_MPU_DDR_SCR_MPUREGION0ADDR_LIMITEXT);

	/* nonmpuregion0addr_limit */
	FW_MPU_DDR_SCR_WRITEL(gd->ram_size - 1,
			      FW_MPU_DDR_SCR_NONMPUREGION0ADDR_LIMIT);

	/* Enable mpuregion0enable and nonmpuregion0enable */
	FW_MPU_DDR_SCR_WRITEL(MPUREGION0_ENABLE | NONMPUREGION0_ENABLE,
			      FW_MPU_DDR_SCR_EN_SET);

	u32 ctrlcfg1 = hmc_readl(plat, CTRLCFG1);

	/* Enable or disable the DDR ECC */
	if (CTRLCFG1_CFG_CTRL_EN_ECC(ctrlcfg1)) {
		setbits_le32(plat->hmc + ECCCTRL1,
			     (DDR_HMC_ECCCTL_AWB_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_ECC_EN_SET_MSK));
		clrbits_le32(plat->hmc + ECCCTRL1,
			     (DDR_HMC_ECCCTL_AWB_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_CNT_RST_SET_MSK));
		setbits_le32(plat->hmc + ECCCTRL2,
			     (DDR_HMC_ECCCTL2_RMW_EN_SET_MSK |
			      DDR_HMC_ECCCTL2_AWB_EN_SET_MSK));
		setbits_le32(plat->hmc + ERRINTEN,
			     DDR_HMC_ERRINTEN_DERRINTEN_EN_SET_MSK);

		if (!cpu_has_been_warmreset())
			sdram_init_ecc_bits(&bd);
	} else {
		clrbits_le32(plat->hmc + ECCCTRL1,
			     (DDR_HMC_ECCCTL_AWB_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_CNT_RST_SET_MSK |
			      DDR_HMC_ECCCTL_ECC_EN_SET_MSK));
		clrbits_le32(plat->hmc + ECCCTRL2,
			     (DDR_HMC_ECCCTL2_RMW_EN_SET_MSK |
			      DDR_HMC_ECCCTL2_AWB_EN_SET_MSK));
	}

	/* Enable non-secure reads/writes to HMC Adapter for SDRAM ECC */
	writel(FW_HMC_ADAPTOR_MPU_MASK, FW_HMC_ADAPTOR_REG_ADDR);

	sdram_size_check(&bd);

	priv->info.base = bd.bi_dram[0].start;
	priv->info.size = gd->ram_size;

	debug("DDR: HMC init success\n");
	return 0;
}
