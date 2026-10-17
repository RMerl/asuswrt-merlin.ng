// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2021 Gateworks Corporation
 */

#include <fdt_support.h>
#include <init.h>
#include <led.h>
#include <mmc.h>
#include <miiphy.h>
#include <mmc.h>
#include <asm/arch/clock.h>
#include <asm/arch/sys_proto.h>
#include <asm/mach-imx/boot_mode.h>

#include "eeprom.h"

int board_phys_sdram_size(phys_size_t *size)
{
	if (!size)
		return -EINVAL;

	*size = get_ram_size((void *)PHYS_SDRAM, (long)PHYS_SDRAM_SIZE + (long)PHYS_SDRAM_2_SIZE);

	return 0;
}

int board_fit_config_name_match(const char *name)
{
	int i  = 0;
	const char *dtb;
	static char init;
	char buf[32];

	do {
		dtb = eeprom_get_dtb_name(i++, buf, sizeof(buf));
		if (!strcmp(dtb, name)) {
			if (!init++)
				printf("DTB     : %s\n", name);
			return 0;
		}
	} while (dtb);

	return -1;
}

static int __maybe_unused setup_fec(void)
{
	struct iomuxc_gpr_base_regs *gpr =
		(struct iomuxc_gpr_base_regs *)IOMUXC_GPR_BASE_ADDR;

#ifndef CONFIG_IMX8MP
	/* Use 125M anatop REF_CLK1 for ENET1, not from external */
	clrsetbits_le32(&gpr->gpr[1], 0x2000, 0);
#else
	/* Enable RGMII TX clk output */
	setbits_le32(&gpr->gpr[1], BIT(22));
#endif

	return 0;
}

#if (IS_ENABLED(CONFIG_NET))
int board_phy_config(struct phy_device *phydev)
{
	unsigned short val;

	switch (phydev->phy_id) {
	case 0x2000a231: /* TI DP83867 GbE PHY */
		puts("DP83867 ");
		/* LED configuration */
		val = 0;
		val |= 0x5 << 4; /* LED1(Amber;Speed)   : 1000BT link */
		val |= 0xb << 8; /* LED2(Green;Link/Act): blink for TX/RX act */
		phy_write(phydev, MDIO_DEVAD_NONE, 24, val);
		break;
	}

	if (phydev->drv->config)
		phydev->drv->config(phydev);

	return 0;
}
#endif // IS_ENABLED(CONFIG_NET)

int board_init(void)
{
	venice_eeprom_init(1);

	if (IS_ENABLED(CONFIG_FEC_MXC))
		setup_fec();

	return 0;
}

int board_late_init(void)
{
	const char *str;
	struct mmc *mmc = NULL;
	char env[32];
	int ret, i;
	u8 enetaddr[6];
	char fdt[64];
	int bootdev;

	/* Set board serial/model */
	if (!env_get("serial#"))
		env_set_ulong("serial#", eeprom_get_serial());
	env_set("model", eeprom_get_model());

	/* Set fdt_file vars */
	i = 0;
	do {
		str = eeprom_get_dtb_name(i, fdt, sizeof(fdt));
		if (str) {
			sprintf(env, "fdt_file%d", i + 1);
			strcat(fdt, ".dtb");
			env_set(env, fdt);
		}
		i++;
	} while (str);

	/* Set mac addrs */
	i = 0;
	do {
		if (i)
			sprintf(env, "eth%daddr", i);
		else
			sprintf(env, "ethaddr");
		str = env_get(env);
		if (!str) {
			ret = eeprom_getmac(i, enetaddr);
			if (!ret)
				eth_env_set_enetaddr(env, enetaddr);
		}
		i++;
	} while (!ret);

	/*
	 * set bootdev/bootblk/bootpart (used in firmware_update script)
	 * dynamically depending on boot device and SoC
	 */
	bootdev = -1;
	switch (get_boot_device()) {
	case SD1_BOOT:
	case MMC1_BOOT: /* SDHC1 */
		bootdev = 0;
		break;
	case SD2_BOOT:
	case MMC2_BOOT: /* SDHC2 */
		bootdev = 1;
		break;
	case SD3_BOOT:
	case MMC3_BOOT: /* SDHC3 */
		bootdev = 2;
		break;
	default:
		bootdev = 2; /* assume SDHC3 (eMMC) if booting over SDP */
		break;
	}
	if (bootdev != -1)
		mmc = find_mmc_device(bootdev);
	if (mmc) {
		int bootblk;

		if (IS_ENABLED(CONFIG_IMX8MN) || IS_ENABLED(CONFIG_IMX8MP))
			bootblk = 32 * SZ_1K / 512;
		else
			bootblk = 33 * SZ_1K / 512;
		mmc_init(mmc);
		if (!IS_SD(mmc)) {
			int bootpart;

			switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
			case 1: /* boot0 */
				bootpart = 1;
				break;
			case 2: /* boot1 */
				bootpart = 2;
				break;
			case 7: /* user */
			default:
				bootpart = 0;
				break;
			}
			/* IMX8MP/IMX8MN BOOTROM v2 uses offset=0 for boot parts */
			if ((IS_ENABLED(CONFIG_IMX8MN) || IS_ENABLED(CONFIG_IMX8MP)) &&
			    (bootpart == 1 || bootpart == 2))
				bootblk = 0;
			env_set_hex("bootpart", bootpart);
			env_set_hex("bootblk", bootblk);
		} else { /* SD */
			env_set("bootpart", "");
			env_set_hex("bootblk", bootblk);
		}
		env_set_hex("dev", bootdev);
	}

	/* override soc=imx8m to provide a more specific soc name */
	if (IS_ENABLED(CONFIG_IMX8MN))
		env_set("soc", "imx8mn");
	else if (IS_ENABLED(CONFIG_IMX8MP))
		env_set("soc", "imx8mp");
	else if (IS_ENABLED(CONFIG_IMX8MM))
		env_set("soc", "imx8mm");

	return 0;
}

int board_mmc_get_env_dev(int devno)
{
	return devno;
}

uint mmc_get_env_part(struct mmc *mmc)
{
	if (!IS_SD(mmc)) {
		switch (EXT_CSD_EXTRACT_BOOT_PART(mmc->part_config)) {
		case 1:
			return 1;
		case 2:
			return 2;
		}
	}

	return 0;
}

int ft_board_setup(void *fdt, struct bd_info *bd)
{
	const char *base_model = eeprom_get_baseboard_model();
	char pcbrev;
	int off;

	/* set board model dt prop */
	fdt_setprop_string(fdt, 0, "board", eeprom_get_model());

	if (!strncmp(base_model, "GW73", 4)) {
		pcbrev = get_pcb_rev(base_model);

		if (pcbrev > 'B' && pcbrev < 'E') {
			printf("adjusting dt for %s\n", base_model);

			/*
			 * revC/D/E has PCIe 4-port switch which changes
			 * ethernet1 PCIe GbE:
			 * from: pcie@0,0/pcie@1,0/pcie@2,4/pcie@6.0
			 *   to: pcie@0,0/pcie@1,0/pcie@2,3/pcie@5.0
			 */
			off = fdt_path_offset(fdt, "ethernet1");
			if (off > 0) {
				u32 reg[5];

				fdt_set_name(fdt, off, "pcie@5,0");
				off = fdt_parent_offset(fdt, off);
				fdt_set_name(fdt, off, "pcie@2,3");
				memset(reg, 0, sizeof(reg));
				reg[0] = cpu_to_fdt32(PCI_DEVFN(3, 0));
				fdt_setprop(fdt, off, "reg", reg, sizeof(reg));
			}
		}
	}

	return 0;
}
