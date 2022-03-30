// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2010 - 2011
 * NVIDIA Corporation <www.nvidia.com>
 */

#include <common.h>
#include <asm/io.h>
#include <linux/errno.h>
#include <asm/arch/clock.h>
#include <asm/arch/emc.h>
#include <asm/arch/gp_padctrl.h>
#include <asm/arch/pinmux.h>
#include <asm/arch/sdram_param.h>
#include <asm/arch/tegra.h>
#include <asm/arch-tegra/ap.h>
#include <asm/arch-tegra/apb_misc.h>
#include <asm/arch-tegra/clk_rst.h>
#include <asm/arch-tegra/pmc.h>
#include <asm/arch-tegra/fuse.h>
#include <asm/arch-tegra/warmboot.h>

DECLARE_GLOBAL_DATA_PTR;

#ifndef CONFIG_TEGRA_CLOCK_SCALING
#error "You must enable CONFIG_TEGRA_CLOCK_SCALING to use CONFIG_TEGRA_LP0"
#endif

/*
 * This is the place in SRAM where the SDRAM parameters are stored. There
 * are 4 blocks, one for each RAM code
 */
#define SDRAM_PARAMS_BASE	(NV_PA_BASE_SRAM + 0x188)

/* TODO: If we later add support for the Misc GP controller, refactor this */
union xm2cfga_reg {
	struct {
		u32 reserved0:2;
		u32 hsm_en:1;
		u32 reserved1:2;
		u32 preemp_en:1;
		u32 vref_en:1;
		u32 reserved2:5;
		u32 cal_drvdn:5;
		u32 reserved3:3;
		u32 cal_drvup:5;
		u32 reserved4:3;
		u32 cal_drvdn_slwr:2;
		u32 cal_drvup_slwf:2;
	};
	u32 word;
};

union xm2cfgd_reg {
	struct {
		u32 reserved0:2;
		u32 hsm_en:1;
		u32 schmt_en:1;
		u32 lpmd:2;
		u32 vref_en:1;
		u32 reserved1:5;
		u32 cal_drvdn:5;
		u32 reserved2:3;
		u32 cal_drvup:5;
		u32 reserved3:3;
		u32 cal_drvdn_slwr:2;
		u32 cal_drvup_slwf:2;
	};
	u32 word;
};

/*
 * TODO: This register is not documented in the TRM yet. We could move this
 * into the EMC and give it a proper interface, but not while it is
 * undocumented.
 */
union fbio_spare_reg {
	struct {
		u32 reserved:24;
		u32 cfg_wb0:8;
	};
	u32 word;
};

/* We pack the resume information into these unions for later */
union scratch2_reg {
	struct {
		u32 pllm_base_divm:5;
		u32 pllm_base_divn:10;
		u32 pllm_base_divp:3;
		u32 pllm_misc_lfcon:4;
		u32 pllm_misc_cpcon:4;
		u32 gp_xm2cfga_padctrl_preemp:1;
		u32 gp_xm2cfgd_padctrl_schmt:1;
		u32 osc_ctrl_xobp:1;
		u32 memory_type:3;
	};
	u32 word;
};

union scratch4_reg {
	struct {
		u32 emc_clock_divider:8;
		u32 pllm_stable_time:8;
		u32 pllx_stable_time:8;
		u32 emc_fbio_spare_cfg_wb0:8;
	};
	u32 word;
};

union scratch24_reg {
	struct {
		u32 emc_auto_cal_wait:8;
		u32 emc_pin_program_wait:8;
		u32 warmboot_wait:8;
		u32 reserved:8;
	};
	u32 word;
};

