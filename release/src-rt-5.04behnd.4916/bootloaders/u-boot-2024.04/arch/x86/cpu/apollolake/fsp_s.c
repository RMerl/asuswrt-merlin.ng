// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Google LLC
 * Written by Simon Glass <sjg@chromium.org>
 */

#include <common.h>
#include <binman.h>
#include <bootstage.h>
#include <dm.h>
#include <init.h>
#include <irq.h>
#include <log.h>
#include <malloc.h>
#include <p2sb.h>
#include <acpi/acpi_s3.h>
#include <asm/global_data.h>
#include <asm/intel_pinctrl.h>
#include <asm/io.h>
#include <asm/intel_regs.h>
#include <asm/msr.h>
#include <asm/msr-index.h>
#include <asm/pci.h>
#include <asm/arch/cpu.h>
#include <asm/arch/systemagent.h>
#include <asm/arch/fsp_bindings.h>
#include <asm/arch/fsp/fsp_configs.h>
#include <asm/arch/fsp/fsp_s_upd.h>
#include <dm/uclass-internal.h>
#include <linux/bitops.h>

#define PCH_P2SB_E0		0xe0
#define HIDE_BIT		BIT(0)

int fsps_update_config(struct udevice *dev, ulong rom_offset,
		       struct fsps_upd *upd)
{
	struct fsp_s_config *cfg = &upd->config;
	ofnode node;

	if (IS_ENABLED(CONFIG_HAVE_VBT)) {
		void *buf;
		int ret;

		ret = binman_entry_map(ofnode_null(), "intel-vbt", &buf, NULL);
		if (ret)
			return log_msg_ret("Cannot find VBT", ret);
		if (*(u32 *)buf != VBT_SIGNATURE)
			return log_msg_ret("VBT signature", -EINVAL);

		/*
		 * Load VBT before devicetree-specific config. This only
		 * supports memory-mapped SPI at present.
		 */
		cfg->graphics_config_ptr = (ulong)buf;
	}

	node = dev_read_subnode(dev, "fsp-s");
	if (!ofnode_valid(node))
		return log_msg_ret("fsp-s settings", -ENOENT);

	return fsp_s_update_config_from_dtb(node, cfg);
}

/* Configure package power limits */
static int set_power_limits(struct udevice *dev)
{
	msr_t rapl_msr_reg, limit;
	u32 power_unit;
	u32 tdp, min_power, max_power;
	u32 pl2_val;
	u32 override_tdp[2];
	int ret;

	/* Get units */
	rapl_msr_reg = msr_read(MSR_PKG_POWER_SKU_UNIT);
	power_unit = 1 << (rapl_msr_reg.lo & 0xf);

	/* Get power defaults for this SKU */
	rapl_msr_reg = msr_read(MSR_PKG_POWER_SKU);
	tdp = rapl_msr_reg.lo & PKG_POWER_LIMIT_MASK;
	pl2_val = rapl_msr_reg.hi & PKG_POWER_LIMIT_MASK;
	min_power = (rapl_msr_reg.lo >> 16) & PKG_POWER_LIMIT_MASK;
	max_power = rapl_msr_reg.hi & PKG_POWER_LIMIT_MASK;

	if (min_power > 0 && tdp < min_power)
		tdp = min_power;

	if (max_power > 0 && tdp > max_power)
		tdp = max_power;

	ret = dev_read_u32_array(dev, "tdp-pl-override-mw", override_tdp,
				 ARRAY_SIZE(override_tdp));
	if (ret)
		return log_msg_ret("tdp-pl-override-mw", ret);

	/* Set PL1 override value */
	if (override_tdp[0])
		tdp = override_tdp[0] * power_unit / 1000;

	/* Set PL2 override value */
	if (override_tdp[1])
		pl2_val = override_tdp[1] * power_unit / 1000;

	/* Set long term power limit to TDP */
	limit.lo = tdp & PKG_POWER_LIMIT_MASK;
	/* Set PL1 Pkg Power clamp bit */
	limit.lo |= PKG_POWER_LIMIT_CLAMP;

	limit.lo |= PKG_POWER_LIMIT_EN;
	limit.lo |= (MB_POWER_LIMIT1_TIME_DEFAULT &
		PKG_POWER_LIMIT_TIME_MASK) << PKG_POWER_LIMIT_TIME_SHIFT;

	/* Set short term power limit PL2 */
	limit.hi = pl2_val & PKG_POWER_LIMIT_MASK;
	limit.hi |= PKG_POWER_LIMIT_EN;

	/* Program package power limits in RAPL MSR */
	msr_write(MSR_PKG_POWER_LIMIT, limit);
	log_debug("RAPL PL1 %d.%dW\n", tdp / power_unit,
		  100 * (tdp % power_unit) / power_unit);
	log_debug("RAPL PL2 %d.%dW\n", pl2_val / power_unit,
		  100 * (pl2_val % power_unit) / power_unit);

	/*
	 * Sett RAPL MMIO register for Power limits. RAPL driver is using MSR
	 * instead of MMIO, so disable LIMIT_EN bit for MMIO
	 */
	writel(limit.lo & ~PKG_POWER_LIMIT_EN, MCHBAR_REG(MCHBAR_RAPL_PPL));
	writel(limit.hi & ~PKG_POWER_LIMIT_EN, MCHBAR_REG(MCHBAR_RAPL_PPL + 4));

	return 0;
}

