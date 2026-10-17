// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2019 Toradex
 */

#include <common.h>
#include <cpu_func.h>
#include <init.h>
#include <asm/global_data.h>

#include <asm/arch/clock.h>
#include <asm/arch/imx8-pins.h>
#include <asm/arch/iomux.h>
#include <asm/arch/snvs_security_sc.h>
#include <firmware/imx/sci/sci.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/io.h>
#include <command.h>
#include <env.h>
#include <errno.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/libfdt.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define UART_PAD_CTRL	((SC_PAD_CONFIG_OUT_IN << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define PCB_VERS_DETECT	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define GPIO_PAD_CTRL	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
			 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
			 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT) | \
			 (SC_PAD_28FDSOI_PS_PU << PADRING_PULL_SHIFT))

#define PCB_VERS_DEFAULT	((SC_PAD_CONFIG_NORMAL << PADRING_CONFIG_SHIFT) | \
				 (SC_PAD_ISO_OFF << PADRING_LPCONFIG_SHIFT) | \
				 (SC_PAD_28FDSOI_PS_PD << PADRING_PULL_SHIFT) | \
				 (SC_PAD_28FDSOI_DSE_DV_HIGH << PADRING_DSE_SHIFT))

#define TDX_USER_FUSE_BLOCK1_A	276
#define TDX_USER_FUSE_BLOCK1_B	277
#define TDX_USER_FUSE_BLOCK2_A	278
#define TDX_USER_FUSE_BLOCK2_B	279

enum pcb_rev_t {
	PCB_VERSION_1_0,
	PCB_VERSION_1_1
};

static iomux_cfg_t pcb_vers_detect[] = {
	SC_P_MIPI_DSI0_GPIO0_00 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(PCB_VERS_DETECT),
	SC_P_MIPI_DSI0_GPIO0_01 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(PCB_VERS_DETECT),
};

static iomux_cfg_t pcb_vers_default[] = {
	SC_P_MIPI_DSI0_GPIO0_00 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(PCB_VERS_DEFAULT),
	SC_P_MIPI_DSI0_GPIO0_01 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(PCB_VERS_DEFAULT),
};

static iomux_cfg_t uart1_pads[] = {
	SC_P_UART1_RX | MUX_PAD_CTRL(UART_PAD_CTRL),
	SC_P_UART1_TX | MUX_PAD_CTRL(UART_PAD_CTRL),
};

struct tdx_user_fuses {
	u16 pid4;
	u16 vers;
	u8 ramid;
};

static void setup_iomux_uart(void)
{
	imx8_iomux_setup_multiple_pads(uart1_pads, ARRAY_SIZE(uart1_pads));
}

static uint32_t do_get_tdx_user_fuse(int a, int b)
{
	int sciErr;
	u32 val_a = 0;
	u32 val_b = 0;

	sciErr = sc_misc_otp_fuse_read(-1, a, &val_a);
	if (sciErr) {
		printf("Error reading out user fuse %d\n", a);
		return 0;
	}

	sciErr = sc_misc_otp_fuse_read(-1, b, &val_b);
	if (sciErr) {
		printf("Error reading out user fuse %d\n", b);
		return 0;
	}

	return ((val_a & 0xffff) << 16) | (val_b & 0xffff);
}

static void get_tdx_user_fuse(struct tdx_user_fuses *tdxuserfuse)
{
	u32 fuse_block;

	fuse_block = do_get_tdx_user_fuse(TDX_USER_FUSE_BLOCK2_A,
					  TDX_USER_FUSE_BLOCK2_B);

	/*
	 * Fuse block 2 acts as a backup area, if this reads 0 we want to
	 * use fuse block 1
	 */
	if (fuse_block == 0)
		fuse_block = do_get_tdx_user_fuse(TDX_USER_FUSE_BLOCK1_A,
						  TDX_USER_FUSE_BLOCK1_B);

	tdxuserfuse->pid4 = (fuse_block >> 18) & GENMASK(13, 0);
	tdxuserfuse->vers = (fuse_block >> 4) & GENMASK(13, 0);
	tdxuserfuse->ramid = fuse_block & GENMASK(3, 0);
}

void board_mem_get_layout(u64 *phys_sdram_1_start,
			  u64 *phys_sdram_1_size,
			  u64 *phys_sdram_2_start,
			  u64 *phys_sdram_2_size)
{
	u32 is_quadplus = 0, val = 0;
	struct tdx_user_fuses tdxramfuses;
	int scierr = sc_misc_otp_fuse_read(-1, 6, &val);

	if (!scierr) {
		/* QP has one A72 core disabled */
		is_quadplus = ((val >> 4) & 0x3) != 0x0;
	}

	get_tdx_user_fuse(&tdxramfuses);

	*phys_sdram_1_start = PHYS_SDRAM_1;
	*phys_sdram_1_size = PHYS_SDRAM_1_SIZE;
	*phys_sdram_2_start = PHYS_SDRAM_2;

	switch (tdxramfuses.ramid) {
	case 1:
		*phys_sdram_2_size = SZ_2G;
		break;
	case 2:
		*phys_sdram_2_size = 0x0UL;
		break;
	case 3:
		*phys_sdram_2_size = SZ_2G;
		break;
	case 4:
		*phys_sdram_2_size = SZ_4G + SZ_2G;
		break;
	default:
		if (is_quadplus)
			/* Our QP based SKUs only have 2 GB RAM (PHYS_SDRAM_1_SIZE) */
			*phys_sdram_2_size = 0x0UL;
		else
			*phys_sdram_2_size = PHYS_SDRAM_2_SIZE;
		break;
	}
}

