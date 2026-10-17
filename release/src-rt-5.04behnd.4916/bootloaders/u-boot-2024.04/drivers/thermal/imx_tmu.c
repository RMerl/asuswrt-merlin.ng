// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2017~2020 NXP
 *
 */

#include <config.h>
#include <common.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <dm.h>
#include <dm/device_compat.h>
#include <dm/device-internal.h>
#include <dm/device.h>
#include <errno.h>
#include <fuse.h>
#include <linux/delay.h>
#include <malloc.h>
#include <thermal.h>

DECLARE_GLOBAL_DATA_PTR;

#define SITES_MAX	16
#define FLAGS_VER2	0x1
#define FLAGS_VER3	0x2
#define FLAGS_VER4	0x4

#define TMR_DISABLE	0x0
#define TMR_ME		0x80000000
#define TMR_ALPF	0x0c000000
#define TMTMIR_DEFAULT	0x00000002
#define TIER_DISABLE	0x0

#define TER_EN			0x80000000
#define TER_ADC_PD		0x40000000
#define TER_ALPF		0x3

#define IMX_TMU_POLLING_DELAY_MS	5000
/*
 * i.MX TMU Registers
 */
struct imx_tmu_site_regs {
	u32 tritsr;		/* Immediate Temperature Site Register */
	u32 tratsr;		/* Average Temperature Site Register */
	u8 res0[0x8];
};

struct imx_tmu_regs {
	u32 tmr;	/* Mode Register */
	u32 tsr;	/* Status Register */
	u32 tmtmir;	/* Temperature measurement interval Register */
	u8 res0[0x14];
	u32 tier;	/* Interrupt Enable Register */
	u32 tidr;	/* Interrupt Detect Register */
	u32 tiscr;	/* Interrupt Site Capture Register */
	u32 ticscr;	/* Interrupt Critical Site Capture Register */
	u8 res1[0x10];
	u32 tmhtcrh;	/* High Temperature Capture Register */
	u32 tmhtcrl;	/* Low Temperature Capture Register */
	u8 res2[0x8];
	u32 tmhtitr;	/* High Temperature Immediate Threshold */
	u32 tmhtatr;	/* High Temperature Average Threshold */
	u32 tmhtactr;	/* High Temperature Average Crit Threshold */
	u8 res3[0x24];
	u32 ttcfgr;	/* Temperature Configuration Register */
	u32 tscfgr;	/* Sensor Configuration Register */
	u8 res4[0x78];
	struct imx_tmu_site_regs site[SITES_MAX];
	u8 res5[0x9f8];
	u32 ipbrr0;	/* IP Block Revision Register 0 */
	u32 ipbrr1;	/* IP Block Revision Register 1 */
	u8 res6[0x310];
	u32 ttr0cr;	/* Temperature Range 0 Control Register */
	u32 ttr1cr;	/* Temperature Range 1 Control Register */
	u32 ttr2cr;	/* Temperature Range 2 Control Register */
	u32 ttr3cr;	/* Temperature Range 3 Control Register */
};

struct imx_tmu_regs_v4 {
	u32 tmr;	/* Mode Register */
	u32 tsr;	/* Status Register */
	u32 tmsr;	/* Monitor Site Register */
	u32 tmtmir;	/* Temperature measurement interval Register */
	u8 res0[0x10];
	u32 tier;	/* Interrupt Enable Register */
	u32 tidr;	/* Interrupt Detect Register */
	u8 res1[0x8];
	u32 tiiscr;	/* Interrupt Immediate Site Capture Register */
	u32 tiascr;	/* Interrupt Average Site Capture Register */
	u32 ticscr;	/* Interrupt Critical Site Capture Register */
	u8 res2[0x4];
	u32 tmhtcr;	/* Monitor High Temperature Capture Register */
	u32 tmltcr;	/* MonitorLow Temperature Capture Register */
	u32 tmrtrcr; /* Monitor Rising Temperature Rate Capture Register */
	u32 tmftrcr; /* Monitor Falling Temperature Rate Capture Register */
	u32 tmhtitr; /* Monitor High Temperature Immediate Threshold */
	u32 tmhtatr; /* Monitor High Temperature Average Threshold */
	u32 tmhtactr; /* Monitor High Temperature Average Crit Threshold */
	u8 res3[0x4];
	u32 tmltitr; /* Monitor Low Temperature Immediate Threshold */
	u32 tmltatr; /* Monitor Low Temperature Average Threshold */
	u32 tmltactr; /* Monitor Low Temperature Average Crit Threshold */
	u8 res4[0x4];
	u32 tmrtrctr; /* Monitor Rising Temperature Rate Critical Threshold Register */
	u32 tmftrctr; /* Monitor Falling Temperature Rate Critical Threshold Register */
	u8 res5[0x8];
	u32 ttcfgr;	/* Temperature Configuration Register */
	u32 tscfgr;	/* Sensor Configuration Register */
	u8 res6[0x78];
	u32 tritsr0; /* Immediate Temperature Site Register */
	u32 tratsr0; /* Average Temperature Site Register */
	u8 res7[0xdf8];
	u32 tcmcfg;	/* Central Module Configuration */
	u8 res8[0xc];
	u32 ttrcr[16];	/* Temperature Range Control Register */
};

