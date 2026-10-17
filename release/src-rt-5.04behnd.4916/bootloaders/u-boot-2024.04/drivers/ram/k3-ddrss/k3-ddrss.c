// SPDX-License-Identifier: GPL-2.0+
/*
 * Texas Instruments' K3 DDRSS driver
 *
 * Copyright (C) 2020-2021 Texas Instruments Incorporated - https://www.ti.com/
 */

#include <common.h>
#include <config.h>
#include <clk.h>
#include <div64.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <fdt_support.h>
#include <ram.h>
#include <hang.h>
#include <log.h>
#include <asm/io.h>
#include <power-domain.h>
#include <wait_bit.h>
#include <power/regulator.h>

#include "lpddr4_obj_if.h"
#include "lpddr4_if.h"
#include "lpddr4_structs_if.h"
#include "lpddr4_ctl_regs.h"

#define SRAM_MAX 512

#define CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS	0x80
#define CTRLMMR_DDR4_FSP_CLKCHNG_ACK_OFFS	0xc0

#define DDRSS_V2A_CTL_REG			0x0020
#define DDRSS_ECC_CTRL_REG			0x0120

#define DDRSS_ECC_CTRL_REG_ECC_EN		BIT(0)
#define DDRSS_ECC_CTRL_REG_RMW_EN		BIT(1)
#define DDRSS_ECC_CTRL_REG_ECC_CK		BIT(2)
#define DDRSS_ECC_CTRL_REG_WR_ALLOC		BIT(4)

#define DDRSS_ECC_R0_STR_ADDR_REG		0x0130
#define DDRSS_ECC_R0_END_ADDR_REG		0x0134
#define DDRSS_ECC_R1_STR_ADDR_REG		0x0138
#define DDRSS_ECC_R1_END_ADDR_REG		0x013c
#define DDRSS_ECC_R2_STR_ADDR_REG		0x0140
#define DDRSS_ECC_R2_END_ADDR_REG		0x0144
#define DDRSS_ECC_1B_ERR_CNT_REG		0x0150

#define SINGLE_DDR_SUBSYSTEM	0x1
#define MULTI_DDR_SUBSYSTEM	0x2

#define MULTI_DDR_CFG0  0x00114100
#define MULTI_DDR_CFG1  0x00114104
#define DDR_CFG_LOAD    0x00114110

enum intrlv_gran {
	GRAN_128B,
	GRAN_512B,
	GRAN_2KB,
	GRAN_4KB,
	GRAN_16KB,
	GRAN_32KB,
	GRAN_512KB,
	GRAN_1GB,
	GRAN_1_5GB,
	GRAN_2GB,
	GRAN_3GB,
	GRAN_4GB,
	GRAN_6GB,
	GRAN_8GB,
	GRAN_16GB
};

enum intrlv_size {
	SIZE_0,
	SIZE_128MB,
	SIZE_256MB,
	SIZE_512MB,
	SIZE_1GB,
	SIZE_2GB,
	SIZE_3GB,
	SIZE_4GB,
	SIZE_6GB,
	SIZE_8GB,
	SIZE_12GB,
	SIZE_16GB,
	SIZE_32GB
};

struct k3_ddrss_data {
	u32 flags;
};

enum ecc_enable {
	DISABLE_ALL = 0,
	ENABLE_0,
	ENABLE_1,
	ENABLE_ALL
};

enum emif_config {
	INTERLEAVE_ALL = 0,
	SEPR0,
	SEPR1
};

enum emif_active {
	EMIF_0 = 1,
	EMIF_1,
	EMIF_ALL
};

struct k3_msmc {
	enum intrlv_gran gran;
	enum intrlv_size size;
	enum ecc_enable enable;
	enum emif_config config;
	enum emif_active active;
};

#define K3_DDRSS_MAX_ECC_REGIONS		3

struct k3_ddrss_ecc_region {
	u32 start;
	u32 range;
};

