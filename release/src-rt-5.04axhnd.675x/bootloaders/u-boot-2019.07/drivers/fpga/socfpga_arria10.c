// SPDX-License-Identifier: GPL-2.0
/*
 * Copyright (C) 2017-2019 Intel Corporation <www.intel.com>
 */
#include <asm/io.h>
#include <asm/arch/fpga_manager.h>
#include <asm/arch/reset_manager.h>
#include <asm/arch/system_manager.h>
#include <asm/arch/sdram.h>
#include <asm/arch/misc.h>
#include <altera.h>
#include <asm/arch/pinmux.h>
#include <common.h>
#include <dm/ofnode.h>
#include <errno.h>
#include <fs_loader.h>
#include <wait_bit.h>
#include <watchdog.h>

#define CFGWDTH_32	1
#define MIN_BITSTREAM_SIZECHECK	230
#define ENCRYPTION_OFFSET	69
#define COMPRESSION_OFFSET	229
#define FPGA_TIMEOUT_MSEC	1000  /* timeout in ms */
#define FPGA_TIMEOUT_CNT	0x1000000
#define DEFAULT_DDR_LOAD_ADDRESS	0x400

DECLARE_GLOBAL_DATA_PTR;

static const struct socfpga_fpga_manager *fpga_manager_base =
		(void *)SOCFPGA_FPGAMGRREGS_ADDRESS;

static const struct socfpga_system_manager *system_manager_base =
		(void *)SOCFPGA_SYSMGR_ADDRESS;

static void fpgamgr_set_cd_ratio(unsigned long ratio);

static uint32_t fpgamgr_get_msel(void)
{
	u32 reg;

	reg = readl(&fpga_manager_base->imgcfg_stat);
	reg = (reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL_SET_MSD) >>
		ALT_FPGAMGR_IMGCFG_STAT_F2S_MSEL0_LSB;

	return reg;
}

static void fpgamgr_set_cfgwdth(int width)
{
	if (width)
		setbits_le32(&fpga_manager_base->imgcfg_ctrl_02,
			ALT_FPGAMGR_IMGCFG_CTL_02_CFGWIDTH_SET_MSK);
	else
		clrbits_le32(&fpga_manager_base->imgcfg_ctrl_02,
			ALT_FPGAMGR_IMGCFG_CTL_02_CFGWIDTH_SET_MSK);
}

int is_fpgamgr_user_mode(void)
{
	return (readl(&fpga_manager_base->imgcfg_stat) &
		ALT_FPGAMGR_IMGCFG_STAT_F2S_USERMODE_SET_MSK) != 0;
}

static int wait_for_user_mode(void)
{
	return wait_for_bit_le32(&fpga_manager_base->imgcfg_stat,
		ALT_FPGAMGR_IMGCFG_STAT_F2S_USERMODE_SET_MSK,
		1, FPGA_TIMEOUT_MSEC, false);
}

int is_fpgamgr_early_user_mode(void)
{
	return (readl(&fpga_manager_base->imgcfg_stat) &
		ALT_FPGAMGR_IMGCFG_STAT_F2S_EARLY_USERMODE_SET_MSK) != 0;
}

int fpgamgr_wait_early_user_mode(void)
{
	u32 sync_data = 0xffffffff;
	u32 i = 0;
	unsigned start = get_timer(0);
	unsigned long cd_ratio;

	/* Getting existing CDRATIO */
	cd_ratio = (readl(&fpga_manager_base->imgcfg_ctrl_02) &
		ALT_FPGAMGR_IMGCFG_CTL_02_CDRATIO_SET_MSK) >>
		ALT_FPGAMGR_IMGCFG_CTL_02_CDRATIO_LSB;

	/* Using CDRATIO_X1 for better compatibility */
	fpgamgr_set_cd_ratio(CDRATIO_x1);

	while (!is_fpgamgr_early_user_mode()) {
		if (get_timer(start) > FPGA_TIMEOUT_MSEC)
			return -ETIMEDOUT;
		fpgamgr_program_write((const long unsigned int *)&sync_data,
				sizeof(sync_data));
		udelay(FPGA_TIMEOUT_MSEC);
		i++;
	}

	debug("FPGA: Additional %i sync word needed\n", i);

	/* restoring original CDRATIO */
	fpgamgr_set_cd_ratio(cd_ratio);

	return 0;
}

