// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright 2013-2019 Arcturus Networks, Inc.
 *           https://www.arcturusnetworks.com/products/ucp1020/
 *           by Oleksandr G Zhadan et al.
 * based on board/freescale/p1_p2_rdb_pc/spl.c
 * original copyright follows:
 * Copyright 2013 Freescale Semiconductor, Inc.
 */

#include <common.h>
#include <command.h>
#include <hwconfig.h>
#include <pci.h>
#include <i2c.h>
#include <miiphy.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <ioports.h>
#include <netdev.h>
#include <micrel.h>
#include <spi_flash.h>
#include <mmc.h>
#include <linux/ctype.h>
#include <asm/fsl_serdes.h>
#include <asm/gpio.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/io.h>
#include <asm/fsl_law.h>
#include <asm/fsl_lbc.h>
#include <asm/mp.h>
#include "ucp1020.h"

void spi_set_speed(struct spi_slave *slave, uint hz)
{
	/* TO DO: It's actially have to be in spi/ */
}

/*
 * To be compatible with cmd_gpio
 */
int name_to_gpio(const char *name)
{
	int gpio = 31 - simple_strtoul(name, NULL, 10);

	if (gpio < 16)
		gpio = -1;

	return gpio;
}

void board_gpio_init(void)
{
	int i;
	char envname[8], *val;

	for (i = 0; i < GPIO_MAX_NUM; i++) {
		sprintf(envname, "GPIO%d", i);
		val = env_get(envname);
		if (val) {
			char direction = toupper(val[0]);
			char level = toupper(val[1]);

			if (direction == 'I') {
				gpio_direction_input(i);
			} else {
				if (direction == 'O') {
					if (level == '1')
						gpio_direction_output(i, 1);
					else
						gpio_direction_output(i, 0);
				}
			}
		}
	}

	val = env_get("PCIE_OFF");
	if (val) {
		gpio_direction_input(GPIO_PCIE1_EN);
		gpio_direction_input(GPIO_PCIE2_EN);
	} else {
		gpio_direction_output(GPIO_PCIE1_EN, 1);
		gpio_direction_output(GPIO_PCIE2_EN, 1);
	}

	val = env_get("SDHC_CDWP_OFF");
	if (!val) {
		ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);

		setbits_be32(&gur->pmuxcr,
			     (MPC85xx_PMUXCR_SDHC_CD | MPC85xx_PMUXCR_SDHC_WP));
	}
}

int board_early_init_f(void)
{
	return 0;	/* Just in case. Could be disable in config file */
}

int checkboard(void)
{
	printf("Board: %s\n", CONFIG_BOARDNAME_LOCAL);
	board_gpio_init();
#ifdef CONFIG_MMC
	printf("SD/MMC: 4-bit Mode\n");
#endif

	return 0;
}

#ifdef CONFIG_PCI
void pci_init_board(void)
{
	fsl_pcie_init_board(0);
}
#endif

int board_early_init_r(void)
{
	const unsigned int flashbase = CONFIG_SYS_FLASH_BASE;
	const u8 flash_esel = find_tlb_idx((void *)flashbase, 1);

	/*
	 * Remap Boot flash region to caching-inhibited
	 * so that flash can be erased properly.
	 */

	/* Flush d-cache and invalidate i-cache of any FLASH data */
	flush_dcache();
	invalidate_icache();

	/* invalidate existing TLB entry for flash */
	disable_tlb(flash_esel);

	set_tlb(1, flashbase, CONFIG_SYS_FLASH_BASE_PHYS, /* tlb, epn, rpn */
		MAS3_SX | MAS3_SW | MAS3_SR, MAS2_I | MAS2_G, /* perms, wimge */
		0, flash_esel, BOOKE_PAGESZ_64M, 1);/* ts, esel, tsize, iprot */

	return 0;
}

int board_phy_config(struct phy_device *phydev)
{
#if defined(CONFIG_PHY_MICREL_KSZ9021)
	int regval;
	static int cnt;

	if (cnt++ == 0)
		printf("PHYs address [");

	if (phydev->addr == TSEC1_PHY_ADDR || phydev->addr == TSEC3_PHY_ADDR) {
		regval =
		    ksz9021_phy_extended_read(phydev,
					      MII_KSZ9021_EXT_STRAP_STATUS);
		/*
		 * min rx data delay
		 */
		ksz9021_phy_extended_write(phydev,
					   MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW,
					   0x6666);
		/*
		 * max rx/tx clock delay, min rx/tx control
		 */
		ksz9021_phy_extended_write(phydev,
					   MII_KSZ9021_EXT_RGMII_CLOCK_SKEW,
					   0xf6f6);
		printf("0x%x", (regval & 0x1f));
	} else {
		printf("0x%x", (TSEC2_PHY_ADDR & 0x1f));
	}
	if (cnt == 3)
		printf("] ");
	else
		printf(",");
#endif

#if defined(CONFIG_PHY_MICREL_KSZ9031_DEBUG)
	regval = ksz9031_phy_extended_read(phydev, 2, 0x01, 0x4000);
	if (regval >= 0)
		printf(" (ADDR 0x%x) ", regval & 0x1f);
#endif

	return 0;
}

