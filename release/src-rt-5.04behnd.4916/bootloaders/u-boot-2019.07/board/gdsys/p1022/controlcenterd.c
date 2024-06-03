/*
 * (C) Copyright 2013
 * Dirk Eibach,  Guntermann & Drunck GmbH, dirk.eibach@gdsys.cc
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
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
#include <command.h>
#include <pci.h>
#include <asm/processor.h>
#include <asm/mmu.h>
#include <asm/cache.h>
#include <asm/immap_85xx.h>
#include <asm/fsl_pci.h>
#include <fsl_ddr_sdram.h>
#include <asm/fsl_serdes.h>
#include <asm/io.h>
#include <linux/libfdt.h>
#include <fdt_support.h>
#include <fsl_mdio.h>
#include <tsec.h>
#include <asm/fsl_law.h>
#include <netdev.h>
#include <i2c.h>
#include <pca9698.h>
#include <watchdog.h>
#include "../common/dp501.h"
#include "controlcenterd-id.h"

enum {
	HWVER_100 = 0,
	HWVER_110 = 1,
	HWVER_120 = 2,
};

struct ihs_fpga {
	u32 reflection_low;	/* 0x0000 */
	u32 versions;		/* 0x0004 */
	u32 fpga_version;	/* 0x0008 */
	u32 fpga_features;	/* 0x000c */
	u32 reserved[4];	/* 0x0010 */
	u32 control;		/* 0x0020 */
};

#ifndef CONFIG_TRAILBLAZER
static struct pci_device_id hydra_supported[] = {
	{ 0x6d5e, 0xcdc0 },
	{}
};

static void hydra_initialize(void);
#endif

int board_early_init_f(void)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO3_ADDR);

	/* Reset eLBC_DIU and SPI_eLBC in case we are booting from SD */
	clrsetbits_be32(&gur->pmuxcr, 0x00600000, 0x80000000);

	/* Set pmuxcr to allow both i2c1 and i2c2 */
	setbits_be32(&gur->pmuxcr, 0x00001000);

	/* Set pmuxcr to enable GPIO 3_11-3_13 */
	setbits_be32(&gur->pmuxcr, 0x00000010);

	/* Set pmuxcr to enable GPIO 2_31,3_9+10 */
	setbits_be32(&gur->pmuxcr, 0x00000020);

	/* Set pmuxcr to enable GPIO 2_28-2_30 */
	setbits_be32(&gur->pmuxcr, 0x000000c0);

	/* Set pmuxcr to enable GPIO 3_20-3_22 */
	setbits_be32(&gur->pmuxcr2, 0x03000000);

	/* Set pmuxcr to enable IRQ0-2 */
	clrbits_be32(&gur->pmuxcr, 0x00000300);

	/* Set pmuxcr to disable IRQ3-11 */
	setbits_be32(&gur->pmuxcr, 0x000000F0);

	/* Read back the register to synchronize the write. */
	in_be32(&gur->pmuxcr);

	/* Set the pin muxing to enable ETSEC2. */
	clrbits_be32(&gur->pmuxcr2, 0x001F8000);

#ifdef CONFIG_TRAILBLAZER
	/*
	 * GPIO3_10 SPERRTRIGGER
	 */
	setbits_be32(&pgpio->gpdir, 0x00200000);
	clrbits_be32(&pgpio->gpdat, 0x00200000);
	udelay(100);
	setbits_be32(&pgpio->gpdat, 0x00200000);
	udelay(100);
	clrbits_be32(&pgpio->gpdat, 0x00200000);
#endif

	/*
	 * GPIO3_11 CPU-TO-FPGA-RESET#
	 */
	setbits_be32(&pgpio->gpdir, 0x00100000);
	clrbits_be32(&pgpio->gpdat, 0x00100000);

	/*
	 * GPIO3_21 CPU-STATUS-WATCHDOG-TRIGGER#
	 */
	setbits_be32(&pgpio->gpdir, 0x00000400);

	return 0;
}

int checkboard(void)
{
	printf("Board: ControlCenter DIGITAL\n");

	return 0;
}

int misc_init_r(void)
{
	return 0;
}

/*
 * A list of PCI and SATA slots
 */