int warmboot_save_sdram_params(void)
{
	u32 ram_code;
	struct sdram_params sdram;
	struct apb_misc_pp_ctlr *apb_misc =
				(struct apb_misc_pp_ctlr *)NV_PA_APB_MISC_BASE;
	struct pmc_ctlr *pmc = (struct pmc_ctlr *)NV_PA_PMC_BASE;
	struct apb_misc_gp_ctlr *gp =
			(struct apb_misc_gp_ctlr *)NV_PA_APB_MISC_GP_BASE;
	struct emc_ctlr *emc = emc_get_controller(gd->fdt_blob);
	union scratch2_reg scratch2;
	union scratch4_reg scratch4;
	union scratch24_reg scratch24;
	union xm2cfga_reg xm2cfga;
	union xm2cfgd_reg xm2cfgd;
	union fbio_spare_reg fbio_spare;

	/* get ram code that is used as index to array sdram_params in BCT */
	ram_code = (readl(&apb_misc->strapping_opt_a) >>
			  STRAP_OPT_A_RAM_CODE_SHIFT) & 3;
	memcpy(&sdram,
	       (char *)((struct sdram_params *)SDRAM_PARAMS_BASE + ram_code),
	       sizeof(sdram));

	xm2cfga.word = readl(&gp->xm2cfga);
	xm2cfgd.word = readl(&gp->xm2cfgd);

	scratch2.word = 0;
	scratch2.osc_ctrl_xobp = clock_get_osc_bypass();

	/* Get the memory PLL settings */
	{
		u32 divm, divn, divp, cpcon, lfcon;

		if (clock_ll_read_pll(CLOCK_ID_MEMORY, &divm, &divn, &divp,
					&cpcon, &lfcon))
			return -1;
		scratch2.pllm_base_divm = divm;
		scratch2.pllm_base_divn = divn;
		scratch2.pllm_base_divp = divp;
		scratch2.pllm_misc_cpcon = cpcon;
		scratch2.pllm_misc_lfcon = lfcon;
	}

	scratch2.gp_xm2cfga_padctrl_preemp = xm2cfga.preemp_en;
	scratch2.gp_xm2cfgd_padctrl_schmt = xm2cfgd.schmt_en;
	scratch2.memory_type = sdram.memory_type;
	writel(scratch2.word, &pmc->pmc_scratch2);

	/* collect data from various sources for pmc_scratch4 */
	fbio_spare.word = readl(&emc->fbio_spare);
	scratch4.word = 0;
	scratch4.emc_fbio_spare_cfg_wb0 = fbio_spare.cfg_wb0;
	scratch4.emc_clock_divider = sdram.emc_clock_divider;
	scratch4.pllm_stable_time = -1;
	scratch4.pllx_stable_time = -1;
	writel(scratch4.word, &pmc->pmc_scratch4);

	/* collect various data from sdram for pmc_scratch24 */
	scratch24.word = 0;
	scratch24.emc_pin_program_wait = sdram.emc_pin_program_wait;
	scratch24.emc_auto_cal_wait = sdram.emc_auto_cal_wait;
	scratch24.warmboot_wait = sdram.warm_boot_wait;
	writel(scratch24.word, &pmc->pmc_scratch24);

	return 0;
}

static u32 get_major_version(void)
{
	u32 major_id;
	struct apb_misc_gp_ctlr *gp =
		(struct apb_misc_gp_ctlr *)NV_PA_APB_MISC_GP_BASE;

	major_id = (readl(&gp->hidrev) & HIDREV_MAJORPREV_MASK) >>
			HIDREV_MAJORPREV_SHIFT;
	return major_id;
}

static int is_production_mode_fuse_set(struct fuse_regs *fuse)
{
	return readl(&fuse->production_mode);
}

static int is_odm_production_mode_fuse_set(struct fuse_regs *fuse)
{
	return readl(&fuse->security_mode);
}

static int is_failure_analysis_mode(struct fuse_regs *fuse)
{
	return readl(&fuse->fa);
}

static int ap20_is_odm_production_mode(void)
{
	struct fuse_regs *fuse = (struct fuse_regs *)NV_PA_FUSE_BASE;

	if (!is_failure_analysis_mode(fuse) &&
	    is_odm_production_mode_fuse_set(fuse))
		return 1;
	else
		return 0;
}

static int ap20_is_production_mode(void)
{
	struct fuse_regs *fuse = (struct fuse_regs *)NV_PA_FUSE_BASE;

	if (get_major_version() == 0)
		return 1;

	if (!is_failure_analysis_mode(fuse) &&
	    is_production_mode_fuse_set(fuse) &&
	    !is_odm_production_mode_fuse_set(fuse))
		return 1;
	else
		return 0;
}

