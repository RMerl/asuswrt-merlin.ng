// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2011  Renesas Solutions Corp.
 */

#include <common.h>
#include <environment.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/mmc.h>
#include <spi.h>
#include <spi_flash.h>

int checkboard(void)
{
	puts("BOARD: R0P7757LC0030RL board\n");

	return 0;
}

static void init_gctrl(void)
{
	struct gctrl_regs *gctrl = GCTRL_BASE;
	unsigned long graofst;

	graofst = (SH7757LCR_SDRAM_PHYS_TOP + SH7757LCR_GRA_OFFSET) >> 24;
	writel(graofst | 0x20000f00, &gctrl->gracr3);
}

static int init_pcie_bridge_from_spi(void *buf, size_t size)
{
#ifdef CONFIG_DEPRECATED
	struct spi_flash *spi;
	int ret;
	unsigned long pcie_addr;

	spi = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}

	if (is_sh7757_b0())
		pcie_addr = SH7757LCR_PCIEBRG_ADDR_B0;
	else
		pcie_addr = SH7757LCR_PCIEBRG_ADDR;

	ret = spi_flash_read(spi, pcie_addr, size, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}
	spi_flash_free(spi);

	return 0;
#else
	printf("No SPI support so no PCIe support\n");
	return 1;
#endif
}

static void init_pcie_bridge(void)
{
	struct pciebrg_regs *pciebrg = PCIEBRG_BASE;
	struct pcie_setup_regs *pcie_setup = PCIE_SETUP_BASE;
	int i;
	unsigned char *data;
	unsigned short tmp;
	unsigned long pcie_size;

	if (!(readw(&pciebrg->ctrl_h8s) & 0x0001))
		return;

	if (is_sh7757_b0())
		pcie_size = SH7757LCR_PCIEBRG_SIZE_B0;
	else
		pcie_size = SH7757LCR_PCIEBRG_SIZE;

	data = malloc(pcie_size);
	if (!data) {
		printf("%s: malloc error.\n", __func__);
		return;
	}
	if (init_pcie_bridge_from_spi(data, pcie_size)) {
		free(data);
		return;
	}

	if (data[0] == 0xff && data[1] == 0xff && data[2] == 0xff &&
	    data[3] == 0xff) {
		free(data);
		printf("%s: skipped initialization\n", __func__);
		return;
	}

	writew(0xa501, &pciebrg->ctrl_h8s);	/* reset */
	writew(0x0000, &pciebrg->cp_ctrl);
	writew(0x0000, &pciebrg->cp_addr);

	for (i = 0; i < pcie_size; i += 2) {
		tmp = (data[i] << 8) | data[i + 1];
		writew(tmp, &pciebrg->cp_data);
	}

	writew(0xa500, &pciebrg->ctrl_h8s);	/* start */
	if (!is_sh7757_b0())
		writel(0x00000001, &pcie_setup->pbictl3);

	free(data);
}

static void init_usb_phy(void)
{
	struct usb_common_regs *common0 = USB0_COMMON_BASE;
	struct usb_common_regs *common1 = USB1_COMMON_BASE;
	struct usb0_phy_regs *phy = USB0_PHY_BASE;
	struct usb1_port_regs *port = USB1_PORT_BASE;
	struct usb1_alignment_regs *align = USB1_ALIGNMENT_BASE;

	writew(0x0100, &phy->reset);		/* set reset */
	/* port0 = USB0, port1 = USB1 */
	writew(0x0002, &phy->portsel);
	writel(0x0001, &port->port1sel);	/* port1 = Host */
	writew(0x0111, &phy->reset);		/* clear reset */

	writew(0x4000, &common0->suspmode);
	writew(0x4000, &common1->suspmode);

#if defined(__LITTLE_ENDIAN)
	writel(0x00000000, &align->ehcidatac);
	writel(0x00000000, &align->ohcidatac);
#endif
}

static void set_mac_to_sh_eth_register(int channel, char *mac_string)
{
	struct ether_mac_regs *ether;
	unsigned char mac[6];
	unsigned long val;

	eth_parse_enetaddr(mac_string, mac);

	if (!channel)
		ether = ETHER0_MAC_BASE;
	else
		ether = ETHER1_MAC_BASE;

	val = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | mac[3];
	writel(val, &ether->mahr);
	val = (mac[4] << 8) | mac[5];
	writel(val, &ether->malr);
}

