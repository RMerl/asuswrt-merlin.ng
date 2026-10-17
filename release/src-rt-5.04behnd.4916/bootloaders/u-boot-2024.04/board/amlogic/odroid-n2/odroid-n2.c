// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2020 BayLibre, SAS
 * Author: Neil Armstrong <narmstrong@baylibre.com>
 */

#include <common.h>
#include <dm.h>
#include <adc.h>
#include <env.h>
#include <init.h>
#include <net.h>
#include <asm/io.h>
#include <asm/arch/boot.h>
#include <asm/arch/sm.h>
#include <asm/arch/eth.h>
#include <asm/arch/boot.h>

#define EFUSE_MAC_OFFSET	20
#define EFUSE_MAC_SIZE		12
#define MAC_ADDR_LEN		6

#define ODROID_HW_VS_ADC_CHANNEL	1

#define MESON_SOC_ID_G12B	0x29
#define MESON_SOC_ID_SM1	0x2b

int mmc_get_env_dev(void)
{
	if (meson_get_boot_device() == BOOT_DEVICE_EMMC)
		return 1;
	return 0;
}

/* Variant detection is based on the ADC RAW values for the channel #1 */
static struct meson_odroid_boards {
	unsigned int soc_id;
	unsigned int adc_min;
	unsigned int adc_max;
	char *variant;
} boards[] = {
	/* OdroidN2 rev 2018,7,23 */
	{ MESON_SOC_ID_G12B, 80 * 4,  90 * 4, "n2" },
	/* OdroidN2 rev 2018,12,6 */
	{ MESON_SOC_ID_G12B, 160 * 4, 170 * 4, "n2" },
	/* OdroidN2 rev 2019,1,17 */
	{ MESON_SOC_ID_G12B, 245 * 4, 255 * 4, "n2" },
	/* OdroidN2 rev 2019,2,7 */
	{ MESON_SOC_ID_G12B, 330 * 4, 350 * 4, "n2" },
	/* OdroidN2plus rev 2019,11,20 */
	{ MESON_SOC_ID_G12B, 410 * 4, 430 * 4, "n2-plus" },
	/* OdroidC4 rev 2020,01,29 */
	{ MESON_SOC_ID_SM1,   80 * 4, 100 * 4, "c4" },
	/* OdroidHC4 rev 2019,12,10 */
	{ MESON_SOC_ID_SM1,  300 * 4, 320 * 4, "hc4" },
	/* OdroidC4 rev 2019,11,29 */
	{ MESON_SOC_ID_SM1,  335 * 4, 345 * 4, "c4" },
	/* OdroidHC4 rev 2020,8,7 */
	{ MESON_SOC_ID_SM1,  590 * 4, 610 * 4, "hc4" },
};

static void odroid_set_fdtfile(char *soc, char *variant)
{
	char s[128];

	snprintf(s, sizeof(s), "amlogic/meson-%s-odroid-%s.dtb", soc, variant);
	env_set("fdtfile", s);
}

static int odroid_detect_variant(void)
{
	char *variant = "", *soc = "";
	unsigned int adcval = 0;
	int ret, i, soc_id = 0;

	if (of_machine_is_compatible("amlogic,sm1")) {
		soc_id = MESON_SOC_ID_SM1;
		soc = "sm1";
	} else if (of_machine_is_compatible("amlogic,g12b")) {
		soc_id = MESON_SOC_ID_G12B;
		soc = "g12b";
	} else {
		return -1;
	}

	ret = adc_channel_single_shot("adc@9000", ODROID_HW_VS_ADC_CHANNEL,
				      &adcval);
	if (ret)
		return ret;

	for (i = 0 ; i < ARRAY_SIZE(boards) ; ++i) {
		if (soc_id == boards[i].soc_id &&
		    adcval >= boards[i].adc_min &&
		    adcval < boards[i].adc_max) {
			variant = boards[i].variant;
			break;
		}
	}

	printf("Board variant: %s\n", variant);
	env_set("variant", variant);

	odroid_set_fdtfile(soc, variant);

	return 0;
}

int misc_init_r(void)
{
	u8 mac_addr[MAC_ADDR_LEN + 1];
	char efuse_mac_addr[EFUSE_MAC_SIZE], tmp[3];
	ssize_t len;

	if (IS_ENABLED(CONFIG_ENV_VARS_UBOOT_RUNTIME_CONFIG) &&
	    meson_get_soc_rev(tmp, sizeof(tmp)) > 0)
		env_set("soc_rev", tmp);

	if (!eth_env_get_enetaddr("ethaddr", mac_addr)) {
		len = meson_sm_read_efuse(EFUSE_MAC_OFFSET,
					  efuse_mac_addr, EFUSE_MAC_SIZE);
		if (len != EFUSE_MAC_SIZE)
			return 0;

		/* MAC is stored in ASCII format, 1bytes = 2characters */
		for (int i = 0; i < 6; i++) {
			tmp[0] = efuse_mac_addr[i * 2];
			tmp[1] = efuse_mac_addr[i * 2 + 1];
			tmp[2] = '\0';
			mac_addr[i] = hextoul(tmp, NULL);
		}
		mac_addr[MAC_ADDR_LEN] = '\0';

		if (is_valid_ethaddr(mac_addr))
			eth_env_set_enetaddr("ethaddr", mac_addr);
		else
			meson_generate_serial_ethaddr();
	}

	odroid_detect_variant();
	return 0;
}