/* Read f2s_nconfig_pin and f2s_nstatus_pin; loop until de-asserted */
static int wait_for_nconfig_pin_and_nstatus_pin(void)
{
	unsigned long mask = ALT_FPGAMGR_IMGCFG_STAT_F2S_NCONFIG_PIN_SET_MSK |
				ALT_FPGAMGR_IMGCFG_STAT_F2S_NSTATUS_PIN_SET_MSK;

	/*
	 * Poll until f2s_nconfig_pin and f2s_nstatus_pin; loop until
	 * de-asserted, timeout at 1000ms
	 */
	return wait_for_bit_le32(&fpga_manager_base->imgcfg_stat, mask,
				 true, FPGA_TIMEOUT_MSEC, false);
}

static int wait_for_f2s_nstatus_pin(unsigned long value)
{
	/* Poll until f2s to specific value, timeout at 1000ms */
	return wait_for_bit_le32(&fpga_manager_base->imgcfg_stat,
		ALT_FPGAMGR_IMGCFG_STAT_F2S_NSTATUS_PIN_SET_MSK,
		value, FPGA_TIMEOUT_MSEC, false);
}

/* set CD ratio */
static void fpgamgr_set_cd_ratio(unsigned long ratio)
{
	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_02,
		ALT_FPGAMGR_IMGCFG_CTL_02_CDRATIO_SET_MSK);

	setbits_le32(&fpga_manager_base->imgcfg_ctrl_02,
		(ratio << ALT_FPGAMGR_IMGCFG_CTL_02_CDRATIO_LSB) &
		ALT_FPGAMGR_IMGCFG_CTL_02_CDRATIO_SET_MSK);
}

/* get the MSEL value, verify we are set for FPP configuration mode */
static int fpgamgr_verify_msel(void)
{
	u32 msel = fpgamgr_get_msel();

	if (msel & ~BIT(0)) {
		printf("Fail: read msel=%d\n", msel);
		return -EPERM;
	}

	return 0;
}

/*
 * Write cdratio and cdwidth based on whether the bitstream is compressed
 * and/or encoded
 */
static int fpgamgr_set_cdratio_cdwidth(unsigned int cfg_width, u32 *rbf_data,
				       size_t rbf_size)
{
	unsigned int cd_ratio;
	bool encrypt, compress;

	/*
         * According to the bitstream specification,
	 * both encryption and compression status are
         * in location before offset 230 of the buffer.
         */
	if (rbf_size < MIN_BITSTREAM_SIZECHECK)
		return -EINVAL;

	encrypt = (rbf_data[ENCRYPTION_OFFSET] >> 2) & 3;
	encrypt = encrypt != 0;

	compress = (rbf_data[COMPRESSION_OFFSET] >> 1) & 1;
	compress = !compress;

	debug("FPGA: Header word %d = %08x.\n", 69, rbf_data[69]);
	debug("FPGA: Header word %d = %08x.\n", 229, rbf_data[229]);
	debug("FPGA: Read from rbf header: encrypt=%d compress=%d.\n", encrypt,
	     compress);

	/*
	 * from the register map description of cdratio in imgcfg_ctrl_02:
	 *  Normal Configuration    : 32bit Passive Parallel
	 *  Partial Reconfiguration : 16bit Passive Parallel
	 */

	/*
	 * cd ratio is dependent on cfg width and whether the bitstream
	 * is encrypted and/or compressed.
	 *
	 * | width | encr. | compr. | cd ratio |
	 * |  16   |   0   |   0    |     1    |
	 * |  16   |   0   |   1    |     4    |
	 * |  16   |   1   |   0    |     2    |
	 * |  16   |   1   |   1    |     4    |
	 * |  32   |   0   |   0    |     1    |
	 * |  32   |   0   |   1    |     8    |
	 * |  32   |   1   |   0    |     4    |
	 * |  32   |   1   |   1    |     8    |
	 */
	if (!compress && !encrypt) {
		cd_ratio = CDRATIO_x1;
	} else {
		if (compress)
			cd_ratio = CDRATIO_x4;
		else
			cd_ratio = CDRATIO_x2;

		/* if 32 bit, double the cd ratio (so register
		   field setting is incremented) */
		if (cfg_width == CFGWDTH_32)
			cd_ratio += 1;
	}

	fpgamgr_set_cfgwdth(cfg_width);
	fpgamgr_set_cd_ratio(cd_ratio);

	return 0;
}

