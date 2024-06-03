// SPDX-License-Identifier: GPL-2.0+
/*
 * Copyright (C) 2007,2008 Nobuhiro Iwamatsu <iwamatsu@nigauri.org>
 * Copyright (C) 2008 Yusuke Goda <goda.yusuke@renesas.com>
 */

#include <common.h>
#include <ide.h>
#include <asm/processor.h>
#include <asm/io.h>
#include <asm/pci.h>
#include <netdev.h>
#include "r7780mp.h"

int checkboard(void)
{
#if defined(CONFIG_R7780MP)
	puts("BOARD: Renesas Solutions R7780MP\n");
#else
	puts("BOARD: Renesas Solutions R7780RP\n");
#endif
	return 0;
}

int board_init(void)
{
	/* SCIF Enable */
	writew(0x0, PHCR);

	return 0;
}

void led_set_state(unsigned short value)
{

}

void ide_set_reset(int idereset)
{
	/* if reset = 1 IDE reset will be asserted */
	if (idereset) {
		writew(0x432, FPGA_CFCTL);
#if defined(CONFIG_R7780MP)
		writew(inw(FPGA_CFPOW)|0x01, FPGA_CFPOW);
#else
		writew(inw(FPGA_CFPOW)|0x02, FPGA_CFPOW);
#endif
		writew(0x01, FPGA_CFCDINTCLR);
	}
}

static struct pci_controller hose;
void pci_init_board(void)
{
	pci_sh7780_init(&hose);
}

int board_eth_init(bd_t *bis)
{
	/* return >= 0 if a chip is found, the board's AX88796L is n2k-based */
	return ne2k_register() + pci_eth_init(bis);
}