static void set_mac_to_sh_giga_eth_register(int channel, char *mac_string)
{
	struct ether_mac_regs *ether;
	unsigned char mac[6];
	unsigned long val;

	eth_parse_enetaddr(mac_string, mac);

	if (!channel)
		ether = GETHER0_MAC_BASE;
	else
		ether = GETHER1_MAC_BASE;

	val = (mac[0] << 24) | (mac[1] << 16) | (mac[2] << 8) | mac[3];
	writel(val, &ether->mahr);
	val = (mac[4] << 8) | mac[5];
	writel(val, &ether->malr);
}

/*****************************************************************
 * This PMB must be set on this timing. The lowlevel_init is run on
 * Area 0(phys 0x00000000), so we have to map it.
 *
 * The new PMB table is following:
 * ent	virt		phys		v	sz	c	wt
 * 0	0xa0000000	0x40000000	1	128M	0	1
 * 1	0xa8000000	0x48000000	1	128M	0	1
 * 2	0xb0000000	0x50000000	1	128M	0	1
 * 3	0xb8000000	0x58000000	1	128M	0	1
 * 4	0x80000000	0x40000000	1	128M	1	1
 * 5	0x88000000	0x48000000	1	128M	1	1
 * 6	0x90000000	0x50000000	1	128M	1	1
 * 7	0x98000000	0x58000000	1	128M	1	1
 */
static void set_pmb_on_board_init(void)
{
	struct mmu_regs *mmu = MMU_BASE;

	/* clear ITLB */
	writel(0x00000004, &mmu->mmucr);

	/* delete PMB for SPIBOOT */
	writel(0, PMB_ADDR_BASE(0));
	writel(0, PMB_DATA_BASE(0));

	/* add PMB for SDRAM(0x40000000 - 0x47ffffff) */
	/*			ppn  ub v s1 s0  c  wt */
	writel(mk_pmb_addr_val(0xa0), PMB_ADDR_BASE(0));
	writel(mk_pmb_data_val(0x40, 1, 1, 1, 0, 0, 1), PMB_DATA_BASE(0));
	writel(mk_pmb_addr_val(0xb0), PMB_ADDR_BASE(2));
	writel(mk_pmb_data_val(0x50, 1, 1, 1, 0, 0, 1), PMB_DATA_BASE(2));
	writel(mk_pmb_addr_val(0xb8), PMB_ADDR_BASE(3));
	writel(mk_pmb_data_val(0x58, 1, 1, 1, 0, 0, 1), PMB_DATA_BASE(3));
	writel(mk_pmb_addr_val(0x80), PMB_ADDR_BASE(4));
	writel(mk_pmb_data_val(0x40, 0, 1, 1, 0, 1, 1), PMB_DATA_BASE(4));
	writel(mk_pmb_addr_val(0x90), PMB_ADDR_BASE(6));
	writel(mk_pmb_data_val(0x50, 0, 1, 1, 0, 1, 1), PMB_DATA_BASE(6));
	writel(mk_pmb_addr_val(0x98), PMB_ADDR_BASE(7));
	writel(mk_pmb_data_val(0x58, 0, 1, 1, 0, 1, 1), PMB_DATA_BASE(7));
}

int board_init(void)
{
	struct gether_control_regs *gether = GETHER_CONTROL_BASE;

	set_pmb_on_board_init();

	/* enable RMII's MDIO (disable GRMII's MDIO) */
	writel(0x00030000, &gether->gbecont);

	init_gctrl();
	init_usb_phy();

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	return mmcif_mmc_init();
}