static int fpgamgr_reset(void)
{
	unsigned long reg;

	/* S2F_NCONFIG = 0 */
	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_00,
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NCONFIG_SET_MSK);

	/* Wait for f2s_nstatus == 0 */
	if (wait_for_f2s_nstatus_pin(0))
		return -ETIME;

	/* S2F_NCONFIG = 1 */
	setbits_le32(&fpga_manager_base->imgcfg_ctrl_00,
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NCONFIG_SET_MSK);

	/* Wait for f2s_nstatus == 1 */
	if (wait_for_f2s_nstatus_pin(1))
		return -ETIME;

	/* read and confirm f2s_condone_pin = 0 and f2s_condone_oe = 1 */
	reg = readl(&fpga_manager_base->imgcfg_stat);
	if ((reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_CONDONE_PIN_SET_MSK) != 0)
		return -EPERM;

	if ((reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_CONDONE_OE_SET_MSK) == 0)
		return -EPERM;

	return 0;
}

/* Start the FPGA programming by initialize the FPGA Manager */
int fpgamgr_program_init(u32 * rbf_data, size_t rbf_size)
{
	int ret;

	/* Step 1 */
	if (fpgamgr_verify_msel())
		return -EPERM;

	/* Step 2 */
	if (fpgamgr_set_cdratio_cdwidth(CFGWDTH_32, rbf_data, rbf_size))
		return -EPERM;

	/*
	 * Step 3:
	 * Make sure no other external devices are trying to interfere with
	 * programming:
	 */
	if (wait_for_nconfig_pin_and_nstatus_pin())
		return -ETIME;

	/*
	 * Step 4:
	 * Deassert the signal drives from HPS
	 *
	 * S2F_NCE = 1
	 * S2F_PR_REQUEST = 0
	 * EN_CFG_CTRL = 0
	 * EN_CFG_DATA = 0
	 * S2F_NCONFIG = 1
	 * S2F_NSTATUS_OE = 0
	 * S2F_CONDONE_OE = 0
	 */
	setbits_le32(&fpga_manager_base->imgcfg_ctrl_01,
		ALT_FPGAMGR_IMGCFG_CTL_01_S2F_NCE_SET_MSK);

	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_01,
		ALT_FPGAMGR_IMGCFG_CTL_01_S2F_PR_REQUEST_SET_MSK);

	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_02,
		ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_DATA_SET_MSK |
		ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_CTRL_SET_MSK);

	setbits_le32(&fpga_manager_base->imgcfg_ctrl_00,
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NCONFIG_SET_MSK);

	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_00,
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NSTATUS_OE_SET_MSK |
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_CONDONE_OE_SET_MSK);

	/*
	 * Step 5:
	 * Enable overrides
	 * S2F_NENABLE_CONFIG = 0
	 * S2F_NENABLE_NCONFIG = 0
	 */
	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_01,
		ALT_FPGAMGR_IMGCFG_CTL_01_S2F_NENABLE_CONFIG_SET_MSK);
	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_00,
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NENABLE_NCONFIG_SET_MSK);

	/*
	 * Disable driving signals that HPS doesn't need to drive.
	 * S2F_NENABLE_NSTATUS = 1
	 * S2F_NENABLE_CONDONE = 1
	 */
	setbits_le32(&fpga_manager_base->imgcfg_ctrl_00,
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NENABLE_NSTATUS_SET_MSK |
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NENABLE_CONDONE_SET_MSK);

	/*
	 * Step 6:
	 * Drive chip select S2F_NCE = 0
	 */
	 clrbits_le32(&fpga_manager_base->imgcfg_ctrl_01,
		ALT_FPGAMGR_IMGCFG_CTL_01_S2F_NCE_SET_MSK);

	/* Step 7 */
	if (wait_for_nconfig_pin_and_nstatus_pin())
		return -ETIME;

	/* Step 8 */
	ret = fpgamgr_reset();

	if (ret)
		return ret;

	/*
	 * Step 9:
	 * EN_CFG_CTRL and EN_CFG_DATA = 1
	 */
	setbits_le32(&fpga_manager_base->imgcfg_ctrl_02,
		ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_DATA_SET_MSK |
		ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_CTRL_SET_MSK);

	return 0;
}

