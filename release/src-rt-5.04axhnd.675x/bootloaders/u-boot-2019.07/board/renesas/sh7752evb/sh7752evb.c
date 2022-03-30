// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2012  Renesas Solutions Corp.
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
	puts("BOARD: SH7752 evaluation board (R0P7752C00000RZ)\n");

	return 0;
}

static void init_gpio(void)
{
	struct gpio_regs *gpio = GPIO_BASE;
	struct sermux_regs *sermux = SERMUX_BASE;

	/* GPIO */
	writew(0x0000, &gpio->pacr);	/* GETHER */
	writew(0x0001, &gpio->pbcr);	/* INTC */
	writew(0x0000, &gpio->pccr);	/* PWMU, INTC */
	writew(0xeaff, &gpio->pecr);	/* GPIO */
	writew(0x0000, &gpio->pfcr);	/* WDT */
	writew(0x0000, &gpio->phcr);	/* SPI1 */
	writew(0x0000, &gpio->picr);	/* SDHI */
	writew(0x0003, &gpio->pkcr);	/* SerMux */
	writew(0x0000, &gpio->plcr);	/* SerMux */
	writew(0x0000, &gpio->pmcr);	/* RIIC */
	writew(0x0000, &gpio->pncr);	/* USB, SGPIO */
	writew(0x0000, &gpio->pocr);	/* SGPIO */
	writew(0xd555, &gpio->pqcr);	/* GPIO */
	writew(0x0000, &gpio->prcr);	/* RIIC */
	writew(0x0000, &gpio->pscr);	/* RIIC */
	writeb(0x00, &gpio->pudr);
	writew(0x5555, &gpio->pucr);	/* Debug LED */
	writew(0x0000, &gpio->pvcr);	/* RSPI */
	writew(0x0000, &gpio->pwcr);	/* EVC */
	writew(0x0000, &gpio->pxcr);	/* LBSC */
	writew(0x0000, &gpio->pycr);	/* LBSC */
	writew(0x0000, &gpio->pzcr);	/* eMMC */
	writew(0xfe00, &gpio->psel0);
	writew(0xff00, &gpio->psel3);
	writew(0x771f, &gpio->psel4);
	writew(0x00ff, &gpio->psel6);
	writew(0xfc00, &gpio->psel7);

	writeb(0x10, &sermux->smr0);	/* SMR0: SerMux mode 0 */
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

static void init_gether_mdio(void)
{
	struct gpio_regs *gpio = GPIO_BASE;

	writew(readw(&gpio->pgcr) | 0x0004, &gpio->pgcr);
	writeb(readb(&gpio->pgdr) | 0x02, &gpio->pgdr);	/* Use ET0-MDIO */
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
	init_gpio();
	set_pmb_on_board_init();

	init_usb_phy();
	init_gether_mdio();

	return 0;
}

int board_mmc_init(bd_t *bis)
{
	struct gpio_regs *gpio = GPIO_BASE;

	writew(readw(&gpio->pgcr) | 0x0040, &gpio->pgcr);
	writeb(readb(&gpio->pgdr) & ~0x08, &gpio->pgdr); /* Reset */
	udelay(1);
	writeb(readb(&gpio->pgdr) | 0x08, &gpio->pgdr);	/* Release reset */
	udelay(200);

	return mmcif_mmc_init();
}

static int get_sh_eth_mac_raw(unsigned char *buf, int size)
{
#ifdef CONFIG_DEPRECATED
	struct spi_flash *spi;
	int ret;

	spi = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
	if (spi == NULL) {
		printf("%s: spi_flash probe failed.\n", __func__);
		return 1;
	}

	ret = spi_flash_read(spi, SH7752EVB_ETHERNET_MAC_BASE, size, buf);
	if (ret) {
		printf("%s: spi_flash read failed.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}
	spi_flash_free(spi);
#endif

	return 0;
}

static int get_sh_eth_mac(int channel, char *mac_string, unsigned char *buf)
{
	memcpy(mac_string, &buf[channel * (SH7752EVB_ETHERNET_MAC_SIZE + 1)],
		SH7752EVB_ETHERNET_MAC_SIZE);
	mac_string[SH7752EVB_ETHERNET_MAC_SIZE] = 0x00;	/* terminate */

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
		printf("%s: malloc failed.\n", __func__);
		return;
	}
	get_sh_eth_mac_raw(buf, 256);

	/* Gigabit Ethernet */
	for (i = 0; i < SH7752EVB_ETHERNET_NUM_CH; i++) {
		get_sh_eth_mac(i, mac_string, buf);
		if (i == 0)
			env_set("ethaddr", mac_string);
		else {
			sprintf(env_string, "eth%daddr", i);
			env_set(env_string, mac_string);
		}
		set_mac_to_sh_giga_eth_register(i, mac_string);
	}

	free(buf);
}

int board_late_init(void)
{
	init_ethernet_mac();

	return 0;
}

#ifdef CONFIG_DEPRECATED
int do_write_mac(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i, ret;
	char mac_string[256];
	struct spi_flash *spi;
	unsigned char *buf;

	if (argc != 3) {
		buf = malloc(256);
		if (!buf) {
			printf("%s: malloc failed.\n", __func__);
			return 1;
		}

		get_sh_eth_mac_raw(buf, 256);

		/* print current MAC address */
		for (i = 0; i < SH7752EVB_ETHERNET_NUM_CH; i++) {
			get_sh_eth_mac(i, mac_string, buf);
			printf("GETHERC ch%d = %s\n", i, mac_string);
		}
		free(buf);
		return 0;
	}

	/* new setting */
	memset(mac_string, 0xff, sizeof(mac_string));
	sprintf(mac_string, "%s\t%s",
		argv[1], argv[2]);

	/* write MAC data to SPI rom */
	spi = spi_flash_probe(0, 0, 1000000, SPI_MODE_3);
	if (!spi) {
		printf("%s: spi_flash probe failed.\n", __func__);
		return 1;
	}

	ret = spi_flash_erase(spi, SH7752EVB_ETHERNET_MAC_BASE_SPI,
				SH7752EVB_SPI_SECTOR_SIZE);
	if (ret) {
		printf("%s: spi_flash erase failed.\n", __func__);
		return 1;
	}

	ret = spi_flash_write(spi, SH7752EVB_ETHERNET_MAC_BASE_SPI,
				sizeof(mac_string), mac_string);
	if (ret) {
		printf("%s: spi_flash write failed.\n", __func__);
		spi_flash_free(spi);
		return 1;
	}
	spi_flash_free(spi);

	puts("The writing of the MAC address to SPI ROM was completed.\n");

	return 0;
}

U_BOOT_CMD(
	write_mac,	3,	1,	do_write_mac,
	"write MAC address for GETHERC",
	"[GETHERC ch0] [GETHERC ch1]\n"
);
#endif
