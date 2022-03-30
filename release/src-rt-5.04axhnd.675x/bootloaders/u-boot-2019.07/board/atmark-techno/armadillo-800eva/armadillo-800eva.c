/*
 * Copyright (C) 2012  Renesas Solutions Corp.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <malloc.h>
#include <asm/processor.h>
#include <asm/mach-types.h>
#include <asm/io.h>
#include <asm/arch/sys_proto.h>
#include <asm/gpio.h>
#include <asm/arch/rmobile.h>

#define s_init_wait(cnt) \
		({	\
			volatile u32 i = 0x10000 * cnt;	\
			while (i > 0)	\
				i--;	\
		})

#define USBCR1 0xE605810A

void s_init(void)
{
	struct r8a7740_rwdt *rwdt0 = (struct r8a7740_rwdt *)RWDT0_BASE;
	struct r8a7740_rwdt *rwdt1 = (struct r8a7740_rwdt *)RWDT1_BASE;
	struct r8a7740_cpg *cpg = (struct r8a7740_cpg *)CPG_BASE;
	struct r8a7740_bsc *bsc = (struct r8a7740_bsc *)BSC_BASE;
	struct r8a7740_ddrp *ddrp = (struct r8a7740_ddrp *)DDRP_BASE;
	struct r8a7740_dbsc *dbsc = (struct r8a7740_dbsc *)DBSC_BASE;

	/* Watchdog init */
	writew(0xA500, &rwdt0->rwtcsra0);
	writew(0xA500, &rwdt1->rwtcsra0);

	/* CPG */
	writel(0xFF800080, &cpg->rmstpcr4);
	writel(0xFF800080, &cpg->smstpcr4);

	/* USB clock */
	writel(0x00000080, &cpg->usbckcr);
	s_init_wait(1);

	/* USBCR1 */
	writew(0x0710, USBCR1);

	/* FRQCR */
	writel(0x00000000, &cpg->frqcrb);
	writel(0x62030533, &cpg->frqcra);
	writel(0x208A354E, &cpg->frqcrc);
	writel(0x80331050, &cpg->frqcrb);
	s_init_wait(1);

	writel(0x00000000, &cpg->frqcrd);
	s_init_wait(1);

	/* SUBClk */
	writel(0x0000010B, &cpg->subckcr);

	/* PLL */
	writel(0x00004004, &cpg->pllc01cr);
	s_init_wait(1);

	writel(0xa0000000, &cpg->pllc2cr);
	s_init_wait(2);

	/* BSC */
	writel(0x0000001B, &bsc->cmncr);

	writel(0x20000000, &dbsc->dbcmd);
	writel(0x10009C40, &dbsc->dbcmd);
	s_init_wait(1);

	writel(0x00000007, &dbsc->dbkind);
	writel(0x0E030A02, &dbsc->dbconf0);
	writel(0x00000001, &dbsc->dbphytype);
	writel(0x00000000, &dbsc->dbbl);
	writel(0x00000006, &dbsc->dbtr0);
	writel(0x00000005, &dbsc->dbtr1);
	writel(0x00000000, &dbsc->dbtr2);
	writel(0x00000006, &dbsc->dbtr3);
	writel(0x00080006, &dbsc->dbtr4);
	writel(0x00000015, &dbsc->dbtr5);
	writel(0x0000000f, &dbsc->dbtr6);
	writel(0x00000004, &dbsc->dbtr7);
	writel(0x00000018, &dbsc->dbtr8);
	writel(0x00000006, &dbsc->dbtr9);
	writel(0x00000006, &dbsc->dbtr10);
	writel(0x0000000F, &dbsc->dbtr11);
	writel(0x0000000D, &dbsc->dbtr12);
	writel(0x000000A0, &dbsc->dbtr13);
	writel(0x000A0003, &dbsc->dbtr14);
	writel(0x00000003, &dbsc->dbtr15);
	writel(0x40005005, &dbsc->dbtr16);
	writel(0x0C0C0000, &dbsc->dbtr17);
	writel(0x00000200, &dbsc->dbtr18);
	writel(0x00000040, &dbsc->dbtr19);
	writel(0x00000001, &dbsc->dbrnk0);
	writel(0x00000110, &dbsc->dbdficnt);
	writel(0x00000101, &ddrp->funcctrl);
	writel(0x00000001, &ddrp->dllctrl);
	writel(0x00000186, &ddrp->zqcalctrl);
	writel(0xB3440051, &ddrp->zqodtctrl);
	writel(0x94449443, &ddrp->rdctrl);
	writel(0x000000C0, &ddrp->rdtmg);
	writel(0x00000101, &ddrp->fifoinit);
	writel(0x02060506, &ddrp->outctrl);
	writel(0x00004646, &ddrp->dqcalofs1);
	writel(0x00004646, &ddrp->dqcalofs2);
	writel(0x800000aa, &ddrp->dqcalexp);
	writel(0x00000000, &ddrp->dllctrl);
	writel(0x00000000, DDRPNCNT);

	writel(0x0000000C, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00000002, DDRPNCNT);

	writel(0x0000000C, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00000187, &ddrp->zqcalctrl);

	writel(0x00009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00000010, &dbsc->dbdficnt);
	writel(0x02060507, &ddrp->outctrl);

	writel(0x00009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x21009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x00009C40, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x11000044, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x2A000000, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x2B000000, &dbsc->dbcmd);
	readl(&dbsc->dbwait);

	writel(0x29000004, &dbsc->dbcmd);
	readl(&dbsc->dbwait);

	writel(0x28001520, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x03000200, &dbsc->dbcmd);
	readl(&dbsc->dbwait);
	s_init_wait(1);

	writel(0x000001FF, &dbsc->dbrfcnf0);
	writel(0x00010C30, &dbsc->dbrfcnf1);
	writel(0x00000000, &dbsc->dbrfcnf2);

	writel(0x00000001, &dbsc->dbrfen);
	writel(0x00000001, &dbsc->dbacen);

	/* BSC */
	writel(0x00410400, &bsc->cs0bcr);
	writel(0x00410400, &bsc->cs2bcr);
	writel(0x00410400, &bsc->cs5bbcr);
	writel(0x02CB0400, &bsc->cs6abcr);

	writel(0x00000440, &bsc->cs0wcr);
	writel(0x00000440, &bsc->cs2wcr);
	writel(0x00000240, &bsc->cs5bwcr);
	writel(0x00000240, &bsc->cs6awcr);

	writel(0x00000005, &bsc->rbwtcnt);
	writel(0x00000002, &bsc->cs0wcr2);
	writel(0x00000002, &bsc->cs2wcr2);
	writel(0x00000002, &bsc->cs4wcr2);
}