int board_early_init_f(void)
{
	sc_pm_clock_rate_t rate = SC_80MHZ;
	int ret;

	/* Set UART1 clock root to 80 MHz and enable it */
	ret = sc_pm_setup_uart(SC_R_UART_1, rate);
	if (ret)
		return ret;

	setup_iomux_uart();

	return 0;
}

#if CONFIG_IS_ENABLED(DM_GPIO)

#define BKL1_GPIO   IMX_GPIO_NR(1, 10)

static iomux_cfg_t board_gpios[] = {
	SC_P_LVDS1_GPIO00 | MUX_MODE_ALT(3) | MUX_PAD_CTRL(GPIO_PAD_CTRL),
};

static void board_gpio_init(void)
{
	imx8_iomux_setup_multiple_pads(board_gpios, ARRAY_SIZE(board_gpios));

	gpio_request(BKL1_GPIO, "BKL1_GPIO");
}
#else
static inline void board_gpio_init(void) {}
#endif

/*
 * Backlight off before OS handover
 */
void board_preboot_os(void)
{
	gpio_direction_output(BKL1_GPIO, 0);
}

static enum pcb_rev_t get_pcb_revision(void)
{
	unsigned int pcb_vers = 0;

	imx8_iomux_setup_multiple_pads(pcb_vers_detect,
				       ARRAY_SIZE(pcb_vers_detect));

	gpio_request(IMX_GPIO_NR(1, 18),
		     "PCB version detection on PAD SC_P_MIPI_DSI0_GPIO0_00");
	gpio_request(IMX_GPIO_NR(1, 19),
		     "PCB version detection on PAD SC_P_MIPI_DSI0_GPIO0_01");
	gpio_direction_input(IMX_GPIO_NR(1, 18));
	gpio_direction_input(IMX_GPIO_NR(1, 19));

	udelay(1000);

	pcb_vers = gpio_get_value(IMX_GPIO_NR(1, 18));
	pcb_vers |= gpio_get_value(IMX_GPIO_NR(1, 19)) << 1;

	/* Set muxing back to default values for saving energy */
	imx8_iomux_setup_multiple_pads(pcb_vers_default,
				       ARRAY_SIZE(pcb_vers_default));

	switch (pcb_vers) {
	case 0b11:
		return PCB_VERSION_1_0;
	case 0b10:
		return PCB_VERSION_1_1;
	default:
		printf("Unknown PCB version=0x%x, default to V1.1\n", pcb_vers);
		return PCB_VERSION_1_1;
	}
}

static void select_dt_from_module_version(void)
{
	env_set("soc", "imx8qm");
	env_set("variant", "-v1.1");

	switch (tdx_hw_tag.prodid) {
	/* Select Apalis iMX8QM device trees */
	case APALIS_IMX8QM_IT:
	case APALIS_IMX8QM_WIFI_BT_IT:
	case APALIS_IMX8QM_8GB_WIFI_BT_IT:
		if (get_pcb_revision() == PCB_VERSION_1_0)
			env_set("variant", "");
		break;
	/* Select Apalis iMX8QP device trees */
	case APALIS_IMX8QP_WIFI_BT:
	case APALIS_IMX8QP:
		env_set("soc", "imx8qp");
		break;
	default:
		printf("Unknown Apalis iMX8 module\n");
		return;
	}
}

static int do_select_dt_from_module_version(struct cmd_tbl *cmdtp, int flag,
					    int argc, char * const argv[])
{
	select_dt_from_module_version();
	return 0;
}

U_BOOT_CMD(select_dt_from_module_version, CONFIG_SYS_MAXARGS, 1, do_select_dt_from_module_version,
	   "\n", "    - select devicetree from module version"
);

int board_init(void)
{
	board_gpio_init();

	if (IS_ENABLED(CONFIG_IMX_SNVS_SEC_SC_AUTO)) {
		int ret = snvs_security_sc_init();

		if (ret)
			return ret;
	}

	return 0;
}

/*
 * Board specific reset that is system reset.
 */
void reset_cpu(void)
{
	/* TODO */
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

int board_late_init(void)
{
#ifdef CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG
/* TODO move to common */
	env_set("board_name", "Apalis iMX8QM");
	env_set("board_rev", "v1.0");
#endif

	build_info();

	select_dt_from_module_version();

	return 0;
}