struct k3_ddrss_desc {
	struct udevice *dev;
	void __iomem *ddrss_ss_cfg;
	void __iomem *ddrss_ctrl_mmr;
	void __iomem *ddrss_ctl_cfg;
	struct power_domain ddrcfg_pwrdmn;
	struct power_domain ddrdata_pwrdmn;
	struct clk ddr_clk;
	struct clk osc_clk;
	u32 ddr_freq0;
	u32 ddr_freq1;
	u32 ddr_freq2;
	u32 ddr_fhs_cnt;
	u32 dram_class;
	struct udevice *vtt_supply;
	u32 instance;
	lpddr4_obj *driverdt;
	lpddr4_config config;
	lpddr4_privatedata pd;
	struct k3_ddrss_ecc_region ecc_regions[K3_DDRSS_MAX_ECC_REGIONS];
	u64 ecc_reserved_space;
	bool ti_ecc_enabled;
};

struct reginitdata {
	u32 ctl_regs[LPDDR4_INTR_CTL_REG_COUNT];
	u16 ctl_regs_offs[LPDDR4_INTR_CTL_REG_COUNT];
	u32 pi_regs[LPDDR4_INTR_PHY_INDEP_REG_COUNT];
	u16 pi_regs_offs[LPDDR4_INTR_PHY_INDEP_REG_COUNT];
	u32 phy_regs[LPDDR4_INTR_PHY_REG_COUNT];
	u16 phy_regs_offs[LPDDR4_INTR_PHY_REG_COUNT];
};

#define TH_MACRO_EXP(fld, str) (fld##str)

#define TH_FLD_MASK(fld)  TH_MACRO_EXP(fld, _MASK)
#define TH_FLD_SHIFT(fld) TH_MACRO_EXP(fld, _SHIFT)
#define TH_FLD_WIDTH(fld) TH_MACRO_EXP(fld, _WIDTH)
#define TH_FLD_WOCLR(fld) TH_MACRO_EXP(fld, _WOCLR)
#define TH_FLD_WOSET(fld) TH_MACRO_EXP(fld, _WOSET)

#define str(s) #s
#define xstr(s) str(s)

#define CTL_SHIFT 11
#define PHY_SHIFT 11
#define PI_SHIFT 10

#define DENALI_CTL_0_DRAM_CLASS_DDR4		0xA
#define DENALI_CTL_0_DRAM_CLASS_LPDDR4		0xB

#define TH_OFFSET_FROM_REG(REG, SHIFT, offset) do {\
	char *i, *pstr = xstr(REG); offset = 0;\
	for (i = &pstr[SHIFT]; *i != '\0'; ++i) {\
		offset = offset * 10 + (*i - '0'); } \
	} while (0)

static u32 k3_lpddr4_read_ddr_type(const lpddr4_privatedata *pd)
{
	u32 status = 0U;
	u32 offset = 0U;
	u32 regval = 0U;
	u32 dram_class = 0U;
	struct k3_ddrss_desc *ddrss = (struct k3_ddrss_desc *)pd->ddr_instance;

	TH_OFFSET_FROM_REG(LPDDR4__DRAM_CLASS__REG, CTL_SHIFT, offset);
	status = ddrss->driverdt->readreg(pd, LPDDR4_CTL_REGS, offset, &regval);
	if (status > 0U) {
		printf("%s: Failed to read DRAM_CLASS\n", __func__);
		hang();
	}

	dram_class = ((regval & TH_FLD_MASK(LPDDR4__DRAM_CLASS__FLD)) >>
		TH_FLD_SHIFT(LPDDR4__DRAM_CLASS__FLD));
	return dram_class;
}

static void k3_lpddr4_freq_update(struct k3_ddrss_desc *ddrss)
{
	unsigned int req_type, counter;

	for (counter = 0; counter < ddrss->ddr_fhs_cnt; counter++) {
		if (wait_for_bit_le32(ddrss->ddrss_ctrl_mmr +
				      CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS + ddrss->instance * 0x10, 0x80,
				      true, 10000, false)) {
			printf("Timeout during frequency handshake\n");
			hang();
		}

		req_type = readl(ddrss->ddrss_ctrl_mmr +
				 CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS + ddrss->instance * 0x10) & 0x03;

		debug("%s: received freq change req: req type = %d, req no. = %d, instance = %d\n",
		      __func__, req_type, counter, ddrss->instance);

		if (req_type == 1)
			clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq1);
		else if (req_type == 2)
			clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq2);
		else if (req_type == 0)
			clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq0);
		else
			printf("%s: Invalid freq request type\n", __func__);

		writel(0x1, ddrss->ddrss_ctrl_mmr +
		       CTRLMMR_DDR4_FSP_CLKCHNG_ACK_OFFS + ddrss->instance * 0x10);
		if (wait_for_bit_le32(ddrss->ddrss_ctrl_mmr +
				      CTRLMMR_DDR4_FSP_CLKCHNG_REQ_OFFS + ddrss->instance * 0x10, 0x80,
				      false, 10, false)) {
			printf("Timeout during frequency handshake\n");
			hang();
		}
		writel(0x0, ddrss->ddrss_ctrl_mmr +
		       CTRLMMR_DDR4_FSP_CLKCHNG_ACK_OFFS + ddrss->instance * 0x10);
	}
}