/* Ensure the FPGA entering config done */
static int fpgamgr_program_poll_cd(void)
{
	unsigned long reg, i;

	for (i = 0; i < FPGA_TIMEOUT_CNT; i++) {
		reg = readl(&fpga_manager_base->imgcfg_stat);
		if (reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_CONDONE_PIN_SET_MSK)
			return 0;

		if ((reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_NSTATUS_PIN_SET_MSK) == 0) {
			printf("nstatus == 0 while waiting for condone\n");
			return -EPERM;
		}
		WATCHDOG_RESET();
	}

	if (i == FPGA_TIMEOUT_CNT)
		return -ETIME;

	return 0;
}

/* Ensure the FPGA entering user mode */
static int fpgamgr_program_poll_usermode(void)
{
	unsigned long reg;
	int ret = 0;

	if (fpgamgr_dclkcnt_set(0xf))
		return -ETIME;

	ret = wait_for_user_mode();
	if (ret < 0) {
		printf("%s: Failed to enter user mode with ", __func__);
		printf("error code %d\n", ret);
		return ret;
	}

	/*
	 * Step 14:
	 * Stop DATA path and Dclk
	 * EN_CFG_CTRL and EN_CFG_DATA = 0
	 */
	clrbits_le32(&fpga_manager_base->imgcfg_ctrl_02,
		ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_DATA_SET_MSK |
		ALT_FPGAMGR_IMGCFG_CTL_02_EN_CFG_CTRL_SET_MSK);

	/*
	 * Step 15:
	 * Disable overrides
	 * S2F_NENABLE_CONFIG = 1
	 * S2F_NENABLE_NCONFIG = 1
	 */
	setbits_le32(&fpga_manager_base->imgcfg_ctrl_01,
		ALT_FPGAMGR_IMGCFG_CTL_01_S2F_NENABLE_CONFIG_SET_MSK);
	setbits_le32(&fpga_manager_base->imgcfg_ctrl_00,
		ALT_FPGAMGR_IMGCFG_CTL_00_S2F_NENABLE_NCONFIG_SET_MSK);

	/* Disable chip select S2F_NCE = 1 */
	setbits_le32(&fpga_manager_base->imgcfg_ctrl_01,
		ALT_FPGAMGR_IMGCFG_CTL_01_S2F_NCE_SET_MSK);

	/*
	 * Step 16:
	 * Final check
	 */
	reg = readl(&fpga_manager_base->imgcfg_stat);
	if (((reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_USERMODE_SET_MSK) !=
		ALT_FPGAMGR_IMGCFG_STAT_F2S_USERMODE_SET_MSK) ||
	    ((reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_CONDONE_PIN_SET_MSK) !=
		ALT_FPGAMGR_IMGCFG_STAT_F2S_CONDONE_PIN_SET_MSK) ||
	    ((reg & ALT_FPGAMGR_IMGCFG_STAT_F2S_NSTATUS_PIN_SET_MSK) !=
		ALT_FPGAMGR_IMGCFG_STAT_F2S_NSTATUS_PIN_SET_MSK))
		return -EPERM;

	return 0;
}

int fpgamgr_program_finish(void)
{
	/* Ensure the FPGA entering config done */
	int status = fpgamgr_program_poll_cd();

	if (status) {
		printf("FPGA: Poll CD failed with error code %d\n", status);
		return -EPERM;
	}

	/* Ensure the FPGA entering user mode */
	status = fpgamgr_program_poll_usermode();
	if (status) {
		printf("FPGA: Poll usermode failed with error code %d\n",
			status);
		return -EPERM;
	}

	printf("Full Configuration Succeeded.\n");

	return 0;
}

ofnode get_fpga_mgr_ofnode(ofnode from)
{
	return ofnode_by_compatible(from, "altr,socfpga-a10-fpga-mgr");
}