enum slot_id {
	SLOT_PCIE1 = 1,
	SLOT_PCIE2,
	SLOT_PCIE3,
	SLOT_PCIE4,
	SLOT_PCIE5,
	SLOT_SATA1,
	SLOT_SATA2
};

/*
 * This array maps the slot identifiers to their names on the P1022DS board.
 */
static const char * const slot_names[] = {
	[SLOT_PCIE1] = "Slot 1",
	[SLOT_PCIE2] = "Slot 2",
	[SLOT_PCIE3] = "Slot 3",
	[SLOT_PCIE4] = "Slot 4",
	[SLOT_PCIE5] = "Mini-PCIe",
	[SLOT_SATA1] = "SATA 1",
	[SLOT_SATA2] = "SATA 2",
};

/*
 * This array maps a given SERDES configuration and SERDES device to the PCI or
 * SATA slot that it connects to.  This mapping is hard-coded in the FPGA.
 */
static u8 serdes_dev_slot[][SATA2 + 1] = {
	[0x01] = { [PCIE3] = SLOT_PCIE4, [PCIE2] = SLOT_PCIE5 },
	[0x02] = { [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x09] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE4,
		   [PCIE2] = SLOT_PCIE5 },
	[0x16] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x17] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE2,
		   [PCIE2] = SLOT_PCIE3 },
	[0x1a] = { [PCIE1] = SLOT_PCIE1, [PCIE2] = SLOT_PCIE3,
		   [PCIE2] = SLOT_PCIE3,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1c] = { [PCIE1] = SLOT_PCIE1,
		   [SATA1] = SLOT_SATA1, [SATA2] = SLOT_SATA2 },
	[0x1e] = { [PCIE1] = SLOT_PCIE1, [PCIE3] = SLOT_PCIE3 },
	[0x1f] = { [PCIE1] = SLOT_PCIE1 },
};


/*
 * Returns the name of the slot to which the PCIe or SATA controller is
 * connected
 */
const char *board_serdes_name(enum srds_prtcl device)
{
	ccsr_gur_t *gur = (void *)CONFIG_SYS_MPC85xx_GUTS_ADDR;
	u32 pordevsr = in_be32(&gur->pordevsr);
	unsigned int srds_cfg = (pordevsr & MPC85xx_PORDEVSR_IO_SEL) >>
				MPC85xx_PORDEVSR_IO_SEL_SHIFT;
	enum slot_id slot = serdes_dev_slot[srds_cfg][device];
	const char *name = slot_names[slot];

	if (name)
		return name;
	else
		return "Nothing";
}

void hw_watchdog_reset(void)
{
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO3_ADDR);

	clrbits_be32(&pgpio->gpdat, 0x00000400);
	setbits_be32(&pgpio->gpdat, 0x00000400);
}

#ifdef CONFIG_TRAILBLAZER
int do_bootd(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	return run_command(env_get("bootcmd"), flag);
}

int board_early_init_r(void)
{
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO3_ADDR);

	/*
	 * GPIO3_12 PPC_SYSTEMREADY#
	 */
	setbits_be32(&pgpio->gpdir, 0x00080000);
	setbits_be32(&pgpio->gpodr, 0x00080000);
	clrbits_be32(&pgpio->gpdat, 0x00080000);

	return ccdm_compute_self_hash();
}

int last_stage_init(void)
{
	startup_ccdm_id_module();
	return 0;
}

#else
void pci_init_board(void)
{
	fsl_pcie_init_board(0);

	hydra_initialize();
}

int board_early_init_r(void)
{
	unsigned int k = 0;
	ccsr_gpio_t *pgpio = (void *)(CONFIG_SYS_MPC85xx_GPIO3_ADDR);

	/* wait for FPGA configuration to finish */
	while (!pca9698_get_value(0x22, 11) && (k++ < 30))
		udelay(100000);

	if (k > 30) {
		puts("FPGA configuration timed out.\n");
	} else {
		/* clear FPGA reset */
		udelay(1000);
		setbits_be32(&pgpio->gpdat, 0x00100000);
	}

	/* give time for PCIe link training */
	udelay(100000);

	/*
	 * GPIO3_12 PPC_SYSTEMREADY#
	 */
	setbits_be32(&pgpio->gpdir, 0x00080000);
	setbits_be32(&pgpio->gpodr, 0x00080000);
	clrbits_be32(&pgpio->gpdat, 0x00080000);

	return 0;
}