int p2sb_unhide(void)
{
	struct udevice *dev;
	int ret;

	ret = uclass_find_first_device(UCLASS_P2SB, &dev);
	if (ret)
		return log_msg_ret("p2sb", ret);
	ret = p2sb_set_hide(dev, false);
	if (ret)
		return log_msg_ret("hide", ret);

	return 0;
}

/* Overwrites the SCI IRQ if another IRQ number is given by device tree */
static void set_sci_irq(void)
{
	/* Skip this for now */
}

int arch_fsps_preinit(void)
{
	struct udevice *itss;
	int ret;

	if (!ll_boot_init())
		return 0;
	ret = irq_first_device_type(X86_IRQT_ITSS, &itss);
	if (ret)
		return log_msg_ret("no itss", ret);

	/*
	 * Clear the GPI interrupt status and enable registers. These
	 * registers do not get reset to default state when booting from S5.
	 */
	ret = pinctrl_gpi_clear_int_cfg();
	if (ret)
		return log_msg_ret("gpi_clear", ret);

	return 0;
}

int arch_fsp_init_r(void)
{
	bool s3wake;
	struct udevice *dev, *itss;
	int ret;

	if (!ll_boot_init())
		return 0;

	s3wake = IS_ENABLED(CONFIG_HAVE_ACPI_RESUME) &&
		gd->arch.prev_sleep_state == ACPI_S3;

	/*
	 * This must be called before any devices are probed. Put any probing
	 * into arch_fsps_preinit() above.
	 *
	 * We don't use CONFIG_APL_BOOT_FROM_FAST_SPI_FLASH here since it will
	 * force PCI to be probed.
	 */
	ret = fsp_silicon_init(s3wake, false);
	if (ret)
		return ret;

	ret = irq_first_device_type(X86_IRQT_ITSS, &itss);
	if (ret)
		return log_msg_ret("no itss", ret);

	/*
	 * Restore GPIO IRQ polarities back to previous settings. This was
	 * stored in reserve_arch() - see X86_IRQT_ITSS
	 */
	irq_restore_polarities(itss);

	/* soc_init() */
	ret = p2sb_unhide();
	if (ret)
		return log_msg_ret("unhide p2sb", ret);

	/* Set RAPL MSR for Package power limits*/
	ret = uclass_first_device_err(UCLASS_NORTHBRIDGE, &dev);
	if (ret)
		return log_msg_ret("Cannot get northbridge", ret);
	set_power_limits(dev);

	/*
	 * FSP-S routes SCI to IRQ 9. With the help of this function you can
	 * select another IRQ for SCI.
	 */
	set_sci_irq();

	return 0;
}