const char *get_fpga_filename(void)
{
	const char *fpga_filename = NULL;

	ofnode fpgamgr_node = get_fpga_mgr_ofnode(ofnode_null());

	if (ofnode_valid(fpgamgr_node))
		fpga_filename = ofnode_read_string(fpgamgr_node,
						"altr,bitstream");

	return fpga_filename;
}

static void get_rbf_image_info(struct rbf_info *rbf, u16 *buffer)
{
	/*
	 * Magic ID starting at:
	 * -> 1st dword[15:0] in periph.rbf
	 * -> 2nd dword[15:0] in core.rbf
	 * Note: dword == 32 bits
	 */
	u32 word_reading_max = 2;
	u32 i;

	for (i = 0; i < word_reading_max; i++) {
		if (*(buffer + i) == FPGA_SOCFPGA_A10_RBF_UNENCRYPTED) {
			rbf->security = unencrypted;
		} else if (*(buffer + i) == FPGA_SOCFPGA_A10_RBF_ENCRYPTED) {
			rbf->security = encrypted;
		} else if (*(buffer + i + 1) ==
				FPGA_SOCFPGA_A10_RBF_UNENCRYPTED) {
			rbf->security = unencrypted;
		} else if (*(buffer + i + 1) ==
				FPGA_SOCFPGA_A10_RBF_ENCRYPTED) {
			rbf->security = encrypted;
		} else {
			rbf->security = invalid;
			continue;
		}

		/* PERIPH RBF(buffer + i + 1), CORE RBF(buffer + i + 2) */
		if (*(buffer + i + 1) == FPGA_SOCFPGA_A10_RBF_PERIPH) {
			rbf->section = periph_section;
			break;
		} else if (*(buffer + i + 1) == FPGA_SOCFPGA_A10_RBF_CORE) {
			rbf->section = core_section;
			break;
		} else if (*(buffer + i + 2) == FPGA_SOCFPGA_A10_RBF_PERIPH) {
			rbf->section = periph_section;
			break;
		} else if (*(buffer + i + 2) == FPGA_SOCFPGA_A10_RBF_CORE) {
			rbf->section = core_section;
			break;
		}

		rbf->section = unknown;
		break;

		WATCHDOG_RESET();
	}
}