int last_stage_init(void)
{
	static char newkernelargs[256];
	static u8 id1[16];
	static u8 id2;
#ifdef CONFIG_MMC
	struct mmc *mmc;
#endif
	char *sval, *kval;

	if (i2c_read(CONFIG_SYS_I2C_IDT6V49205B, 7, 1, &id1[0], 2) < 0) {
		printf("Error reading i2c IDT6V49205B information!\n");
	} else {
		printf("IDT6V49205B(0x%02x): ready\n", id1[1]);
		i2c_read(CONFIG_SYS_I2C_IDT6V49205B, 4, 1, &id1[0], 2);
		if (!(id1[1] & 0x02)) {
			id1[1] |= 0x02;
			i2c_write(CONFIG_SYS_I2C_IDT6V49205B, 4, 1, &id1[0], 2);
			asm("nop; nop");
		}
	}

	if (i2c_read(CONFIG_SYS_I2C_NCT72_ADDR, 0xFE, 1, &id2, 1) < 0)
		printf("Error reading i2c NCT72 information!\n");
	else
		printf("NCT72(0x%x): ready\n", id2);

	kval = env_get("kernelargs");

#ifdef CONFIG_MMC
	mmc = find_mmc_device(0);
	if (mmc)
		if (!mmc_init(mmc)) {
			printf("MMC/SD card detected\n");
			if (kval) {
				int n = strlen(defkargs);
				char *tmp = strstr(kval, defkargs);

				*tmp = 0;
				strcpy(newkernelargs, kval);
				strcat(newkernelargs, " ");
				strcat(newkernelargs, mmckargs);
				strcat(newkernelargs, " ");
				strcat(newkernelargs, &tmp[n]);
				env_set("kernelargs", newkernelargs);
			} else {
				env_set("kernelargs", mmckargs);
			}
		}
#endif
	get_arc_info();

	if (kval) {
		sval = env_get("SERIAL");
		if (sval) {
			strcpy(newkernelargs, "SN=");
			strcat(newkernelargs, sval);
			strcat(newkernelargs, " ");
			strcat(newkernelargs, kval);
			env_set("kernelargs", newkernelargs);
		}
	} else {
		printf("Error reading kernelargs env variable!\n");
	}

	return 0;
}

int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[4];
#ifdef CONFIG_TSEC2
	ccsr_gur_t *gur = (void *)(CONFIG_SYS_MPC85xx_GUTS_ADDR);
#endif
	int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	if (is_serdes_configured(SGMII_TSEC2)) {
		if (!(in_be32(&gur->pordevsr) & MPC85xx_PORDEVSR_SGMII2_DIS)) {
			puts("eTSEC2 is in sgmii mode.\n");
			tsec_info[num].flags |= TSEC_SGMII;
			tsec_info[num].phyaddr = TSEC2_PHY_ADDR_SGMII;
		}
	}
	num++;
#endif
#ifdef CONFIG_TSEC3
	SET_STD_TSEC_INFO(tsec_info[num], 3);
	num++;
#endif

	if (!num) {
		printf("No TSECs initialized\n");
		return 0;
	}

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;

	fsl_pq_mdio_init(bis, &mdio_info);

	tsec_eth_init(bis, tsec_info, num);

	return pci_eth_init(bis);
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;
	const char *soc_usb_compat = "fsl-usb2-dr";
	int err, usb1_off, usb2_off;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

	FT_FSL_PCI_SETUP;

#if defined(CONFIG_HAS_FSL_DR_USB)
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

#if defined(CONFIG_SDCARD) || defined(CONFIG_SPIFLASH)
	/* Delete eLBC node as it is muxed with USB2 controller */
	if (hwconfig("usb2")) {
		const char *soc_elbc_compat = "fsl,p1020-elbc";
		int off = fdt_node_offset_by_compatible(blob, -1,
							soc_elbc_compat);
		if (off < 0) {
			printf
			    ("WARNING: could not find compatible node %s: %s\n",
			     soc_elbc_compat, fdt_strerror(off));
			return off;
		}
		err = fdt_del_node(blob, off);
		if (err < 0) {
			printf("WARNING: could not remove %s: %s\n",
			       soc_elbc_compat, fdt_strerror(err));
		}
		return err;
	}
#endif

/* Delete USB2 node as it is muxed with eLBC */
	usb1_off = fdt_node_offset_by_compatible(blob, -1, soc_usb_compat);
	if (usb1_off < 0) {
		printf("WARNING: could not find compatible node %s: %s.\n",
		       soc_usb_compat, fdt_strerror(usb1_off));
		return usb1_off;
	}
	usb2_off =
	    fdt_node_offset_by_compatible(blob, usb1_off, soc_usb_compat);
	if (usb2_off < 0) {
		printf("WARNING: could not find compatible node %s: %s.\n",
		       soc_usb_compat, fdt_strerror(usb2_off));
		return usb2_off;
	}
	err = fdt_del_node(blob, usb2_off);
	if (err < 0) {
		printf("WARNING: could not remove %s: %s.\n",
		       soc_usb_compat, fdt_strerror(err));
	}
	return 0;
}
#endif
