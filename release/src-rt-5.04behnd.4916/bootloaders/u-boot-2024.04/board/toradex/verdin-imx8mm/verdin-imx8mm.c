// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2020-2021 Toradex
 */

#include <common.h>
#include <init.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <hang.h>
#include <i2c.h>
#include <micrel.h>
#include <miiphy.h>
#include <netdev.h>

#include "../common/tdx-cfg-block.h"

DECLARE_GLOBAL_DATA_PTR;

#define I2C_PMIC	0

enum pcb_rev_t {
	PCB_VERSION_1_0,
	PCB_VERSION_1_1
};

#if IS_ENABLED(CONFIG_FEC_MXC)
static int setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);

	return 0;
}
#endif

int board_init(void)
{
	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

static enum pcb_rev_t get_pcb_revision(void)
{
	struct udevice *bus;
	struct udevice *i2c_dev = NULL;
	int ret;
	u8 is_bd71837 = 0;

	ret = uclass_get_device_by_seq(UCLASS_I2C, I2C_PMIC, &bus);
	if (!ret)
		ret = dm_i2c_probe(bus, 0x4b, 0, &i2c_dev);
	if (!ret)
		ret = dm_i2c_read(i2c_dev, 0x0, &is_bd71837, 1);

	/* BD71837_REV, High Nibble is major version, fix 1010 */
	is_bd71837 = !ret && ((is_bd71837 & 0xf0) == 0xa0);
	return is_bd71837 ? PCB_VERSION_1_0 : PCB_VERSION_1_1;
}

static void select_dt_from_module_version(void)
{
	char variant[32];
	char *env_variant = env_get("variant");
	int is_wifi = 0;

	if (IS_ENABLED(CONFIG_TDX_CFG_BLOCK)) {
		/*
		 * If we have a valid config block and it says we are a
		 * module with Wi-Fi/Bluetooth make sure we use the -wifi
		 * device tree.
		 */
		is_wifi = (tdx_hw_tag.prodid == VERDIN_IMX8MMQ_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_IMX8MMDL_WIFI_BT_IT) ||
			  (tdx_hw_tag.prodid == VERDIN_IMX8MMQ_WIFI_BT_IT_NO_CAN);
	}

	switch (get_pcb_revision()) {
	case PCB_VERSION_1_0:
		printf("Detected a V1.0 module which is no longer supported in this BSP version\n");
		hang();
	default:
		if (is_wifi)
			strlcpy(&variant[0], "wifi", sizeof(variant));
		else
			strlcpy(&variant[0], "nonwifi", sizeof(variant));
		break;
	}

	if (strcmp(variant, env_variant)) {
		printf("Setting variant to %s\n", variant);
		env_set("variant", variant);
	}
}

int board_late_init(void)
{
	select_dt_from_module_version();

	return 0;
}

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	*size = get_ram_size((void *)PHYS_SDRAM, PHYS_SDRAM_SIZE);

	return 0;
}

#if defined(CONFIG_OF_LIBFDT) && defined(CONFIG_OF_BOARD_SETUP)
int ft_board_setup(void *blob, struct bd_info *bd)
{
	return ft_common_board_setup(blob, bd);
}
#endif