struct imx_tmu_regs_v2 {
	u32 ter;	/* TMU enable Register */
	u32 tsr;	/* Status Register */
	u32 tier;	/* Interrupt enable register */
	u32 tidr;	/* Interrupt detect  register */
	u32 tmhtitr;	/* Monitor high temperature immediate threshold register */
	u32 tmhtatr;	/* Monitor high temperature average threshold register */
	u32 tmhtactr;	/* TMU monitor high temperature average critical  threshold register */
	u32 tscr;	/* Sensor value capture register */
	u32 tritsr;	/* Report immediate temperature site register 0 */
	u32 tratsr;	/* Report average temperature site register 0 */
	u32 tasr;	/* Amplifier setting register */
	u32 ttmc;	/* Test MUX control */
	u32 tcaliv;
};

struct imx_tmu_regs_v3 {
	u32 ter;	/* TMU enable Register */
	u32 tps;	/* Status Register */
	u32 tier;	/* Interrupt enable register */
	u32 tidr;	/* Interrupt detect  register */
	u32 tmhtitr;	/* Monitor high temperature immediate threshold register */
	u32 tmhtatr;	/* Monitor high temperature average threshold register */
	u32 tmhtactr;	/* TMU monitor high temperature average critical  threshold register */
	u32 tscr;	/* Sensor value capture register */
	u32 tritsr;	/* Report immediate temperature site register 0 */
	u32 tratsr;	/* Report average temperature site register 0 */
	u32 tasr;	/* Amplifier setting register */
	u32 ttmc;	/* Test MUX control */
	u32 tcaliv0;
	u32 tcaliv1;
	u32 tcaliv_m40;
	u32 trim;
};

union tmu_regs {
	struct imx_tmu_regs regs_v1;
	struct imx_tmu_regs_v2 regs_v2;
	struct imx_tmu_regs_v3 regs_v3;
	struct imx_tmu_regs_v4 regs_v4;
};

struct imx_tmu_plat {
	int critical;
	int alert;
	int polling_delay;
	int id;
	bool zone_node;
	union tmu_regs *regs;
};

static int read_temperature(struct udevice *dev, int *temp)
{
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	ulong drv_data = dev_get_driver_data(dev);
	u32 val;
	u32 retry = 10;
	u32 valid = 0;

	do {
		mdelay(100);
		retry--;

		if (drv_data & FLAGS_VER3) {
			val = readl(&pdata->regs->regs_v3.tritsr);
			valid = val & (1 << (30 + pdata->id));
		} else if (drv_data & FLAGS_VER2) {
			val = readl(&pdata->regs->regs_v2.tritsr);
			/*
			 * Check if TEMP is in valid range, the V bit in TRITSR
			 * only reflects the RAW uncalibrated data
			 */
			valid =  ((val & 0xff) < 10 || (val & 0xff) > 125) ? 0 : 1;
		} else if (drv_data & FLAGS_VER4) {
			val = readl(&pdata->regs->regs_v4.tritsr0);
			valid = val & 0x80000000;
		} else {
			val = readl(&pdata->regs->regs_v1.site[pdata->id].tritsr);
			valid = val & 0x80000000;
		}
	} while (!valid && retry > 0);

	if (retry > 0) {
		if (drv_data & FLAGS_VER3) {
			val = (val >> (pdata->id * 16)) & 0xff;
			if (val & 0x80) /* Negative */
				val = (~(val & 0x7f) + 1);

			*temp = val;
			if (*temp < -40 || *temp > 125) /* Check the range */
				return -EINVAL;

			*temp *= 1000;
		} else if (drv_data & FLAGS_VER4) {
			*temp = (val & 0x1ff) * 1000;
			if (val & 0x200)
				*temp += 500;

			/* Convert Kelvin to Celsius */
			*temp -= 273000;
		} else {
			*temp = (val & 0xff) * 1000;
		}
	} else {
		return -EINVAL;
	}

	return 0;
}

