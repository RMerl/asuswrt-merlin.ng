// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2016 Texas Instruments, Inc.
 */

#include <common.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <malloc.h>

#include <asm/omap_common.h>
#include <asm/arch-omap5/sys_proto.h>

#ifdef CONFIG_TI_SECURE_DEVICE

/* Give zero values if not already defined */
#ifndef TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ
#define TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ (0)
#endif
#ifndef CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ
#define CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ (0)
#endif

static u32 hs_irq_skip[] = {
	8,	/* Secure violation reporting interrupt */
	15,	/* One interrupt for SDMA by secure world */
	118	/* One interrupt for Crypto DMA by secure world */
};

static int ft_hs_fixup_crossbar(void *fdt, bd_t *bd)
{
	const char *path;
	int offs;
	int ret;
	int len, i, old_cnt, new_cnt;
	u32 *temp;
	const u32 *p_data;

	/*
	 * Increase the size of the fdt
	 * so we have some breathing room
	 */
	ret = fdt_increase_size(fdt, 512);
	if (ret < 0) {
		printf("Could not increase size of device tree: %s\n",
		       fdt_strerror(ret));
		return ret;
	}

	/* Reserve IRQs that are used/needed by secure world */
	path = "/ocp/crossbar";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found.\n", path);
		return 0;
	}

	/* Get current entries */
	p_data = fdt_getprop(fdt, offs, "ti,irqs-skip", &len);
	if (p_data)
		old_cnt = len / sizeof(u32);
	else
		old_cnt = 0;

	new_cnt = sizeof(hs_irq_skip) /
				sizeof(hs_irq_skip[0]);

	/* Create new/updated skip list for HS parts */
	temp = malloc(sizeof(u32) * (old_cnt + new_cnt));
	for (i = 0; i < new_cnt; i++)
		temp[i] = cpu_to_fdt32(hs_irq_skip[i]);
	for (i = 0; i < old_cnt; i++)
		temp[i + new_cnt] = p_data[i];

	/* Blow away old data and set new data */
	fdt_delprop(fdt, offs, "ti,irqs-skip");
	ret = fdt_setprop(fdt, offs, "ti,irqs-skip",
			  temp,
			  (old_cnt + new_cnt) * sizeof(u32));
	free(temp);

	/* Check if the update worked */
	if (ret < 0) {
		printf("Could not add ti,irqs-skip property to node %s: %s\n",
		       path, fdt_strerror(ret));
		return ret;
	}

	return 0;
}

#if ((TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ != 0) || \
    (CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ != 0))
static int ft_hs_fixup_sram(void *fdt, bd_t *bd)
{
	const char *path;
	int offs;
	int ret;
	u32 temp[2];

	/*
	 * Update SRAM reservations on secure devices. The OCMC RAM
	 * is always reserved for secure use from the start of that
	 * memory region
	 */
	path = "/ocp/ocmcram@40300000/sram-hs";
	offs = fdt_path_offset(fdt, path);
	if (offs < 0) {
		debug("Node %s not found.\n", path);
		return 0;
	}

	/* relative start offset */
	temp[0] = cpu_to_fdt32(0);
	/* reservation size */
	temp[1] = cpu_to_fdt32(max(TI_OMAP5_SECURE_BOOT_RESV_SRAM_SZ,
				   CONFIG_SECURE_RUNTIME_RESV_SRAM_SZ));
	fdt_delprop(fdt, offs, "reg");
	ret = fdt_setprop(fdt, offs, "reg", temp, 2 * sizeof(u32));
	if (ret < 0) {
		printf("Could not add reg property to node %s: %s\n",
		       path, fdt_strerror(ret));
		return ret;
	}

	return 0;
}
#else
static int ft_hs_fixup_sram(void *fdt, bd_t *bd) { return 0; }
#endif

static void ft_hs_fixups(void *fdt, bd_t *bd)
{
	/* Check we are running on an HS/EMU device type */
	if (GP_DEVICE != get_device_type()) {
		if ((ft_hs_fixup_crossbar(fdt, bd) == 0) &&
		    (ft_hs_disable_rng(fdt, bd) == 0) &&
		    (ft_hs_fixup_sram(fdt, bd) == 0) &&
		    (ft_hs_fixup_dram(fdt, bd) == 0) &&
		    (ft_hs_add_tee(fdt, bd) == 0))
			return;
	} else {
		printf("ERROR: Incorrect device type (GP) detected!");
	}
	/* Fixup failed or wrong device type */
	hang();
}
#else
static void ft_hs_fixups(void *fdt, bd_t *bd)
{
}
#endif /* #ifdef CONFIG_TI_SECURE_DEVICE */

#if defined(CONFIG_TARGET_DRA7XX_EVM) || defined(CONFIG_TARGET_AM57XX_EVM)
#define OPP_DSP_CLK_NUM	3
#define OPP_IVA_CLK_NUM	2
#define OPP_GPU_CLK_NUM	2

const char *dra7_opp_dsp_clk_names[OPP_DSP_CLK_NUM] = {
	"dpll_dsp_ck",
	"dpll_dsp_m2_ck",
	"dpll_dsp_m3x2_ck",
};