#define GPIO_ICCR (0xE60581A0)
#define ICCR_15BIT (1 << 15) /* any time 1 */
#define IIC0_CONTA (1 << 7)
#define IIC0_CONTB (1 << 6)
#define IIC1_CONTA (1 << 5)
#define IIC1_CONTB (1 << 4)
#define IIC0_PS33E (1 << 1)
#define IIC1_PS33E (1 << 0)
#define GPIO_ICCR_DATA	\
		(ICCR_15BIT |	\
		IIC0_CONTA | IIC0_CONTB | IIC1_CONTA |	\
		IIC1_CONTB | IIC0_PS33E | IIC1_PS33E)

#define MSTPCR1         0xE6150134
#define TMU0_MSTP125    (1 << 25)
#define I2C0_MSTP116    (1 << 16)

#define MSTPCR3         0xE615013C
#define I2C1_MSTP323    (1 << 23)
#define GETHER_MSTP309	(1 << 9)

#define GPIO_SCIFA1_TXD (0xE60520C4)
#define GPIO_SCIFA1_RXD (0xE60520C3)

int board_early_init_f(void)
{
	/* TMU */
	clrbits_le32(MSTPCR1, TMU0_MSTP125);

	/* GETHER */
	clrbits_le32(MSTPCR3, GETHER_MSTP309);

	/* I2C 0/1 */
	clrbits_le32(MSTPCR1, I2C0_MSTP116);
	clrbits_le32(MSTPCR3, I2C1_MSTP323);

	/* SCIFA1 */
	writeb(1, GPIO_SCIFA1_TXD); /* SCIFA1_TXD */
	writeb(1, GPIO_SCIFA1_RXD); /* SCIFA1_RXD */

	/* IICCR */
	writew(GPIO_ICCR_DATA, GPIO_ICCR);

	return 0;
}

DECLARE_GLOBAL_DATA_PTR;
int board_init(void)
{
	/* board id for linux */
	gd->bd->bi_arch_number = MACH_TYPE_ARMADILLO800EVA;
	/* adress of boot parameters */
	gd->bd->bi_boot_params = ARMADILLO_800EVA_SDRAM_BASE + 0x100;

	/* Init PFC controller */
	r8a7740_pinmux_init();

	/* GETHER Enable */
	gpio_request(GPIO_FN_ET_CRS, NULL);
	gpio_request(GPIO_FN_ET_MDC, NULL);
	gpio_request(GPIO_FN_ET_MDIO, NULL);
	gpio_request(GPIO_FN_ET_TX_ER, NULL);
	gpio_request(GPIO_FN_ET_RX_ER, NULL);
	gpio_request(GPIO_FN_ET_ERXD0, NULL);
	gpio_request(GPIO_FN_ET_ERXD1, NULL);
	gpio_request(GPIO_FN_ET_ERXD2, NULL);
	gpio_request(GPIO_FN_ET_ERXD3, NULL);
	gpio_request(GPIO_FN_ET_TX_CLK, NULL);
	gpio_request(GPIO_FN_ET_TX_EN, NULL);
	gpio_request(GPIO_FN_ET_ETXD0, NULL);
	gpio_request(GPIO_FN_ET_ETXD1, NULL);
	gpio_request(GPIO_FN_ET_ETXD2, NULL);
	gpio_request(GPIO_FN_ET_ETXD3, NULL);
	gpio_request(GPIO_FN_ET_PHY_INT, NULL);
	gpio_request(GPIO_FN_ET_COL, NULL);
	gpio_request(GPIO_FN_ET_RX_DV, NULL);
	gpio_request(GPIO_FN_ET_RX_CLK, NULL);

	gpio_request(GPIO_PORT18, NULL); /* PHY_RST */
	gpio_direction_output(GPIO_PORT18, 1);

	return 0;
}

int dram_init(void)
{
	gd->bd->bi_dram[0].start = CONFIG_SYS_SDRAM_BASE;
	gd->ram_size = CONFIG_SYS_SDRAM_SIZE;

	return 0;
}

int board_late_init(void)
{
	return 0;
}

void reset_cpu(ulong addr)
{
}