int imx_tmu_get_temp(struct udevice *dev, int *temp)
{
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	int cpu_tmp = 0;
	int ret;

	ret = read_temperature(dev, &cpu_tmp);
	if (ret)
		return ret;

	while (cpu_tmp >= pdata->alert) {
		dev_crit(dev, "CPU Temperature (%dC) is beyond alert (%dC), close to critical (%dC) waiting...\n",
			 cpu_tmp / 1000, pdata->alert / 1000, pdata->critical / 1000);
		mdelay(pdata->polling_delay);
		ret = read_temperature(dev, &cpu_tmp);
		if (ret)
			return ret;
	}

	*temp = cpu_tmp / 1000;

	return 0;
}

static const struct dm_thermal_ops imx_tmu_ops = {
	.get_temp	= imx_tmu_get_temp,
};

static int imx_tmu_calibration(struct udevice *dev)
{
	int i, val, len, ret;
	int index;
	u32 range[4];
	const fdt32_t *calibration;
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	ulong drv_data = dev_get_driver_data(dev);

	dev_dbg(dev, "%s\n", __func__);

	if (drv_data & (FLAGS_VER2 | FLAGS_VER3))
		return 0;

	if (drv_data & FLAGS_VER4) {
		calibration = dev_read_prop(dev, "fsl,tmu-calibration", &len);
		if (!calibration || len % 8 || len > 128) {
			printf("TMU: invalid calibration data.\n");
			return -ENODEV;
		}

		for (i = 0; i < len; i += 8, calibration += 2) {
			index = i / 8;
			writel(index, &pdata->regs->regs_v4.ttcfgr);
			val = fdt32_to_cpu(*calibration);
			writel(val, &pdata->regs->regs_v4.tscfgr);
			val = fdt32_to_cpu(*(calibration + 1));
			writel((1 << 31) | val, &pdata->regs->regs_v4.ttrcr[index]);
		}

		return 0;
	}

	ret = dev_read_u32_array(dev, "fsl,tmu-range", range, 4);
	if (ret) {
		dev_err(dev, "TMU: missing calibration range, ret = %d.\n", ret);
		return ret;
	}

	/* Init temperature range registers */
	writel(range[0], &pdata->regs->regs_v1.ttr0cr);
	writel(range[1], &pdata->regs->regs_v1.ttr1cr);
	writel(range[2], &pdata->regs->regs_v1.ttr2cr);
	writel(range[3], &pdata->regs->regs_v1.ttr3cr);

	calibration = dev_read_prop(dev, "fsl,tmu-calibration", &len);
	if (!calibration || len % 8) {
		dev_err(dev, "TMU: invalid calibration data.\n");
		return -ENODEV;
	}

	for (i = 0; i < len; i += 8, calibration += 2) {
		val = fdt32_to_cpu(*calibration);
		writel(val, &pdata->regs->regs_v1.ttcfgr);
		val = fdt32_to_cpu(*(calibration + 1));
		writel(val, &pdata->regs->regs_v1.tscfgr);
	}

	return 0;
}