const char *dra7_opp_iva_clk_names[OPP_IVA_CLK_NUM] = {
	"dpll_iva_ck",
	"dpll_iva_m2_ck",
};

const char *dra7_opp_gpu_clk_names[OPP_GPU_CLK_NUM] = {
	"dpll_gpu_ck",
	"dpll_gpu_m2_ck",
};

/* DSPEVE voltage domain */
u32 dra7_opp_dsp_clk_rates[NUM_OPPS][OPP_DSP_CLK_NUM] = {
	{}, /*OPP_LOW */
	{600000000, 600000000, 400000000}, /* OPP_NOM */
	{700000000, 700000000, 466666667}, /* OPP_OD */
	{750000000, 750000000, 500000000}, /* OPP_HIGH */
};

/* IVA voltage domain */
u32 dra7_opp_iva_clk_rates[NUM_OPPS][OPP_IVA_CLK_NUM] = {
	{}, /* OPP_LOW */
	{1165000000, 388333334}, /* OPP_NOM */
	{860000000, 430000000}, /* OPP_OD */
	{1064000000, 532000000}, /* OPP_HIGH */
};

/* GPU voltage domain */
u32 dra7_opp_gpu_clk_rates[NUM_OPPS][OPP_GPU_CLK_NUM] = {
	{}, /* OPP_LOW */
	{1277000000, 425666667}, /* OPP_NOM */
	{1000000000, 500000000}, /* OPP_OD */
	{1064000000, 532000000}, /* OPP_HIGH */
};

static int ft_fixup_clocks(void *fdt, const char **names, u32 *rates, int num)
{
	int offs, node_offs, ret, i;
	uint32_t phandle;

	offs = fdt_path_offset(fdt, "/ocp/l4@4a000000/cm_core_aon@5000/clocks");
	if (offs < 0) {
		debug("Could not find cm_core_aon clocks node path offset : %s\n",
		      fdt_strerror(offs));
		return offs;
	}

	for (i = 0; i < num; i++) {
		node_offs = fdt_subnode_offset(fdt, offs, names[i]);
		if (node_offs < 0) {
			debug("Could not find clock sub-node %s: %s\n",
			      names[i], fdt_strerror(node_offs));
			return offs;
		}

		phandle = fdt_get_phandle(fdt, node_offs);
		if (!phandle) {
			debug("Could not find phandle for clock %s\n",
			      names[i]);
			return -1;
		}

		ret = fdt_setprop_u32(fdt, node_offs, "assigned-clocks",
				      phandle);
		if (ret < 0) {
			debug("Could not add assigned-clocks property to clock node %s: %s\n",
			      names[i], fdt_strerror(ret));
			return ret;
		}

		ret = fdt_setprop_u32(fdt, node_offs, "assigned-clock-rates",
				      rates[i]);
		if (ret < 0) {
			debug("Could not add assigned-clock-rates property to clock node %s: %s\n",
			      names[i], fdt_strerror(ret));
			return ret;
		}
	}

	return 0;
}

static void ft_opp_clock_fixups(void *fdt, bd_t *bd)
{
	const char **clk_names;
	u32 *clk_rates;
	int ret;

	if (!is_dra72x() && !is_dra7xx())
		return;

	/* fixup DSP clocks */
	clk_names = dra7_opp_dsp_clk_names;
	clk_rates = dra7_opp_dsp_clk_rates[get_voltrail_opp(VOLT_EVE)];
	ret = ft_fixup_clocks(fdt, clk_names, clk_rates, OPP_DSP_CLK_NUM);
	if (ret) {
		printf("ft_fixup_clocks failed for DSP voltage domain: %s\n",
		       fdt_strerror(ret));
		return;
	}

	/* fixup IVA clocks */
	clk_names = dra7_opp_iva_clk_names;
	clk_rates = dra7_opp_iva_clk_rates[get_voltrail_opp(VOLT_IVA)];
	ret = ft_fixup_clocks(fdt, clk_names, clk_rates, OPP_IVA_CLK_NUM);
	if (ret) {
		printf("ft_fixup_clocks failed for IVA voltage domain: %s\n",
		       fdt_strerror(ret));
		return;
	}

	/* fixup GPU clocks */
	clk_names = dra7_opp_gpu_clk_names;
	clk_rates = dra7_opp_gpu_clk_rates[get_voltrail_opp(VOLT_GPU)];
	ret = ft_fixup_clocks(fdt, clk_names, clk_rates, OPP_GPU_CLK_NUM);
	if (ret) {
		printf("ft_fixup_clocks failed for GPU voltage domain: %s\n",
		       fdt_strerror(ret));
		return;
	}
}
#else
static void ft_opp_clock_fixups(void *fdt, bd_t *bd) { }
#endif /* CONFIG_TARGET_DRA7XX_EVM || CONFIG_TARGET_AM57XX_EVM */

/*
 * Place for general cpu/SoC FDT fixups. Board specific
 * fixups should remain in the board files which is where
 * this function should be called from.
 */
void ft_cpu_setup(void *fdt, bd_t *bd)
{
	ft_hs_fixups(fdt, bd);
	ft_opp_clock_fixups(fdt, bd);
}