#ifdef CONFIG_FS_LOADER
static int first_loading_rbf_to_buffer(struct udevice *dev,
				struct fpga_loadfs_info *fpga_loadfs,
				u32 *buffer, size_t *buffer_bsize)
{
	u32 *buffer_p = (u32 *)*buffer;
	u32 *loadable = buffer_p;
	size_t buffer_size = *buffer_bsize;
	size_t fit_size;
	int ret, i, count, confs_noffset, images_noffset, rbf_offset, rbf_size;
	const char *fpga_node_name = NULL;
	const char *uname = NULL;

	/* Load image header into buffer */
	ret = request_firmware_into_buf(dev,
					fpga_loadfs->fpga_fsinfo->filename,
					buffer_p, sizeof(struct image_header),
					0);
	if (ret < 0) {
		debug("FPGA: Failed to read image header from flash.\n");
		return -ENOENT;
	}

	if (image_get_magic((struct image_header *)buffer_p) != FDT_MAGIC) {
		debug("FPGA: No FDT magic was found.\n");
		return -EBADF;
	}

	fit_size = fdt_totalsize(buffer_p);

	if (fit_size > buffer_size) {
		debug("FPGA: FIT image is larger than available buffer.\n");
		debug("Please use FIT external data or increasing buffer.\n");
		return -ENOMEM;
	}

	/* Load entire FIT into buffer */
	ret = request_firmware_into_buf(dev,
					fpga_loadfs->fpga_fsinfo->filename,
					buffer_p, fit_size, 0);
	if (ret < 0)
		return ret;

	ret = fit_check_format(buffer_p);
	if (!ret) {
		debug("FPGA: No valid FIT image was found.\n");
		return -EBADF;
	}

	confs_noffset = fdt_path_offset(buffer_p, FIT_CONFS_PATH);
	images_noffset = fdt_path_offset(buffer_p, FIT_IMAGES_PATH);
	if (confs_noffset < 0 || images_noffset < 0) {
		debug("FPGA: No Configurations or images nodes were found.\n");
		return -ENOENT;
	}

	/* Get default configuration unit name from default property */
	confs_noffset = fit_conf_get_node(buffer_p, NULL);
	if (confs_noffset < 0) {
		debug("FPGA: No default configuration was found in config.\n");
		return -ENOENT;
	}

	count = fit_conf_get_prop_node_count(buffer_p, confs_noffset,
					    FIT_FPGA_PROP);
	if (count < 0) {
		debug("FPGA: Invalid configuration format for FPGA node.\n");
		return count;
	}
	debug("FPGA: FPGA node count: %d\n", count);

	for (i = 0; i < count; i++) {
		images_noffset = fit_conf_get_prop_node_index(buffer_p,
							     confs_noffset,
							     FIT_FPGA_PROP, i);
		uname = fit_get_name(buffer_p, images_noffset, NULL);
		if (uname) {
			debug("FPGA: %s\n", uname);

			if (strstr(uname, "fpga-periph") &&
				(!is_fpgamgr_early_user_mode() ||
				is_fpgamgr_user_mode())) {
				fpga_node_name = uname;
				printf("FPGA: Start to program ");
				printf("peripheral/full bitstream ...\n");
				break;
			} else if (strstr(uname, "fpga-core") &&
					(is_fpgamgr_early_user_mode() &&
					!is_fpgamgr_user_mode())) {
				fpga_node_name = uname;
				printf("FPGA: Start to program core ");
				printf("bitstream ...\n");
				break;
			}
		}
		WATCHDOG_RESET();
	}

	if (!fpga_node_name) {
		debug("FPGA: No suitable bitstream was found, count: %d.\n", i);
		return 1;
	}

	images_noffset = fit_image_get_node(buffer_p, fpga_node_name);
	if (images_noffset < 0) {
		debug("FPGA: No node '%s' was found in FIT.\n",
		     fpga_node_name);
		return -ENOENT;
	}

	if (!fit_image_get_data_position(buffer_p, images_noffset,
					&rbf_offset)) {
		debug("FPGA: Data position was found.\n");
	} else if (!fit_image_get_data_offset(buffer_p, images_noffset,
		  &rbf_offset)) {
		/*
		 * For FIT with external data, figure out where
		 * the external images start. This is the base
		 * for the data-offset properties in each image.
		 */
		rbf_offset += ((fdt_totalsize(buffer_p) + 3) & ~3);
		debug("FPGA: Data offset was found.\n");
	} else {
		debug("FPGA: No data position/offset was found.\n");
		return -ENOENT;
	}

	ret = fit_image_get_data_size(buffer_p, images_noffset, &rbf_size);
	if (ret < 0) {
		debug("FPGA: No data size was found (err=%d).\n", ret);
		return -ENOENT;
	}

	if (gd->ram_size < rbf_size) {
		debug("FPGA: Using default OCRAM buffer and size.\n");
	} else {
		ret = fit_image_get_load(buffer_p, images_noffset,
					(ulong *)loadable);
		if (ret < 0) {
			buffer_p = (u32 *)DEFAULT_DDR_LOAD_ADDRESS;
			debug("FPGA: No loadable was found.\n");
			debug("FPGA: Using default DDR load address: 0x%x .\n",
			     DEFAULT_DDR_LOAD_ADDRESS);
		} else {
			buffer_p = (u32 *)*loadable;
			debug("FPGA: Found loadable address = 0x%x.\n",
			     *loadable);
		}

		buffer_size = rbf_size;
	}

	debug("FPGA: External data: offset = 0x%x, size = 0x%x.\n",
	      rbf_offset, rbf_size);

	fpga_loadfs->remaining = rbf_size;

	/*
	 * Determine buffer size vs bitstream size, and calculating number of
	 * chunk by chunk transfer is required due to smaller buffer size
	 * compare to bitstream
	 */
	if (rbf_size <= buffer_size) {
		/* Loading whole bitstream into buffer */
		buffer_size = rbf_size;
		fpga_loadfs->remaining = 0;
	} else {
		fpga_loadfs->remaining -= buffer_size;
	}

	fpga_loadfs->offset = rbf_offset;
	/* Loading bitstream into buffer */
	ret = request_firmware_into_buf(dev,
					fpga_loadfs->fpga_fsinfo->filename,
					buffer_p, buffer_size,
					fpga_loadfs->offset);
	if (ret < 0) {
		debug("FPGA: Failed to read bitstream from flash.\n");
		return -ENOENT;
	}

	/* Getting info about bitstream types */
	get_rbf_image_info(&fpga_loadfs->rbfinfo, (u16 *)buffer_p);

	/* Update next reading bitstream offset */
	fpga_loadfs->offset += buffer_size;

	/* Update the final addr for bitstream */
	*buffer = (u32)buffer_p;

	/* Update the size of bitstream to be programmed into FPGA */
	*buffer_bsize = buffer_size;

	return 0;
}