static void k3_lpddr4_ack_freq_upd_req(const lpddr4_privatedata *pd)
{
	struct k3_ddrss_desc *ddrss = (struct k3_ddrss_desc *)pd->ddr_instance;

	debug("--->>> LPDDR4 Initialization is in progress ... <<<---\n");

	switch (ddrss->dram_class) {
	case DENALI_CTL_0_DRAM_CLASS_DDR4:
		break;
	case DENALI_CTL_0_DRAM_CLASS_LPDDR4:
		k3_lpddr4_freq_update(ddrss);
		break;
	default:
		printf("Unrecognized dram_class cannot update frequency!\n");
	}
}

static int k3_ddrss_init_freq(struct k3_ddrss_desc *ddrss)
{
	int ret;
	lpddr4_privatedata *pd = &ddrss->pd;

	ddrss->dram_class = k3_lpddr4_read_ddr_type(pd);

	switch (ddrss->dram_class) {
	case DENALI_CTL_0_DRAM_CLASS_DDR4:
		/* Set to ddr_freq1 from DT for DDR4 */
		ret = clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq1);
		break;
	case DENALI_CTL_0_DRAM_CLASS_LPDDR4:
		ret = clk_set_rate(&ddrss->ddr_clk, ddrss->ddr_freq0);
		break;
	default:
		ret = -EINVAL;
		printf("Unrecognized dram_class cannot init frequency!\n");
	}

	if (ret < 0)
		dev_err(ddrss->dev, "ddr clk init failed: %d\n", ret);
	else
		ret = 0;

	return ret;
}

static void k3_lpddr4_info_handler(const lpddr4_privatedata *pd,
				   lpddr4_infotype infotype)
{
	if (infotype == LPDDR4_DRV_SOC_PLL_UPDATE)
		k3_lpddr4_ack_freq_upd_req(pd);
}