static int get_sh_eth_mac_raw(unsigned char *buf, int size)
{
#ifdef CONFIG_DEPRECATED
	struct spi_flash *spi;
	int ret;

	spi = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
	if (spi == NULL) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}

	ret = spi_flash_read(spi, SH7757LCR_ETHERNET_MAC_BASE, size, buf);
	if (ret) {
		printf("%s: spi_flash read error.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}
	spi_flash_free(spi);
#endif

	return 0;
}

static int get_sh_eth_mac(int channel, char *mac_string, unsigned char *buf)
{
	memcpy(mac_string, &buf[channel * (SH7757LCR_ETHERNET_MAC_SIZE + 1)],
		SH7757LCR_ETHERNET_MAC_SIZE);
	mac_string[SH7757LCR_ETHERNET_MAC_SIZE] = 0x00;	/* terminate */

	return 0;
}

static void init_ethernet_mac(void)
{
	char mac_string[64];
	char env_string[64];
	int i;
	unsigned char *buf;

	buf = malloc(256);
	if (!buf) {
		printf("%s: malloc error.\n", __func__);
		return;
	}
	get_sh_eth_mac_raw(buf, 256);

	/* Fast Ethernet */
	for (i = 0; i < SH7757LCR_ETHERNET_NUM_CH; i++) {
		get_sh_eth_mac(i, mac_string, buf);
		if (i == 0)
			env_set("ethaddr", mac_string);
		else {
			sprintf(env_string, "eth%daddr", i);
			env_set(env_string, mac_string);
		}

		set_mac_to_sh_eth_register(i, mac_string);
	}

	/* Gigabit Ethernet */
	for (i = 0; i < SH7757LCR_GIGA_ETHERNET_NUM_CH; i++) {
		get_sh_eth_mac(i + SH7757LCR_ETHERNET_NUM_CH, mac_string, buf);
		sprintf(env_string, "eth%daddr", i + SH7757LCR_ETHERNET_NUM_CH);
		env_set(env_string, mac_string);

		set_mac_to_sh_giga_eth_register(i, mac_string);
	}

	free(buf);
}

static void init_pcie(void)
{
	struct pcie_setup_regs *pcie_setup = PCIE_SETUP_BASE;
	struct pcie_system_bus_regs *pcie_sysbus = PCIE_SYSTEM_BUS_BASE;

	writel(0x00000ff2, &pcie_setup->ladmsk0);
	writel(0x00000001, &pcie_setup->barmap);
	writel(0xffcaa000, &pcie_setup->lad0);
	writel(0x00030000, &pcie_sysbus->endictl0);
	writel(0x00000003, &pcie_sysbus->endictl1);
	writel(0x00000004, &pcie_setup->pbictl2);
}

static void finish_spiboot(void)
{
	struct gctrl_regs *gctrl = GCTRL_BASE;
	/*
	 *  SH7757 B0 does not use LBSC.
	 *  So if we set SPIBOOTCAN to 1, SH7757 can not access Area0.
	 *  This setting is not cleared by manual reset, So we have to set it
	 *  to 0.
	 */
	writel(0x00000000, &gctrl->spibootcan);
}

int board_late_init(void)
{
	init_ethernet_mac();
	init_pcie_bridge();
	init_pcie();
	finish_spiboot();

	return 0;
}

int do_sh_g200(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	struct gctrl_regs *gctrl = GCTRL_BASE;
	unsigned long graofst;

	writel(0xfedcba98, &gctrl->wprotect);
	graofst = (SH7757LCR_SDRAM_PHYS_TOP + SH7757LCR_GRA_OFFSET) >> 24;
	writel(graofst | 0xa0000f00, &gctrl->gracr3);

	return 0;
}

U_BOOT_CMD(
	sh_g200,	1,	1,	do_sh_g200,
	"enable sh-g200",
	"enable SH-G200 bus (disable PCIe-G200)"
);

#ifdef CONFIG_DEPRECATED
int do_write_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i, ret;
	char mac_string[256];
	struct spi_flash *spi;
	unsigned char *buf;

	if (argc != 5) {
		buf = malloc(256);
		if (!buf) {
			printf("%s: malloc error.\n", __func__);
			return 1;
		}

		get_sh_eth_mac_raw(buf, 256);

		/* print current MAC address */
		for (i = 0; i < 4; i++) {
			get_sh_eth_mac(i, mac_string, buf);
			if (i < 2)
				printf(" ETHERC ch%d = %s\n", i, mac_string);
			else
				printf("GETHERC ch%d = %s\n", i-2, mac_string);
		}
		free(buf);
		return 0;
	}

	/* new setting */
	memset(mac_string, 0xff, sizeof(mac_string));
	sprintf(mac_string, "%s\t%s\t%s\t%s",
		argv[1], argv[2], argv[3], argv[4]);

	/* write MAC data to SPI rom */
	spi = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
	if (!spi) {
		printf("%s: spi_flash probe error.\n", __func__);
		return 1;
	}

	ret = spi_flash_erase(spi, SH7757LCR_ETHERNET_MAC_BASE_SPI,
				SH7757LCR_SPI_SECTOR_SIZE);
	if (ret) {
		printf("%s: spi_flash erase error.\n", __func__);
		return 1;
	}

	ret = spi_flash_write(spi, SH7757LCR_ETHERNET_MAC_BASE_SPI,
				sizeof(mac_string), mac_string);
	if (ret) {
		printf("%s: spi_flash write error.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}
	spi_flash_free(spi);

	puts("The writing of the MAC address to SPI ROM was completed.\n");

	return 0;
}

U_BOOT_CMD(
	write_mac,	5,	1,	do_write_mac,
	"write MAC address for ETHERC/GETHERC",
	"[ETHERC ch0] [ETHERC ch1] [GETHERC ch0] [GETHERC ch1]\n"
);
#endif