static int subsequent_loading_rbf_to_buffer(struct udevice *dev,
					struct fpga_loadfs_info *fpga_loadfs,
					u32 *buffer, size_t *buffer_bsize)
{
	int ret = 0;
	u32 *buffer_p = (u32 *)*buffer;

	/* Read the bitstream chunk by chunk. */
	if (fpga_loadfs->remaining > *buffer_bsize) {
		fpga_loadfs->remaining -= *buffer_bsize;
	} else {
		*buffer_bsize = fpga_loadfs->remaining;
		fpga_loadfs->remaining = 0;
	}

	ret = request_firmware_into_buf(dev,
					fpga_loadfs->fpga_fsinfo->filename,
					buffer_p, *buffer_bsize,
					fpga_loadfs->offset);
	if (ret < 0) {
		debug("FPGA: Failed to read bitstream from flash.\n");
		return -ENOENT;
	}

	/* Update next reading bitstream offset */
	fpga_loadfs->offset += *buffer_bsize;

	return 0;
}

int socfpga_loadfs(fpga_fs_info *fpga_fsinfo, const void *buf, size_t bsize,
			u32 offset)
{
	struct fpga_loadfs_info fpga_loadfs;
	struct udevice *dev;
	int status, ret, size;
	u32 buffer = (uintptr_t)buf;
	size_t buffer_sizebytes = bsize;
	size_t buffer_sizebytes_ori = bsize;
	size_t total_sizeof_image = 0;
	ofnode node;
	const fdt32_t *phandle_p;
	u32 phandle;

	node = get_fpga_mgr_ofnode(ofnode_null());

	if (ofnode_valid(node)) {
		phandle_p = ofnode_get_property(node, "firmware-loader", &size);
		if (!phandle_p) {
			node = ofnode_path("/chosen");
			if (!ofnode_valid(node)) {
				debug("FPGA: /chosen node was not found.\n");
				return -ENOENT;
			}

			phandle_p = ofnode_get_property(node, "firmware-loader",
						       &size);
			if (!phandle_p) {
				debug("FPGA: firmware-loader property was not");
				debug(" found.\n");
				return -ENOENT;
			}
		}
	} else {
		debug("FPGA: FPGA manager node was not found.\n");
		return -ENOENT;
	}

	phandle = fdt32_to_cpu(*phandle_p);
	ret = uclass_get_device_by_phandle_id(UCLASS_FS_FIRMWARE_LOADER,
					     phandle, &dev);
	if (ret)
		return ret;

	memset(&fpga_loadfs, 0, sizeof(fpga_loadfs));

	fpga_loadfs.fpga_fsinfo = fpga_fsinfo;
	fpga_loadfs.offset = offset;

	printf("FPGA: Checking FPGA configuration setting ...\n");

	/*
	 * Note: Both buffer and buffer_sizebytes values can be altered by
	 * function below.
	 */
	ret = first_loading_rbf_to_buffer(dev, &fpga_loadfs, &buffer,
					   &buffer_sizebytes);
	if (ret == 1) {
		printf("FPGA: Skipping configuration ...\n");
		return 0;
	} else if (ret) {
		return ret;
	}

	if (fpga_loadfs.rbfinfo.section == core_section &&
		!(is_fpgamgr_early_user_mode() && !is_fpgamgr_user_mode())) {
		debug("FPGA : Must be in Early Release mode to program ");
		debug("core bitstream.\n");
		return -EPERM;
	}

	/* Disable all signals from HPS peripheral controller to FPGA */
	writel(0, &system_manager_base->fpgaintf_en_global);

	/* Disable all axi bridges (hps2fpga, lwhps2fpga & fpga2hps) */
	socfpga_bridges_reset();

	if (fpga_loadfs.rbfinfo.section == periph_section) {
		/* Initialize the FPGA Manager */
		status = fpgamgr_program_init((u32 *)buffer, buffer_sizebytes);
		if (status) {
			debug("FPGA: Init with peripheral bitstream failed.\n");
			return -EPERM;
		}
	}

	/* Transfer bitstream to FPGA Manager */
	fpgamgr_program_write((void *)buffer, buffer_sizebytes);

	total_sizeof_image += buffer_sizebytes;

	while (fpga_loadfs.remaining) {
		ret = subsequent_loading_rbf_to_buffer(dev,
							&fpga_loadfs,
							&buffer,
							&buffer_sizebytes_ori);

		if (ret)
			return ret;

		/* Transfer data to FPGA Manager */
		fpgamgr_program_write((void *)buffer,
					buffer_sizebytes_ori);

		total_sizeof_image += buffer_sizebytes_ori;

		WATCHDOG_RESET();
	}

	if (fpga_loadfs.rbfinfo.section == periph_section) {
		if (fpgamgr_wait_early_user_mode() != -ETIMEDOUT) {
			config_pins(gd->fdt_blob, "shared");
			puts("FPGA: Early Release Succeeded.\n");
		} else {
			debug("FPGA: Failed to see Early Release.\n");
			return -EIO;
		}

		/* For monolithic bitstream */
		if (is_fpgamgr_user_mode()) {
			/* Ensure the FPGA entering config done */
			status = fpgamgr_program_finish();
			if (status)
				return status;

			config_pins(gd->fdt_blob, "fpga");
			puts("FPGA: Enter user mode.\n");
		}
	} else if (fpga_loadfs.rbfinfo.section == core_section) {
		/* Ensure the FPGA entering config done */
		status = fpgamgr_program_finish();
		if (status)
			return status;

		config_pins(gd->fdt_blob, "fpga");
		puts("FPGA: Enter user mode.\n");
	} else {
		debug("FPGA: Config Error: Unsupported bitstream type.\n");
		return -ENOEXEC;
	}

	return (int)total_sizeof_image;
}