#if defined(CONFIG_IMX8MM) || defined(CONFIG_IMX8MN)
static void imx_tmu_mx8mm_mx8mn_init(struct udevice *dev)
{
	/* Load TCALIV and TASR from fuses */
	struct ocotp_regs *ocotp =
		(struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[3];
	struct fuse_bank3_regs *fuse =
		(struct fuse_bank3_regs *)bank->fuse_regs;
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	void *reg_base = (void *)pdata->regs;

	u32 tca_rt, tca_hr, tca_en;
	u32 buf_vref, buf_slope;

	tca_rt = fuse->ana0 & 0xFF;
	tca_hr = (fuse->ana0 & 0xFF00) >> 8;
	tca_en = (fuse->ana0 & 0x2000000) >> 25;

	buf_vref = (fuse->ana0 & 0x1F00000) >> 20;
	buf_slope = (fuse->ana0 & 0xF0000) >> 16;

	writel(buf_vref | (buf_slope << 16), (ulong)reg_base + 0x28);
	writel((tca_en << 31) | (tca_hr << 16) | tca_rt,
	       (ulong)reg_base + 0x30);
}
#else
static inline void imx_tmu_mx8mm_mx8mn_init(struct udevice *dev) { }
#endif

#if defined(CONFIG_IMX8MP)
static void imx_tmu_mx8mp_init(struct udevice *dev)
{
	/* Load TCALIV0/1/m40 and TRIM from fuses */
	struct ocotp_regs *ocotp = (struct ocotp_regs *)OCOTP_BASE_ADDR;
	struct fuse_bank *bank = &ocotp->bank[38];
	struct fuse_bank38_regs *fuse =
		(struct fuse_bank38_regs *)bank->fuse_regs;
	struct fuse_bank *bank2 = &ocotp->bank[39];
	struct fuse_bank39_regs *fuse2 =
		(struct fuse_bank39_regs *)bank2->fuse_regs;
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	void *reg_base = (void *)pdata->regs;
	u32 buf_vref, buf_slope, bjt_cur, vlsb, bgr;
	u32 reg;
	u32 tca40[2], tca25[2], tca105[2];

	/* For blank sample */
	if (!fuse->ana_trim2 && !fuse->ana_trim3 &&
	    !fuse->ana_trim4 && !fuse2->ana_trim5) {
		/* Use a default 25C binary codes */
		tca25[0] = 1596;
		tca25[1] = 1596;
		writel(tca25[0], (ulong)reg_base + 0x30);
		writel(tca25[1], (ulong)reg_base + 0x34);
		return;
	}

	buf_vref = (fuse->ana_trim2 & 0xc0) >> 6;
	buf_slope = (fuse->ana_trim2 & 0xF00) >> 8;
	bjt_cur = (fuse->ana_trim2 & 0xF000) >> 12;
	bgr = (fuse->ana_trim2 & 0xF0000) >> 16;
	vlsb = (fuse->ana_trim2 & 0xF00000) >> 20;
	writel(buf_vref | (buf_slope << 16), (ulong)reg_base + 0x28);

	reg = (bgr << 28) | (bjt_cur << 20) | (vlsb << 12) | (1 << 7);
	writel(reg, (ulong)reg_base + 0x3c);

	tca40[0] = (fuse->ana_trim3 & 0xFFF0000) >> 16;
	tca25[0] = (fuse->ana_trim3 & 0xF0000000) >> 28;
	tca25[0] |= ((fuse->ana_trim4 & 0xFF) << 4);
	tca105[0] = (fuse->ana_trim4 & 0xFFF00) >> 8;
	tca40[1] = (fuse->ana_trim4 & 0xFFF00000) >> 20;
	tca25[1] = fuse2->ana_trim5 & 0xFFF;
	tca105[1] = (fuse2->ana_trim5 & 0xFFF000) >> 12;

	/* use 25c for 1p calibration */
	writel(tca25[0] | (tca105[0] << 16), (ulong)reg_base + 0x30);
	writel(tca25[1] | (tca105[1] << 16), (ulong)reg_base + 0x34);
	writel(tca40[0] | (tca40[1] << 16), (ulong)reg_base + 0x38);
}
#else
static inline void imx_tmu_mx8mp_init(struct udevice *dev) { }
#endif

static inline void imx_tmu_mx93_init(struct udevice *dev) { }

static void imx_tmu_arch_init(struct udevice *dev)
{
	if (is_imx8mm() || is_imx8mn())
		imx_tmu_mx8mm_mx8mn_init(dev);
	else if (is_imx8mp())
		imx_tmu_mx8mp_init(dev);
	else if (is_imx93())
		imx_tmu_mx93_init(dev);
	else
		dev_err(dev, "Unsupported SoC, TMU calibration not loaded!\n");
}

static void imx_tmu_init(struct udevice *dev)
{
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	ulong drv_data = dev_get_driver_data(dev);

	dev_dbg(dev, "%s\n", __func__);

	if (drv_data & FLAGS_VER3) {
		/* Disable monitoring */
		writel(0x0, &pdata->regs->regs_v3.ter);

		/* Disable interrupt, using polling instead */
		writel(0x0, &pdata->regs->regs_v3.tier);

	} else if (drv_data & FLAGS_VER2) {
		/* Disable monitoring */
		writel(0x0, &pdata->regs->regs_v2.ter);

		/* Disable interrupt, using polling instead */
		writel(0x0, &pdata->regs->regs_v2.tier);
	} else if (drv_data & FLAGS_VER4) {
		/* Disable monitoring */
		writel(TMR_DISABLE, &pdata->regs->regs_v4.tmr);

		/* Disable interrupt, using polling instead */
		writel(TIER_DISABLE, &pdata->regs->regs_v4.tier);

		/* Set update_interval */
		writel(TMTMIR_DEFAULT, &pdata->regs->regs_v4.tmtmir);
	} else {
		/* Disable monitoring */
		writel(TMR_DISABLE, &pdata->regs->regs_v1.tmr);

		/* Disable interrupt, using polling instead */
		writel(TIER_DISABLE, &pdata->regs->regs_v1.tier);

		/* Set update_interval */
		writel(TMTMIR_DEFAULT, &pdata->regs->regs_v1.tmtmir);
	}

	imx_tmu_arch_init(dev);
}

static int imx_tmu_enable_msite(struct udevice *dev)
{
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	ulong drv_data = dev_get_driver_data(dev);
	u32 reg;

	dev_dbg(dev, "%s\n", __func__);

	if (!pdata->regs)
		return -EIO;

	if (drv_data & FLAGS_VER3) {
		reg = readl(&pdata->regs->regs_v3.ter);
		reg &= ~TER_EN;
		writel(reg, &pdata->regs->regs_v3.ter);

		writel(pdata->id << 30, &pdata->regs->regs_v3.tps);

		reg &= ~TER_ALPF;
		reg |= 0x1;
		reg &= ~TER_ADC_PD;
		writel(reg, &pdata->regs->regs_v3.ter);

		/* Enable monitor */
		reg |= TER_EN;
		writel(reg, &pdata->regs->regs_v3.ter);
	} else if (drv_data & FLAGS_VER2) {
		reg = readl(&pdata->regs->regs_v2.ter);
		reg &= ~TER_EN;
		writel(reg, &pdata->regs->regs_v2.ter);

		reg &= ~TER_ALPF;
		reg |= 0x1;
		writel(reg, &pdata->regs->regs_v2.ter);

		/* Enable monitor */
		reg |= TER_EN;
		writel(reg, &pdata->regs->regs_v2.ter);
	} else if (drv_data & FLAGS_VER4) {
		reg = readl(&pdata->regs->regs_v4.tcmcfg);
		reg |= (1 << 30) | (1 << 28);
		reg &= ~0xF000; /* set SAR clk =  IPG clk /16 */
		writel(reg, &pdata->regs->regs_v4.tcmcfg);

		/* Set ALPF*/
		reg = readl(&pdata->regs->regs_v4.tmr);
		reg |= TMR_ALPF;
		writel(reg, &pdata->regs->regs_v4.tmr);

		writel(1, &pdata->regs->regs_v4.tmsr);

		/* Enable ME */
		reg |= TMR_ME;
		writel(reg, &pdata->regs->regs_v4.tmr);
	} else {
		/* Clear the ME before setting MSITE and ALPF*/
		reg = readl(&pdata->regs->regs_v1.tmr);
		reg &= ~TMR_ME;
		writel(reg, &pdata->regs->regs_v1.tmr);

		reg |= 1 << (15 - pdata->id);
		reg |= TMR_ALPF;
		writel(reg, &pdata->regs->regs_v1.tmr);

		/* Enable ME */
		reg |= TMR_ME;
		writel(reg, &pdata->regs->regs_v1.tmr);
	}

	return 0;
}

static int imx_tmu_bind(struct udevice *dev)
{
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	int ret;
	ofnode node, offset;
	const char *name;
	const void *prop;
	int minc, maxc;

	dev_dbg(dev, "%s\n", __func__);

	prop = dev_read_prop(dev, "compatible", NULL);
	if (!prop)
		return 0;

	pdata->zone_node = 1;
	/* default alert/crit temps based on temp grade */
	get_cpu_temp_grade(&minc, &maxc);
	pdata->critical = maxc * 1000;
	pdata->alert = (maxc - 10) * 1000;

	node = ofnode_path("/thermal-zones");
	ofnode_for_each_subnode(offset, node) {
		/* Bind the subnode to this driver */
		name = ofnode_get_name(offset);

		ret = device_bind_with_driver_data(dev, dev->driver, name,
						   dev->driver_data, offset,
						   NULL);
		if (ret)
			dev_err(dev, "Error binding driver: %d\n", ret);
	}

	return 0;
}

static int imx_tmu_parse_fdt(struct udevice *dev)
{
	struct imx_tmu_plat *pdata = dev_get_plat(dev), *p_parent_data;
	struct ofnode_phandle_args args;
	ofnode trips_np;
	int ret;

	dev_dbg(dev, "%s\n", __func__);

	pdata->polling_delay = IMX_TMU_POLLING_DELAY_MS;

	if (pdata->zone_node) {
		pdata->regs = (union tmu_regs *)dev_read_addr_ptr(dev);

		if (!pdata->regs)
			return -EINVAL;
		return 0;
	}

	p_parent_data = dev_get_plat(dev->parent);
	if (p_parent_data->zone_node)
		pdata->regs = p_parent_data->regs;

	ret = dev_read_phandle_with_args(dev, "thermal-sensors",
					 "#thermal-sensor-cells",
					 0, 0, &args);
	if (ret)
		return ret;

	if (!ofnode_equal(args.node, dev_ofnode(dev->parent)))
		return -EFAULT;

	if (args.args_count >= 1)
		pdata->id = args.args[0];
	else
		pdata->id = 0;

	dev_dbg(dev, "args.args_count %d, id %d\n", args.args_count, pdata->id);

	pdata->polling_delay = dev_read_u32_default(dev, "polling-delay",
						    IMX_TMU_POLLING_DELAY_MS);

	trips_np = ofnode_path("/thermal-zones/cpu-thermal/trips");
	ofnode_for_each_subnode(trips_np, trips_np) {
		const char *type;

		type = ofnode_get_property(trips_np, "type", NULL);
		if (!type)
			continue;
		if (!strcmp(type, "critical"))
			pdata->critical = ofnode_read_u32_default(trips_np, "temperature", 85);
		else if (strcmp(type, "passive") == 0)
			pdata->alert = ofnode_read_u32_default(trips_np, "temperature", 80);
		else
			continue;
	}

	dev_dbg(dev, "id %d polling_delay %d, critical %d, alert %d\n",
		pdata->id, pdata->polling_delay, pdata->critical, pdata->alert);

	return 0;
}

static int imx_tmu_probe(struct udevice *dev)
{
	struct imx_tmu_plat *pdata = dev_get_plat(dev);
	int ret;

	ret = imx_tmu_parse_fdt(dev);
	if (ret) {
		dev_err(dev, "Error in parsing TMU FDT %d\n", ret);
		return ret;
	}

	if (pdata->zone_node) {
		imx_tmu_init(dev);
		imx_tmu_calibration(dev);
		imx_tmu_enable_msite(dev);
	} else {
		imx_tmu_enable_msite(dev);
	}

	return 0;
}

static const struct udevice_id imx_tmu_ids[] = {
	{ .compatible = "fsl,imx8mq-tmu", },
	{ .compatible = "fsl,imx8mm-tmu", .data = FLAGS_VER2, },
	{ .compatible = "fsl,imx8mp-tmu", .data = FLAGS_VER3, },
	{ .compatible = "fsl,imx93-tmu", .data = FLAGS_VER4, },
	{ }
};

U_BOOT_DRIVER(imx_tmu) = {
	.name	= "imx_tmu",
	.id	= UCLASS_THERMAL,
	.ops	= &imx_tmu_ops,
	.of_match = imx_tmu_ids,
	.bind = imx_tmu_bind,
	.probe	= imx_tmu_probe,
	.plat_auto	= sizeof(struct imx_tmu_plat),
	.flags  = DM_FLAG_PRE_RELOC,
};