static enum fuse_operating_mode fuse_get_operation_mode(void)
{
	u32 chip_id;
	struct apb_misc_gp_ctlr *gp =
		(struct apb_misc_gp_ctlr *)NV_PA_APB_MISC_GP_BASE;

	chip_id = (readl(&gp->hidrev) & HIDREV_CHIPID_MASK) >>
			HIDREV_CHIPID_SHIFT;
	if (chip_id == CHIPID_TEGRA20) {
		if (ap20_is_odm_production_mode()) {
			printf("!! odm_production_mode is not supported !!\n");
			return MODE_UNDEFINED;
		} else
			if (ap20_is_production_mode())
				return MODE_PRODUCTION;
			else
				return MODE_UNDEFINED;
	}
	return MODE_UNDEFINED;
}

static void determine_crypto_options(int *is_encrypted, int *is_signed,
				     int *use_zero_key)
{
	switch (fuse_get_operation_mode()) {
	case MODE_PRODUCTION:
		*is_encrypted = 0;
		*is_signed = 1;
		*use_zero_key = 1;
		break;
	case MODE_UNDEFINED:
	default:
		*is_encrypted = 0;
		*is_signed = 0;
		*use_zero_key  = 0;
		break;
	}
}

static int sign_wb_code(u32 start, u32 length, int use_zero_key)
{
	int err;
	u8 *source;		/* Pointer to source */
	u8 *hash;

	/* Calculate AES block parameters. */
	source = (u8 *)(start + offsetof(struct wb_header, random_aes_block));
	length -= offsetof(struct wb_header, random_aes_block);
	hash = (u8 *)(start + offsetof(struct wb_header, hash));
	err = sign_data_block(source, length, hash);

	return err;
}

int warmboot_prepare_code(u32 seg_address, u32 seg_length)
{
	int err = 0;
	u32 length;			/* length of the signed/encrypt code */
	struct wb_header *dst_header;	/* Pointer to dest WB header */
	int is_encrypted;		/* Segment is encrypted */
	int is_signed;			/* Segment is signed */
	int use_zero_key;		/* Use key of all zeros */

	/* Determine crypto options. */
	determine_crypto_options(&is_encrypted, &is_signed, &use_zero_key);

	/* Get the actual code limits. */
	length = roundup(((u32)wb_end - (u32)wb_start), 16);

	/*
	 * The region specified by seg_address must be in SDRAM and must be
	 * nonzero in length.
	 */
	if (seg_length == 0 || seg_address < NV_PA_SDRAM_BASE ||
		seg_address + seg_length >= NV_PA_SDRAM_BASE + gd->ram_size) {
		err = -EFAULT;
		goto fail;
	}

	/* Things must be 16-byte aligned. */
	if ((seg_length & 0xF) || (seg_address & 0xF)) {
		err = -EINVAL;
		goto fail;
	}

	/* Will the code fit? (destination includes wb_header + wb code) */
	if (seg_length < (length + sizeof(struct wb_header))) {
		err = -EINVAL;
		goto fail;
	}

	dst_header = (struct wb_header *)seg_address;
	memset((char *)dst_header, 0, sizeof(struct wb_header));

	/* Populate the random_aes_block as requested. */
	{
		u32 *aes_block = (u32 *)&(dst_header->random_aes_block);
		u32 *end = (u32 *)(((u32)aes_block) +
				   sizeof(dst_header->random_aes_block));

		do {
			*aes_block++ = 0;
		} while (aes_block < end);
	}

	/* Populate the header. */
	dst_header->length_insecure = length + sizeof(struct wb_header);
	dst_header->length_secure = length + sizeof(struct wb_header);
	dst_header->destination = NV_WB_RUN_ADDRESS;
	dst_header->entry_point = NV_WB_RUN_ADDRESS;
	dst_header->code_length = length;

	if (is_encrypted) {
		printf("!!!! Encryption is not supported !!!!\n");
		dst_header->length_insecure = 0;
		err = -EACCES;
		goto fail;
	} else
		/* copy the wb code directly following dst_header. */
		memcpy((char *)(dst_header+1), (char *)wb_start, length);

	if (is_signed)
		err = sign_wb_code(seg_address, dst_header->length_insecure,
				   use_zero_key);

fail:
	if (err)
		printf("Warning: warmboot code copy failed (error=%d)\n", err);

	return err;
}