void fpgamgr_program(const void *buf, size_t bsize, u32 offset)
{
	fpga_fs_info fpga_fsinfo;

	fpga_fsinfo.filename = get_fpga_filename();

	if (fpga_fsinfo.filename)
		socfpga_loadfs(&fpga_fsinfo, buf, bsize, offset);
}
#endif

/* This function is used to load the core bitstream from the OCRAM. */
int socfpga_load(Altera_desc *desc, const void *rbf_data, size_t rbf_size)
{
	unsigned long status;
	struct rbf_info rbfinfo;

	memset(&rbfinfo, 0, sizeof(rbfinfo));

	/* Disable all signals from hps peripheral controller to fpga */
	writel(0, &system_manager_base->fpgaintf_en_global);

	/* Disable all axi bridge (hps2fpga, lwhps2fpga & fpga2hps) */
	socfpga_bridges_reset();

	/* Getting info about bitstream types */
	get_rbf_image_info(&rbfinfo, (u16 *)rbf_data);

	if (rbfinfo.section == periph_section) {
		/* Initialize the FPGA Manager */
		status = fpgamgr_program_init((u32 *)rbf_data, rbf_size);
		if (status)
			return status;
	}

	if (rbfinfo.section == core_section &&
		!(is_fpgamgr_early_user_mode() && !is_fpgamgr_user_mode())) {
		debug("FPGA : Must be in early release mode to program ");
		debug("core bitstream.\n");
		return -EPERM;
	}

	/* Write the bitstream to FPGA Manager */
	fpgamgr_program_write(rbf_data, rbf_size);

	status = fpgamgr_program_finish();
	if (status) {
		config_pins(gd->fdt_blob, "fpga");
		puts("FPGA: Enter user mode.\n");
	}

	return status;
}