int last_stage_init(void)
{
	/* Turn on Parade DP501 */
	pca9698_direction_output(0x22, 7, 1);
	udelay(500000);

	dp501_powerup(0x08);

	startup_ccdm_id_module();

	return 0;
}

/*
 * Initialize on-board and/or PCI Ethernet devices
 *
 * Returns:
 *      <0, error
 *       0, no ethernet devices found
 *      >0, number of ethernet devices initialized
 */
int board_eth_init(bd_t *bis)
{
	struct fsl_pq_mdio_info mdio_info;
	struct tsec_info_struct tsec_info[2];
	unsigned int num = 0;

#ifdef CONFIG_TSEC1
	SET_STD_TSEC_INFO(tsec_info[num], 1);
	num++;
#endif
#ifdef CONFIG_TSEC2
	SET_STD_TSEC_INFO(tsec_info[num], 2);
	num++;
#endif

	mdio_info.regs = (struct tsec_mii_mng *)CONFIG_SYS_MDIO_BASE_ADDR;
	mdio_info.name = DEFAULT_MII_NAME;
	fsl_pq_mdio_init(bis, &mdio_info);

	return tsec_eth_init(bis, tsec_info, num) + pci_eth_init(bis);
}

#ifdef CONFIG_OF_BOARD_SETUP
int ft_board_setup(void *blob, bd_t *bd)
{
	phys_addr_t base;
	phys_size_t size;

	ft_cpu_setup(blob, bd);

	base = env_get_bootm_low();
	size = env_get_bootm_size();

	fdt_fixup_memory(blob, (u64)base, (u64)size);

#ifdef CONFIG_HAS_FSL_DR_USB
	fsl_fdt_fixup_dr_usb(blob, bd);
#endif

	FT_FSL_PCI_SETUP;

	return 0;
}
#endif

static void hydra_initialize(void)
{
	unsigned int i;
	pci_dev_t devno;

	/* Find and probe all the matching PCI devices */
	for (i = 0; (devno = pci_find_devices(hydra_supported, i)) >= 0; i++) {
		u32 val;
		struct ihs_fpga *fpga;
		u32 versions;
		u32 fpga_version;
		u32 fpga_features;

		unsigned hardware_version;
		unsigned feature_uart_channels;
		unsigned feature_sb_channels;

		/* Try to enable I/O accesses and bus-mastering */
		val = PCI_COMMAND_MEMORY | PCI_COMMAND_MASTER;
		pci_write_config_dword(devno, PCI_COMMAND, val);

		/* Make sure it worked */
		pci_read_config_dword(devno, PCI_COMMAND, &val);
		if (!(val & PCI_COMMAND_MEMORY)) {
			puts("Can't enable I/O memory\n");
			continue;
		}
		if (!(val & PCI_COMMAND_MASTER)) {
			puts("Can't enable bus-mastering\n");
			continue;
		}

		/* read FPGA details */
		fpga = pci_map_bar(devno, PCI_BASE_ADDRESS_0,
			PCI_REGION_MEM);

		/* disable sideband clocks */
		writel(1, &fpga->control);

		versions = readl(&fpga->versions);
		fpga_version = readl(&fpga->fpga_version);
		fpga_features = readl(&fpga->fpga_features);

		hardware_version = versions & 0xf;
		feature_uart_channels = (fpga_features >> 6) & 0x1f;
		feature_sb_channels = fpga_features & 0x1f;

		printf("FPGA%d: ", i);

		switch (hardware_version) {
		case HWVER_100:
			printf("HW-Ver 1.00\n");
			break;

		case HWVER_110:
			printf("HW-Ver 1.10\n");
			break;

		case HWVER_120:
			printf("HW-Ver 1.20\n");
			break;

		default:
			printf("HW-Ver %d(not supported)\n",
			       hardware_version);
			break;
		}

		printf("       FPGA V %d.%02d, features:",
		       fpga_version / 100, fpga_version % 100);

		printf(" %d uart channel(s)", feature_uart_channels);
		printf(" %d sideband channel(s)\n", feature_sb_channels);
	}
}
#endif