static int k3_ddrss_power_on(struct k3_ddrss_desc *ddrss)
{
	int ret;

	debug("%s(ddrss=%p)\n", __func__, ddrss);

	ret = power_domain_on(&ddrss->ddrcfg_pwrdmn);
	if (ret) {
		dev_err(ddrss->dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_on(&ddrss->ddrdata_pwrdmn);
	if (ret) {
		dev_err(ddrss->dev, "power_domain_on() failed: %d\n", ret);
		return ret;
	}

	ret = device_get_supply_regulator(ddrss->dev, "vtt-supply",
					  &ddrss->vtt_supply);
	if (ret) {
		dev_dbg(ddrss->dev, "vtt-supply not found.\n");
	} else {
		ret = regulator_set_value(ddrss->vtt_supply, 3300000);
		if (ret)
			return ret;
		dev_dbg(ddrss->dev, "VTT regulator enabled, volt = %d\n",
			regulator_get_value(ddrss->vtt_supply));
	}

	return 0;
}

static int k3_ddrss_ofdata_to_priv(struct udevice *dev)
{
	struct k3_ddrss_desc *ddrss = dev_get_priv(dev);
	struct k3_ddrss_data *ddrss_data = (struct k3_ddrss_data *)dev_get_driver_data(dev);
	void *reg;
	int ret;

	debug("%s(dev=%p)\n", __func__, dev);

	reg = dev_read_addr_name_ptr(dev, "cfg");
	if (!reg) {
		dev_err(dev, "No reg property for DDRSS wrapper logic\n");
		return -EINVAL;
	}
	ddrss->ddrss_ctl_cfg = reg;

	reg = dev_read_addr_name_ptr(dev, "ctrl_mmr_lp4");
	if (!reg) {
		dev_err(dev, "No reg property for CTRL MMR\n");
		return -EINVAL;
	}
	ddrss->ddrss_ctrl_mmr = reg;

	reg = dev_read_addr_name_ptr(dev, "ss_cfg");
	if (!reg)
		dev_dbg(dev, "No reg property for SS Config region, but this is optional so continuing.\n");
	ddrss->ddrss_ss_cfg = reg;

	ret = power_domain_get_by_index(dev, &ddrss->ddrcfg_pwrdmn, 0);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = power_domain_get_by_index(dev, &ddrss->ddrdata_pwrdmn, 1);
	if (ret) {
		dev_err(dev, "power_domain_get() failed: %d\n", ret);
		return ret;
	}

	ret = clk_get_by_index(dev, 0, &ddrss->ddr_clk);
	if (ret)
		dev_err(dev, "clk get failed%d\n", ret);

	ret = clk_get_by_index(dev, 1, &ddrss->osc_clk);
	if (ret)
		dev_err(dev, "clk get failed for osc clk %d\n", ret);

	/* Reading instance number for multi ddr subystems */
	if (ddrss_data->flags & MULTI_DDR_SUBSYSTEM) {
		ret = dev_read_u32(dev, "instance", &ddrss->instance);
		if (ret) {
			dev_err(dev, "missing instance property");
			return -EINVAL;
		}
	} else {
		ddrss->instance = 0;
	}

	ret = dev_read_u32(dev, "ti,ddr-freq0", &ddrss->ddr_freq0);
	if (ret) {
		ddrss->ddr_freq0 = clk_get_rate(&ddrss->osc_clk);
		dev_dbg(dev,
			"ddr freq0 not populated, using bypass frequency.\n");
	}

	ret = dev_read_u32(dev, "ti,ddr-freq1", &ddrss->ddr_freq1);
	if (ret)
		dev_err(dev, "ddr freq1 not populated %d\n", ret);

	ret = dev_read_u32(dev, "ti,ddr-freq2", &ddrss->ddr_freq2);
	if (ret)
		dev_err(dev, "ddr freq2 not populated %d\n", ret);

	ret = dev_read_u32(dev, "ti,ddr-fhs-cnt", &ddrss->ddr_fhs_cnt);
	if (ret)
		dev_err(dev, "ddr fhs cnt not populated %d\n", ret);

	ddrss->ti_ecc_enabled = dev_read_bool(dev, "ti,ecc-enable");

	return ret;
}

void k3_lpddr4_probe(struct k3_ddrss_desc *ddrss)
{
	u32 status = 0U;
	u16 configsize = 0U;
	lpddr4_config *config = &ddrss->config;

	status = ddrss->driverdt->probe(config, &configsize);

	if ((status != 0) || (configsize != sizeof(lpddr4_privatedata))
	    || (configsize > SRAM_MAX)) {
		printf("%s: FAIL\n", __func__);
		hang();
	} else {
		debug("%s: PASS\n", __func__);
	}
}

void k3_lpddr4_init(struct k3_ddrss_desc *ddrss)
{
	u32 status = 0U;
	lpddr4_config *config = &ddrss->config;
	lpddr4_obj *driverdt = ddrss->driverdt;
	lpddr4_privatedata *pd = &ddrss->pd;

	if ((sizeof(*pd) != sizeof(lpddr4_privatedata)) || (sizeof(*pd) > SRAM_MAX)) {
		printf("%s: FAIL\n", __func__);
		hang();
	}

	config->ctlbase = (struct lpddr4_ctlregs_s *)ddrss->ddrss_ctl_cfg;
	config->infohandler = (lpddr4_infocallback) k3_lpddr4_info_handler;

	status = driverdt->init(pd, config);

	/* linking ddr instance to lpddr4  */
	pd->ddr_instance = (void *)ddrss;

	if ((status > 0U) ||
	    (pd->ctlbase != (struct lpddr4_ctlregs_s *)config->ctlbase) ||
	    (pd->ctlinterrupthandler != config->ctlinterrupthandler) ||
	    (pd->phyindepinterrupthandler != config->phyindepinterrupthandler)) {
		printf("%s: FAIL\n", __func__);
		hang();
	} else {
		debug("%s: PASS\n", __func__);
	}
}

void populate_data_array_from_dt(struct k3_ddrss_desc *ddrss,
				 struct reginitdata *reginit_data)
{
	int ret, i;

	ret = dev_read_u32_array(ddrss->dev, "ti,ctl-data",
				 (u32 *)reginit_data->ctl_regs,
				 LPDDR4_INTR_CTL_REG_COUNT);
	if (ret)
		printf("Error reading ctrl data %d\n", ret);

	for (i = 0; i < LPDDR4_INTR_CTL_REG_COUNT; i++)
		reginit_data->ctl_regs_offs[i] = i;

	ret = dev_read_u32_array(ddrss->dev, "ti,pi-data",
				 (u32 *)reginit_data->pi_regs,
				 LPDDR4_INTR_PHY_INDEP_REG_COUNT);
	if (ret)
		printf("Error reading PI data\n");

	for (i = 0; i < LPDDR4_INTR_PHY_INDEP_REG_COUNT; i++)
		reginit_data->pi_regs_offs[i] = i;

	ret = dev_read_u32_array(ddrss->dev, "ti,phy-data",
				 (u32 *)reginit_data->phy_regs,
				 LPDDR4_INTR_PHY_REG_COUNT);
	if (ret)
		printf("Error reading PHY data %d\n", ret);

	for (i = 0; i < LPDDR4_INTR_PHY_REG_COUNT; i++)
		reginit_data->phy_regs_offs[i] = i;
}

void k3_lpddr4_hardware_reg_init(struct k3_ddrss_desc *ddrss)
{
	u32 status = 0U;
	struct reginitdata reginitdata;
	lpddr4_obj *driverdt = ddrss->driverdt;
	lpddr4_privatedata *pd = &ddrss->pd;

	populate_data_array_from_dt(ddrss, &reginitdata);

	status = driverdt->writectlconfig(pd, reginitdata.ctl_regs,
					  reginitdata.ctl_regs_offs,
					  LPDDR4_INTR_CTL_REG_COUNT);
	if (!status)
		status = driverdt->writephyindepconfig(pd, reginitdata.pi_regs,
						       reginitdata.pi_regs_offs,
						       LPDDR4_INTR_PHY_INDEP_REG_COUNT);
	if (!status)
		status = driverdt->writephyconfig(pd, reginitdata.phy_regs,
						  reginitdata.phy_regs_offs,
						  LPDDR4_INTR_PHY_REG_COUNT);
	if (status) {
		printf("%s: FAIL\n", __func__);
		hang();
	}
}

void k3_lpddr4_start(struct k3_ddrss_desc *ddrss)
{
	u32 status = 0U;
	u32 regval = 0U;
	u32 offset = 0U;
	lpddr4_obj *driverdt = ddrss->driverdt;
	lpddr4_privatedata *pd = &ddrss->pd;

	TH_OFFSET_FROM_REG(LPDDR4__START__REG, CTL_SHIFT, offset);

	status = driverdt->readreg(pd, LPDDR4_CTL_REGS, offset, &regval);
	if ((status > 0U) || ((regval & TH_FLD_MASK(LPDDR4__START__FLD)) != 0U)) {
		printf("%s: Pre start FAIL\n", __func__);
		hang();
	}

	status = driverdt->start(pd);
	if (status > 0U) {
		printf("%s: FAIL\n", __func__);
		hang();
	}

	status = driverdt->readreg(pd, LPDDR4_CTL_REGS, offset, &regval);
	if ((status > 0U) || ((regval & TH_FLD_MASK(LPDDR4__START__FLD)) != 1U)) {
		printf("%s: Post start FAIL\n", __func__);
		hang();
	} else {
		debug("%s: Post start PASS\n", __func__);
	}
}

static void k3_ddrss_set_ecc_range_r0(u32 base, u32 start_address, u32 size)
{
	writel((start_address) >> 16, base + DDRSS_ECC_R0_STR_ADDR_REG);
	writel((start_address + size - 1) >> 16, base + DDRSS_ECC_R0_END_ADDR_REG);
}

static void k3_ddrss_preload_ecc_mem_region(u32 *addr, u32 size, u32 word)
{
	int i;

	printf("ECC is enabled, priming DDR which will take several seconds.\n");

	for (i = 0; i < (size / 4); i++)
		addr[i] = word;
}

static void k3_ddrss_lpddr4_ecc_calc_reserved_mem(struct k3_ddrss_desc *ddrss)
{
	fdtdec_setup_mem_size_base_lowest();

	ddrss->ecc_reserved_space = gd->ram_size;
	do_div(ddrss->ecc_reserved_space, 9);

	/* Round to clean number */
	ddrss->ecc_reserved_space = 1ull << (fls(ddrss->ecc_reserved_space));
}

static void k3_ddrss_lpddr4_ecc_init(struct k3_ddrss_desc *ddrss)
{
	u32 ecc_region_start = ddrss->ecc_regions[0].start;
	u32 ecc_range = ddrss->ecc_regions[0].range;
	u32 base = (u32)ddrss->ddrss_ss_cfg;
	u32 val;

	/* Only Program region 0 which covers full ddr space */
	k3_ddrss_set_ecc_range_r0(base, ecc_region_start - gd->ram_base, ecc_range);

	/* Enable ECC, RMW, WR_ALLOC */
	writel(DDRSS_ECC_CTRL_REG_ECC_EN | DDRSS_ECC_CTRL_REG_RMW_EN |
	       DDRSS_ECC_CTRL_REG_WR_ALLOC, base + DDRSS_ECC_CTRL_REG);

	/* Preload ECC Mem region with 0's */
	k3_ddrss_preload_ecc_mem_region((u32 *)ecc_region_start, ecc_range,
					0x00000000);

	/* Clear Error Count Register */
	writel(0x1, base + DDRSS_ECC_1B_ERR_CNT_REG);

	/* Enable ECC Check */
	val = readl(base + DDRSS_ECC_CTRL_REG);
	val |= DDRSS_ECC_CTRL_REG_ECC_CK;
	writel(val, base + DDRSS_ECC_CTRL_REG);
}

static int k3_ddrss_probe(struct udevice *dev)
{
	int ret;
	struct k3_ddrss_desc *ddrss = dev_get_priv(dev);

	debug("%s(dev=%p)\n", __func__, dev);

	ret = k3_ddrss_ofdata_to_priv(dev);
	if (ret)
		return ret;

	ddrss->dev = dev;
	ret = k3_ddrss_power_on(ddrss);
	if (ret)
		return ret;

#ifdef CONFIG_K3_AM64_DDRSS
	/* AM64x supports only up to 2 GB SDRAM */
	writel(0x000001EF, ddrss->ddrss_ss_cfg + DDRSS_V2A_CTL_REG);
	writel(0x0, ddrss->ddrss_ss_cfg + DDRSS_ECC_CTRL_REG);
#endif

	ddrss->driverdt = lpddr4_getinstance();

	k3_lpddr4_probe(ddrss);
	k3_lpddr4_init(ddrss);
	k3_lpddr4_hardware_reg_init(ddrss);

	ret = k3_ddrss_init_freq(ddrss);
	if (ret)
		return ret;

	k3_lpddr4_start(ddrss);

	if (ddrss->ti_ecc_enabled) {
		if (!ddrss->ddrss_ss_cfg) {
			printf("%s: ss_cfg is required if ecc is enabled but not provided.",
			       __func__);
			return -EINVAL;
		}

		k3_ddrss_lpddr4_ecc_calc_reserved_mem(ddrss);

		/* Always configure one region that covers full DDR space */
		ddrss->ecc_regions[0].start = gd->ram_base;
		ddrss->ecc_regions[0].range = gd->ram_size - ddrss->ecc_reserved_space;
		k3_ddrss_lpddr4_ecc_init(ddrss);
	}

	return ret;
}

int k3_ddrss_ddr_fdt_fixup(struct udevice *dev, void *blob, struct bd_info *bd)
{
	struct k3_ddrss_desc *ddrss = dev_get_priv(dev);
	u64 start[CONFIG_NR_DRAM_BANKS];
	u64 size[CONFIG_NR_DRAM_BANKS];
	int bank;

	if (ddrss->ecc_reserved_space == 0)
		return 0;

	for (bank = CONFIG_NR_DRAM_BANKS - 1; bank >= 0; bank--) {
		if (ddrss->ecc_reserved_space > bd->bi_dram[bank].size) {
			ddrss->ecc_reserved_space -= bd->bi_dram[bank].size;
			bd->bi_dram[bank].size = 0;
		} else {
			bd->bi_dram[bank].size -= ddrss->ecc_reserved_space;
			break;
		}
	}

	for (bank = 0; bank < CONFIG_NR_DRAM_BANKS; bank++) {
		start[bank] =  bd->bi_dram[bank].start;
		size[bank] = bd->bi_dram[bank].size;
	}

	return fdt_fixup_memory_banks(blob, start, size, CONFIG_NR_DRAM_BANKS);
}

static int k3_ddrss_get_info(struct udevice *dev, struct ram_info *info)
{
	return 0;
}

static struct ram_ops k3_ddrss_ops = {
	.get_info = k3_ddrss_get_info,
};

static const struct k3_ddrss_data k3_data = {
	.flags = SINGLE_DDR_SUBSYSTEM,
};

static const struct k3_ddrss_data j721s2_data = {
	.flags = MULTI_DDR_SUBSYSTEM,
};

static const struct udevice_id k3_ddrss_ids[] = {
	{.compatible = "ti,am62a-ddrss", .data = (ulong)&k3_data, },
	{.compatible = "ti,am64-ddrss", .data = (ulong)&k3_data, },
	{.compatible = "ti,j721e-ddrss", .data = (ulong)&k3_data, },
	{.compatible = "ti,j721s2-ddrss", .data = (ulong)&j721s2_data, },
	{}
};

U_BOOT_DRIVER(k3_ddrss) = {
	.name			= "k3_ddrss",
	.id			= UCLASS_RAM,
	.of_match		= k3_ddrss_ids,
	.ops			= &k3_ddrss_ops,
	.probe			= k3_ddrss_probe,
	.priv_auto		= sizeof(struct k3_ddrss_desc),
};

static int k3_msmc_set_config(struct k3_msmc *msmc)
{
	u32 ddr_cfg0 = 0;
	u32 ddr_cfg1 = 0;

	ddr_cfg0 |= msmc->gran << 24;
	ddr_cfg0 |= msmc->size << 16;
	/* heartbeat_per, bit[4:0] setting to 3 is advisable */
	ddr_cfg0 |= 3;

	/* Program MULTI_DDR_CFG0 */
	writel(ddr_cfg0, MULTI_DDR_CFG0);

	ddr_cfg1 |= msmc->enable << 16;
	ddr_cfg1 |= msmc->config << 8;
	ddr_cfg1 |= msmc->active;

	/* Program MULTI_DDR_CFG1 */
	writel(ddr_cfg1, MULTI_DDR_CFG1);

	/* Program DDR_CFG_LOAD */
	writel(0x60000000, DDR_CFG_LOAD);

	return 0;
}

static int k3_msmc_probe(struct udevice *dev)
{
	struct k3_msmc *msmc = dev_get_priv(dev);
	int ret = 0;

	/* Read the granular size from DT */
	ret = dev_read_u32(dev, "intrlv-gran", &msmc->gran);
	if (ret) {
		dev_err(dev, "missing intrlv-gran property");
		return -EINVAL;
	}

	/* Read the interleave region from DT */
	ret = dev_read_u32(dev, "intrlv-size", &msmc->size);
	if (ret) {
		dev_err(dev, "missing intrlv-size property");
		return -EINVAL;
	}

	/* Read ECC enable config */
	ret = dev_read_u32(dev, "ecc-enable", &msmc->enable);
	if (ret) {
		dev_err(dev, "missing ecc-enable property");
		return -EINVAL;
	}

	/* Read EMIF configuration */
	ret = dev_read_u32(dev, "emif-config", &msmc->config);
	if (ret) {
		dev_err(dev, "missing emif-config property");
		return -EINVAL;
	}

	/* Read EMIF active */
	ret = dev_read_u32(dev, "emif-active", &msmc->active);
	if (ret) {
		dev_err(dev, "missing emif-active property");
		return -EINVAL;
	}

	ret = k3_msmc_set_config(msmc);
	if (ret) {
		dev_err(dev, "error setting msmc config");
		return -EINVAL;
	}

	return 0;
}

static const struct udevice_id k3_msmc_ids[] = {
	{ .compatible = "ti,j721s2-msmc"},
	{}
};

U_BOOT_DRIVER(k3_msmc) = {
	.name = "k3_msmc",
	.of_match = k3_msmc_ids,
	.id = UCLASS_MISC,
	.probe = k3_msmc_probe,
	.priv_auto = sizeof(struct k3_msmc),
	.flags = DM_FLAG_DEFAULT_PD_CTRL_OFF,
};
